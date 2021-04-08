/*!
 * \file FuzzySlpPosInference.cpp
 * \date 2015/04/09 14:12
 *
 * \author Liangjun Zhu
 * Contact: zlj@lreis.ac.cn
 *
 * \brief Fuzzy slope position inference function.
 *
 */
/// include fundamental libraries
#include <stdlib.h>
#include <iostream>
#include <queue>
//#include <math.h>
//#include <time.h>
//#include <vector>
/// include MPI
#include <mpi.h>
/// include TauDEM header files
#include "commonLib.h"
//#include "linearpart.h"
#include "createpart.h"
#include "tiffIO.h"
/// include FuzzySlpPosInference header
#include "FuzzySlpPosInference.h"

// using namespace std; // Avoid to using the entire namespace of std. Comment by Liangjun, 01/23/19

/*!
 * \brief Calculate fuzzy slope position.
 *
 *
 * \param[in] prototype file
 * \param[in] terrain attribute parameters files used to infer
 * \param[out] similarity GeoTIFF file
 */
int FuzzySlpPosInf(char *protofile, int prototag, int paramsNum, paramInfGRID *paramsgrd, float exponent, char *simfile)
{
    MPI_Init(NULL, NULL);
    {
        int rank, size;
        MPI_Comm_rank(MCW, &rank);
        MPI_Comm_size(MCW, &size);
        MPI_Status status;
        int num;  //!< define a variable for iterate
        if (rank == 0)
        {
            printf("FuzzySlpPosInference -h version %s, added by Liangjun Zhu, Apr 9, 2015\n", TDVERSION);
            printf("PrototypeGRID: %s\n", protofile);
            printf("ProtoTag: %d\n", prototag);
            printf("ParametersNum: %d\n", paramsNum);
            for (num = 0; num < paramsNum; num++)
            {
                printf("TerrainAttri.No.%d: %s\n", num, paramsgrd[num].path);
                printf("   Similarity Function Type: %s-shaped\n", paramsgrd[num].shape);
                printf("   w1=%f;r1=%f;k1=%f;\n   w2=%f;r2=%f;k2=%f\n", paramsgrd[num].w1, paramsgrd[num].r1,
                       paramsgrd[num].k1, paramsgrd[num].w2, paramsgrd[num].r2, paramsgrd[num].k2);
            }
            printf("DistanceExponent: %.1f\n", exponent);
            printf("Similarity file: %s\n", simfile);
            fflush(stdout);
        }
        double begint = MPI_Wtime();  //!< start time
        //!< read tiff header information using tiffIO
        tiffIO protof(protofile, FLOAT_TYPE);
        long totalX = protof.getTotalX();
        long totalY = protof.getTotalY();
        double dx = protof.getdxA();
        double dy = protof.getdyA();

        //!< read tiff data into partition
        tdpartition *proto;
        proto = CreateNewPartition(protof.getDatatype(), totalX, totalY, dx, dy, protof.getNodata());
        //!< get the size of current partition
        int nx = proto->getnx();
        int ny = proto->getny();
        int xstart, ystart;
        proto->localToGlobal(0, 0, xstart, ystart); //!< calculate current partition's first cell's position
        proto->savedxdyc(protof);
        protof.read(xstart, ystart, ny, nx, proto->getGridPointer()); //!< get the current partition's pointer

        //!< read parameters data into *partition
        linearpart<float> *params = new linearpart<float>[paramsNum];
        for (num = 0; num < paramsNum; num++)
        {
            tiffIO paramsf(paramsgrd[num].path, FLOAT_TYPE);
            if (!protof.compareTiff(paramsf))
            {
                printf("File size do not match\n%s\n", paramsgrd[num].path);
                MPI_Abort(MCW, 5);
                return 1;
            }
            params[num].init(totalX, totalY, dx, dy, MPI_FLOAT, static_cast<float>(paramsf.getNodata()));
            paramsf.read(xstart, ystart, ny, nx, params[num].getGridPointer());
        }
        double readt = MPI_Wtime(); //!< record reading time

        int i, j, k, iall, jall;
        float tempTag, tempAttr;
        queue<TypLocAttr> TypLocAttrQue;

        //!< COMPUTING CODE BLOCK FOR EXTRACTING TYPICAL LOCATION AND ATTRIBUTES

        for (j = 0; j < ny; j++) //!< rows
        {
            for (i = 0; i < nx; i++) //!< cols
            {
                if (!proto->isNodata(i, j) && proto->getData(i, j, tempTag) == prototag)
                {
                    TypLocAttr tempTypLocAttr;
                    tempTypLocAttr.Value = new float[paramsNum]; //!< allocate memory space for typical location's parameter attributes
                    bool hasnodata = false;
                    for (num = 0; num < paramsNum; num++)
                        if (!params[num].isNodata(i, j) && !hasnodata)
                        {
                            tempTypLocAttr.Value[num] = params[num].getData(i, j, tempAttr);
                            //printf("paramNo:%d,%s\n",paramNo,tempTypLocAttr.Value[paramNo]);
                        }
                        else
                            hasnodata = true;
                    if (!hasnodata)
                    {
                        proto->localToGlobal(i, j, iall, jall);
                        tempTypLocAttr.col = iall;
                        tempTypLocAttr.row = jall;
                        TypLocAttrQue.push(tempTypLocAttr);
                    }
                    //delete[] tempTypLocAttr.Value;
                    //tempTypLocAttr.Value = NULL;
                }
            }
        }

        int LocTypLocNum = 0;
        float *LocTypLocAttr = new float[TypLocAttrQue.size() * (2 + paramsNum)];
        int locNum = 0;
        while (!TypLocAttrQue.empty())
        {
            TypLocAttr temp;
            temp = TypLocAttrQue.front();
            TypLocAttrQue.pop();
            LocTypLocAttr[LocTypLocNum * (2 + paramsNum)] = (float) temp.col;
            LocTypLocAttr[LocTypLocNum * (2 + paramsNum) + 1] = (float) temp.row;
            for (locNum = 0; locNum < paramsNum; locNum++)
                LocTypLocAttr[LocTypLocNum * (2 + paramsNum) + 2 + locNum] = temp.Value[locNum];
            LocTypLocNum++;
        }
        //!< END COMPUTING CODE BLOCK FOR EXTRACTING TYPICAL LOCATION AND ATTRIBUTES

        int AllTypLocNum;
        int *LocTypLocNums;
        LocTypLocNums = new int[size];
        MPI_Allreduce(&LocTypLocNum, &AllTypLocNum, 1, MPI_INT, MPI_SUM, MCW);
        MPI_Allgather(&LocTypLocNum, 1, MPI_INT, LocTypLocNums, 1, MPI_INT, MCW);
        float *AllTypLocAttr = new float[AllTypLocNum * (2 + paramsNum)];
        int *displs;
        displs = new int[size];
        displs[0] = 0;
        for (i = 1; i < size; i++)
        {
            displs[i] = (displs[i - 1] + LocTypLocNums[i - 1] * (2 + paramsNum));
            LocTypLocNums[i - 1] *= (2 + paramsNum);
        }
        LocTypLocNums[size - 1] *= (2 + paramsNum);
        MPI_Allgatherv(LocTypLocAttr, LocTypLocNum * (2 + paramsNum), MPI_FLOAT, AllTypLocAttr, LocTypLocNums, displs,
                       MPI_FLOAT, MCW);

		Release1DArray(LocTypLocAttr);
		Release1DArray(LocTypLocNums);
		Release1DArray(displs);
        //////////////////////////////////////////////////////////////////////////
        // Next, calculate every cell's similarity to all typical locations
        // create empty partition to store similarity result
        // define intermediate variables
        //////////////////////////////////////////////////////////////////////////
        tdpartition *simi;
        simi = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
        k = 0; //!< k-th prototype location

        //!< calculate
        for (j = 0; j < ny; j++) //!< rows
        {
            for (i = 0; i < nx; i++) //!< cols
            {
                //!< if params[0-paramsNum] are not nodata, then next
                bool Calculable = true;
                for (num = 0; num < paramsNum; num++)
                    if (params[num].isNodata(i, j)) Calculable = false;
                if (Calculable)
                {
                    float dSijtv, dSij = 0.f, dij = 0.f, dDist_ijt, tempSijt; //!< temp variables
                    for (k = 0; k < AllTypLocNum; k++)  //!< Loop every prototype point, t
                    {
                        int i2all, j2all;
                        float tempValue;
                        int tempX = (int) AllTypLocAttr[k * (2 + paramsNum)];
                        int tempY = (int) AllTypLocAttr[k * (2 + paramsNum) + 1];
                        tempSijt = -(float) MISSINGSHORT;
                        dSijtv = 0.f;
                        for (num = 0; num < paramsNum; num++) //!< Loop terrain parameters, v
                        {
                            if (!params[num].isNodata(i, j))
                            {
                                tempValue = AllTypLocAttr[k * (2 + paramsNum) + 2 + num];
                                params[num].getData(i, j, tempAttr);
                                if (tempAttr == tempValue)
                                    dSijtv = 1.f;
                                else if (tempAttr < tempValue) if (paramsgrd[num].k1 == 1.0) //!< Z-shaped function
                                    dSijtv = 1.f;
                                else
                                    dSijtv = exp(
                                            pow((abs(tempAttr - tempValue) / paramsgrd[num].w1), paramsgrd[num].r1) *
                                            log(paramsgrd[num].k1));
                                else   //!< tempAttr > tempValue
                                if (paramsgrd[num].k2 == 1.0)  //!< S-shaped function
                                    dSijtv = 1.f;
                                else
                                    dSijtv = exp(
                                            pow((abs(tempAttr - tempValue) / paramsgrd[num].w2), paramsgrd[num].r2) *
                                            log(paramsgrd[num].k2));
                                if (tempSijt >= dSijtv) tempSijt = dSijtv;
                            }
                        }
                        //////////////////////////////////////////////////////////////////////////
                        // tempSijt is the minimum of similarity between cell P(i,j) and prototype cell T(tempX,tempY)
                        // now calculate inverse distance weight
                        //////////////////////////////////////////////////////////////////////////
                        proto->localToGlobal(i, j, i2all, j2all);
                        if (!((i2all == tempX) && (j2all == tempY)))  //!< not include itself
                        {
                            dDist_ijt = pow(sqrt((float) (i2all - tempX) * (i2all - tempX) +
                                                 (float) (j2all - tempY) * (j2all - tempY)) * (float) dx, -exponent);
                            dSij += dDist_ijt * tempSijt;
                            dij += dDist_ijt;
                        }
                    }
                    //!< now calculate overall similarity of P(i,j) to Slope position C.
                    float Sij = dSij / dij;
                    if (Sij < MINEPS)
                        simi->setData(i, j, 0.f);
                    else
                        simi->setData(i, j, Sij);
                }
                else
                    simi->setToNodata(i, j);
            }
        }
        for (k = 0; k < AllTypLocNum; k++)
        {
            int tempX = (int) AllTypLocAttr[k * (2 + paramsNum)];
            int tempY = (int) AllTypLocAttr[k * (2 + paramsNum) + 1];
            int locali, localj;
            proto->globalToLocal(tempX, tempY, locali, localj);
            simi->setData(locali, localj, 1.f);
        }
        double computet = MPI_Wtime(); //!< record computing time
        //!< create and write tiff
        float nodata = MISSINGFLOAT;
        tiffIO simif(simfile, FLOAT_TYPE, nodata, protof);
        simif.write(xstart, ystart, ny, nx, simi->getGridPointer());
        double writet = MPI_Wtime(); //!< record writing time


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
        {
            printf("Total %d prototype positions were applied into computing similarity.\nProcesses:%d\n    Read time:%f\n    Compute time:%f\n    Write time:%f\n    Total time:%f\n",
                   AllTypLocNum, size, dataRead, compute, write, total);
            fflush(stdout);
        }
		/// free memory
		delete proto, protof;
		delete simi, simif;
		delete[] params;
    }
    MPI_Finalize();
    return 0;
}
