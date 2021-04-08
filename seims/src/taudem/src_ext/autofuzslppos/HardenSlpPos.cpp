// include fundamental libraries
#include <stdlib.h>
#include <iostream>
#include <math.h>
// include mpich and openmp
#include <mpi.h>
// include TauDEM header files
#include "commonLib.h"
#include "createpart.h"
#include "tiffIO.h"
#include "HardenSlpPos.h"

// using namespace std; // Avoid to using the entire namespace of std. Comment by Liangjun, 01/23/19

int HardenSlpPos(vector<string> infiles, vector<int> tags, char *hardfile, char *maxsimifile, bool calsec,
                 char *sechardfile, char *secsimifile, bool calspsi, int spsimodel, char *spsifile)
{
    MPI_Init(NULL, NULL);
    {
        int rank, size;
        MPI_Comm_rank(MCW, &rank);
        MPI_Comm_size(MCW, &size);
        int inf_num = infiles.size();
        if (rank == 0)
        {
            printf("HardenSlpPos -h version %s, added by Liangjun Zhu, Aug 2, 2017\n", TDVERSION);
            for (int i = 0; i < inf_num; i++) {
                printf("Slope position type %d: %s\n", tags[i], infiles[i].c_str());
            }
            //printf("RDG:%s\n", rdgfile);
            //printf("SHD:%s\n", shdfile);
            //printf("BKS:%s\n", bksfile);
            //printf("FTS:%s\n", ftsfile);
            //printf("VLY:%s\n", vlyfile);
            printf("Harden Slope Position:%s\n", hardfile);
            printf("MaxSimilarity:%s\n", maxsimifile);
            if (calsec)
            {
                printf("Second Harden Slope Position:%s\n", sechardfile);
                printf("SecMaxSimilarity:%s\n", secsimifile);
            }
            if (calsec && calspsi)
            {
                printf("Slope Position Sequence Index:%s\n", spsifile);
            }
            fflush(stdout);
        }
        // begin timer
        double begint = MPI_Wtime();
        // read the first one
        tiffIO rdgf(convertStringToCharPtr(infiles[0]), FLOAT_TYPE);
        long totalX = rdgf.getTotalX();
        long totalY = rdgf.getTotalY();
        double dx = rdgf.getdxA();
        double dy = rdgf.getdyA();

        // read ridge similarity data into partition
        tdpartition *rdg;
        rdg = CreateNewPartition(rdgf.getDatatype(), totalX, totalY, dx, dy, rdgf.getNodata());
        // get the size of current partition
        int nx = rdg->getnx();
        int ny = rdg->getny();
        int xstart, ystart;
        rdg->localToGlobal(0, 0, xstart, ystart); // calculate current partition's first cell's position
        rdg->savedxdyc(rdgf);
        rdgf.read(xstart, ystart, ny, nx, rdg->getGridPointer()); // get the current partition's pointer

        // read the other slope position's similarity tiff data into a *linearpart...
        linearpart<float> *slpposSimi = new linearpart<float>[inf_num - 1];
        for (int num = 0; num < inf_num - 1; num++)
        {
            tiffIO paramsf(convertStringToCharPtr(infiles[num + 1]), FLOAT_TYPE);
            if (!rdgf.compareTiff(paramsf))
            {
                printf("File size do not match\n%s\n", infiles[num + 1].c_str());
                MPI_Abort(MCW, 5);
                return 1;
            }
            slpposSimi[num].init(totalX, totalY, dx, dy, MPI_FLOAT, static_cast<float>(paramsf.getNodata()));
            paramsf.read(xstart, ystart, ny, nx, slpposSimi[num].getGridPointer());
        }
        double readt = MPI_Wtime(); // record reading time

        // create empty partition to store new result
        tdpartition *hard, *maxsimi, *sechard, *secsimi, *spsi;
        hard = CreateNewPartition(SHORT_TYPE, totalX, totalY, dx, dy, MISSINGSHORT);
        maxsimi = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
        if (calsec)
        {
            sechard = CreateNewPartition(SHORT_TYPE, totalX, totalY, dx, dy, MISSINGSHORT);
            secsimi = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
        }
        if (calsec && calspsi)
            spsi = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);

        // COMPUTING CODE BLOCK
        int i, j, num;
        short maxSlpPosTag, secSlpPosTag;
        float maxSimilarity, secSimilarity, tempSimilarity;
        for (j = 0; j < ny; j++) // rows
        {
            for (i = 0; i < nx; i++) // cols
            {
                if (rdg->isNodata(i, j))
                {
                    hard->setToNodata(i, j);
                    maxsimi->setToNodata(i, j);
                    if (calsec)
                    {
                        sechard->setToNodata(i, j);
                        secsimi->setToNodata(i, j);
                    }
                }
                else
                {
                    // main calculation block
                    // initial
                    maxSlpPosTag = tags[0];
                    rdg->getData(i, j, maxSimilarity);
                    if (calsec)
                    {
                        secSlpPosTag = tags[0];
                        rdg->getData(i, j, secSimilarity);
                    }
                    // loop the other four slope position's similarity
                    for (num = 0; num < inf_num - 1; num++)
                    {
                        slpposSimi[num].getData(i, j, tempSimilarity);
                        if (tempSimilarity > maxSimilarity)
                        {
                            if (calsec)
                            {
                                secSlpPosTag = maxSlpPosTag;
                                secSimilarity = maxSimilarity;
                            }
                            maxSlpPosTag = tags[num + 1];
                            maxSimilarity = tempSimilarity;
                        }
                        else if ((tempSimilarity < maxSimilarity) && calsec)
                        {
                            if ((tempSimilarity > secSimilarity) ||
                                ((tempSimilarity == secSimilarity) && (tags[num + 1] > secSlpPosTag)))
                            {
                                secSlpPosTag = tags[num + 1];
                                secSimilarity = tempSimilarity;
                            }
                        }
                        else if ((tempSimilarity == maxSimilarity))
                        {
                            if (tags[num + 1] > maxSlpPosTag)
                            {
                                maxSlpPosTag = tags[num + 1];
                                maxSimilarity = tempSimilarity;
                            }
                            else if (((tempSimilarity > secSimilarity) ||
                                (tempSimilarity == secSimilarity && tags[num + 1] > secSlpPosTag)) && calsec)
                            {
                                secSlpPosTag = tags[num + 1];
                                secSimilarity = tempSimilarity;
                            }
                        }
                    }

                    // assign value to output variables
                    hard->setData(i, j, maxSlpPosTag);
                    maxsimi->setData(i, j, maxSimilarity);
                    if (calsec)
                    {
                        sechard->setData(i, j, secSlpPosTag);
                        secsimi->setData(i, j, secSimilarity);
                    }
                }
            }
        }
        if (calsec && calspsi) // calculate SPSI
        {
            float tempSPSI = 0.f, flag = 0.f;
            for (j = 0; j < ny; j++) // rows
            {
                for (i = 0; i < nx; i++) // cols
                {
                    if (!(hard->isNodata(i, j) || sechard->isNodata(i, j)))
                    {
                        hard->getData(i, j, maxSlpPosTag);
                        sechard->getData(i, j, secSlpPosTag);
                        maxsimi->getData(i, j, maxSimilarity);
                        secsimi->getData(i, j, secSimilarity);
                        if (secSlpPosTag > maxSlpPosTag)
                            flag = 1.f;
                        else if (secSlpPosTag < maxSlpPosTag)
                            flag = -1.f;
                        else
                            flag = 0.f;
                        if (spsimodel == 1)
                            tempSPSI = (log((float) maxSlpPosTag) / log(2.f) + 1.f) +
                                       flag * (1.f - maxSimilarity) / 2.f;
                        else if (spsimodel == 2)
                            tempSPSI = (log((float) maxSlpPosTag) / log(2.f) + 1.f) +
                                       flag * (1.f - (maxSimilarity - secSimilarity)) / 2.f;
                        else if (spsimodel == 3) if (maxSimilarity != 0.0)
                            tempSPSI = (log((float) maxSlpPosTag) / log(2.f) + 1.f) +
                                       flag * (secSimilarity / maxSimilarity) / 2.f;
                        else
                            tempSPSI = MISSINGFLOAT;
                        spsi->setData(i, j, tempSPSI);
                    }
                    else
                        spsi->setToNodata(i, j);
                }
            }
        }
        // END COMPUTING CODE BLOCK
        double computet = MPI_Wtime(); // record computing time
        // create and write TIFF file
        float nodata = MISSINGFLOAT;
        int nodataShort = MISSINGSHORT;
        tiffIO hardTIFF(hardfile, SHORT_TYPE, nodataShort, rdgf);
        hardTIFF.write(xstart, ystart, ny, nx, hard->getGridPointer());
        tiffIO maxsimiTIFF(maxsimifile, FLOAT_TYPE, nodata, rdgf);
        maxsimiTIFF.write(xstart, ystart, ny, nx, maxsimi->getGridPointer());
        if (calsec)
        {
            tiffIO sechardTIFF(sechardfile, SHORT_TYPE, nodataShort, rdgf);
            sechardTIFF.write(xstart, ystart, ny, nx, sechard->getGridPointer());
            tiffIO secsimiTIFF(secsimifile, FLOAT_TYPE, nodata, rdgf);
            secsimiTIFF.write(xstart, ystart, ny, nx, secsimi->getGridPointer());
        }
        if (calspsi)
        {
            tiffIO spsiTIFF(spsifile, FLOAT_TYPE, nodata, rdgf);
            spsiTIFF.write(xstart, ystart, ny, nx, spsi->getGridPointer());
        }
        double writet = MPI_Wtime(); // record writing time

        double dataRead, compute, write, total, tempd;
        dataRead = readt - begint;
        compute = computet - readt;
        write = writet - computet;
        total = writet - begint;

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
		delete rdg, rdgf;
		delete[] slpposSimi;
		delete hard, maxsimi, sechard, secsimi, spsi;
    }
    MPI_Finalize();
    return 0;
}
