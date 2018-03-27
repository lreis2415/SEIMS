/*  Curvature algorithm calculate ProfileCurvature, PlanCurvature...
    algorithm is adpoted from Shary et al.(2002).
     
  Liangjun, Zhu
  Lreis, CAS  
  Apr 8, 2015 
  
*/
// include TauDEM header files
#include "commonLib.h"
#include "createpart.h"
// include algorithm header file
#include "Curvature.h"

using namespace std;

int Curvature(char *demfile, char *profcfile, char *plancfile, char *horizcfile, char *unspherfile, char *meancfile,
              char *maxcfile, char *mincfile, bool calprof, bool calplan, bool calhoriz, bool calunspher, bool calmeanc,
              bool calmaxc, bool calminc) {
    MPI_Init(NULL, NULL);
    {
        int rank, size;
        MPI_Comm_rank(MCW, &rank);
        MPI_Comm_size(MCW, &size);
        if (rank == 0) printf("Curvature -h version %s, by Liangjun Zhu, Apr 8, 2015\n", TDVERSION);
        // begin timer
        double begint = MPI_Wtime();
        // read dem tiff header information using tiffIO
        tiffIO demf(demfile, FLOAT_TYPE);
        long totalX = demf.getTotalX();
        long totalY = demf.getTotalY();
        double dx = demf.getdxA();
        double dy = demf.getdyA();

        // read tiff data into partition
        tdpartition *dem;
        dem = CreateNewPartition(demf.getDatatype(), totalX, totalY, dx, dy, demf.getNodata());
        // get the size of current partition
        int nx = dem->getnx();
        int ny = dem->getny();
        int xstart, ystart;
        dem->localToGlobal(0, 0, xstart, ystart); // calculate current partition's first cell's position
        demf.read(xstart, ystart, ny, nx, dem->getGridPointer()); // get the current partition's pointer

        double readt = MPI_Wtime(); // record reading time

        // create empty partition to store curvature result, and share information
        tdpartition *prof;
        tdpartition *plan;
        tdpartition *horiz;
        tdpartition *unspher;
        tdpartition *meanc;
        tdpartition *maxc;
        tdpartition *minc;
        if (calprof) {
            prof = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
            prof->share();
        }
        if (calplan) {
            plan = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
            plan->share();
        }
        if (calhoriz) {
            horiz = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
            horiz->share();
        }
        if (calunspher) {
            unspher = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
            unspher->share();
        }
        if (calmeanc) {
            meanc = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
            meanc->share();
        }
        if (calmaxc) {
            maxc = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
            maxc->share();
        }
        if (calminc) {
            minc = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
            minc->share();
        }
        dem->share();

        int i, j, k, in, jn;
        float p, q, r, s, t;
        float z[9];
        float cellsize2;
        cellsize2 = float(dx * dy);
        // COMPUTING CODE BLOCK
//        omp_set_num_threads(4);
//#pragma omp parallel for
        for (j = 0; j < ny; j++) {
            for (i = 0; i < nx; i++) {
                //If i,j is on the border or dem has no data, set curvature(i,j) to NoData
                if (dem->isNodata(i, j) || !dem->hasAccess(i - 1, j) || !dem->hasAccess(i + 1, j) ||
                    !dem->hasAccess(i, j - 1) || !dem->hasAccess(i, j + 1)) {
                    if (calprof) {
                        prof->setToNodata(i, j);
                    }
                    if (calplan) {
                        plan->setToNodata(i, j);
                    }
                    if (calhoriz) {
                        horiz->setToNodata(i, j);
                    }
                    if (calunspher) {
                        unspher->setToNodata(i, j);
                    }
                    if (calmeanc) {
                        meanc->setToNodata(i, j);
                    }
                    if (calmaxc) {
                        maxc->setToNodata(i, j);
                    }
                    if (calminc) {
                        minc->setToNodata(i, j);
                    }
                } else {
                    bool hasNodata = false;
                    dem->getData(i, j, z[0]);
                    for (k = 1; k <= 8; k++) {
                        in = i + d1[k];
                        jn = j + d2[k];
                        if (dem->isNodata(in, jn)) {
                            //z[k] = z[0];
                            hasNodata = true;
                        } else {
                            dem->getData(in, jn, z[k]);
                        }
                    }
                    if (hasNodata) {
                        if (calprof) {
                            prof->setToNodata(i, j);
                        }
                        if (calplan) {
                            plan->setToNodata(i, j);
                        }
                        if (calhoriz) {
                            horiz->setToNodata(i, j);
                        }
                        if (calunspher) {
                            unspher->setToNodata(i, j);
                        }
                        if (calmeanc) {
                            meanc->setToNodata(i, j);
                        }
                        if (calmaxc) {
                            maxc->setToNodata(i, j);
                        }
                        if (calminc) {
                            minc->setToNodata(i, j);
                        }
                    } else {
                        p = (z[2] + z[1] + z[8] - z[4] - z[5] - z[6]) / (6.f * (float) dx);
                        q = (z[4] + z[3] + z[2] - z[6] - z[7] - z[8]) / (6.f * (float) dx);
                        r = (z[4] + z[2] + z[5] + z[1] + z[6] + z[8] - 2.f * (z[3] + z[0] + z[7])) / (3.f * cellsize2);
                        s = (-z[4] + z[2] + z[6] - z[8]) / (4.f * cellsize2);
                        t = (z[4] + z[3] + z[2] + z[6] + z[7] + z[8] - 2.f * (z[5] + z[0] + z[1])) / (3.f * cellsize2);
                        if (p == 0 && q == 0) {
                            if (calprof) {
                                prof->setData(i, j, 0.f);
                            }
                            if (calplan) {
                                plan->setData(i, j, 0.f);
                            }
                            if (calhoriz) {
                                horiz->setData(i, j, 0.f);
                            }
                        } else {
                            if (calprof) {
                                prof->setData(i, j, -(r * p * p + t * q * q + 2.f * p * q * s) /
                                    ((p * p + q * q) * pow((1.f + p * p + q * q), 1.5f)));
                            }
                            if (calplan) {
                                plan->setData(i, j, -(r * q * q + t * p * p - 2.f * p * q * s) /
                                    (pow((p * p + q * q), (float) 1.5)));
                            }
                            if (calhoriz) {
                                horiz->setData(i, j, -(q * q * r - 2.f * p * q * s + p * p * t) /
                                    ((p * p + q * q) * pow((1.f + p * p + q * q), 1.5f)));
                            }
                        }
                        if (calunspher || calmeanc || calmaxc || calminc) {
                            float meancurv = 0.f, unspherV = 0.f;
                            meancurv = -((1.f + q * q) * r - 2.f * p * q * s + (1.f + p * p) * t) /
                                (2.f * pow((1.f + p * p + q * q), 1.5f));
                            unspherV = sqrt(pow(
                                (r * sqrt((1.f + q * q) / (1.f + p * p)) - t / sqrt((1.f + q * q) / (1.f + p * p))),
                                2.f) / (1.f + p * p + q * q) +
                                pow((p * q * r * sqrt((1.f + q * q) / (1.f + p * p)) -
                                    2.f * sqrt((1.f + q * q) * (1.f + p * p)) * s +
                                    p * q * t / sqrt((1.f + q * q) / (1.f + p * p))), 2.f)) /
                                (2.f * pow((1.f + p * p + q * q), 1.5f));
                            if (calmeanc) {
                                meanc->setData(i, j, meancurv);
                            }
                            if (calmaxc) {
                                maxc->setData(i, j, meancurv + unspherV);
                            }
                            if (calminc) {
                                minc->setData(i, j, meancurv - unspherV);
                            }
                            if (calunspher) {
                                unspher->setData(i, j, unspherV);
                            }
                        }
                    }
                }
            }
        }
        // END COMPUTING CODE BLOCK
        double computet = MPI_Wtime(); // record computing time
        // create and write TIFF file
        float nodata = MISSINGFLOAT;
        if (calprof) {
            tiffIO profTIFF(profcfile, FLOAT_TYPE, &nodata, demf);
            profTIFF.write(xstart, ystart, ny, nx, prof->getGridPointer());
        }
        if (calplan) {
            tiffIO planTIFF(plancfile, FLOAT_TYPE, &nodata, demf);
            planTIFF.write(xstart, ystart, ny, nx, plan->getGridPointer());
        }
        if (calhoriz) {
            tiffIO horizTIFF(horizcfile, FLOAT_TYPE, &nodata, demf);
            horizTIFF.write(xstart, ystart, ny, nx, horiz->getGridPointer());
        }
        if (calunspher) {
            tiffIO unspherTIFF(unspherfile, FLOAT_TYPE, &nodata, demf);
            unspherTIFF.write(xstart, ystart, ny, nx, unspher->getGridPointer());
        }
        if (calmeanc) {
            tiffIO meancTIFF(meancfile, FLOAT_TYPE, &nodata, demf);
            meancTIFF.write(xstart, ystart, ny, nx, meanc->getGridPointer());
        }
        if (calmaxc) {
            tiffIO maxcTIFF(maxcfile, FLOAT_TYPE, &nodata, demf);
            maxcTIFF.write(xstart, ystart, ny, nx, maxc->getGridPointer());
        }
        if (calminc) {
            tiffIO mincTIFF(mincfile, FLOAT_TYPE, &nodata, demf);
            mincTIFF.write(xstart, ystart, ny, nx, minc->getGridPointer());
        }

        double writet = MPI_Wtime(); // record writing time

        double dataRead, compute, write, total, tempd;
        dataRead = readt - begint;
        compute = computet - readt;
        write = writet - computet;
        total = writet - begint;

        //MPI_Allreduce(&dataRead, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        //dataRead = tempd / size;
        //MPI_Allreduce(&compute, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        //compute = tempd / size;
        //MPI_Allreduce(&write, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        //write = tempd / size;
        //MPI_Allreduce(&total, &tempd, 1, MPI_DOUBLE, MPI_SUM, MCW);
        //total = tempd / size;

        MPI_Allreduce(&dataRead, &tempd, 1, MPI_DOUBLE, MPI_MAX, MCW);
        dataRead = tempd;
        MPI_Allreduce(&compute, &tempd, 1, MPI_DOUBLE, MPI_MAX, MCW);
        compute = tempd;
        MPI_Allreduce(&write, &tempd, 1, MPI_DOUBLE, MPI_MAX, MCW);
        write = tempd;
        MPI_Allreduce(&total, &tempd, 1, MPI_DOUBLE, MPI_MAX, MCW);
        total = tempd;

        if (rank == 0) {
            printf("Processors: %d\nRead time: %f\nCompute time: %f\nWrite time: %f\nTotal time: %f\n",
                   size, dataRead, compute, write, total);
        }
    }
    MPI_Finalize();
    return 0;
}