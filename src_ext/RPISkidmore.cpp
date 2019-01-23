// include fundamental libraries
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <queue>
#include <vector>
#include <algorithm>
#include <numeric>
#include <time.h>
#include <vector>
// include mpich
#include <mpi.h>
// include TauDEM header files
#include "commonLib.h"
#include "linearpart.h"
#include "createpart.h"
#include "tiffIO.h"
#include "RPISkidmore.h"
// using namespace std; // Avoid to using the entire namespace of std. Comment by Liangjun, 01/23/19

int RPISkidmore(char *vlysrcfile,char *rdgsrcfile,int vlytag, int rdgtag, char *rpifile,char *dist2vlyfile,char *dist2rdgfile,bool dist2vlyExport,bool dist2rdgExport)
{
	MPI_Init(NULL,NULL);
	{
		int rank,size;
		MPI_Comm_rank(MCW,&rank);
		MPI_Comm_size(MCW,&size);
		//MPI_Status status;
		if(rank == 0)
		{
			printf("RPISkidmore version %s, added by Liangjun Zhu, Jun.17, 2015\n",TDVERSION);
			printf("Valley Source File: %s\n",vlysrcfile);
			printf("Ridge Source File: %s\n",rdgsrcfile);
			printf("RPI File: %s\n",rpifile);
			if(dist2vlyExport)
				printf("Distance to Valley File: %s\n",dist2vlyfile);
			if(dist2rdgExport)
				printf("Distance to Ridge File: %s\n",dist2rdgfile);
		}
		double begint = MPI_Wtime();  // start time

		// read vlysrc tiff header information using tiffIO
		tiffIO vlysrc(vlysrcfile,FLOAT_TYPE);
		long totalX = vlysrc.getTotalX();
		long totalY = vlysrc.getTotalY();
		double dx = vlysrc.getdxA();
		double dy = vlysrc.getdyA();

		// read tiff data into partition
		tdpartition *vly;
		vly = CreateNewPartition(vlysrc.getDatatype(),totalX,totalY,dx,dy,vlysrc.getNodata());
		// get the size of current partition
		int nx = vly->getnx();
		int ny = vly->getny();
		int xstart,ystart;
		vly->localToGlobal(0,0,xstart,ystart); // calculate current partition's first cell's position
		vly->savedxdyc(vlysrc);
		vlysrc.read(xstart,ystart,ny,nx,vly->getGridPointer()); // get the current partition's pointer

		// read parameters data into *partition
		tiffIO rdgsrc(rdgsrcfile,FLOAT_TYPE);
		if(!vlysrc.compareTiff(rdgsrc))
		{
			printf("File size do not match\n%s\n", rdgsrcfile);
			MPI_Abort(MCW,5);
			return 1;
		}
		tdpartition *rdg;
		rdg = CreateNewPartition(rdgsrc.getDatatype(),totalX,totalY,dx,dy,rdgsrc.getNodata());
		rdgsrc.read(xstart,ystart,ny,nx,rdg->getGridPointer());

		double readt = MPI_Wtime(); // record reading time

		unsigned int i,j;
		int iAll,jAll;
		float tempValue;
		vector<SourcePt> localVlyPoints,localRdgPoints;
		SourcePt tempPt;
		for (j = 0; j < ny; j++) // rows
		{
			for (i = 0; i < nx; i++) // cols
			{
				//if (!vly->isNodata(i,j))
				//{
					if(int(vly->getData(i,j,tempValue))==vlytag)
					{
						vly->localToGlobal(i,j,iAll,jAll);
						tempPt.col = iAll;
						tempPt.row = jAll;
						localVlyPoints.push_back(tempPt);
					}
					else if(int(rdg->getData(i,j,tempValue))==rdgtag)
					{
						rdg->localToGlobal(i,j,iAll,jAll);
						tempPt.col = iAll;
						tempPt.row = jAll;
						localRdgPoints.push_back(tempPt);
					}
				//}
			}
		}
		int vlyNumLocal = localVlyPoints.size();
		int rdgNumLocal = localRdgPoints.size();
		int *vlyPointsLocal = new int[2 * vlyNumLocal];
		int *rdgPointsLocal = new int[2 * rdgNumLocal];
		for(i = 0; i < vlyNumLocal; i++)
		{
			vlyPointsLocal[i * 2] = localVlyPoints[i].col;
			vlyPointsLocal[i * 2 + 1] = localVlyPoints[i].row;
		}
		for(i = 0; i < rdgNumLocal; i++)
		{
			rdgPointsLocal[i * 2] = localRdgPoints[i].col;
			rdgPointsLocal[i * 2 + 1] = localRdgPoints[i].row;
		}
		int vlyNum, rdgNum;
		MPI_Allreduce(&vlyNumLocal,&vlyNum,1,MPI_INT,MPI_SUM,MCW);
		MPI_Allreduce(&rdgNumLocal,&rdgNum,1,MPI_INT,MPI_SUM,MCW);
        if (vlyNum == 0 || rdgNum == 0) {
            printf("Vly Num: %d\nRdg Num: %d\n", vlyNum, rdgNum);
            cout << "Valley or ridge number MUST greater than zero!" << endl;
            MPI_Abort(MCW, -1);
        }
		int *localvlyNumArray = new int[size];
		int *localrdgNumArray = new int[size];
		MPI_Allgather(&vlyNumLocal,1,MPI_INT,localvlyNumArray,1,MPI_INT,MCW);
		MPI_Allgather(&rdgNumLocal,1,MPI_INT,localrdgNumArray,1,MPI_INT,MCW);
		int *displsVly = new int[size];
		int *displsRdg = new int[size];
		displsVly[0] = 0;
		displsRdg[0] = 0;
		for(i = 0; i < size; i++)
		{
			localvlyNumArray[i] *= 2;
			localrdgNumArray[i] *= 2;
		}
		for(i = 1; i < size; i++)
		{
			displsVly[i] = displsVly[i-1] + localvlyNumArray[i-1];
			displsRdg[i] = displsRdg[i-1] + localrdgNumArray[i-1];
		}
		int *vlyPointsAll = new int[vlyNum * 2];
		int *rdgPointsAll = new int[rdgNum * 2];
		MPI_Allgatherv(vlyPointsLocal,vlyNumLocal*2,MPI_INT,vlyPointsAll,localvlyNumArray,displsVly,MPI_INT,MCW);
		MPI_Allgatherv(rdgPointsLocal,rdgNumLocal*2,MPI_INT,rdgPointsAll,localrdgNumArray,displsRdg,MPI_INT,MCW);

		//! Create output partitions
		tdpartition *rpi,*dist2rdg,*dist2vly;
		rpi = CreateNewPartition(FLOAT_TYPE,totalX,totalY,dx,dy,MISSINGFLOAT);
		dist2rdg = CreateNewPartition(FLOAT_TYPE,totalX,totalY,dx,dy,MISSINGFLOAT);
		dist2vly = CreateNewPartition(FLOAT_TYPE,totalX,totalY,dx,dy,MISSINGFLOAT);
		float tempDistVly,tempDistRdg,DistVly=-1*MISSINGFLOAT,DistRdg=-1*MISSINGFLOAT;
		//int k;
		//printf("rdgNum:%d,vlyNum:%d\n",rdgNum, vlyNum);
		//if (rank == 0)
		//{
		//	for (k = 0; k < rdgNum; k++)
		//	{
		//		printf("col:%d,row:%d\n",vlyPointsAll[k*2],vlyPointsAll[k*2+1]);
		//	}
		//}
		//if (rank ==0)
		//{
		//	for (int k = 0; k < rdgNum; k++)
		//		cout<<"col: "<<rdgPointsAll[k * 2]<<"row: "<<rdgPointsAll[k * 2 + 1] <<endl;
		//}
		for (j = 0; j < ny; j++) //! rows
		{
			for (i = 0; i < nx; i++) //! cols
			{
				rpi->localToGlobal(i,j,iAll,jAll);
				//cout<<"col: "<<iAll<<"row: "<<jAll <<endl;
				DistVly=-1*MISSINGFLOAT;
				DistRdg=-1*MISSINGFLOAT;
				if (iAll == 0 || iAll == totalX-1 || jAll == 0 || jAll == totalY-1)
				{
					dist2vly->setToNodata(i,j);
					dist2rdg->setToNodata(i,j);
					rpi->setToNodata(i,j);
				}
				else
				{
					if (coorInList(iAll, jAll, rdgPointsAll, rdgNum))
					{
						DistRdg = 0;
					}
					else{
						for (int k = 0; k < rdgNum; k++)
						{
							tempDistRdg = (rdgPointsAll[k * 2]-iAll)*(rdgPointsAll[k * 2]-iAll)+(rdgPointsAll[k * 2 + 1]-jAll)*(rdgPointsAll[k * 2 + 1]-jAll);
							if(tempDistRdg < DistRdg)
								DistRdg = tempDistRdg;
							if(DistRdg <= 2) break;
						}
					}

					dist2rdg->setData(i,j,float(sqrt(DistRdg)));
					if (coorInList(iAll, jAll, vlyPointsAll, vlyNum))
					{
						DistVly = 0;
					}
					else{
						for (int k = 0; k < vlyNum; k++)
						{
							tempDistVly = (vlyPointsAll[k * 2]-iAll)*(vlyPointsAll[k * 2]-iAll)+(vlyPointsAll[k * 2 + 1]-jAll)*(vlyPointsAll[k * 2 + 1]-jAll);
							if(tempDistVly < DistVly)
								DistVly = tempDistVly;
							if(DistVly <= 2) break;
						}
					}

					dist2vly->setData(i,j,float(sqrt(DistVly)));
					if ((DistVly+DistRdg)==0)
						rpi->setToNodata(i,j);
					else
					{
						DistVly = float(sqrt(DistVly));
						DistRdg = float(sqrt(DistRdg));
						rpi->setData(i,j,DistVly/(DistVly + DistRdg));
					}
				}
			}
		}
		double computet = MPI_Wtime(); //! record computing time

		//! create and write tiff
		float nodata = MISSINGFLOAT;
		tiffIO rpiout(rpifile,FLOAT_TYPE,nodata,vlysrc);
		rpiout.write(xstart,ystart,ny,nx,rpi->getGridPointer());
		if(dist2rdgExport)
		{
			tiffIO dist2rdgout(dist2rdgfile,FLOAT_TYPE,nodata,vlysrc);
			dist2rdgout.write(xstart,ystart,ny,nx,dist2rdg->getGridPointer());
		}
		if(dist2vlyExport)
		{
			tiffIO dist2vlyout(dist2vlyfile,FLOAT_TYPE,nodata,vlysrc);
			dist2vlyout.write(xstart,ystart,ny,nx,dist2vly->getGridPointer());
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

		if (rank == 0)
		{
			printf("Processor:%d\n    Read time:%f\n    Compute time:%f\n    Write time:%f\n    Total time:%f\n",size,dataRead,compute,write,total);
			fflush(stdout);
		}

		/// free memory
		delete vly;
		delete rdg;
		delete rpi;
	}
	MPI_Finalize();
	return 0;
}
