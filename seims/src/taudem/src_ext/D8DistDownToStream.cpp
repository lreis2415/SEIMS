/*  D8DistDownToStream

  This function computes the distance from each grid cell moving downstream until a stream
  grid cell as defined by the Stream Raster grid is encountered.  The optional threshold
  input is to specify a threshold to be applied to the Stream Raster grid (src).
  Stream grid cells are defined as having src value >= the threshold, or >=1 if a
  threshold is not specified.
  Distance method used to calculate the distance down to the stream include: the total straight line path (Pythagoras),
  the horizontal component of the straight line path, the vertical component of the straight line path,
  or the total surface flow path.

  Liangjun Zhu
  Lreis, CAS
  Apr 2, 2015

*/


#include <mpi.h>
#include <math.h>
#include <queue>
#include "commonLib.h"
#include "linearpart.h"
#include "createpart.h"
#include "tiffIO.h"

// using namespace std; // Avoid to using the entire namespace of std. Comment by Liangjun, 01/23/19

// header definition
int distgrid(char *pfile, char *felfile, char *srcfile, char *distfile, int typemethod, int thresh);
int hdisttostrmgrd(char *pfile, char *srcfile, char *distfile, int thresh);
int vdroptostrmgrd(char *pfile, char *felfile, char *srcfile, char *distfile, int thresh);
int pdisttostrmgrd(char *pfile, char *felfile, char *srcfile, char *distfile, int thresh);
int sdisttostrmgrd(char *pfile, char *felfile, char *srcfile, char *distfile, int thresh);

int distgrid(char *pfile, char *felfile, char *srcfile, char *distfile, int typemethod, int thresh) {
    int er;
    switch (typemethod) {
        case 0: er = hdisttostrmgrd(pfile, srcfile, distfile, thresh);
            break;
        case 1: er = vdroptostrmgrd(pfile, felfile, srcfile, distfile, thresh);
            break;
        case 2: er = pdisttostrmgrd(pfile, felfile, srcfile, distfile, thresh);
            break;
        case 3: er = sdisttostrmgrd(pfile, felfile, srcfile, distfile, thresh);
            break;
    }
    return (er);
}

int hdisttostrmgrd(char *pfile, char *srcfile, char *distfile, int thresh) {
    MPI_Init(NULL, NULL);
    {  //  All code within braces so that objects go out of context and destruct before MPI is closed
        int rank, size;
        MPI_Comm_rank(MCW, &rank);
        MPI_Comm_size(MCW, &size);
        if (rank == 0)printf("D8DistDownToStream version %s, added by Liangjun Zhu, Apr 2, 2015\n", TDVERSION);
        int i, j, in, jn;
        float tempFloat;
        short tempShort, k;
        int32_t tempLong;
        bool finished;

        //  Begin timer
        double begint = MPI_Wtime();

        //Read Flow Direction header using tiffIO
        tiffIO pf(pfile, SHORT_TYPE);
        long totalX = pf.getTotalX();
        long totalY = pf.getTotalY();
        double dx = pf.getdxA();
        double dy = pf.getdyA();
        //if(rank==0)
        //	{
        //		float timeestimate=(1.2e-6*totalX*totalY/pow((double) size,0.65))/60+1;  // Time estimate in minutes
        //		fprintf(stderr,"This run may take on the order of %.0f minutes to complete.\n",timeestimate);
        //		fprintf(stderr,"This estimate is very approximate. \nRun time is highly uncertain as it depends on the complexity of the input data \nand speed and memory of the computer. This estimate is based on our testing on \na dual quad core Dell Xeon E5405 2.0GHz PC with 16GB RAM.\n");
        //		fflush(stderr);
        //	}

        //Read flow direction data into partition
        tdpartition *p;
        p = CreateNewPartition(pf.getDatatype(), totalX, totalY, dx, dy, pf.getNodata());
        int nx = p->getnx();
        int ny = p->getny();
        int xstart, ystart;
        p->localToGlobal(0, 0, xstart, ystart);
        pf.read(xstart, ystart, ny, nx, p->getGridPointer());

        //Read stream source file
        tdpartition *src;
        tiffIO srcf(srcfile, LONG_TYPE);
        if (!pf.compareTiff(srcf)) {
            printf("File sizes do not match\n%s\n", srcfile);
            MPI_Abort(MCW, 5);
            return 1;  //And maybe an unhappy error message
        }
        src = CreateNewPartition(srcf.getDatatype(), totalX, totalY, dx, dy, srcf.getNodata());
        srcf.read(xstart, ystart, ny, nx, src->getGridPointer());

        //Record time reading files
        double readt = MPI_Wtime();

        //Create empty partition to store distance information
        tdpartition *fdarr;
        fdarr = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);

        /*  Calculate Distances  */
        float dist[9];
        for (i = 1; i <= 8; i++) {
            dist[i] = sqrt(d1[i] * d1[i] * dx * dx + d2[i] * d2[i] * dy * dy);
        }

        //  Set neighbor partition to 1 because all grid cells drain to one other grid cell in D8
        //  Set stream source grid cell's value to 0, and put stream cell to que
        tdpartition *neighbor;
        neighbor = CreateNewPartition(SHORT_TYPE, totalX, totalY, dx, dy, MISSINGSHORT);

        node temp;
        queue <node> que;
        for (j = 0; j < ny; j++) { // rows
            for (i = 0; i < nx; i++) { // cols
                if (!p->isNodata(i, j)) {
                    //Set contributing neighbors to 1
                    neighbor->setData(i, j, (short) 1);
                }
                if (!src->isNodata(i, j) && src->getData(i, j, tempLong) >= thresh) {
                    neighbor->setData(i, j, (short) 0);
                    temp.x = i;
                    temp.y = j;
                    que.push(temp);
                }
            }
        }

        //Share information and set borders to zero
        p->share();
        src->share();
        fdarr->share();
        neighbor->clearBorders();

        finished = false;
        //Ring terminating while loop
        while (!finished) {
            while (!que.empty()) {
                //Takes next node with no contributing neighbors
                temp = que.front();
                que.pop();
                i = temp.x;
                j = temp.y;
                //  FLOW ALGEBRA EXPRESSION EVALUATION
                //  If on stream
                if (!src->isNodata(i, j) && src->getData(i, j, tempLong) >= thresh) {
                    fdarr->setData(i, j, (float) 0.0);
                } else {
                    p->getData(i, j, k);  //  Get downstream neighbor
                    in = i + d1[k];
                    jn = j + d2[k];
                    if (fdarr->isNodata(in, jn)) { fdarr->setToNodata(i, j); }
                    else {
                        fdarr->setData(i, j, (float) (dist[k] + fdarr->getData(in, jn, tempFloat)));
                    }
                }
                //  Now find upslope cells and reduce dependencies
                for (k = 1; k <= 8; k++) {
                    in = i + d1[k];
                    jn = j + d2[k];
                    //test if neighbor drains towards cell excluding boundaries
                    if (!p->isNodata(in, jn)) {
                        p->getData(in, jn, tempShort);
                        if (tempShort - k == 4 || tempShort - k == -4) {
                            //Decrement the number of contributing neighbors in neighbor
                            neighbor->addToData(in, jn, (short) -1);
                            //Check if neighbor needs to be added to que
                            if (p->isInPartition(in, jn) && neighbor->getData(in, jn, tempShort) == 0) {
                                temp.x = in;
                                temp.y = jn;
                                que.push(temp);
                            }
                        }
                    }
                }
            }
            //  Here the queue is empty
            //Pass information
            fdarr->share();
            neighbor->addBorders();

            //If this created a cell with no contributing neighbors, put it on the queue
            for (i = 0; i < nx; i++) {
                if (neighbor->getData(i, -1, tempShort) != 0 && neighbor->getData(i, 0, tempShort) == 0) {
                    temp.x = i;
                    temp.y = 0;
                    que.push(temp);
                }
                if (neighbor->getData(i, ny, tempShort) != 0 && neighbor->getData(i, ny - 1, tempShort) == 0) {
                    temp.x = i;
                    temp.y = ny - 1;
                    que.push(temp);
                }
            }
            //Clear out borders
            neighbor->clearBorders();

            //Check if done
            finished = que.empty();
            finished = fdarr->ringTerm(finished);
        }
        //Stop timer
        double computet = MPI_Wtime();

        //Create and write TIFF file
        float aNodata = MISSINGFLOAT;
        tiffIO a(distfile, FLOAT_TYPE, aNodata, pf);
        a.write(xstart, ystart, ny, nx, fdarr->getGridPointer());
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

        if (rank == 0) {
            printf("Processors: %d\nRead time: %f\nCompute time: %f\nWrite time: %f\nTotal time: %f\n",
                   size, dataRead, compute, write, total);
        }

        //Brackets force MPI-dependent objects to go out of scope before Finalize is called
    }
    MPI_Finalize();
    return (0);
}

int vdroptostrmgrd(char *pfile, char *felfile, char *srcfile, char *distfile, int thresh) {
    MPI_Init(NULL, NULL);
    {  //  All code within braces so that objects go out of context and destruct before MPI is closed
        int rank, size;
        MPI_Comm_rank(MCW, &rank);
        MPI_Comm_size(MCW, &size);
        if (rank == 0)printf("D8DistDownToStream version %s, added by Liangjun Zhu, Apr 2, 2015\n", TDVERSION);
        int i, j, in, jn;
        float tempFloat;
        short tempShort, k;
        int32_t tempLong;
        bool finished;
        float elev, elevn;

        //  Begin timer
        double begint = MPI_Wtime();

        //Read Flow Direction header using tiffIO
        tiffIO pf(pfile, SHORT_TYPE);
        long totalX = pf.getTotalX();
        long totalY = pf.getTotalY();
        double dx = pf.getdxA();
        double dy = pf.getdyA();
        //if(rank==0)
        //{
        //	float timeestimate=(1.2e-6*totalX*totalY/pow((double) size,0.65))/60+1;  // Time estimate in minutes
        //	fprintf(stderr,"This run may take on the order of %.0f minutes to complete.\n",timeestimate);
        //	fprintf(stderr,"This estimate is very approximate. \nRun time is highly uncertain as it depends on the complexity of the input data \nand speed and memory of the computer. This estimate is based on our testing on \na dual quad core Dell Xeon E5405 2.0GHz PC with 16GB RAM.\n");
        //	fflush(stderr);
        //}

        //Read flow direction data into partition
        tdpartition *p;
        p = CreateNewPartition(pf.getDatatype(), totalX, totalY, dx, dy, pf.getNodata());
        int nx = p->getnx();
        int ny = p->getny();
        int xstart, ystart;
        p->localToGlobal(0, 0, xstart, ystart);
        pf.read(xstart, ystart, ny, nx, p->getGridPointer());

        //Read filled dem data into partition
        tdpartition *fel;
        tiffIO felf(felfile, FLOAT_TYPE);
        if (!pf.compareTiff(felf)) {
            printf("File sizes do not match\n%s\n", felfile);
            MPI_Abort(MCW, 5);
            return 1;
        }
        fel = CreateNewPartition(felf.getDatatype(), totalX, totalY, dx, dy, felf.getNodata());
        felf.read(xstart, ystart, ny, nx, fel->getGridPointer());
        //Read stream source file
        tdpartition *src;
        tiffIO srcf(srcfile, LONG_TYPE);
        if (!pf.compareTiff(srcf)) {
            printf("File sizes do not match\n%s\n", srcfile);
            MPI_Abort(MCW, 5);
            return 1;  //And maybe an unhappy error message
        }
        src = CreateNewPartition(srcf.getDatatype(), totalX, totalY, dx, dy, srcf.getNodata());
        srcf.read(xstart, ystart, ny, nx, src->getGridPointer());

        //Record time reading files
        double readt = MPI_Wtime();

        //Create empty partition to store distance information
        tdpartition *fdarr;
        fdarr = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
        /*  Calculate Distances  */
        float dist[9];
        for (i = 1; i <= 8; i++) {
            dist[i] = sqrt(d1[i] * d1[i] * dx * dx + d2[i] * d2[i] * dy * dy);
        }

        //  Set neighbor partition to 1 because all grid cells drain to one other grid cell in D8
        //  Set stream source grid cell's value to 0, and put stream cell to que
        tdpartition *neighbor;
        neighbor = CreateNewPartition(SHORT_TYPE, totalX, totalY, dx, dy, MISSINGSHORT);

        node temp;
        queue <node> que;
        for (j = 0; j < ny; j++) { // rows
            for (i = 0; i < nx; i++) { // cols
                if (!p->isNodata(i, j)) {
                    //Set contributing neighbors to 1
                    neighbor->setData(i, j, (short) 1);
                }
                if (!src->isNodata(i, j) && src->getData(i, j, tempLong) >= thresh) {
                    neighbor->setData(i, j, (short) 0);
                    temp.x = i;
                    temp.y = j;
                    que.push(temp);
                }
            }
        }

        //Share information and set borders to zero
        p->share();
        fel->share();
        src->share();
        fdarr->share();
        neighbor->clearBorders();

        finished = false;
        //Ring terminating while loop
        while (!finished) {
            while (!que.empty()) {
                //Takes next node with no contributing neighbors
                temp = que.front();
                que.pop();
                i = temp.x;
                j = temp.y;
                //  FLOW ALGEBRA EXPRESSION EVALUATION
                //  If on stream
                if (!src->isNodata(i, j) && src->getData(i, j, tempLong) >= thresh) {
                    fdarr->setData(i, j, (float)0.0);
                } else if (fel->isNodata(i, j)) { fdarr->setToNodata(i, j);
                } else {
                    p->getData(i, j, k);  //  Get neighbor downstream
                    fel->getData(i, j, elev);
                    in = i + d1[k];
                    jn = j + d2[k];
                    if (fdarr->isNodata(in, jn)) { fdarr->setToNodata(i, j); }
                    else {
                        fel->getData(in, jn, elevn);
                        if (fel->isNodata(in, jn)) {
                            elevn = -9999.f;
                            // If downstream is on stream, use the average elevation of its surrounded stream cells
                            if (!src->isNodata(in, jn) && src->getData(in, jn, tempLong) >= thresh) {
                                for (int ii = 1; ii <= 8; ii++) {
                                    int ix = in + d1[ii];
                                    int iy = jn + d2[ii];
                                    if (!src->isNodata(ix, iy) && src->getData(ix, iy, tempLong) >= thresh && !fel->isNodata(ix, iy)) {
                                        fel->getData(ix, iy, tempFloat);
                                        if (elevn > tempFloat) {
                                            elevn = tempFloat;
                                            fdarr->setData(i, j, (float)(elev - elevn + fdarr->getData(in, jn, tempFloat)));
                                        }
                                        //printf("ix:%d,iy:%d, elev sum:%f\n",ix,iy, elevn);
                                    }
                                }
                                if (elevn < 0.f) { fdarr->setToNodata(i, j); }
                            } else { fdarr->setToNodata(i, j); }
                        } else {
                            fdarr->setData(i, j, (float) (elev - elevn + fdarr->getData(in, jn, tempFloat)));
                        }
                    }
                }
                //  Now find upslope cells and reduce dependencies
                for (k = 1; k <= 8; k++) {
                    in = i + d1[k];
                    jn = j + d2[k];
                    //test if neighbor drains towards cell excluding boundaries
                    if (!p->isNodata(in, jn)) {
                        p->getData(in, jn, tempShort);
                        if (tempShort - k == 4 || tempShort - k == -4) {
                            //Decrement the number of contributing neighbors in neighbor
                            neighbor->addToData(in, jn, (short) -1);
                            //Check if neighbor needs to be added to que
                            if (p->isInPartition(in, jn) && neighbor->getData(in, jn, tempShort) == 0) {
                                temp.x = in;
                                temp.y = jn;
                                que.push(temp);
                            }
                        }
                    }
                }
            }
            //  Here the queue is empty
            //Pass information
            fdarr->share();
            neighbor->addBorders();

            //If this created a cell with no contributing neighbors, put it on the queue
            for (i = 0; i < nx; i++) {
                if (neighbor->getData(i, -1, tempShort) != 0 && neighbor->getData(i, 0, tempShort) == 0) {
                    temp.x = i;
                    temp.y = 0;
                    que.push(temp);
                }
                if (neighbor->getData(i, ny, tempShort) != 0 && neighbor->getData(i, ny - 1, tempShort) == 0) {
                    temp.x = i;
                    temp.y = ny - 1;
                    que.push(temp);
                }
            }
            //Clear out borders
            neighbor->clearBorders();

            //Check if done
            finished = que.empty();
            finished = fdarr->ringTerm(finished);
        }
        //Stop timer
        double computet = MPI_Wtime();

        //Create and write TIFF file
        float aNodata = MISSINGFLOAT;
        tiffIO a(distfile, FLOAT_TYPE, aNodata, pf);
        a.write(xstart, ystart, ny, nx, fdarr->getGridPointer());
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

        if (rank == 0) {
            printf("Processors: %d\nRead time: %f\nCompute time: %f\nWrite time: %f\nTotal time: %f\n",
                   size, dataRead, compute, write, total);
        }
        //Brackets force MPI-dependent objects to go out of scope before Finalize is called
    }
    MPI_Finalize();
    return (0);
}

int pdisttostrmgrd(char *pfile, char *felfile, char *srcfile, char *distfile, int thresh) {
    MPI_Init(NULL, NULL);
    {  //  All code within braces so that objects go out of context and destruct before MPI is closed
        int rank, size;
        MPI_Comm_rank(MCW, &rank);
        MPI_Comm_size(MCW, &size);
        if (rank == 0)printf("D8DistDownToStream version %s, added by Liangjun Zhu, Apr 2, 2015\n", TDVERSION);
        int i, j, in, jn;
        float tempFloat;
        short tempShort, k;
        int32_t tempLong;
        bool finished;
        float elev, elevn, fdhv, fdvv;

        //  Begin timer
        double begint = MPI_Wtime();

        //Read Flow Direction header using tiffIO
        tiffIO pf(pfile, SHORT_TYPE);
        long totalX = pf.getTotalX();
        long totalY = pf.getTotalY();
        double dx = pf.getdxA();
        double dy = pf.getdyA();
        //if(rank==0)
        //{
        //	float timeestimate=(1.2e-6*totalX*totalY/pow((double) size,0.65))/60+1;  // Time estimate in minutes
        //	fprintf(stderr,"This run may take on the order of %.0f minutes to complete.\n",timeestimate);
        //	fprintf(stderr,"This estimate is very approximate. \nRun time is highly uncertain as it depends on the complexity of the input data \nand speed and memory of the computer. This estimate is based on our testing on \na dual quad core Dell Xeon E5405 2.0GHz PC with 16GB RAM.\n");
        //	fflush(stderr);
        //}

        //Read flow direction data into partition
        tdpartition *p;
        p = CreateNewPartition(pf.getDatatype(), totalX, totalY, dx, dy, pf.getNodata());
        int nx = p->getnx();
        int ny = p->getny();
        int xstart, ystart;
        p->localToGlobal(0, 0, xstart, ystart);
        pf.read(xstart, ystart, ny, nx, p->getGridPointer());

        //Read filled dem data into partition
        tdpartition *fel;
        tiffIO felf(felfile, FLOAT_TYPE);
        if (!pf.compareTiff(felf)) {
            printf("File sizes do not match\n%s\n", felfile);
            MPI_Abort(MCW, 5);
            return 1;
        }
        fel = CreateNewPartition(felf.getDatatype(), totalX, totalY, dx, dy, felf.getNodata());
        felf.read(xstart, ystart, ny, nx, fel->getGridPointer());
        //Read stream source file
        tdpartition *src;
        tiffIO srcf(srcfile, LONG_TYPE);
        if (!pf.compareTiff(srcf)) {
            printf("File sizes do not match\n%s\n", srcfile);
            MPI_Abort(MCW, 5);
            return 1; //And maybe an unhappy error message
        }
        src = CreateNewPartition(srcf.getDatatype(), totalX, totalY, dx, dy, srcf.getNodata());
        srcf.read(xstart, ystart, ny, nx, src->getGridPointer());

        //Record time reading files
        double readt = MPI_Wtime();

        //Create empty partition to store horizontal and vertical distance information
        tdpartition *fdh;
        tdpartition *fdv;
        fdh = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
        fdv = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
        //Create empty partition to store distance information
        //tdpartition *fdarr;
        //fdarr = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);

        /*  Calculate Distances  */
        float dist[9];
        for (i = 1; i <= 8; i++) {
            dist[i] = sqrt(d1[i] * d1[i] * dx * dx + d2[i] * d2[i] * dy * dy);
        }

        //  Set neighbor partition to 1 because all grid cells drain to one other grid cell in D8
        //  Set stream source grid cell's value to 0, and put stream cell to que
        tdpartition *neighbor;
        neighbor = CreateNewPartition(SHORT_TYPE, totalX, totalY, dx, dy, MISSINGSHORT);

        node temp;
        queue <node> que;
        for (j = 0; j < ny; j++) {     // rows
            for (i = 0; i < nx; i++) { // cols
                if (!p->isNodata(i, j)) {
                    //Set contributing neighbors to 1
                    neighbor->setData(i, j, (short) 1);
                }
                if (!src->isNodata(i, j) && src->getData(i, j, tempLong) >= thresh) {
                    neighbor->setData(i, j, (short) 0);
                    temp.x = i;
                    temp.y = j;
                    que.push(temp);
                }
            }
        }

        //Share information and set borders to zero
        p->share();
        src->share();
        fel->share();
        //fdarr->share();
        fdh->share();
        fdv->share();
        neighbor->clearBorders();

        finished = false;
        //Ring terminating while loop
        while (!finished) {
            while (!que.empty()) {
                //Takes next node with no contributing neighbors
                temp = que.front();
                que.pop();
                i = temp.x;
                j = temp.y;
                //  FLOW ALGEBRA EXPRESSION EVALUATION
                //  If on stream
                if (!src->isNodata(i, j) && src->getData(i, j, tempLong) >= thresh) {
                    fdh->setData(i, j, (float) 0.0);
                    fdv->setData(i, j, (float) 0.0);
                } else if (fel->isNodata(i, j)) {
                    fdh->setToNodata(i, j);
                    fdv->setToNodata(i, j);
                } else {
                    p->getData(i, j, k); //  Get neighbor downstream
                    fel->getData(i, j, elev);
                    in = i + d1[k];
                    jn = j + d2[k];
                    fdv->getData(in, jn, fdvv);
                    fdh->getData(in, jn, fdhv);
                    if (fdh->isNodata(in, jn)) {
                        fdh->setToNodata(i, j);
                        fdv->setToNodata(i, j);
                    } else {
                        fel->getData(in, jn, elevn);
                        fdv->getData(in, jn, fdvv);
                        fdh->getData(in, jn, fdhv);
                        if (fel->isNodata(in, jn)) {
                            elevn = -9999.f;
                            // If downstream is on stream, use the average elevation of its surrounded stream cells
                            if (!src->isNodata(in, jn) && src->getData(in, jn, tempLong) >= thresh) {
                                for (int ii = 1; ii <= 8; ii++) {
                                    int ix = in + d1[ii];
                                    int iy = jn + d2[ii];
                                    if (!src->isNodata(ix, iy) && src->getData(ix, iy, tempLong) >= thresh && !fel->isNodata(ix, iy)) {
                                        fel->getData(ix, iy, tempFloat);
                                        if (elevn > tempFloat) {
                                            elevn = tempFloat;
                                            fdvv += elev - elevn;
                                            fdhv += dist[k];
                                            fdv->setData(i, j, fdvv);
                                            fdh->setData(i, j, fdhv);
                                        }
                                        //printf("ix:%d,iy:%d, elev sum:%f\n", ix, iy, elevn);
                                    }
                                }
                                if (elevn < 0.f) {
                                    fdh->setToNodata(i, j);
                                    fdv->setToNodata(i, j);
                                }
                            }
                            else {
                                fdh->setToNodata(i, j);
                                fdv->setToNodata(i, j);
                            }
                        }
                        else {
                            fdvv += elev - elevn;
                            fdhv += dist[k];
                            fdv->setData(i, j, fdvv);
                            fdh->setData(i, j, fdhv);
                        }
                    }
                }
                //  Now find upslope cells and reduce dependencies
                for (k = 1; k <= 8; k++) {
                    in = i + d1[k];
                    jn = j + d2[k];
                    //test if neighbor drains towards cell excluding boundaries
                    if (!p->isNodata(in, jn)) {
                        p->getData(in, jn, tempShort);
                        if (tempShort - k == 4 || tempShort - k == -4) {
                            //Decrement the number of contributing neighbors in neighbor
                            neighbor->addToData(in, jn, (short) -1);
                            //Check if neighbor needs to be added to que
                            if (p->isInPartition(in, jn) && neighbor->getData(in, jn, tempShort) == 0) {
                                temp.x = in;
                                temp.y = jn;
                                que.push(temp);
                            }
                        }
                    }
                }
            }
            //  Here the queue is empty
            //Pass information
            fdh->share();
            fdv->share();
            //fdarr->share();
            neighbor->addBorders();

            //If this created a cell with no contributing neighbors, put it on the queue
            for (i = 0; i < nx; i++) {
                if (neighbor->getData(i, -1, tempShort) != 0 && neighbor->getData(i, 0, tempShort) == 0) {
                    temp.x = i;
                    temp.y = 0;
                    que.push(temp);
                }
                if (neighbor->getData(i, ny, tempShort) != 0 && neighbor->getData(i, ny - 1, tempShort) == 0) {
                    temp.x = i;
                    temp.y = ny - 1;
                    que.push(temp);
                }
            }
            //Clear out borders
            neighbor->clearBorders();

            //Check if done
            finished = que.empty();
            finished = fdh->ringTerm(finished);
        }
        //Now, compute the pythagorus difference
        for (j = 0; j < ny; j++) {
            for (i = 0; i < nx; i++) {
                if (fdv->isNodata(i, j)) { fdh->setToNodata(i, j); }
                else if (!fdh->isNodata(i, j)) {
                    fdh->getData(i, j, fdhv);
                    fdv->getData(i, j, fdvv);
                    //fdhv=sqrt(fdhv*fdhv+fdvv*fdvv);
                    //float tempff;
                    fdh->setData(i, j, (float) sqrt(fdhv * fdhv + fdvv * fdvv));
                    //printf("%f,%f:%f\n",fdhv,fdvv,fdh->getData(i,j,tempff));
                }
            }
        }
        //Stop timer
        double computet = MPI_Wtime();

        //Create and write TIFF file
        float aNodata = MISSINGFLOAT;
        tiffIO a(distfile, FLOAT_TYPE, aNodata, pf);
        a.write(xstart, ystart, ny, nx, fdh->getGridPointer());
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

        if (rank == 0) {
            printf("Processors: %d\nRead time: %f\nCompute time: %f\nWrite time: %f\nTotal time: %f\n",
                   size, dataRead, compute, write, total);
        }

        //Brackets force MPI-dependent objects to go out of scope before Finalize is called
    }
    MPI_Finalize();
    return (0);
}
int sdisttostrmgrd(char *pfile, char *felfile, char *srcfile, char *distfile, int thresh) {
    MPI_Init(NULL, NULL);
    {  //  All code within braces so that objects go out of context and destruct before MPI is closed
        int rank, size;
        MPI_Comm_rank(MCW, &rank);
        MPI_Comm_size(MCW, &size);
        if (rank == 0)printf("D8DistDownToStream version %s, added by Liangjun Zhu, Apr 2, 2015\n", TDVERSION);
        int i, j, in, jn;
        float tempFloat;
        short tempShort, k;
        int32_t tempLong;
        bool finished;
        float elev, elevn, fdsv;

        //  Begin timer
        double begint = MPI_Wtime();

        //Read Flow Direction header using tiffIO
        tiffIO pf(pfile, SHORT_TYPE);
        long totalX = pf.getTotalX();
        long totalY = pf.getTotalY();
        double dx = pf.getdxA();
        double dy = pf.getdyA();
        //if(rank==0)
        //{
        //	float timeestimate=(1.2e-6*totalX*totalY/pow((double) size,0.65))/60+1;  // Time estimate in minutes
        //	fprintf(stderr,"This run may take on the order of %.0f minutes to complete.\n",timeestimate);
        //	fprintf(stderr,"This estimate is very approximate. \nRun time is highly uncertain as it depends on the complexity of the input data \nand speed and memory of the computer. This estimate is based on our testing on \na dual quad core Dell Xeon E5405 2.0GHz PC with 16GB RAM.\n");
        //	fflush(stderr);
        //}

        //Read flow direction data into partition
        tdpartition *p;
        p = CreateNewPartition(pf.getDatatype(), totalX, totalY, dx, dy, pf.getNodata());
        int nx = p->getnx();
        int ny = p->getny();
        int xstart, ystart;
        p->localToGlobal(0, 0, xstart, ystart);
        pf.read(xstart, ystart, ny, nx, p->getGridPointer());

        //Read filled dem data into partition
        tdpartition *fel;
        tiffIO felf(felfile, FLOAT_TYPE);
        if (!pf.compareTiff(felf)) {
            printf("File sizes do not match\n%s\n", felfile);
            MPI_Abort(MCW, 5);
            return 1;
        }
        fel = CreateNewPartition(felf.getDatatype(), totalX, totalY, dx, dy, felf.getNodata());
        felf.read(xstart, ystart, ny, nx, fel->getGridPointer());
        //Read stream source file
        tdpartition *src;
        tiffIO srcf(srcfile, LONG_TYPE);
        if (!pf.compareTiff(srcf)) {
            printf("File sizes do not match\n%s\n", srcfile);
            MPI_Abort(MCW, 5);
            return 1;  //And maybe an unhappy error message
        }
        src = CreateNewPartition(srcf.getDatatype(), totalX, totalY, dx, dy, srcf.getNodata());
        srcf.read(xstart, ystart, ny, nx, src->getGridPointer());

        //Record time reading files
        double readt = MPI_Wtime();

        //Create empty partition to store surface distance information
        tdpartition *fdarr;
        fdarr = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);

        /*  Calculate Distances  */
        float dist[9];
        for (i = 1; i <= 8; i++) {
            dist[i] = sqrt(d1[i] * d1[i] * dx * dx + d2[i] * d2[i] * dy * dy);
        }

        //  Set neighbor partition to 1 because all grid cells drain to one other grid cell in D8
        //  Set stream source grid cell's value to 0, and put stream cell to que
        tdpartition *neighbor;
        neighbor = CreateNewPartition(SHORT_TYPE, totalX, totalY, dx, dy, MISSINGSHORT);

        node temp;
        queue <node> que;
        for (j = 0; j < ny; j++) { // rows
            for (i = 0; i < nx; i++) { // cols
                if (!p->isNodata(i, j)) {
                    //Set contributing neighbors to 1
                    neighbor->setData(i, j, (short) 1);
                }
                if (!src->isNodata(i, j) && src->getData(i, j, tempLong) >= thresh) {
                    neighbor->setData(i, j, (short) 0);
                    temp.x = i;
                    temp.y = j;
                    que.push(temp);
                }
            }
        }

        //Share information and set borders to zero
        p->share();
        fel->share();
        src->share();
        fdarr->share();
        neighbor->clearBorders();

        finished = false;
        //Ring terminating while loop
        while (!finished) {
            while (!que.empty()) {
                //Takes next node with no contributing neighbors
                temp = que.front();
                que.pop();
                i = temp.x;
                j = temp.y;
                //  FLOW ALGEBRA EXPRESSION EVALUATION
                //  If on stream
                if (!src->isNodata(i, j) && src->getData(i, j, tempLong) >= thresh) {
                    fdarr->setData(i, j, (float)0.0);
                } else if (fel->isNodata(i, j)) {
                    fdarr->setToNodata(i, j);
                } else {
                    p->getData(i, j, k); //  Get neighbor downstream
                    fel->getData(i, j, elev);
                    in = i + d1[k];
                    jn = j + d2[k];
                    if (fdarr->isNodata(in, jn)) { fdarr->setToNodata(i, j); }
                    else {
                        fel->getData(in, jn, elevn);
                        if (fel->isNodata(in, jn)) {
                            elevn = -9999.f;
                            // If downstream is on stream, use the average elevation of its surrounded stream cells
                            if (!src->isNodata(in, jn) && src->getData(in, jn, tempLong) >= thresh) {
                                for (int ii = 1; ii <= 8; ii++) {
                                    int ix = in + d1[ii];
                                    int iy = jn + d2[ii];
                                    if (!src->isNodata(ix, iy) && src->getData(ix, iy, tempLong) >= thresh && !fel->isNodata(ix, iy)) {
                                        fel->getData(ix, iy, tempFloat);
                                        if (elevn > tempFloat) {
                                            elevn = tempFloat;
                                            fdarr->getData(in, jn, fdsv);
                                            fdsv += sqrt((float)(elev - elevn) * (elev - elevn) + dist[k] * dist[k]);
                                            fdarr->setData(i, j, fdsv);
                                        }
                                        //printf("ix:%d,iy:%d, elev sum:%f\n", ix, iy, elevn);
                                    }
                                }
                                if (elevn < 0) { fdarr->setToNodata(i, j); }
                            }
                            else { fdarr->setToNodata(i, j); }
                        }
                        else {
                            fel->getData(in, jn, elevn);
                            fdarr->getData(in, jn, fdsv);
                            fdsv += sqrt((float)(elev - elevn) * (elev - elevn) + dist[k] * dist[k]);
                            fdarr->setData(i, j, fdsv);
                        }
                    }
                }
                //  Now find upslope cells and reduce dependencies
                for (k = 1; k <= 8; k++) {
                    in = i + d1[k];
                    jn = j + d2[k];
                    //test if neighbor drains towards cell excluding boundaries
                    if (!p->isNodata(in, jn)) {
                        p->getData(in, jn, tempShort);
                        if (tempShort - k == 4 || tempShort - k == -4) {
                            //Decrement the number of contributing neighbors in neighbor
                            neighbor->addToData(in, jn, (short) -1);
                            //Check if neighbor needs to be added to que
                            if (p->isInPartition(in, jn) && neighbor->getData(in, jn, tempShort) == 0) {
                                temp.x = in;
                                temp.y = jn;
                                que.push(temp);
                            }
                        }
                    }
                }
            }
            //  Here the queue is empty
            //Pass information
            fdarr->share();
            neighbor->addBorders();

            //If this created a cell with no contributing neighbors, put it on the queue
            for (i = 0; i < nx; i++) {
                if (neighbor->getData(i, -1, tempShort) != 0 && neighbor->getData(i, 0, tempShort) == 0) {
                    temp.x = i;
                    temp.y = 0;
                    que.push(temp);
                }
                if (neighbor->getData(i, ny, tempShort) != 0 && neighbor->getData(i, ny - 1, tempShort) == 0) {
                    temp.x = i;
                    temp.y = ny - 1;
                    que.push(temp);
                }
            }
            //Clear out borders
            neighbor->clearBorders();

            //Check if done
            finished = que.empty();
            finished = fdarr->ringTerm(finished);
        }

        //Stop timer
        double computet = MPI_Wtime();

        //Create and write TIFF file
        float aNodata = MISSINGFLOAT;
        tiffIO a(distfile, FLOAT_TYPE, aNodata, pf);
        a.write(xstart, ystart, ny, nx, fdarr->getGridPointer());
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

        if (rank == 0) {
            printf("Processors: %d\nRead time: %f\nCompute time: %f\nWrite time: %f\nTotal time: %f\n",
                   size, dataRead, compute, write, total);
        }

        //Brackets force MPI-dependent objects to go out of scope before Finalize is called
    }
    MPI_Finalize();
    return (0);
}

