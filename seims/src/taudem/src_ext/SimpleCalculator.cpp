// include fundamental libraries
#include <stdlib.h>
#include <iostream>
//#include <math.h>
// include mpich and openmp
#include <mpi.h>
//#include <omp.h>
// include TauDEM header files
#include "commonLib.h"
//#include "linearpart.h"
#include "createpart.h"
#include "tiffIO.h"
#include "SimpleCalculator.h"

int SimpleCalc(char *Afile, char *Bfile, char *outfile, int op)
{
    MPI_Init(NULL, NULL);
    {
        int rank, size;
        MPI_Comm_rank(MCW, &rank);
        MPI_Comm_size(MCW, &size);
        if (rank == 0)
        {
            printf("SimpleCalculator -h version %s, added by Liangjun Zhu, Apr 14, 2015\n", TDVERSION);
            printf("Input File1 :%s\n", Afile);
            printf("Input File2 :%s\n", Bfile);
            printf("Output File  :%s\n", outfile);
            fflush(stdout);
        }
        // begin timer
        double begint = MPI_Wtime();
        // read tiff header information using tiffIO
        tiffIO inputAf(Afile, FLOAT_TYPE);
        long totalX = inputAf.getTotalX();
        long totalY = inputAf.getTotalY();
        double dx = inputAf.getdxA();
        double dy = inputAf.getdyA();

        // read tiff data into partition
        tdpartition *inputa;
        inputa = CreateNewPartition(inputAf.getDatatype(), totalX, totalY, dx, dy, inputAf.getNodata());
        // get the size of current partition
        int nx = inputa->getnx();
        int ny = inputa->getny();
        int xstart, ystart;
        inputa->localToGlobal(0, 0, xstart, ystart); // calculate current partition's first cell's position
        inputa->savedxdyc(inputAf);
        inputAf.read(xstart, ystart, ny, nx, inputa->getGridPointer()); // get the current partition's pointer

        tiffIO inputBf(Bfile, FLOAT_TYPE);
        if (!inputAf.compareTiff(inputBf))
        {
            printf("File size do not match\n%s\n", Bfile);
            MPI_Abort(MCW, 5);
            return 1;
        }
        tdpartition *inputb;
        inputb = CreateNewPartition(inputBf.getDatatype(), totalX, totalY, dx, dy, inputBf.getNodata());
        inputBf.read(xstart, ystart, ny, nx, inputb->getGridPointer());
        double readt = MPI_Wtime(); // record reading time

        // create empty partition to store new result
        tdpartition *dest;
        dest = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);

        int i, j;
        float a, b;
        // COMPUTING CODE BLOCK
        float ave_b = 0.f, sum_b_loc = 0.f, sum_b = 0.f;
        int countb_loc = 0, countb = 0;
        for (j = 0; j < ny; j++) // rows
        {
            for (i = 0; i < nx; i++) // cols
            {
                if (!inputb->isNodata(i, j))
                {
                    inputb->getData(i, j, b);
                    sum_b_loc += b;
                    countb_loc += 1;
                }
            }
        }

        MPI_Allreduce(&sum_b_loc, &sum_b, 1, MPI_FLOAT, MPI_SUM, MCW);
        MPI_Allreduce(&countb_loc, &countb, 1, MPI_INT, MPI_SUM, MCW);
        ave_b = sum_b / (float) countb;
        //printf("average:%f\n",ave_b);
        for (j = 0; j < ny; j++) // rows
        {
            for (i = 0; i < nx; i++) // cols
            {
                if (!(inputa->isNodata(i, j) || inputb->isNodata(i, j)))
                {
                    inputa->getData(i, j, a);
                    inputb->getData(i, j, b);
                    switch (op)
                    {
                        case 0:
                            dest->setData(i, j, a + b);
                            break;
                        case 1:
                            dest->setData(i, j, a - b);
                            break;
                        case 2:
                            dest->setData(i, j, a * b);
                            break;
                        case 3:
                            if (b != 0.0)
                                dest->setData(i, j, a / b);
                            else
                                dest->setToNodata(i, j);
                            break;
                        case 4:
                            if ((a + b) != 0.0)
                                dest->setData(i, j, a / (a + b));
                            else if (a == 0.0 && b == 0.0)
                            {
                                dest->setData(i, j, (float) 0.0);
                            }
                            else
                                dest->setToNodata(i, j);
                            break;
                        case 5:
                            dest->setData(i, j, a);
                            break;
                        case 6:
                            // calculate the average of input b
                            if (b > ave_b)
                                dest->setData(i, j, a);
                            break;
                        default:
                            break;
                    }
                }
                else
                    dest->setToNodata(i, j);
            }
        }
        // END COMPUTING CODE BLOCK
        double computet = MPI_Wtime(); // record computing time
        // create and write TIFF file
        float nodata = MISSINGFLOAT;
        tiffIO destTIFF(outfile, FLOAT_TYPE, nodata, inputAf);
        destTIFF.write(xstart, ystart, ny, nx, dest->getGridPointer());
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

        if (rank == 0)
            printf("Processors: %d\nRead time: %f\nCompute time: %f\nWrite time: %f\nTotal time: %f\n",
                   size, dataRead, compute, write, total);

		/// free memory
		delete inputa;
		delete inputb;
    }
    MPI_Finalize();
    return 0;
}
