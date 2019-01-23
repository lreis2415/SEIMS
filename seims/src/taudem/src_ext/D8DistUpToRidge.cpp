/*  DinfDistUp function to compute distance to ridge in DEM
    based on D-infinity flow direction model, ridge is
	assigned by user.

  Liangjun, Zhu
  Lreis, CAS
  Apr 2, 2015

*/
#include <mpi.h>
#include <math.h>
#include <queue>
#include "commonLib.h"
#include "createpart.h"
#include "tiffIO.h"
#include "initneighbor.h"

// using namespace std; // Avoid to using the entire namespace of std. Comment by Liangjun, 01/23/19
using std::sqrt;

float dist[9];

int nameadd(char *full, char *arg, char *suff);

int d8distup(char *pfile, char *felfile, char *rdgfile, char *dtsfile, int statmethod, int userdg);

int hd8uptoridgegrd(char *pfile, char *rdgfile, char *dtsfile, int statmethod, int userdg);

int vrisetoridgegrd(char *pfile, char *felfile, char *rdgfile, char *dtsfile, int statmethod, int userdg);

int pdisttoridgegrd(char *pfile, char *felfile, char *rdgfile, char *dtsfile, int statmethod, int userdg);

int sdisttoridgegrd(char *pfile, char *felfile, char *rdgfile, char *dtsfile, int statmethod, int userdg);

//Calling function
int d8distup(char *pfile, char *felfile, char *rdgfile, char *dtsfile, int typemethod, int statmethod, int userdg)
{
    int er = -1;
    switch (typemethod)
    {
        case 0:
            er = hd8uptoridgegrd(pfile, rdgfile, dtsfile, statmethod, userdg);
            break;
        case 1:
            er = vrisetoridgegrd(pfile, felfile, rdgfile, dtsfile, statmethod, userdg);
            break;
        case 2:
            er = pdisttoridgegrd(pfile, felfile, rdgfile, dtsfile, statmethod, userdg);
            break;
        case 3:
            er = sdisttoridgegrd(pfile, felfile, rdgfile, dtsfile, statmethod, userdg);
            break;
    }
    return (er);
}

//*****************************//
//Horizontal distance to ridge //
//*****************************//
int hd8uptoridgegrd(char *pfile, char *rdgfile, char *dtsfile, int statmethod, int userdg)
{
    MPI_Init(NULL, NULL);
    {
        int rank, size;
        MPI_Comm_rank(MCW, &rank);
        MPI_Comm_size(MCW, &size);
        if (rank == 0) printf("D8DistUpToRidge -h version %s, added by Liangjun Zhu, Apr 7, 2015\n", TDVERSION);
        double begint = MPI_Wtime();
        tiffIO pf(pfile, SHORT_TYPE);
        long totalX = pf.getTotalX();
        long totalY = pf.getTotalY();
        double dx = pf.getdxA();
        double dy = pf.getdyA();
        //if (rank == 0)
        //{
        //	//float timeestimate=(1.2e-6*totalX*totalY/pow((double) size,0.65))/60+1;  // Time estimate in minutes
        //	//fprintf(stderr,"This run may take on the order of %.0f minutes to complete.\n",timeestimate);
        //	//fprintf(stderr,"This estimate is very approximate. \nRun time is highly uncertain as it depends on the complexity of the input data \nand speed and memory of the computer. This estimate is based on our testing on \na dual quad core Dell Xeon E5405 2.0GHz PC with 16GB RAM.\n");
        //	fprintf(stderr,"Start Processing...\n");
        //	fflush(stderr);
        //}
        int kk;
        for (kk = 1; kk <= 8; kk++)
        {
            dist[kk] = sqrt((float) dx * (float) dx * d2[kk] * d2[kk] + (float) dy * (float) dy * d1[kk] * d1[kk]);
        }
        // Read data
        tdpartition *flowdir;
        flowdir = CreateNewPartition(pf.getDatatype(), totalX, totalY, dx, dy, pf.getNodata());
        int nx = flowdir->getnx();
        int ny = flowdir->getny();
        int xstart, ystart;
        flowdir->localToGlobal(0, 0, xstart, ystart);
        pf.read(xstart, ystart, ny, nx, flowdir->getGridPointer());
        // if using ridgeData, get ridge sources from file
        tdpartition *ridgeData;
        if (userdg)
        {
            tiffIO rdg(rdgfile, SHORT_TYPE);
            if (!pf.compareTiff(rdg))
            {
                printf("File size do not match\n%s\n", rdgfile);
                MPI_Abort(MCW, 5);
                return 1;
            }
            ridgeData = CreateNewPartition(rdg.getDatatype(), totalX, totalY, dx, dy, rdg.getNodata());
            rdg.read(xstart, ystart, ny, nx, ridgeData->getGridPointer());
        }
        double readt = MPI_Wtime();
        // create new empty partition to store distance data
        tdpartition *dts;
        dts = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
        long i, j, in, jn;
        short k, angle, tempShort, sump;
        float dtss, distr;
        tdpartition *neighbor;
        neighbor = CreateNewPartition(SHORT_TYPE, totalX, totalY, dx, dy, MISSINGSHORT);
        // share information between ranks and set borders to zero
        flowdir->share();
        if (userdg)ridgeData->share();
        dts->share();
        neighbor->clearBorders();
        node temp;
        queue<node> que;
        int useOutlets = 0;
        long numOutlets = 0;
        int *outletsX = 0, *outletsY = 0;
        //Count the flow receiving neighbors and put node with no contributing neighbors on que
        initNeighborD8up(neighbor, flowdir, &que, nx, ny, useOutlets, outletsX, outletsY, numOutlets);
        //printf("initial ridge count is %d",que.size());
        bool finished = false;
        while (!finished)
        {
            while (!que.empty())
            {
                temp = que.front();
                que.pop();
                i = temp.x;
                j = temp.y;
                sump = 0;
                distr = 0.f;
                //distr = 0.5 * sqrt(2.0); // update 2015/5/15 by Liangjun Zhu, assume that the distance is calculated based on the left bottom corner

                bool first = true;
                // EVALUATE UP FLOW ALGEBRA EXPRESSION
                for (k = 1; k <= 8; k++)
                {
                    in = i + d1[k];
                    jn = j + d2[k];
                    if (!flowdir->hasAccess(in, jn) || flowdir->isNodata(in, jn))
                        dts->setToNodata(i, j);
                    else
                    {
                        flowdir->getData(in, jn, angle);
                        if (angle - k == 4 || angle - k == -4)
                        {
                            if (dts->isNodata(in, jn)) dts->setToNodata(i, j);
                            else
                            {
                                dts->getData(in, jn, dtss);
                                if (statmethod == 0)
                                {
                                    sump += 1;
                                    //printf("distr:%f  ",distr);
                                    distr += (dist[k] + dtss);
                                    //printf("dtss:%f,dist[k]:%f,distr:%f\n",dtss,dist[k],distr);

                                }
                                else if (statmethod == 1)
                                {
                                    if (dist[k] + dtss > distr) distr = dist[k] + dtss;
                                }
                                else if (statmethod == 2)
                                {
                                    if (first)
                                    {
                                        distr = dist[k] + dtss;
                                        first = false;
                                    }
                                    else if (dist[k] + dtss < distr) distr = dist[k] + dtss;
                                }
                            }
                        }
                    }
                }
                if (statmethod == 0 && sump >= 1)
                    dts->setData(i, j, distr / sump);
                else
                    dts->setData(i, j, distr);
                // END UP FLOW ALGEBRA EXPRESSION
                flowdir->getData(i, j, angle);
                in = i + d1[angle];
                jn = j + d2[angle];
                neighbor->addToData(in, jn, (short) -1);
                if (flowdir->isInPartition(in, jn) && neighbor->getData(in, jn, tempShort) == 0)
                {
                    temp.x = in;
                    temp.y = jn;
                    que.push(temp);
                }
                if (userdg)
                {
                    short rdgornot;
                    ridgeData->getData(i, j, rdgornot);
                    if (rdgornot == 1) dts->setData(i, j, (float) 0.);
                }
            }
            dts->share();
            neighbor->addBorders();
            for (i = 0; i < nx; i++)
            {
                if (neighbor->getData(i, -1, tempShort) != 0 && neighbor->getData(i, 0, tempShort) == 0)
                {
                    temp.x = i;
                    temp.y = 0;
                    que.push(temp);
                }
                if (neighbor->getData(i, ny, tempShort) != 0 && neighbor->getData(i, ny - 1, tempShort) == 0)
                {
                    temp.x = i;
                    temp.y = ny - 1;
                    que.push(temp);
                }
            }
            neighbor->clearBorders();
            finished = que.empty();
            finished = (bool) dts->ringTerm(finished);
        }
        //Stop timer
        double computet = MPI_Wtime();

        //Create and write TIFF file
        float ddNodata = MISSINGFLOAT;
        tiffIO dd(dtsfile, FLOAT_TYPE, ddNodata, pf);
        dd.write(xstart, ystart, ny, nx, dts->getGridPointer());

        double writet = MPI_Wtime();
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

        if (rank == 0)
            printf("Processors: %d\nRead time: %f\nCompute time: %f\nWrite time: %f\nTotal time: %f\n",
                   size, dataRead, compute, write, total);
		/// free memory
		delete flowdir;
		delete ridgeData;
		delete dts;
		delete neighbor;
    }
    MPI_Finalize();
    return 0;
}

//***************************//
//Vertical rise to the ridge //
//**************************//
int vrisetoridgegrd(char *pfile, char *felfile, char *rdgfile, char *dtsfile, int statmethod, int userdg)
{
    MPI_Init(NULL, NULL);
    {
        int rank, size;
        MPI_Comm_rank(MCW, &rank);
        MPI_Comm_size(MCW, &size);
        if (rank == 0) printf("D8DistUpToRidge -h version %s, added by Liangjun Zhu, Apr 7, 2015\n", TDVERSION);
        double begint = MPI_Wtime();
        tiffIO pf(pfile, SHORT_TYPE);
        long totalX = pf.getTotalX();
        long totalY = pf.getTotalY();
        double dx = pf.getdxA();
        double dy = pf.getdyA();
        //if (rank == 0)
        //{
        //	//float timeestimate=(1.2e-6*totalX*totalY/pow((double) size,0.65))/60+1;  // Time estimate in minutes
        //	//fprintf(stderr,"This run may take on the order of %.0f minutes to complete.\n",timeestimate);
        //	//fprintf(stderr,"This estimate is very approximate. \nRun time is highly uncertain as it depends on the complexity of the input data \nand speed and memory of the computer. This estimate is based on our testing on \na dual quad core Dell Xeon E5405 2.0GHz PC with 16GB RAM.\n");
        //	fprintf(stderr,"Start Processing...\n");
        //	fflush(stderr);
        //}

        // Read data
        tdpartition *flowdir;
        flowdir = CreateNewPartition(pf.getDatatype(), totalX, totalY, dx, dy, pf.getNodata());
        int nx = flowdir->getnx();
        int ny = flowdir->getny();
        int xstart, ystart;
        flowdir->localToGlobal(0, 0, xstart, ystart);
        pf.read(xstart, ystart, ny, nx, flowdir->getGridPointer());

        tdpartition *felData;
        tiffIO felf(felfile, FLOAT_TYPE);
        if (!pf.compareTiff(felf))
        {
            printf("File size do not match\n%s\n", rdgfile);
            MPI_Abort(MCW, 5);
            return 1;
        }
        felData = CreateNewPartition(felf.getDatatype(), totalX, totalY, dx, dy, felf.getNodata());
        felf.read(xstart, ystart, ny, nx, felData->getGridPointer());

        // if using ridgeData, get ridge sources from file
        tdpartition *ridgeData;
        if (userdg)
        {
            tiffIO rdg(rdgfile, SHORT_TYPE);
            if (!pf.compareTiff(rdg))
            {
                printf("File size do not match\n%s\n", rdgfile);
                MPI_Abort(MCW, 5);
                return 1;
            }
            ridgeData = CreateNewPartition(rdg.getDatatype(), totalX, totalY, dx, dy, rdg.getNodata());
            rdg.read(xstart, ystart, ny, nx, ridgeData->getGridPointer());
        }
        double readt = MPI_Wtime();
        // create new empty partition to store distance data
        tdpartition *dts;
        dts = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
        long i, j, in, jn;
        short k, angle, tempShort, sump;
        float dtss, distr;
        float elev, elevn, distk;
        tdpartition *neighbor;
        neighbor = CreateNewPartition(SHORT_TYPE, totalX, totalY, dx, dy, MISSINGSHORT);
        // share information between ranks and set borders to zero
        flowdir->share();
        if (userdg)ridgeData->share();
        felData->share();
        dts->share();
        neighbor->clearBorders();
        node temp;
        queue<node> que;
        int useOutlets = 0;
        long numOutlets = 0;
        int *outletsX = 0, *outletsY = 0;
        //Count the flow receiving neighbors and put node with no contributing neighbors on que
        initNeighborD8up(neighbor, flowdir, &que, nx, ny, useOutlets, outletsX, outletsY, numOutlets);
        //printf("initial ridge count is %d",que.size());
        bool finished = false;
        while (!finished)
        {
            while (!que.empty())
            {
                temp = que.front();
                que.pop();
                i = temp.x;
                j = temp.y;
                sump = 0;
                distr = 0.f;
                bool first = true;
                felData->getData(i, j, elev);
                // EVALUATE UP FLOW ALGEBRA EXPRESSION
                for (k = 1; k <= 8; k++)
                {
                    in = i + d1[k];
                    jn = j + d2[k];
                    if (!flowdir->hasAccess(in, jn) || flowdir->isNodata(in, jn))
                        dts->setToNodata(i, j);
                    else
                    {
                        flowdir->getData(in, jn, angle);
                        if (angle - k == 4 || angle - k == -4)
                        {
                            if (dts->isNodata(in, jn)) dts->setToNodata(i, j);
                            else
                            {
                                felData->getData(in, jn, elevn);
                                dts->getData(in, jn, dtss);
                                distk = elevn - elev;
                                if (statmethod == 0)
                                {
                                    sump += 1;
                                    distr += (distk + dtss);
                                }
                                else if (statmethod == 1)
                                {
                                    if (distk + dtss > distr) distr = distk + dtss;
                                }
                                else if (statmethod == 2)
                                {
                                    if (first)
                                    {
                                        distr = distk + dtss;
                                        first = false;
                                    }
                                    else if (dist[k] + dtss < distr) distr = distk + dtss;
                                }
                            }
                        }
                    }
                }
                if (statmethod == 0 && sump >= 1)
                    dts->setData(i, j, distr / sump);
                else
                    dts->setData(i, j, distr);
                // END UP FLOW ALGEBRA EXPRESSION
                flowdir->getData(i, j, angle);
                in = i + d1[angle];
                jn = j + d2[angle];
                neighbor->addToData(in, jn, (short) -1);
                if (flowdir->isInPartition(in, jn) && neighbor->getData(in, jn, tempShort) == 0)
                {
                    temp.x = in;
                    temp.y = jn;
                    que.push(temp);
                }
                if (userdg)
                {
                    short rdgornot;
                    ridgeData->getData(i, j, rdgornot);
                    if (rdgornot == 1) dts->setData(i, j, (float) 0.);
                }
            }
            dts->share();
            neighbor->addBorders();
            for (i = 0; i < nx; i++)
            {
                if (neighbor->getData(i, -1, tempShort) != 0 && neighbor->getData(i, 0, tempShort) == 0)
                {
                    temp.x = i;
                    temp.y = 0;
                    que.push(temp);
                }
                if (neighbor->getData(i, ny, tempShort) != 0 && neighbor->getData(i, ny - 1, tempShort) == 0)
                {
                    temp.x = i;
                    temp.y = ny - 1;
                    que.push(temp);
                }
            }
            neighbor->clearBorders();
            finished = que.empty();
            finished = dts->ringTerm(finished);
        }
        //Stop timer
        double computet = MPI_Wtime();

        //Create and write TIFF file
        float ddNodata = MISSINGFLOAT;
        tiffIO dd(dtsfile, FLOAT_TYPE, ddNodata, pf);
        dd.write(xstart, ystart, ny, nx, dts->getGridPointer());

        double writet = MPI_Wtime();
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

        if (rank == 0)
            printf("Processors: %d\nRead time: %f\nCompute time: %f\nWrite time: %f\nTotal time: %f\n",
                   size, dataRead, compute, write, total);
    }
    MPI_Finalize();
    return 0;
}
//******************************//
//Surface distance to the ridge //
//*****************************//

int sdisttoridgegrd(char *pfile, char *felfile, char *rdgfile, char *dtsfile, int statmethod, int userdg)
{
    MPI_Init(NULL, NULL);
    {
        int rank, size;
        MPI_Comm_rank(MCW, &rank);
        MPI_Comm_size(MCW, &size);
        if (rank == 0) printf("D8DistUpToRidge -h version %s, added by Liangjun Zhu, Apr 7, 2015\n", TDVERSION);
        double begint = MPI_Wtime();
        tiffIO pf(pfile, SHORT_TYPE);
        long totalX = pf.getTotalX();
        long totalY = pf.getTotalY();
        double dx = pf.getdxA();
        double dy = pf.getdyA();
        //if (rank == 0)
        //{
        //	//float timeestimate=(1.2e-6*totalX*totalY/pow((double) size,0.65))/60+1;  // Time estimate in minutes
        //	//fprintf(stderr,"This run may take on the order of %.0f minutes to complete.\n",timeestimate);
        //	//fprintf(stderr,"This estimate is very approximate. \nRun time is highly uncertain as it depends on the complexity of the input data \nand speed and memory of the computer. This estimate is based on our testing on \na dual quad core Dell Xeon E5405 2.0GHz PC with 16GB RAM.\n");
        //	fprintf(stderr,"Start Processing...\n");
        //	fflush(stderr);
        //}
        int kk;
        for (kk = 1; kk <= 8; kk++)
        {
            dist[kk] = sqrt((float) dx * (float) dx * d2[kk] * d2[kk] + (float) dy * (float) dy * d1[kk] * d1[kk]);
        }
        // Read data
        tdpartition *flowdir;
        flowdir = CreateNewPartition(pf.getDatatype(), totalX, totalY, dx, dy, pf.getNodata());
        int nx = flowdir->getnx();
        int ny = flowdir->getny();
        int xstart, ystart;
        flowdir->localToGlobal(0, 0, xstart, ystart);
        pf.read(xstart, ystart, ny, nx, flowdir->getGridPointer());

        tdpartition *felData;
        tiffIO felf(felfile, FLOAT_TYPE);
        if (!pf.compareTiff(felf))
        {
            printf("File size do not match\n%s\n", rdgfile);
            MPI_Abort(MCW, 5);
            return 1;
        }
        felData = CreateNewPartition(felf.getDatatype(), totalX, totalY, dx, dy, felf.getNodata());
        felf.read(xstart, ystart, ny, nx, felData->getGridPointer());

        // if using ridgeData, get ridge sources from file
        tdpartition *ridgeData;
        if (userdg)
        {
            tiffIO rdg(rdgfile, SHORT_TYPE);
            if (!pf.compareTiff(rdg))
            {
                printf("File size do not match\n%s\n", rdgfile);
                MPI_Abort(MCW, 5);
                return 1;
            }
            ridgeData = CreateNewPartition(rdg.getDatatype(), totalX, totalY, dx, dy, rdg.getNodata());
            rdg.read(xstart, ystart, ny, nx, ridgeData->getGridPointer());
        }
        double readt = MPI_Wtime();
        // create new empty partition to store distance data
        tdpartition *dts;
        dts = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
        long i, j, in, jn;
        short k, angle, tempShort, sump;
        float dtss, distr;
        float elev, elevn, distk;
        tdpartition *neighbor;
        neighbor = CreateNewPartition(SHORT_TYPE, totalX, totalY, dx, dy, MISSINGSHORT);
        // share information between ranks and set borders to zero
        flowdir->share();
        if (userdg)ridgeData->share();
        felData->share();
        dts->share();
        neighbor->clearBorders();
        node temp;
        queue<node> que;
        int useOutlets = 0;
        long numOutlets = 0;
        int *outletsX = 0, *outletsY = 0;
        //Count the flow receiving neighbors and put node with no contributing neighbors on que
        initNeighborD8up(neighbor, flowdir, &que, nx, ny, useOutlets, outletsX, outletsY, numOutlets);
        //printf("initial ridge count is %d",que.size());
        bool finished = false;
        while (!finished)
        {
            while (!que.empty())
            {
                temp = que.front();
                que.pop();
                i = temp.x;
                j = temp.y;
                sump = 0;
                distr = 0.f;
                bool first = true;
                felData->getData(i, j, elev);
                // EVALUATE UP FLOW ALGEBRA EXPRESSION
                for (k = 1; k <= 8; k++)
                {
                    in = i + d1[k];
                    jn = j + d2[k];
                    if (!flowdir->hasAccess(in, jn) || flowdir->isNodata(in, jn))
                        dts->setToNodata(i, j);
                    else
                    {
                        flowdir->getData(in, jn, angle);
                        if (angle - k == 4 || angle - k == -4)
                        {
                            if (dts->isNodata(in, jn)) dts->setToNodata(i, j);
                            else
                            {
                                felData->getData(in, jn, elevn);
                                dts->getData(in, jn, dtss);
                                distk = sqrt((elevn - elev) * (elevn - elev) + dist[k] * dist[k]);
                                if (statmethod == 0)
                                {
                                    sump += 1;
                                    distr += (distk + dtss);
                                }
                                else if (statmethod == 1)
                                {
                                    if (distk + dtss > distr) distr = distk + dtss;
                                }
                                else if (statmethod == 2)
                                {
                                    if (first)
                                    {
                                        distr = distk + dtss;
                                        first = false;
                                    }
                                    else if (dist[k] + dtss < distr) distr = distk + dtss;
                                }
                            }
                        }
                    }
                }
                if (statmethod == 0 && sump >= 1)
                    dts->setData(i, j, distr / sump);
                else
                    dts->setData(i, j, distr);
                // END UP FLOW ALGEBRA EXPRESSION
                flowdir->getData(i, j, angle);
                in = i + d1[angle];
                jn = j + d2[angle];
                neighbor->addToData(in, jn, (short) -1);
                if (flowdir->isInPartition(in, jn) && neighbor->getData(in, jn, tempShort) == 0)
                {
                    temp.x = in;
                    temp.y = jn;
                    que.push(temp);
                }
                if (userdg)
                {
                    short rdgornot;
                    ridgeData->getData(i, j, rdgornot);
                    if (rdgornot == 1) dts->setData(i, j, (float) 0.);
                }
            }
            dts->share();
            neighbor->addBorders();
            for (i = 0; i < nx; i++)
            {
                if (neighbor->getData(i, -1, tempShort) != 0 && neighbor->getData(i, 0, tempShort) == 0)
                {
                    temp.x = i;
                    temp.y = 0;
                    que.push(temp);
                }
                if (neighbor->getData(i, ny, tempShort) != 0 && neighbor->getData(i, ny - 1, tempShort) == 0)
                {
                    temp.x = i;
                    temp.y = ny - 1;
                    que.push(temp);
                }
            }
            neighbor->clearBorders();
            finished = que.empty();
            finished = (bool) dts->ringTerm(finished);
        }
        //Stop timer
        double computet = MPI_Wtime();

        //Create and write TIFF file
        float ddNodata = MISSINGFLOAT;
        tiffIO dd(dtsfile, FLOAT_TYPE, ddNodata, pf);
        dd.write(xstart, ystart, ny, nx, dts->getGridPointer());

        double writet = MPI_Wtime();
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

        if (rank == 0)
            printf("Processors: %d\nRead time: %f\nCompute time: %f\nWrite time: %f\nTotal time: %f\n",
                   size, dataRead, compute, write, total);
    }
    MPI_Finalize();
    return 0;
}


//*********************************//
//Pythagoras distance to the ridge //
//********************************//
int pdisttoridgegrd(char *pfile, char *felfile, char *rdgfile, char *dtsfile, int statmethod, int userdg)
{
    MPI_Init(NULL, NULL);
    {
        int rank, size;
        MPI_Comm_rank(MCW, &rank);
        MPI_Comm_size(MCW, &size);
        if (rank == 0) printf("D8DistUpToRidge -h version %s, added by Liangjun Zhu, Apr 7, 2015\n", TDVERSION);
        double begint = MPI_Wtime();
        tiffIO pf(pfile, SHORT_TYPE);
        long totalX = pf.getTotalX();
        long totalY = pf.getTotalY();
        double dx = pf.getdxA();
        double dy = pf.getdyA();
        //if (rank == 0)
        //{
        //	//float timeestimate=(1.2e-6*totalX*totalY/pow((double) size,0.65))/60+1;  // Time estimate in minutes
        //	//fprintf(stderr,"This run may take on the order of %.0f minutes to complete.\n",timeestimate);
        //	//fprintf(stderr,"This estimate is very approximate. \nRun time is highly uncertain as it depends on the complexity of the input data \nand speed and memory of the computer. This estimate is based on our testing on \na dual quad core Dell Xeon E5405 2.0GHz PC with 16GB RAM.\n");
        //	fprintf(stderr,"Start Processing...\n");
        //	fflush(stderr);
        //}
        int kk;
        for (kk = 1; kk <= 8; kk++)
        {
            dist[kk] = sqrt((float) dx * (float) dx * d2[kk] * d2[kk] + (float) dy * (float) dy * d1[kk] * d1[kk]);
        }
        // Read data
        tdpartition *flowdir;
        flowdir = CreateNewPartition(pf.getDatatype(), totalX, totalY, dx, dy, pf.getNodata());
        int nx = flowdir->getnx();
        int ny = flowdir->getny();
        int xstart, ystart;
        flowdir->localToGlobal(0, 0, xstart, ystart);
        flowdir->savedxdyc(pf);
        pf.read(xstart, ystart, ny, nx, flowdir->getGridPointer());

        tdpartition *felData;
        tiffIO felf(felfile, FLOAT_TYPE);
        if (!pf.compareTiff(felf))
        {
            printf("File size do not match\n%s\n", rdgfile);
            MPI_Abort(MCW, 5);
            return 1;
        }
        felData = CreateNewPartition(felf.getDatatype(), totalX, totalY, dx, dy, felf.getNodata());
        felf.read(xstart, ystart, ny, nx, felData->getGridPointer());

        // if using ridgeData, get ridge sources from file
        tdpartition *ridgeData;
        if (userdg)
        {
            tiffIO rdg(rdgfile, SHORT_TYPE);
            if (!pf.compareTiff(rdg))
            {
                printf("File size do not match\n%s\n", rdgfile);
                MPI_Abort(MCW, 5);
                return 1;
            }
            ridgeData = CreateNewPartition(rdg.getDatatype(), totalX, totalY, dx, dy, rdg.getNodata());
            rdg.read(xstart, ystart, ny, nx, ridgeData->getGridPointer());
        }
        double readt = MPI_Wtime();
        // create new empty partition to store distance data
        tdpartition *dts;  // store horizontal and pythagoras distance
        dts = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
        tdpartition *dtsv; // store vertical distance
        dtsv = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);

        long i, j, in, jn;
        short k, angle, tempShort, sump;
        float dtss, dtsvv, distr, distrh;
        float elev, elevn, distk;
        tdpartition *neighbor;
        neighbor = CreateNewPartition(SHORT_TYPE, totalX, totalY, dx, dy, MISSINGSHORT);
        // share information between ranks and set borders to zero
        flowdir->share();
        if (userdg)ridgeData->share();
        felData->share();
        dts->share();
        dtsv->share();
        neighbor->clearBorders();

        node temp;
        queue<node> que;
        int useOutlets = 0;
        long numOutlets = 0;
        int *outletsX = 0, *outletsY = 0;
        //Count the flow receiving neighbors and put node with no contributing neighbors on que
        initNeighborD8up(neighbor, flowdir, &que, nx, ny, useOutlets, outletsX, outletsY, numOutlets);
        //printf("initial ridge count is %d",que.size());
        bool finished = false;
        while (!finished)
        {
            while (!que.empty())
            {
                temp = que.front();
                que.pop();
                i = temp.x;
                j = temp.y;
                sump = 0;
                distr = 0.f;
                distrh = 0.f;
                //distr = 0.5 * sqrt(2.0);
                bool first = true;
                felData->getData(i, j, elev);
                // EVALUATE UP FLOW ALGEBRA EXPRESSION
                for (k = 1; k <= 8; k++)
                {
                    in = i + d1[k];
                    jn = j + d2[k];
                    if (!flowdir->hasAccess(in, jn) || flowdir->isNodata(in, jn))
                    {
                        dts->setToNodata(i, j);
                        dtsv->setToNodata(i, j);
                    }
                    else
                    {
                        flowdir->getData(in, jn, angle);
                        if (angle - k == 4 || angle - k == -4)
                        {
                            if (dts->isNodata(in, jn) || dtsv->isNodata(in, jn))
                            {
                                dts->setToNodata(i, j);
                                dtsv->setToNodata(i, j);
                            }
                            else if (felData->isNodata(in, jn))
                            {
                                dts->setToNodata(i, j);
                                dtsv->setToNodata(i, j);
                            }
                            else
                            {
                                felData->getData(in, jn, elevn);
                                dts->getData(in, jn, dtss);
                                dtsv->getData(in, jn, dtsvv);
                                distk = elevn - elev;
                                if (statmethod == 0)
                                {
                                    sump += 1;
                                    distrh += (dtss + dist[k]);
                                    distr += (distk + dtsvv);
                                }
                                else if (statmethod == 1)
                                {
                                    if (distk + dtsvv > distr) distr = distk + dtsvv;
                                    if (dtss + dist[k] > distrh) distrh = dtss + dist[k];
                                }
                                else if (statmethod == 2)
                                {
                                    if (first)
                                    {
                                        distr = distk + dtsvv;
                                        distrh = dtss + dist[k];
                                        first = false;
                                    }
                                    else
                                    {
                                        if (distk + dtsvv < distr) distr = distk + dtsvv;
                                        if (dtss + dist[k] < distrh) distrh = dtss + dist[k];
                                    }
                                }
                            }
                        }
                    }
                }
                if (statmethod == 0 && sump >= 1)
                {
                    dts->setData(i, j, distrh / sump);
                    dtsv->setData(i, j, distr / sump);
                }
                else
                {
                    dts->setData(i, j, distrh);
                    dtsv->setData(i, j, distr);
                }
                // END UP FLOW ALGEBRA EXPRESSION
                flowdir->getData(i, j, angle);
                in = i + d1[angle];
                jn = j + d2[angle];
                neighbor->addToData(in, jn, (short) -1);
                if (flowdir->isInPartition(in, jn) && neighbor->getData(in, jn, tempShort) == 0)
                {
                    temp.x = in;
                    temp.y = jn;
                    que.push(temp);
                }
                if (userdg)
                {
                    short rdgornot;
                    ridgeData->getData(i, j, rdgornot);
                    if (rdgornot == 1)
                    {
                        dts->setData(i, j, 0.f);
                        dtsv->setData(i, j, 0.f);
                    }
                }
            }
            dts->share();
            dtsv->share();
            neighbor->addBorders();
            for (i = 0; i < nx; i++)
            {
                if (neighbor->getData(i, -1, tempShort) != 0 && neighbor->getData(i, 0, tempShort) == 0)
                {
                    temp.x = i;
                    temp.y = 0;
                    que.push(temp);
                }
                if (neighbor->getData(i, ny, tempShort) != 0 && neighbor->getData(i, ny - 1, tempShort) == 0)
                {
                    temp.x = i;
                    temp.y = ny - 1;
                    que.push(temp);
                }
            }
            neighbor->clearBorders();
            finished = que.empty();
            finished = (bool) dts->ringTerm(finished);
        }
        //  Now compute the pythagorus difference
        for (j = 0; j < ny; j++)
        {
            for (i = 0; i < nx; i++)
            {
                if (dtsv->isNodata(i, j))dts->setToNodata(i, j);
                else if (!dts->isNodata(i, j))
                {
                    dts->getData(i, j, distrh);
                    dtsv->getData(i, j, distr);
                    dts->setData(i, j, (float) sqrt(distrh * distrh + distr * distr));
                }
            }
        }
        //Stop timer
        double computet = MPI_Wtime();

        //Create and write TIFF file
        float ddNodata = MISSINGFLOAT;
        tiffIO dd(dtsfile, FLOAT_TYPE, ddNodata, pf);
        dd.write(xstart, ystart, ny, nx, dts->getGridPointer());

        double writet = MPI_Wtime();
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

        if (rank == 0)
            printf("Processors: %d\nRead time: %f\nCompute time: %f\nWrite time: %f\nTotal time: %f\n",
                   size, dataRead, compute, write, total);
    }
    MPI_Finalize();
    return 0;
}
