/// include fundamental libraries
#include <stdlib.h>
#include <iostream>
#include <queue>
/// include MPI
#include <mpi.h>
/// include TauDEM header files
#include "commonLib.h"
#include "createpart.h"
#include "tiffIO.h"
/// include RidgeExtraction header
#include "RidgeExtraction.h"
/// include statistics header
#include "stats.h"
// using namespace std; // Avoid to using the entire namespace of std. Comment by Liangjun, 01/23/19

vector<node> downstream_coors(float &dirv, int &col, int &row)
{
	vector<node> downcells;
	vector<int> downdirs;
	if(dirv > 0.f && fmodf(dirv, 1.f) == 0.f) /// means D8 flow model, valued 1 to 8. TODO, add a flag to indicate flow model.
	{
		int tmpdirv = (int)dirv;
		if(tmpdirv >= 1 && tmpdirv <= 8){
			// printf("%d,",tmpdirv);
			downdirs.push_back(tmpdirv);
		}
		else{
			printf("%f is beyond the valid flow direction values, please check!",dirv);
			exit(-1); /// TODO, add some model throw exception code.
		}
	}
	else /// D-inf flow model
	{
		for (int i = 1; i <= 8; i++)
		{
			if(floatequal((double) dirv, dinfang[i]))
				downdirs.push_back(i);
		}
		if (downdirs.empty())
		{
			for (int i = 2; i <= 8; i++)
			{
				if(dirv < dinfang[i]){
					downdirs.push_back(i);
					downdirs.push_back(i - 1);
					break;
				}
			}
			if (downdirs.empty() || dirv >= dinfang[8])
			{
				downdirs.push_back(8);
				downdirs.push_back(1);
			}
		}
	}
	for (vector<int>::iterator iter = downdirs.begin(); iter != downdirs.end(); iter++)
	{
		node tmpnode;
		tmpnode.x = col + d1[*iter]; /// new col
		tmpnode.y = row + d2[*iter]; /// new row
		downcells.push_back(tmpnode);
	}
	vector<node>(downcells).swap(downcells);
	downdirs.clear();
	return downcells;
}

int ExtractRidges(char *dirsfile, char *felfile, float threshold, char *rdgsrcfile)
{
    MPI_Init(NULL, NULL);
    {
        int rank, size;
        MPI_Comm_rank(MCW, &rank);
        MPI_Comm_size(MCW, &size);
        MPI_Status status;
        if (rank == 0)
        {
            printf("RidgeExtraction -h version %s, added by Liangjun Zhu, Nov 21, 2016\n", TDVERSION);
            printf("Flow direction: %s\n", dirsfile);
            printf("Filled elevation: %s\n", felfile);
			printf("Elevation threshold: %.1f\n", threshold);
            printf("Ridge sources: %s\n", rdgsrcfile);
            fflush(stdout);
        }
        double begint = MPI_Wtime();  //!< start time
        //!< read tiff header information using tiffIO
        tiffIO dirsf(dirsfile, FLOAT_TYPE);
        long totalX = dirsf.getTotalX();
        long totalY = dirsf.getTotalY();
        double dx = dirsf.getdxA();
        double dy = dirsf.getdyA();

        //!< read flow direction data into partition
        tdpartition *dirs;
        dirs = CreateNewPartition(dirsf.getDatatype(), totalX, totalY, dx, dy, dirsf.getNodata());
        //!< get the size of current partition
        int nx = dirs->getnx();
        int ny = dirs->getny();
        int xstart, ystart;
        dirs->localToGlobal(0, 0, xstart, ystart); //!< calculate current partition's first cell's position
        dirs->savedxdyc(dirsf);
		dirsf.read(xstart, ystart, ny, nx, dirs->getGridPointer()); //!< get the current partition's pointer
		dirs->share(); //!< share border information

        //!< read filled elevation data into partition
		tiffIO elevf(felfile, FLOAT_TYPE);
		if (!dirsf.compareTiff(elevf))
		{
			printf("File size do not match\n%s\n", felfile);
			MPI_Abort(MCW, 5);
			return 1;
		}
		tdpartition *elev;
		elev = CreateNewPartition(elevf.getDatatype(),totalX,totalY,dx,dy,elevf.getNodata());
		elevf.read(xstart,ystart,ny,nx,elev->getGridPointer());
		elev->share();
        double readt = MPI_Wtime(); //!< record reading time

        //!< Create and initialize ridge sources grid
		tdpartition *rdg;
		rdg = CreateNewPartition(FLOAT_TYPE, totalX, totalY, dx, dy, MISSINGFLOAT);
        //!< COMPUTING CODE BLOCK FOR EXTRACTING RIDGE SOURCES
		int i, j;
		for (j = 0; j < ny; j++) //!< rows
		{
			for (i = 0; i < nx; i++) //!< cols
			{
				rdg->setData(i, j, 1.f);
			}
		}
		/// share up and bottom borders with the initialized value 1.f
		rdg->share();
		//cout<<rank<<": create and initialize ridge partition done!"<<endl;
		/// construct ridge source vector with elevation etc. attributes
		vector<RdgSrc> curRdgSrcs;
        for (j = 0; j < ny; j++) //!< rows
        {
            for (i = 0; i < nx; i++) //!< cols
            {
                if (dirs->hasAccess(i, j) && !dirs->isNodata(i, j))
                {
					float tmpdir;
                    dirs->getData(i, j, tmpdir);
					vector<node> downcells = downstream_coors(tmpdir, i, j);
					for (vector<node>::iterator iter = downcells.begin(); iter != downcells.end(); iter++)
					{
						rdg->setToNodata(iter->x, iter->y);
					}
                }
				else
					rdg->setToNodata(i, j);
			}
		}
		/// IMPORTANT!!!
		///    Firstly, Shares border information between adjacent processes.
		///    Secondly, override nodata, otherwise assign 1.f as ridge source.
		rdg->passBorders();
		for (i = 0; i < nx; i++)
		{
			if (rdg->isNodata(i, -1) || rdg->isNodata(i, 0)) rdg->setData(i, 0, MISSINGFLOAT);
			else rdg->setData(i, 0, 1.f);

			if (rdg->isNodata(i, ny) || rdg->isNodata(i, ny - 1)) rdg->setData(i, ny - 1, MISSINGFLOAT);
			else rdg->setData(i, ny - 1, 1.f);
		}

		for (j = 0; j < ny; j++) //!< rows
		{
			for (i = 0; i < nx; i++) //!< cols
			{
				float tmpelev;
				if(rdg->isNodata(i, j))
					continue;
				if (elev->hasAccess(i, j) && !elev->isNodata(i, j))
				{
					elev->getData(i, j, tmpelev);
					RdgSrc tmprdgsrc;
					tmprdgsrc.Coor.x = i;
					tmprdgsrc.Coor.y = j;
					tmprdgsrc.elev = tmpelev;
					curRdgSrcs.push_back(tmprdgsrc);
				}
				else
					rdg->setToNodata(i, j);
			}
		}
		vector<RdgSrc>(curRdgSrcs).swap(curRdgSrcs);
		//cout<<rank<<": gather ridge sources information done!"<<endl;
		float *curRdgElevs = new float[curRdgSrcs.size()];
		int curCount = 0;
		for (vector<RdgSrc>::iterator iter = curRdgSrcs.begin(); iter != curRdgSrcs.end(); iter++)
		{
			curRdgElevs[curCount] = iter->elev;
			curCount++;
		}
        //!< END COMPUTING CODE BLOCK

		/// gather information from all nodes to the root node
		int allCount;
		int *locCount = new int[size];
		MPI_Reduce(&curCount, &allCount, 1, MPI_INT, MPI_SUM, 0, MCW);
		MPI_Gather(&curCount, 1, MPI_INT, locCount, 1, MPI_INT, 0, MCW);
		float *allRdgElevs = NULL;
		if (rank == 0) /// only the root node needs to allocate memory
		{
			allRdgElevs = new float[allCount];
		}
		int *displs = new int[size];
		displs[0] = 0;
		for (i = 1; i < size; i++)
		{
			displs[i] = displs[i - 1] + locCount[i - 1];
		}
		MPI_Gatherv(curRdgElevs, curCount, MPI_FLOAT, allRdgElevs, locCount, displs, MPI_FLOAT, 0, MCW);
		delete[] displs;
		displs = NULL;
		delete[] curRdgElevs;
		curRdgElevs = NULL;
		delete[] locCount;
		locCount = NULL;
		/// Now root node has the gathered information of all ridge sources
		//cout<<"rank: "<<rank<<", curCount: "<<curCount<<", allcount: "<<allCount<<", threshold: "<<threshold<<endl;
		if (rank == 0)
		{
			int *orderedIdx = order(allRdgElevs, allCount);
			vector<float> allRdgElevsVector;
			for (int i = 0; i < allCount; i++)
			{
				float tmp = allRdgElevs[orderedIdx[i]];
				if (tmp > 0.f){  /// assume the HAND is at least greater than 5.0m to be a potential ridge
					allRdgElevsVector.push_back(tmp);
				}
			}
			vector<float>(allRdgElevsVector).swap(allRdgElevsVector);

			//float *x = new float[100];
			//float *y = new float[100];
			//for(int i = 0; i < 100; i++) y[i] = 0;
			//float minv = allRdgElevsVector.at(0);
			//float maxv = allRdgElevsVector.at(allRdgElevsVector.size()-1);
			//float interval = (maxv - minv) / 100.f;

			//for (vector<float>::iterator iter = allRdgElevsVector.begin(); iter != allRdgElevsVector.end(); iter++)
			//{
			//	y[(int) floor((*iter - minv) / interval)]++;
			//}
			//for (i = 0; i < 100; i++)
			//{
			//	x[i] = minv + interval * (i + 0.5f);
			//}
			//vector<float> tempx, tempy;

			////ofstream logf;
			////char *logfile = "C:\\z_data_m\\AutoFuzSlpPos\\youwuzhen\\DinfpreDir\\ele.txt";
			////FILE *fp;
			////fp = fopen(logfile, "w+");
			////if (fp == NULL) return 0;
			////fclose(fp);
			////logf.open(logfile, ios_base::app | ios_base::out);
			//for (int i = 0; i < 100; i++)
			//{
			//	if (y[i] > 1){
			//		//logf<<x[i]<<","<<y[i]<<endl;
			//		tempx.push_back(x[i]);
			//		tempy.push_back(y[i]);
			//	}
			//}
			////logf.close();
			//vector<float>(tempy).swap(tempy); /// swap to save memory
			//vector<float>(tempx).swap(tempx);

			///// use BiGaussian Fitting to Select Parameters Automatically
			///// these settings are default, and it is good enough to run BiGaussian model. Rewrite from R version by Yu and Peng (2010)
			//vector<float> sigma_ratio_limit;
			//sigma_ratio_limit.push_back(0.1f);
			//sigma_ratio_limit.push_back(10.f);
			//float bandwidth = 0.5f;
			//float power = 1.f;
			//int esti_method = 1; /// Two possible values: 0:"moment" and 1:"em". By default, "em" is selected.
			//float eliminate = 0.05f;
			//float epsilon = 0.005f;
			//int max_iter = 30;
			//vector<vector<float> > bigauss_results;

			///// Be sure that x are ascend
			//int bigauss = BiGaussianMix(tempx, tempy, sigma_ratio_limit, bandwidth, power, esti_method,
			//	eliminate, epsilon, max_iter, bigauss_results);

			//if (bigauss == 1 && bigauss_results.size() == 1)
			//{
			//	float peakCenter = bigauss_results[0][0]; /// fitted central value
			//	float sigmaLeftFitted = bigauss_results[0][1]; /// fitted left sigma
			//	float sigmaRightFitted = bigauss_results[0][2]; /// fitted right sigma
			//	float deltaFitted = bigauss_results[0][3]; /// fitted delta
			//	if (sigmaRightFitted / sigmaLeftFitted > 4.f) /// z-shaped
			//	{
			//		threshold = peakCenter + sigmaRightFitted;
			//	}
			//	else if ((sigmaLeftFitted / sigmaRightFitted > 4.f)) /// s-shaped
			//	{
			//		threshold = peakCenter + sigmaLeftFitted;
			//	}
			//	else
			//		threshold = peakCenter;
			//}
			//else{
				float mean = mean_vector(allRdgElevsVector);
				float sigma = std_vector(allRdgElevsVector, mean);
				float percentile75 = percentile_vector(allRdgElevsVector, 50.f);
				//if (mean < sigma)
				//	threshold = mean;
				//else
				//	threshold = mean - sigma;
				//threshold = max(threshold, percentile75);
				threshold = min(mean, percentile75);
				//cout<<"mean: "<<mean<<", std: "<<sigma<<", percentile75: "<<percentile75<<endl;
			//}
			cout<<"Ridge filter threshold of HAND: "<<threshold<<endl;

			/// release memory
			Release1DArray(orderedIdx);
			Release1DArray(allRdgElevs);
			allRdgElevsVector.clear();
			//tempx.clear();
			//tempy.clear();
			//sigma_ratio_limit.clear();
			//bigauss_results.clear();
			//Release1DArray(x);
			//Release1DArray(y);
		}
		MPI_Bcast(&threshold, 1, MPI_FLOAT, 0, MCW);
		cout<<"rank: "<<rank<<", threshold: "<<threshold<<endl;

		/// Filter by elevation threshold
		for (vector<RdgSrc>::iterator iter = curRdgSrcs.begin(); iter != curRdgSrcs.end(); iter++)
		{
			if(iter->elev <= threshold){
				rdg->setToNodata(iter->Coor.x, iter->Coor.y);
			}
		}

        double computet = MPI_Wtime(); //!< record computing time
        //!< create and write tiff
        float nodata = MISSINGFLOAT;
        tiffIO rdgsrcf(rdgsrcfile, FLOAT_TYPE, nodata, dirsf);
        rdgsrcf.write(xstart, ystart, ny, nx, rdg->getGridPointer());
        double writet = MPI_Wtime(); //!< record writing time


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
            printf("Processes:%d\n    Read time:%f\n    Compute time:%f\n    Write time:%f\n    Total time:%f\n",
                   size, dataRead, compute, write, total);
            fflush(stdout);
        }
		/// free memory
		delete rdg, rdgsrcf;
		delete dirs, dirsf;
		delete elev, elevf;
    }
    MPI_Finalize();
    return 0;
}
