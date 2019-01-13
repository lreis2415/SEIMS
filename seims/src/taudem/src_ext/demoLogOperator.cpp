// include TauDEM header files
#include "commonLib.h"
#include "createpart.h"
// include algorithm header file
#include "demoLogOperator.h"

using namespace std;

int logOperator(char *srcfile, char *destfile) {
    // Initialize MPI
    MPI_Init(NULL, NULL);
    {
        int rank, size;
        MPI_Comm_rank(MCW, &rank);
        MPI_Comm_size(MCW, &size);
        // begin timer
        double begint = MPI_Wtime();
        // read tiff header information using tiffIO
        tiffIO srcf(srcfile, FLOAT_TYPE);
        long totalX = srcf.getTotalX();
        long totalY = srcf.getTotalY();
        double dx = srcf.getdxA();
        double dy = srcf.getdyA();

        // read tiff data into partition
        tdpartition* src;
        src = CreateNewPartition(srcf.getDatatype(), totalX, totalY, dx, dy, srcf.getNodata());
        // get the size of current partition, and get the current partition's pointer
        int nx = src->getnx();
        int ny = src->getny();
        int xstart, ystart;
        src->localToGlobal(0, 0, xstart, ystart);                 // calculate current partition's first cell's position
        srcf.read(xstart, ystart, ny, nx, src->getGridPointer()); // get the current partition's pointer

        double readt = MPI_Wtime(); // record reading time

        // create empty partition to store new result
        tdpartition* dest;
        dest = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
        //share information
        src->share();
        dest->share();
        int i, j;
        float tempV;
        // COMPUTING CODE BLOCK
        for (j = 0; j < ny; j++) // rows
        {
            for (i = 0; i < nx; i++) // cols
            {
                if (src->isNodata(i, j)) {
                    continue;
                }
                src->getData(i, j, tempV);
                dest->setData(i, j, log(tempV));
            }
        }
        // END COMPUTING CODE BLOCK
        double computet = MPI_Wtime(); // record computing time
        // create and write TIFF file
        float nodata = MISSINGFLOAT;
        tiffIO destTIFF(destfile, FLOAT_TYPE, nodata, srcf);
        destTIFF.write(xstart, ystart, ny, nx, dest->getGridPointer());
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
