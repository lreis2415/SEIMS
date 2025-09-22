#include "MultiFlowDirMaxDown.h"
#include "d8.h"

#include "commonLib.h"
#include "createpart.h"

int flowdirection_mfd_md(char* dem, char* fdir, char* fportion,
                         double p0 /* = 1.1 */, double p_range /* = 8.9 */,
                         double tanb_lb /* = 0. */, double tanb_ub /* = 1. */,
                         double min_portion /* = 0.05 */) {
    MPI_Init(NULL, NULL);
    {
        int rank, size;
        MPI_Comm_rank(MCW, &rank);
        MPI_Comm_size(MCW, &size);
        // begin timer
        double begint = MPI_Wtime();
        // read tiff header information using tiffIO
        tiffIO srcf(dem, FLOAT_TYPE);
        long totalX = srcf.getTotalX();
        long totalY = srcf.getTotalY();
        double dx = srcf.getdxA();
        double dy = srcf.getdyA();

        int i, j, k, lyr, con;

        // read tiff data into partition
        tdpartition* src;
        src = CreateNewPartition(srcf.getDatatype(), totalX, totalY, dx, dy, srcf.getNodata());
        // get the size of current partition, and get the current partition's pointer
        int nx = src->getnx();
        int ny = src->getny();
        int xstart, ystart;
        src->localToGlobal(0, 0, xstart, ystart); // calculate current partition's first cell's position
        src->savedxdyc(srcf);
        srcf.read(xstart, ystart, ny, nx, src->getGridPointer()); // get the current partition's pointer

        double readt = MPI_Wtime(); // record reading time

        // create empty partition to store new result
        tdpartition* dest;
        dest = CreateNewPartition(SHORT_TYPE, totalX, totalY, dx, dy, static_cast<short>(DEFAULTNODATA_INT));

        linearpart<float> *flowfractions = new linearpart<float>[8];
        for (lyr = 1; lyr <= 8; lyr++) {
            flowfractions[lyr - 1].init(totalX, totalY, dx, dy, MPI_FLOAT, DEFAULTNODATA);
        }

        //share information
        src->share();
        dest->share();
        for (lyr = 1; lyr <= 8; lyr++) flowfractions[lyr - 1].share();

        // COMPUTING CODE BLOCK
        long num_flat = 0;
        long total_num_flat = 0;
        long last_num_flat = 0;
        double a = p_range / (tanb_ub - tanb_lb);
        double b = p0 - p_range * tanb_lb / (tanb_ub - tanb_lb);
        float dem_values[9];
        double downslp[9];
        node temp_node;
        vector<node> flats_list;

        //Set direction factors, double **fact is a global variable declared and defined in d8.h and d8.cpp
        double tempdxc, tempdyc;
        fact = new double *[ny]; // initialize 2d array by Nazmus 2/16
        for (int m = 0; m < ny; m++) {
            fact[m] = new double[9];
        }
        for (int m = 0; m < ny; m++) {
            for (k = 1; k <= 8; k++) {
                src->getdxdyc(m, tempdxc, tempdyc);
                fact[m][k] = (double) (1. / sqrt(d1[k] * d1[k] * tempdxc * tempdxc +
                                            d2[k] * d2[k] * tempdyc * tempdyc));
            }
        }

        for (j = 0; j < ny; j++) { // rows
            for (i = 0; i < nx; i++) { // cols
                // Set flowdir to nodata if it is on the border or elevation has no data
                if (src->isNodata(i, j) ||
                    !src->hasAccess(i - 1, j) || !src->hasAccess(i + 1, j) ||
                    !src->hasAccess(i, j - 1) || !src->hasAccess(i, j + 1)) {
                    dest->setToNodata(i, j);
                    for (lyr = 1; lyr <= 8; lyr++) {
                        flowfractions[lyr - 1].setToNodata(i, j);
                    }
                    continue;
                }
                // Check if cell is "contaminated", i.e., neighbors have nodata,
                //   set flowdir to nodata if contaminated
                con = 0;
                for (k = 1; k <= 8 && con != -1; k++) {
                    int icol = i + d1[k];
                    int irow = j + d2[k];
                    if (src->isNodata(icol, irow)) con = -1;
                }
                if (con == -1) {
                    dest->setToNodata(i, j);
                    for (lyr = 1; lyr <= 8; lyr++) {
                        flowfractions[lyr - 1].setToNodata(i, j);
                    }
                    continue;
                }
                // calculate MFD-md
                float cdem;
                double maxslp = -9999.;
                src->getData(i, j, cdem);
                short compounddir = 0;
                for (k = 1; k <= 8; k++) {
                    int icol = i + d1[k];
                    int irow = j + d2[k];
                    if (!src->hasAccess(icol, irow) || src->isNodata(icol, irow)) continue;
                    src->getData(icol, irow, dem_values[k]);
                    if (dem_values[k] >= cdem) continue;
                    compounddir += esri_flowdir[k];           // accumulate all downslope directions
                    // downslp[k] = (cdem - dem_values[k]) / dx; // tan(SlopeDegree)
                    // if (k % 2 == 0) downslp[k] /= SQRT2;      // diagonal
                    downslp[k] = (cdem - dem_values[k]) * fact[j][k];
                    if (downslp[k] > maxslp) maxslp = downslp[k];
                }
                if (compounddir == 0) {
                    // No outflow, indicating flats here
                    num_flat++;
                    dest->setData(i, j, static_cast<short>(0));
                    for (lyr = 1; lyr <= 8; lyr++) {
                        flowfractions[lyr - 1].setData(i, j, -1.f);
                    }
                    temp_node.x = i;
                    temp_node.y = j;
                    flats_list.push_back(temp_node);
                    continue;
                }

                if (maxslp <= tanb_lb) maxslp = p0;
                else if (maxslp >= tanb_ub) maxslp = p0 + p_range;
                else maxslp = a * maxslp + b;

                double dsum = 0.;
                int downcount = 0; // count of downslope cells
                for (k = 1; k <= 8; k++) {
                    if (!(compounddir & esri_flowdir[k])) continue;
                    downcount++;
                    if (k % 2 == 0) {
                        dsum += pow(downslp[k], maxslp) * SQRT2 * 0.25; // diagonal
                    } else {
                        dsum += pow(downslp[k], maxslp) * 0.5;
                    }
                }
                if (dsum <= ZERO || downcount == 0)
                    printf("err");
                // remove very tiny flow portion according to user-specified parameter
                double tiny_portion = 0.; // sum of tiny portions
                double cur_tot_portion = 0.;
                double portion[9] = {0., 0., 0., 0., 0., 0., 0., 0., 0.};
                for (k = 1; k <= 8; k++) {
                    if (!(compounddir & esri_flowdir[k])) continue;
                    if (k % 2 == 0) {
                        portion[k] = pow(downslp[k], maxslp) * SQRT2 * 0.25 / dsum; // diagonal
                    } else {
                        portion[k] = pow(downslp[k], maxslp) * 0.5 / dsum;
                    }
                    if (portion[k] < min_portion) {
                        compounddir -= esri_flowdir[k];
                        downcount -= 1;
                        tiny_portion += portion[k];
                        portion[k] = 0.;
                    }
                    cur_tot_portion += portion[k];
                }
                if (cur_tot_portion <= ZERO)
                    printf("err");
                // add very tiny flow portions to other downslope cells proportionally
                for (lyr = 1; lyr <= 8; lyr++) {
                    if (portion[lyr] <= 0.) {
                        flowfractions[lyr - 1].setData(i, j, -1.f);
                        continue;
                    }
                    portion[lyr] += portion[lyr] * tiny_portion / cur_tot_portion;
                    flowfractions[lyr - 1].setData(i, j, portion[lyr]);
                }
                // save the final compound flow direction
                dest->setData(i, j, compounddir);
            }
        }
        MPI_Allreduce(&num_flat, &total_num_flat, 1, MPI_LONG, MPI_SUM, MCW);
        if (rank == 0) {
            fprintf(stderr, "All valid cells evaluated. %ld flats to resolve.\n", total_num_flat);
            fflush(stderr);
        }
        queue<node> que; // que to be used in resolveflats
        bool first = true; // variable to be used in iteration to know whether first or subsequent iteration
        if (total_num_flat > 0) {
            last_num_flat = total_num_flat;
            total_num_flat = resolveflats(src, dest, &que, 2, first);
            // repeatedly call resolve flats until there is no change
            while (total_num_flat > 0 && total_num_flat < last_num_flat) {
                if (rank == 0) {
                    fprintf(stderr, "Iteration complete. Number of flats remaining: %ld\n", total_num_flat);
                    fflush(stderr);
                }
                last_num_flat = total_num_flat;
                total_num_flat = resolveflats(src, dest, &que, 2, first);
            }
        }
        short temp_short;
        for (vector<node>::iterator it = flats_list.begin(); it != flats_list.end(); ++it) {
            int i = it->x;
            int j = it->y;
            dest->getData(i, j, temp_short);
            if (temp_short <= 0 || temp_short > 8 ) continue;
            dest->setData(i, j, (short) esri_flowdir[temp_short]);
            flowfractions[temp_short - 1].setData(i, j, 1.f);
        }

        // END COMPUTING CODE BLOCK
        double computet = MPI_Wtime(); // record computing time

        // create and write TIFF file
        tiffIO destTIFF(fdir, SHORT_TYPE, static_cast<short>(DEFAULTNODATA_INT), srcf);
        destTIFF.write(xstart, ystart, ny, nx, dest->getGridPointer());
        // write flow fractions data into separated raster files
        for (lyr = 1; lyr <= 8; lyr++) {
            char ffracfile[MAXLN];
            std::string intstr = std::to_string((long long)lyr);
            intstr.insert(0, "_");
            nameadd(ffracfile, fportion, intstr.c_str());
            tiffIO ffractTIFF(ffracfile, FLOAT_TYPE, static_cast<double>(DEFAULTNODATA), srcf);
            ffractTIFF.write(xstart, ystart, ny, nx, flowfractions[lyr - 1].getGridPointer());
        }
        double writet = MPI_Wtime(); // record writing time

        double dataRead, compute, write, total, tempd;
        dataRead = readt - begint;
        compute = computet - readt;
        write = writet - computet;
        total = writet - begint;

        MPI_Allreduce(&dataRead, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        dataRead = tempd / size;
        MPI_Allreduce(&compute, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        compute = tempd / size;
        MPI_Allreduce(&write, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        write = tempd / size;
        MPI_Allreduce(&total, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        total = tempd / size;

        if (rank == 0) {
            printf("Processors: %d\nRead time: %f\nCompute time: %f\nWrite time: %f\nTotal time: %f\n",
                   size, dataRead, compute, write, total);
        }
    }
    MPI_Finalize();
    return 0;
}
