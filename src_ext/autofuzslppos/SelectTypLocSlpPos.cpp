/*!
 * \file SelectTypLocSlpPos.cpp
 *
 * \date 2015/04/24
 *
 * \brief Select typical locations as prototype of slope position type.
 *
 * \author Liangjun Zhu
 * Contact: zlj@lreis.ac.cn
 *
 */
#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */
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
// include MPI 
#include <mpi.h>
// include TauDEM header files
#include "commonLib.h"
//#include "linearpart.h"
#include "createpart.h"
#include "tiffIO.h"
#include "SelectTypLocSlpPos.h"
#include "stats.h"

// using namespace std; // Avoid to using the entire namespace of std. Comment by Liangjun, 01/23/19
using std::ios_base;
using std::fstream;
using std::ofstream;

/// sqrt(2*ln2), e.g., convertor from bi-Gaussion equation to fuzzy membership function
const float shapeConvertor = 1.1774100225154747f;

/*!
 * \brief Do not use this terrain attribute
 *
 */
void dropParam(paramExtGRID &paramgrd)
{
    paramgrd.shape = 'D';
    paramgrd.maxTyp = MISSINGFLOAT;
    paramgrd.minTyp = MISSINGFLOAT;
    paramgrd.k1 = MISSINGFLOAT;
    paramgrd.k2 = MISSINGFLOAT;
    paramgrd.r1 = MISSINGFLOAT;
    paramgrd.r2 = MISSINGFLOAT;
    paramgrd.w1 = MISSINGFLOAT;
    paramgrd.w2 = MISSINGFLOAT;
}

/*!
 * \brief Set fuzzy membership function parameters for terrain attribute
 *
 */
int SetFuzFuncShape(paramExtGRID &paramgrd, ExtInfo &paramExt, char shape, float fittedCenter, float *allvalues,
                    float min_ratio, float max_ratio, int SELECTION_MODE, float w_expander)
{
    w_expander *= shapeConvertor;
    float maxx = fittedCenter;
    int i, defaultSelectNum;
    if (SELECTION_MODE)
        defaultSelectNum = (int) round(paramExt.num * max_ratio * 2.5f);
    else
        defaultSelectNum = (int) round(paramExt.num * min_ratio * 2.5f);
	//cout<<defaultSelectNum<<endl;
    vector<float> valueVector;
    for (i = 0; i < paramExt.num; i++)
        valueVector.push_back(allvalues[i]);
	vector<float>(valueVector).swap(valueVector);
	//cout<<"attribute values number: "<<valueVector.size()<<endl;
    sort(valueVector.begin(), valueVector.end());
    int leftCenterNum = CountIF(allvalues, paramExt.num, false, maxx);
    int rightCenterNum = CountIF(allvalues, paramExt.num, true, maxx);
	//cout<<"leftCenterNum: "<<leftCenterNum<<", right: "<<rightCenterNum<<endl;
    pair<int, int> minMaxIdx = findValue(valueVector, maxx);
    int leftNum = floor((float)defaultSelectNum * (float)leftCenterNum / (float)(leftCenterNum + rightCenterNum));
    int rightNum = defaultSelectNum - leftNum;
    int middleIdx = int(round(float(minMaxIdx.first + minMaxIdx.second) / 2.f));
	//cout<<leftNum<<", "<<rightNum<<", "<<middleIdx<<endl;
    if (shape == 'B')
    {
        paramgrd.shape = shape;
        paramgrd.r1 = 2.f;
        paramgrd.r2 = 2.f;
        paramgrd.k1 = 0.5f;
        paramgrd.k2 = 0.5f;
        if (middleIdx <= leftNum - 1)
        {
            paramgrd.minTyp = valueVector[0];
			int tmp = int(min(middleIdx + rightNum + middleIdx - leftNum + 1, int(valueVector.size() - 1)));
			//cout<<"middleIdx < leftNum: "<<tmp<<endl;
            paramgrd.maxTyp = valueVector[tmp];
        }
        else
        {
            paramgrd.minTyp = valueVector[middleIdx - leftNum + 1];
			int tmp = int(min(middleIdx + rightNum, int(valueVector.size() - 1)));
			//cout<<tmp<<endl;
            paramgrd.maxTyp = valueVector[tmp];
        }
        /// Deprecated code.
        ///paramgrd.maxTyp = min((float)(maxx + (paramExt.maxValue - paramExt.minValue) * DEFAULT_SELECT_RATIO  * k2_2/(k1_2+k2_2)),paramExt.maxValue);
        ///paramgrd.minTyp = max((float)(maxx - (paramExt.maxValue - paramExt.minValue) * DEFAULT_SELECT_RATIO  * k1_2/(k1_2+k2_2)),paramExt.minValue);
        paramgrd.w1 = w_expander * STDcal(allvalues, paramExt.num, false, paramgrd.maxTyp);
        paramgrd.w2 = w_expander * STDcal(allvalues, paramExt.num, true, paramgrd.minTyp);
		//cout<<"w1: "<<paramgrd.w1<<", w2: "<<paramgrd.w2<<endl;
    }
    else if (shape == 'S')
    {
        paramgrd.shape = shape;
        paramgrd.r1 = 2.f;
        paramgrd.k1 = 0.5f;
        paramgrd.w2 = 1.f;
        paramgrd.r2 = 0.f;
        paramgrd.k2 = 1.f;
        paramgrd.maxTyp = paramExt.maxValue;
        paramgrd.minTyp = min(maxx, (float) (valueVector[valueVector.size() - 1 - defaultSelectNum]));

        ///paramgrd.minTyp = min((float)(maxx + (paramExt.maxValue - paramExt.minValue) * DEFAULT_SELECT_RATIO),paramgrd.maxTyp);
        if (paramgrd.minTyp == paramgrd.maxTyp)
            paramgrd.minTyp = paramgrd.maxTyp - paramExt.interval;
        paramgrd.w1 = w_expander * STDcal(allvalues, paramExt.num, false, paramgrd.maxTyp);
    }
    else if (shape == 'Z')
    {
        paramgrd.shape = shape;
        paramgrd.r2 = 2.f;
        paramgrd.k2 = 0.5f;
        paramgrd.w1 = 1.f;
        paramgrd.r1 = 0.f;
        paramgrd.k1 = 1.f;
        paramgrd.minTyp = paramExt.minValue;
        paramgrd.maxTyp = max(maxx, (float) (valueVector[defaultSelectNum]));
        ///paramgrd.maxTyp = max((float)(maxx - (paramExt.maxValue - paramExt.minValue) * DEFAULT_SELECT_RATIO),paramgrd.minTyp);
        if (paramgrd.minTyp == paramgrd.maxTyp)
            paramgrd.maxTyp = paramgrd.minTyp + paramExt.interval;
        paramgrd.w2 = w_expander * STDcal(allvalues, paramExt.num, true, paramgrd.minTyp);
    }
    else{
		dropParam(paramgrd);
	}
	valueVector.clear(); // release memory
	//cout<<"Set fuzzy membership function parameters for "<<paramgrd.name<<" done!"<<" FinalShape: "<<shape<<endl;
	return 0;
}

/*!
 * \brief Main function for selecting typical locations
 *
 */
int SelectTypLocSlpPos(char *inconfigfile, int prototag, int paramsNum, paramExtGRID *paramsgrd, int addparamsNum,
                       paramExtGRID *addparamgrd, vector<DefaultFuzInf> fuzinf, float *baseInputParameters,
                       char *typlocfile, char *outconffile, bool writelog, char *logfile) {
    MPI_Init(NULL, NULL);
    {
        int rank, size;
        MPI_Comm_rank(MCW, &rank);
        MPI_Comm_size(MCW, &size);
        //MPI_Status status;
        int num = 0;  /// define a variable for iterate
        int RPIindex = 0; /// if autoCal is true, all calculation will based on the given rough RPI value range
        bool autoSel = true;
        int temp = 0;
        for (num = 0; num < paramsNum; num++)
            if (!(paramsgrd[num].minTyp == paramsgrd[num].maxTyp && paramsgrd[num].maxTyp == 0.0))
                temp++;
        if (temp == paramsNum)
            autoSel = false;
        int MIN_FREQUENCY = 1, MIN_TYPLOC_NUM = 200, MAX_TYPLOC_NUM = 2000, MAX_LOOP_NUM_TYPLOC_SELECTION = 100;
        float MIN_TYPLOC_NUM_PECENT = 0.f;
        float MAX_TYPLOC_NUM_PECENT = 0.f;
        int SELECTION_MODE = 1;
        float DEFAULT_INCREMENT_RATIO = 0.1f, DEFAULT_SIGMA_MULTIPLIER = 1.f, DEFAULT_BiGaussian_Ratio = 4.f;
        if (baseInputParameters != nullptr) {
            MIN_FREQUENCY = int(baseInputParameters[0]);
            MIN_TYPLOC_NUM_PECENT = baseInputParameters[1];
            MAX_TYPLOC_NUM_PECENT = baseInputParameters[2];
            SELECTION_MODE = int(baseInputParameters[3]);
            DEFAULT_INCREMENT_RATIO = baseInputParameters[4];
            DEFAULT_SIGMA_MULTIPLIER = baseInputParameters[5];
            MAX_LOOP_NUM_TYPLOC_SELECTION = int(baseInputParameters[6]);
            DEFAULT_BiGaussian_Ratio = baseInputParameters[7];
        }
        if (rank == 0) {
            printf("SelectTypLocSlpPos -h version %s, added by Liangjun Zhu, Apr 24, 2015\n", TDVERSION);
            printf("ProtoTag: %d\n", prototag);
            printf("ParametersNum: %d\n", paramsNum);
            for (num = 0; num < paramsNum; num++) {
                if (!(paramsgrd[num].minTyp == paramsgrd[num].maxTyp && paramsgrd[num].maxTyp == 0.f)) {
                    printf("TerrainAttri.No.%d: %s\n", num, paramsgrd[num].name);
                    printf("   Path: %s\n", paramsgrd[num].path);
                    printf("   min: %f\n   max: %f\n", paramsgrd[num].minTyp, paramsgrd[num].maxTyp);
                }
            }
            if (addparamsNum != 0) {
                for (num = 0; num < addparamsNum; num++) {
                    if (!(addparamgrd[num].minTyp == addparamgrd[num].maxTyp && addparamgrd[num].maxTyp == 0.f)) {
                        printf("Additional TerrainAttri.No.%d: %s\n", num, addparamgrd[num].name);
                        printf("   Path: %s\n", addparamgrd[num].path);
                        printf("   min: %f\n   max: %f\n", addparamgrd[num].minTyp, addparamgrd[num].maxTyp);
                    }
                }
            }
            //printf("%d\n",RPIindex);
            printf("Typical Location Output: %s\n", typlocfile);
            //printf("Fuzzy Inference Configuration File: %s\n", outconffile);
            fflush(stdout);
            /// if logfile exists, delete and recreate it, else if logfile does not exist, create it.
            if (autoSel && writelog) {
                fstream tempf;
                tempf.open(logfile);
                if (!tempf) {
                    tempf.close();
                    FILE *fp;
                    fp = fopen(logfile, "w+");
                    if (fp == NULL) return 0;
                    fclose(fp);
                }
                else {
                    tempf.close();
                    remove(logfile);
                    FILE *fp;
                    fp = fopen(logfile, "w+");
                    if (fp == NULL) return 0;
                    fclose(fp);
                }
            }
        }
        for (num = 0; num < paramsNum; num++) {
            if (strcmp(paramsgrd[num].name, "rpi") == 0) {
                RPIindex = num;
                break;
            }
        }
        double begint = MPI_Wtime();  /// start time

        /// read tiff header information using tiffIO
        tiffIO RPIf(paramsgrd[RPIindex].path, FLOAT_TYPE);
        long totalX = RPIf.getTotalX();
        long totalY = RPIf.getTotalY();
        double dx = RPIf.getdxA();
        double dy = RPIf.getdyA();

        /// read RPI data into partition
        tdpartition *rpi;
        rpi = CreateNewPartition(RPIf.getDatatype(), totalX, totalY, dx, dy, RPIf.getNodata());
        /// get the size of current partition
        int nx = rpi->getnx();
        int ny = rpi->getny();
        int xstart, ystart;
        rpi->localToGlobal(0, 0, xstart, ystart); /// calculate current partition's first cell's position
        rpi->savedxdyc(RPIf);
        RPIf.read(xstart, ystart, ny, nx, rpi->getGridPointer()); /// get the current partition's pointer

        /// read parameters data into *partition
        linearpart<float> *params = new linearpart<float>[paramsNum]; /// include RPI
        for (num = 0; num < paramsNum; num++) {
            tiffIO paramsf(paramsgrd[num].path, FLOAT_TYPE);
            if (!RPIf.compareTiff(paramsf)) {
                printf("File size do not match\n%s\n", paramsgrd[num].path);
                MPI_Abort(MCW, 5);
                return 1;
            }
            params[num].init(totalX, totalY, dx, dy, MPI_FLOAT, static_cast<float>(paramsf.getNodata()));
            paramsf.read(xstart, ystart, ny, nx, params[num].getGridPointer());
        }
        /// read additional parameters data into *partition
        linearpart<float> *addparams = nullptr;
        if (addparamsNum != 0) {
            addparams = new linearpart<float>[addparamsNum];
            for (num = 0; num < addparamsNum; num++) {
                tiffIO paramsf(addparamgrd[num].path, FLOAT_TYPE);
                if (!RPIf.compareTiff(paramsf)) {
                    printf("File size do not match\n%s\n", addparamgrd[num].path);
                    MPI_Abort(MCW, 5);
                    return 1;
                }
                addparams[num].init(totalX, totalY, dx, dy, MPI_FLOAT, static_cast<float>(paramsf.getNodata()));
                paramsf.read(xstart, ystart, ny, nx, addparams[num].getGridPointer());
            }
        }

        delete rpi, RPIf; /// to release memory
        double readt = MPI_Wtime(); /// record reading time
		//printf("read data done.\n");
        unsigned int i, j;
        float tempRPI; /// temporary RPI value
        float tempAttr; /// temporary topographic attribute value
        int err;
        float **AllCellValues = new float *[paramsNum]; /// used to store all cell values from all processors selected of each topographic attributes
        ExtInfo *paramsExtInfo = new ExtInfo[paramsNum]; /// frequency distribution array, and store other statistics values
        if (autoSel) { /// determine the value range according to RPI and additional parameters
            float *minTypValue = new float[addparamsNum +1];/// READ RPI value range and additional topographic attributes' value ranges
            float *maxTypValue = new float[addparamsNum + 1];/// update 2015/5/18
            int *CellCount = new int[paramsNum]; /// selected cell numbers
            float **CellValues = new float *[paramsNum]; /// store cell values selected of each topographic attributes for current processor
            queue<float> *tempCellValues = new queue<float>[paramsNum]; /// dynamic queue to store temp cell values selected for current processor
            float *maxValue = new float[paramsNum]; /// maximum topographic attribute
            float *minValue = new float[paramsNum]; /// minimum topographic attribute
            for (i = 0; i < addparamsNum; i++) {
                minTypValue[i] = addparamgrd[i].minTyp;
                maxTypValue[i] = addparamgrd[i].maxTyp;
            }
            minTypValue[addparamsNum] = paramsgrd[RPIindex].minTyp;
            maxTypValue[addparamsNum] = paramsgrd[RPIindex].maxTyp;

            for (num = 0; num < paramsNum; num++) {
				if(num == RPIindex) {
					maxValue[num] = paramsgrd[RPIindex].maxTyp;;
					minValue[num] = paramsgrd[RPIindex].minTyp;;
				}
				else {
					maxValue[num] = MISSINGFLOAT;
					minValue[num] = -1 * MISSINGFLOAT;
				}
            }
            /// extract candidate attribute values according to RPI range and additional attribute
            for (j = 0; j < ny; j++) { /// rows
                for (i = 0; i < nx; i++) { /// cols
                    bool selected = true;
                    if (!params[RPIindex].isNodata(i, j)) {
                        params[RPIindex].getData(i, j, tempRPI);
                        if (tempRPI < minTypValue[addparamsNum] || tempRPI > maxTypValue[addparamsNum])
                            selected = false;
                        else {
                            int tempAddParamCount;
                            for (tempAddParamCount = 0; tempAddParamCount < addparamsNum; tempAddParamCount++) {
                                if (!addparams[tempAddParamCount].isNodata(i, j)) {
                                    float tempAddValue;
                                    addparams[tempAddParamCount].getData(i, j, tempAddValue);
                                    if (tempAddValue < minTypValue[tempAddParamCount] ||
                                        tempAddValue > maxTypValue[tempAddParamCount])
                                        selected = false;
                                }
                            }
                        }
                        if (selected) {
                            for (num = 0; num < paramsNum; num++) {
                                if (num != RPIindex) {
                                    if (!params[num].isNodata(i, j)) {
                                        params[num].getData(i, j, tempAttr);
                                        if (tempAttr < minValue[num])
                                            minValue[num] = tempAttr;
                                        else if (tempAttr > maxValue[num])
                                            maxValue[num] = tempAttr;
                                        tempCellValues[num].push(tempAttr);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            /// dump cell values from tempCellValues to CellValues
            for (num = 0; num < paramsNum; num++) {
                if (num != RPIindex) {
                    int tempCellCount = tempCellValues[num].size();
                    CellCount[num] = tempCellCount;
                    CellValues[num] = new float[tempCellCount];
                    while (!tempCellValues[num].empty()) {
                        CellValues[num][tempCellCount - 1] = tempCellValues[num].front();
                        tempCellValues[num].pop();
                        tempCellCount--;
                    }
                }
                else { /// special handing for RPI
                    CellCount[num] = 1;
                    CellValues[num] = new float[1];
                    CellValues[num][0] = 0.f;
                }				
            }
			//for (int k = 0;k < paramsNum;k++)
			//	printf("%s:%d,min:%f,max:%f\n",paramsgrd[k].name,CellCount[k],minValue[k],maxValue[k]);
            /// use host process(i.e., rank = 0) to gather all cell values and do Gaussian Fitting.
            for (num = 0; num < paramsNum; num++) {
                int *localCellCount = new int[size]; /// cell numbers in each processor
                MPI_Allreduce(&maxValue[num], &paramsExtInfo[num].maxValue, 1, MPI_FLOAT, MPI_MAX, MCW);
                MPI_Allreduce(&minValue[num], &paramsExtInfo[num].minValue, 1, MPI_FLOAT, MPI_MIN, MCW);
                MPI_Allreduce(&CellCount[num], &paramsExtInfo[num].num, 1, MPI_INT, MPI_SUM, MCW);
                MPI_Allgather(&CellCount[num], 1, MPI_INT, localCellCount, 1, MPI_INT, MCW);
                AllCellValues[num] = new float[paramsExtInfo[num].num];
                int *displs = new int[size];
                displs[0] = 0;
                for (i = 1; i < size; i++)
                {
                    displs[i] = displs[i - 1] + localCellCount[i - 1];
                }
                //MPI_Allgatherv(CellValues[num],CellCount[num],MPI_FLOAT,AllCellValues[num],localCellCount,displs,MPI_FLOAT,MCW); //!< Gather and scatter to all processor
                MPI_Gatherv(CellValues[num], CellCount[num], MPI_FLOAT, AllCellValues[num], localCellCount, displs,
                            MPI_FLOAT, 0, MCW);
            }
            int maxCellCount = 0;
            for (num = 0; num < paramsNum; num++)
            {
                if (paramsExtInfo[num].num > maxCellCount)
                    maxCellCount = paramsExtInfo[num].num;
            }
            MAX_TYPLOC_NUM = int(MAX_TYPLOC_NUM_PECENT * maxCellCount);
            MIN_TYPLOC_NUM = int(MIN_TYPLOC_NUM_PECENT * maxCellCount); /// update 2015/08/07
			
            /// do Bi-Gaussian fitting model and set parameters for finding typical locations and fuzzy inference
            if (rank == 0) {
				/// test code
				//for (int i = 0;i < paramsNum;i++)
				//	printf("%s:%d,min:%f,max:%f\n",paramsgrd[i].name,paramsExtInfo[i].num,paramsExtInfo[i].minValue,paramsExtInfo[i].maxValue);
				//for(num = 0; num < paramsNum; num++)
				//	cout<<paramsgrd[num].name<<","<<paramsgrd[num].shape<<","<<paramsgrd[num].minTyp<<","<<paramsgrd[num].maxTyp<<endl;
				/// end test code
                for (num = 0; num < paramsNum; num++) {
                    paramsExtInfo[num].interval = 0.f;
                    if (num == RPIindex) { /// set default
                        paramsgrd[num].shape = 'B';
                        paramsgrd[num].w1 = 6.f;
                        paramsgrd[num].k1 = 0.5f;
                        paramsgrd[num].r1 = 2.f;
                        paramsgrd[num].w2 = 6.f;
                        paramsgrd[num].k2 = 0.5f;
                        paramsgrd[num].r2 = 2.f;
                        continue;
                    }
                    for (i = 0; i < FREQUENCY_GROUP; i++) { /// initialize frequency distribution arrays
                        paramsExtInfo[num].x[i] = 0.f;
                        paramsExtInfo[num].y[i] = 0.f;
                        paramsExtInfo[num].XRange[i] = 0.f;
                    }
                    paramsExtInfo[num].XRange[FREQUENCY_GROUP] = 0.f;
					int group = FREQUENCY_GROUP;
					//cout<<paramsExtInfo[num].num<<endl;
					if (paramsExtInfo[num].num < 2 * FREQUENCY_GROUP)
						group /= 10;
					else if (paramsExtInfo[num].num < 4 * FREQUENCY_GROUP)
						group /= 5;
					else if (paramsExtInfo[num].num < 10 * FREQUENCY_GROUP)
						group /= 2;
                    paramsExtInfo[num].interval =
                            (paramsExtInfo[num].maxValue - paramsExtInfo[num].minValue) / (float)group;
                    for (i = 0; i < FREQUENCY_GROUP; i++) {
                        paramsExtInfo[num].x[i] =
                                paramsExtInfo[num].minValue + paramsExtInfo[num].interval * (i + 0.5f);
                        paramsExtInfo[num].XRange[i] =
                                paramsExtInfo[num].minValue + paramsExtInfo[num].interval * i;
                    }
                    paramsExtInfo[num].XRange[FREQUENCY_GROUP] = paramsExtInfo[num].maxValue;
                    for (i = 0; i < paramsExtInfo[num].num; i++) { /// calculate frequency
                        paramsExtInfo[num].y[(int) floor((AllCellValues[num][i] - paramsExtInfo[num].minValue) /
                                                            paramsExtInfo[num].interval)]++;
                    }
					//printf("%s:%d,min:%f,max:%f,interval:%f\n",paramsgrd[num].name,paramsExtInfo[num].num,
					//	paramsExtInfo[num].minValue,paramsExtInfo[num].maxValue,paramsExtInfo[num].interval);
					//for (int k = 0; k<FREQUENCY_GROUP;k++)
					//{
					//	cout<<paramsExtInfo[num].y[k]<<",";
					//}
					//cout<<endl;

					// if MIN_FREQUENCY is greater than the 50% of the frequencies, then use the 70%
					int gt = 0;
					vector<float> tmpY;
					for (j = 0; j < FREQUENCY_GROUP; j++) {
                        if (paramsExtInfo[num].y[j] >= MIN_FREQUENCY)
                            gt++;
                        tmpY.push_back(paramsExtInfo[num].y[j]);
					}
					vector<float>(tmpY).swap(tmpY);
					//cout<<"there are "<<gt<<" greater than MIN_FREQUENCY: "<<MIN_FREQUENCY<<endl;
					int newLowestFreq = MIN_FREQUENCY;
					if (gt <= FREQUENCY_GROUP / 2) {
						sort(tmpY.begin(), tmpY.end());
						newLowestFreq = tmpY[round(FREQUENCY_GROUP * 0.3f)];
						newLowestFreq = newLowestFreq <= 0 ? 1 : newLowestFreq;
					}
					//for(vector<float>::iterator it = tmpY.begin(); it != tmpY.end(); it++)
					//	cout<<*it<<", ";
					//cout<<endl;
					//cout<<"MIN_FREQUENCY is: "<<newLowestFreq<<endl;
                    vector<float> tempx, tempy;
                    for (j = 0; j < FREQUENCY_GROUP; j++) { /// eliminate frequency which less than MIN_FREQUENCY
                        if (paramsExtInfo[num].y[j] >= newLowestFreq) {
                            tempx.push_back(paramsExtInfo[num].x[j]);
                            tempy.push_back(paramsExtInfo[num].y[j]);
                        }
                    }
                    vector<float>(tempy).swap(tempy); /// swap to save memory
                    vector<float>(tempx).swap(tempx);
                    if (writelog) { /// append frequency distribution values to log file
                        ofstream logf;
                        logf.open(logfile, ios_base::app | ios_base::out);
                        logf << "Frequencies of " << paramsgrd[num].name << endl;
                        for (j = 0; j < tempx.size(); j++)
                            logf << tempx[j] << "," << tempy[j] << endl;
                        logf.close();
                    }
                    /// update: 2015/5/14, use the prior defined shape of fuzzy membership function
                    vector<char> priorShape;
                    for (i = 0; i < fuzinf.size(); i++) {
                        if (strcmp(fuzinf[i].param, paramsgrd[num].name) == 0) {
                            for (j = 0; j < 4; j++) {
                                if (fuzinf[i].shape[j] != '\0')
                                    priorShape.push_back(fuzinf[i].shape[j]);
                                else
                                    break;
                            }
                        }
                    }
                    vector<float>::iterator max_freq_y = max_element(tempy.begin(), tempy.end());
                    int max_freq_idx = distance(tempy.begin(), max_freq_y);
                    int max_freq_idx_origin;  /// because of eliminate of zero, max_freq_idx may less than max_freq_idx_origin
                    for (i = max_freq_idx; i < FREQUENCY_GROUP; i++) {
                        if (paramsExtInfo[num].y[i] == tempy[max_freq_idx]) {
                            max_freq_idx_origin = i;
                            break;
                        }
                    }

                    //printf("%f,%f\n",tempx[max_freq_idx],tempy[max_freq_idx]);

                    /// use BiGaussian Fitting to Select Parameters Automatically
                    /// these settings are default, and it is good enough to run BiGaussian model. Rewrite from R version by Yu and Peng (2010)
                    vector<float> sigma_ratio_limit;
                    sigma_ratio_limit.push_back(0.1f);
                    sigma_ratio_limit.push_back(1.f);
                    float bandwidth = 0.5f;
                    float power = 1.f;
                    int esti_method = 1; /// Two possible values: 0:"moment" and 1:"em".
                    float eliminate = 0.05f;
                    float epsilon = 0.005f;
                    int max_iter = 30;
                    vector<vector<float> > bigauss_results;
						
                    /// Be sure that x are ascend
                    int bigauss = BiGaussianMix(tempx, tempy, sigma_ratio_limit, bandwidth, power, esti_method,
                                                eliminate, epsilon, max_iter, bigauss_results);
					//cout<<"bigaussian fitting done!"<<endl;
                    /// End BiGaussian Fitting
                    char fitShape[2];
                    fitShape[0] = 'N';
                    fitShape[1] = 'N'; /// Fuzzy inference function shape recommended by BiGaussian Fitting, 'N' means no recommendation.
                    float centralValue[2];
                    centralValue[0] = 0.f;
                    centralValue[1] = 0.f;
                    char finalShape = 'N';
                    float finalCentralValue = 0.f;
                    /// simple rules to figure out which curve shape should be
                    float cum2maxFreq_default = 0.25f, dist2center_default = 0.05f, disthalf2end_default = 0.5f;
                    float biRatio = 0.f; /// used to determine the curve shape of FMF
                    if (bigauss == 1 && bigauss_results.size() == 1) { /// Only one fitted BiGaussian model returned
                        float peakCenter = bigauss_results[0][0]; /// fitted central value
                        float sigmaLeftFitted = bigauss_results[0][1]; /// fitted left sigma
                        float sigmaRightFitted = bigauss_results[0][2]; /// fitted right sigma
                        float deltaFitted = bigauss_results[0][3]; /// fitted delta
                        float nashCoefFitted = bigauss_results[0][4]; /// fitted nash-sutcliffe coefficient, -infinity ~ 1
                        float max_freq_x; /// x value of maximum frequency
                        float dist2center; /// distance from fitted central value to max_freq_x

						//cout<<"  Only one fitted BiGaussian model returned"<<endl;
						//cout<<"peak: "<<peakCenter<<", leftS: "<<sigmaLeftFitted<<", rightS: "<<sigmaRightFitted<<endl;
                        max_freq_x = tempx[max_freq_idx];
                        dist2center = abs(max_freq_x - peakCenter) / paramsExtInfo[num].interval; 
						/// this means the fitted result is quite good
                        if (dist2center < dist2center_default * FREQUENCY_GROUP) {
							//cout<<"    the fitted result is quite good"<<endl;
                            biRatio = sigmaLeftFitted / sigmaRightFitted;
                            if (biRatio >= DEFAULT_BiGaussian_Ratio) { /// regard as s-shaped
                                fitShape[0] = 'S';
                            }
							/// bell-shaped or s-shaped, and bell-shaped is prevail
                            else if (biRatio > 1 && biRatio < DEFAULT_BiGaussian_Ratio) {
                                fitShape[0] = 'B';
                                fitShape[1] = 'S';
                            }
                            else if (biRatio == 1) { /// regard as bell-shaped
                                fitShape[0] = 'B';
                            }
							/// bell-shaped or z-shaped, and bell-shaped is prevail
                            else if (biRatio < 1 && biRatio > 1.f / DEFAULT_BiGaussian_Ratio) {
                                fitShape[0] = 'B';
                                fitShape[1] = 'Z';
                            }
                            else /// biRatio <= 1.f / DEFAULT_BiGaussian_Ratio
                                fitShape[0] = 'Z';
                            centralValue[0] = peakCenter;
                            centralValue[1] = peakCenter;
                        }
                        else { /// it means that the fitted result is not satisfied, use simple rules to figure out fitShape
							//cout<<"    the fitted result is not satisfied, use simple rules instead"<<endl;
                            int cumFreq = 0;  /// accumulative frequency
                            int validNum = accumulate(tempy.begin(), tempy.end(), 0); /// all frequency number
                            float cumFreqRatio = 0.f; /// accumulative frequency divided by validNum
                            float cum1 = -9999.f, cum2 = -9999.f, cum3 = -9999.f; /// represent cum0.25, cum0.5, cum0.75, respectively

                            //float cum2MaxFreq = 0.f; /// accumulative frequency to max_freq divided validNum
                            //float disthalf2end; /// distance from accFreqRatio >= 0.5 to the maximum attribute value

                            for (i = 0; i < tempx.size(); i++)
                            {
                                cumFreq += (int) tempy[i];
                                cumFreqRatio = (float) cumFreq / validNum;
                                if (cumFreqRatio >= 0.25f && cum1 == -9999.f)
                                {
                                    cum1 = tempx[i];
                                }
                                else if (cumFreqRatio >= 0.5f && cum2 == -9999.f)
                                {
                                    cum2 = tempx[i];
                                }
                                else if (cumFreqRatio >= 0.75f && cum3 == -9999.f)
                                {
                                    cum3 = tempx[i];
                                    break;
                                }
                                /*if(i == max_freq_idx)
                                    cum2MaxFreq = cumFreq / validNum;
                                if(cumFreqRatio < 0.5){
                                    cumFreqRatio = (float)cumFreq / validNum;
                                    disthalf2end = abs(tempx[i] - paramsExtInfo[num].maxValue)/paramsExtInfo[num].interval;
                                }*/
                            }
                            biRatio = (cum2 - cum1) / (cum3 - cum2);
                            //cout<<cum1<<endl<<cum2<<endl<<cum3<<endl;
                            //cout<<max_freq_x<<endl;
                            if (cum1 >= max_freq_x)
                            {
                                fitShape[0] = 'Z';/// z-shaped, central value = max(cum2,max_freq_x)
                                centralValue[0] = cum2 >= max_freq_x ? cum2 : max_freq_x;
                            }
                            else if (cum3 <= max_freq_x)
                            {
                                fitShape[0] = 'S';/// s-shaped, central value = min(cum2,max_freq_x)
                                centralValue[0] = cum2 >= max_freq_x ? max_freq_x : cum2;
                            }
                            else
                            {
                                if (biRatio >= DEFAULT_BiGaussian_Ratio) /// regard as s-shaped
                                {
                                    fitShape[0] = 'S';
                                    centralValue[0] = cum2 >= max_freq_x ? max_freq_x : cum2;
                                }
								/// bell-shaped or s-shaped, and bell-shaped is prevail
                                else if (biRatio > 1 && biRatio < DEFAULT_BiGaussian_Ratio) 
                                {
                                    fitShape[0] = 'B';
                                    fitShape[1] = 'S';
                                    centralValue[0] = cum2;
                                    centralValue[1] = cum2 >= max_freq_x ? max_freq_x : cum2;
                                }
                                else if (biRatio == 1)  /// regard as bell-shaped
                                {
                                    fitShape[0] = 'B';
                                    centralValue[0] = cum2;
                                }
								/// bell-shaped or z-shaped, and bell-shaped is prevail
                                else if (biRatio < 1 && biRatio > 1.f / DEFAULT_BiGaussian_Ratio) 
                                {
                                    fitShape[0] = 'B';
                                    fitShape[1] = 'Z';
                                    centralValue[0] = cum2;
                                    /// z-shaped, central value = max(cum2,max_freq_x)
                                    centralValue[1] = cum2 >= max_freq_x ? cum2 : max_freq_x;
                                }
                                else /// biRatio <= 1.f / DEFAULT_BiGaussian_Ratio
                                {
                                    fitShape[0] = 'Z';
                                    centralValue[0] = cum2 >= max_freq_x ? cum2 : max_freq_x;
                                }
                            }
                        }
                        //cout<<priorShape.size()<<endl;
                        if (priorShape.size() == 1) /// if defined only one shape
                        {
                            if (priorShape[0] != 'N')
                            {
                                finalShape = priorShape[0];
                                if (fitShape[1] != 'N')  /// it means the fitShape has two possibility.
                                {
                                    finalCentralValue = (fitShape[0] == finalShape) ? centralValue[0]
                                                                                    : centralValue[1];
                                }
                                else /// it means the fitShape has one possibility.
                                {
                                    finalCentralValue = centralValue[0];
                                }
								//cout<<"set fuzzy membership function"<<", shape: "<<finalShape<<
								//	", central value: "<<finalCentralValue<<endl;
                                if ((err = SetFuzFuncShape(paramsgrd[num], paramsExtInfo[num], finalShape,
                                                            finalCentralValue, AllCellValues[num],
                                                            MIN_TYPLOC_NUM_PECENT, MAX_TYPLOC_NUM_PECENT,
                                                            SELECTION_MODE, DEFAULT_SIGMA_MULTIPLIER)) != 0)
                                    return 1;
                            }
                            else
                            {
                                finalShape = 'N';
                                dropParam(paramsgrd[num]);
                            }
                        }
                        else if (priorShape.size() > 1) /// priorShape may be two or three possibility.
                        {
                            bool match = true;
                            int matchIdx[2];
                            matchIdx[0] = -9999;
                            matchIdx[1] = -9999;
                            for (i = 0; i < priorShape.size(); i++)
                            {
                                for (j = 0; j < 2; j++)
                                {
                                    if (priorShape[i] == fitShape[j])
                                    {
                                        matchIdx[j] = j;
                                    }
                                }
                            }

                            if (matchIdx[0] == -9999 && matchIdx[1] == 1)
                            {
                                finalShape = fitShape[1];
                                finalCentralValue = centralValue[1];
                            }
                            else if (!(matchIdx[0] == -9999 && matchIdx[1] == -9999))
                            {
                                finalShape = fitShape[0];
                                finalCentralValue = centralValue[0];
                            }
                            else
                                match = false;
                            if (match)
                            {
								cout<<"fitShape: "<<fitShape[0]<<", "<<fitShape[1]<<endl;
								cout<<"fitShape matched with one of the priorShape: "<<finalShape<<endl;
                                if ((err = SetFuzFuncShape(paramsgrd[num], paramsExtInfo[num], finalShape,
                                                            finalCentralValue, AllCellValues[num],
                                                            MIN_TYPLOC_NUM_PECENT, MAX_TYPLOC_NUM_PECENT,
                                                            SELECTION_MODE, DEFAULT_SIGMA_MULTIPLIER)) != 0)
                                    return 1;
                            }
                            else /// if the fitted shapes are not coincident with prior shapes, take the first of prior shape as well
                            {
                                //dropParam(paramsgrd[num]);
                                finalShape = priorShape[0];
                                if ((err = SetFuzFuncShape(paramsgrd[num], paramsExtInfo[num], finalShape,
                                                            max_freq_x, AllCellValues[num], MIN_TYPLOC_NUM_PECENT,
                                                            MAX_TYPLOC_NUM_PECENT, SELECTION_MODE,
                                                            DEFAULT_SIGMA_MULTIPLIER)) != 0)
                                    return 1;
                            }
                        }
                        else
                            dropParam(paramsgrd[num]);
                        if (writelog) { /// append fitting result to log file
                            ofstream logf;
                            logf.open(logfile, ios_base::app | ios_base::out);
                            logf << endl << endl;
                            logf << "BiGaussian model Fitting result: " << endl;
                            logf << "Peak Center: " << peakCenter << ",";
                            logf << "Left Sigma: " << sigmaLeftFitted << ",";
                            logf << "Right Sigma: " << sigmaRightFitted << ",";
                            logf << "Delta: " << deltaFitted << ",";
                            logf << "Nash Coef: " << nashCoefFitted << endl;

                            if (fitShape[1] != 'N')
                                logf << "Fitted curve shape: " << fitShape[0] << ", " << fitShape[1] << endl;
                            else
                                logf << "Fitted curve shape: " << fitShape[0] << endl;
                            logf << "Final curve shape: " << finalShape << endl << endl;
                            logf.close();
                        }
                    }
                    else { /// the bi-Gaussian fitted failed!
                        dropParam(paramsgrd[num]);
                        if (writelog) {
                            ofstream logf;
                            logf.open(logfile, ios_base::app | ios_base::out);
                            logf << endl << endl;
                            logf << "BiGaussian model Fitting result indicates a mixture model! " << endl;
                            logf.close();
                        }
                    }
                }
            }
            /// Broadcast the extracted parameters to all processors
            for (num = 0; num < paramsNum; num++) {
                MPI_Bcast(&paramsgrd[num].shape, 1, MPI_CHAR, 0, MCW);
                MPI_Bcast(&paramsgrd[num].maxTyp, 1, MPI_FLOAT, 0, MCW);
                MPI_Bcast(&paramsgrd[num].minTyp, 1, MPI_FLOAT, 0, MCW);
                MPI_Bcast(&paramsgrd[num].w1, 1, MPI_FLOAT, 0, MCW);
                MPI_Bcast(&paramsgrd[num].k1, 1, MPI_FLOAT, 0, MCW);
                MPI_Bcast(&paramsgrd[num].r1, 1, MPI_FLOAT, 0, MCW);
                MPI_Bcast(&paramsgrd[num].w2, 1, MPI_FLOAT, 0, MCW);
                MPI_Bcast(&paramsgrd[num].k2, 1, MPI_FLOAT, 0, MCW);
                MPI_Bcast(&paramsgrd[num].r2, 1, MPI_FLOAT, 0, MCW);
            }
        }
		
        /// Now extract typical location according to maxTyp and minTyp of each topographic attribute
        tdpartition *typloc;
        typloc = CreateNewPartition(SHORT_TYPE, totalX, totalY, dx, dy, MISSINGSHORT);

        int selectedNum = 0; /// number of selected topographic attributes
        int validCount; /// to figure out if all topographic attributes are located in the typical value ranges
        int validCountAdd; /// to figure out if all the additional topographic attributes are located in the typical value ranges
        int TypLocCount = 0; /// cell number of typical locations from current processor
        int TypLocCountAll = 0; /// cell number of typical locations from all processors
        int LoopNum = 0; /// record loop number to avoid endless loop
        int previousTypLocCountAll; /// cell number of typical locations from last loop, this is aimed to judge when to stop.
        selectedNum = 0;
        for (num = 0; num < paramsNum; num++) {
            if (paramsgrd[num].shape != 'D' && paramsgrd[num].maxTyp > paramsgrd[num].minTyp)
                selectedNum++;
        }
        while ((TypLocCountAll <= MIN_TYPLOC_NUM || TypLocCountAll >= MAX_TYPLOC_NUM) &&
               LoopNum <= MAX_LOOP_NUM_TYPLOC_SELECTION) {
            TypLocCount = 0;
            previousTypLocCountAll = TypLocCountAll;
            TypLocCountAll = 0;
            typloc->init(totalX, totalY, dx, dy, MPI_SHORT, MISSINGSHORT);
            for (j = 0; j < ny; j++) { /// rows
                for (i = 0; i < nx; i++) { /// cols
                    validCount = 0;
                    validCountAdd = 0;
                    for (num = 0; num < paramsNum; num++) {
                        if (!params[num].isNodata(i, j)) {
                            if (paramsgrd[num].shape != 'D' && paramsgrd[num].maxTyp > paramsgrd[num].minTyp) {
                                params[num].getData(i, j, tempAttr);
                                if (tempAttr >= paramsgrd[num].minTyp && tempAttr <= paramsgrd[num].maxTyp)
                                    validCount++;
                            }
                        }
                    }
                    for (num = 0; num < addparamsNum; num++) {
                        if (!addparams[num].isNodata(i, j)) {
                            if (addparamgrd[num].maxTyp > addparamgrd[num].minTyp) {
                                addparams[num].getData(i, j, tempAttr);
                                if (tempAttr >= addparamgrd[num].minTyp && tempAttr <= addparamgrd[num].maxTyp)
                                    validCountAdd++;
                            }
                        }
                    }
                    if (validCount == selectedNum && validCountAdd == addparamsNum) {
                        typloc->setData(i, j, (short) prototag);
                        TypLocCount++;
                    }
                    else
                        typloc->setToNodata(i, j);
                }
            }
            //cout<<"rank:"<<rank<<",curNum: "<<TypLocCount<<endl;
            MPI_Allreduce(&TypLocCount, &TypLocCountAll, 1, MPI_INT, MPI_SUM, MCW);
            //printf("%d\n",TypLocCountAll);
            if (rank == 0) {
                if (TypLocCountAll < MIN_TYPLOC_NUM) { /// if selected cell number is less than MIN_TYPLOC_NUM
                    for (num = 0; num < paramsNum; num++) {
                        if (paramsgrd[num].shape != 'D' && paramsgrd[num].maxTyp > paramsgrd[num].minTyp &&
                            strcmp(paramsgrd[num].name, "rpi") != 0) {
                            float oldMaxTyp = paramsgrd[num].maxTyp;
                            float oldMinTyp = paramsgrd[num].minTyp;
                            if (paramsgrd[num].shape == 'B') {
                                if (autoSel && (paramsgrd[num].w1 + paramsgrd[num].w2) > ZERO) {
                                    paramsgrd[num].maxTyp += abs(
                                            paramsgrd[num].maxTyp * DEFAULT_INCREMENT_RATIO * paramsgrd[num].w2 /
                                            (paramsgrd[num].w1 + paramsgrd[num].w2));
                                    paramsgrd[num].minTyp -= abs(
                                            paramsgrd[num].minTyp * DEFAULT_INCREMENT_RATIO * paramsgrd[num].w1 /
                                            (paramsgrd[num].w1 + paramsgrd[num].w2));
                                }
                                else {
                                    paramsgrd[num].maxTyp += abs(paramsgrd[num].maxTyp * DEFAULT_INCREMENT_RATIO / 2.f);
                                    paramsgrd[num].minTyp -= abs(paramsgrd[num].minTyp * DEFAULT_INCREMENT_RATIO / 2.f);
                                }
                            }
                            else if (paramsgrd[num].shape == 'S') {
                                if (paramsgrd[num].minTyp < ZERO)
                                    paramsgrd[num].minTyp -= abs(paramsExtInfo[num].interval * DEFAULT_INCREMENT_RATIO);
                                else
                                    paramsgrd[num].minTyp -= abs(paramsgrd[num].minTyp * DEFAULT_INCREMENT_RATIO);
                            }
                            else if (paramsgrd[num].shape == 'Z') {
                                if (paramsgrd[num].maxTyp < ZERO)
                                    paramsgrd[num].maxTyp += abs(paramsExtInfo[num].interval * DEFAULT_INCREMENT_RATIO);
                                else
                                    paramsgrd[num].maxTyp += abs(paramsgrd[num].maxTyp * DEFAULT_INCREMENT_RATIO);
                            }
                            paramsgrd[num].maxTyp = min(paramsgrd[num].maxTyp, paramsExtInfo[num].maxValue);
                            paramsgrd[num].minTyp = max(paramsgrd[num].minTyp, paramsExtInfo[num].minValue);
                            if (paramsgrd[num].minTyp >= paramsgrd[num].maxTyp) {
                                paramsgrd[num].minTyp = oldMinTyp;
                                paramsgrd[num].maxTyp = oldMaxTyp;
                            }
                        }
                    }
                    LoopNum++;
                }
                else if (TypLocCountAll > MAX_TYPLOC_NUM) { /// if selected cell number is greater than MAX_TYPLOC_NUM
                    for (num = 0; num < paramsNum; num++) {
                        if (paramsgrd[num].shape != 'D' && paramsgrd[num].maxTyp > paramsgrd[num].minTyp &&
                            strcmp(paramsgrd[num].name, "rpi") != 0) {
                            float oldMaxTyp = paramsgrd[num].maxTyp;
                            float oldMinTyp = paramsgrd[num].minTyp;
                            if (paramsgrd[num].shape == 'B') {
                                if (autoSel && (abs(paramsgrd[num].w1) + abs(paramsgrd[num].w2)) > ZERO) {
                                    paramsgrd[num].maxTyp -= abs(
                                            paramsgrd[num].maxTyp * DEFAULT_INCREMENT_RATIO * paramsgrd[num].w2 /
                                            (paramsgrd[num].w1 + paramsgrd[num].w2));
                                    paramsgrd[num].minTyp += abs(
                                            paramsgrd[num].minTyp * DEFAULT_INCREMENT_RATIO * paramsgrd[num].w1 /
                                            (paramsgrd[num].w1 + paramsgrd[num].w2));
                                }
                                else {
                                    paramsgrd[num].maxTyp -= abs(paramsgrd[num].maxTyp * DEFAULT_INCREMENT_RATIO / 2.f);
                                    paramsgrd[num].minTyp += abs(paramsgrd[num].minTyp * DEFAULT_INCREMENT_RATIO / 2.f);
                                }

                            }
                            else if (paramsgrd[num].shape == 'S') {
                                if (paramsgrd[num].minTyp < ZERO)
                                    paramsgrd[num].minTyp += abs(paramsExtInfo[num].interval * DEFAULT_INCREMENT_RATIO);
                                else
                                    paramsgrd[num].minTyp += abs(paramsgrd[num].minTyp * DEFAULT_INCREMENT_RATIO);
                            }
                            else if (paramsgrd[num].shape == 'Z') {
                                if (paramsgrd[num].maxTyp < ZERO)
                                    paramsgrd[num].maxTyp -= abs(paramsExtInfo[num].interval * DEFAULT_INCREMENT_RATIO);
                                else
                                    paramsgrd[num].maxTyp -= abs(paramsgrd[num].maxTyp * DEFAULT_INCREMENT_RATIO);
                            }
                            paramsgrd[num].maxTyp = min(paramsgrd[num].maxTyp, paramsExtInfo[num].maxValue);
                            paramsgrd[num].minTyp = max(paramsgrd[num].minTyp, paramsExtInfo[num].minValue);
                            if (paramsgrd[num].minTyp >= paramsgrd[num].maxTyp) {
                                paramsgrd[num].minTyp = oldMinTyp;
                                paramsgrd[num].maxTyp = oldMaxTyp;
                            }
                        }
                    }
                    LoopNum++;
                }
                if (LoopNum >= 1) {
                    /// ReCalculate Parameters of Fuzzy Membership Function
                    for (num = 0; num < paramsNum; num++) {
                        if (paramsgrd[num].shape == 'B') {
                            paramsgrd[num].w1 = DEFAULT_SIGMA_MULTIPLIER * shapeConvertor *
                                                STDcal(AllCellValues[num], paramsExtInfo[num].num, false,
                                                       paramsgrd[num].maxTyp);
                            paramsgrd[num].w2 = DEFAULT_SIGMA_MULTIPLIER * shapeConvertor *
                                                STDcal(AllCellValues[num], paramsExtInfo[num].num, true,
                                                       paramsgrd[num].minTyp);
                        }
                        else if (paramsgrd[num].shape == 'S')
                            paramsgrd[num].w1 = DEFAULT_SIGMA_MULTIPLIER * shapeConvertor *
                                                STDcal(AllCellValues[num], paramsExtInfo[num].num, false,
                                                       paramsgrd[num].maxTyp);
                        else if (paramsgrd[num].shape == 'Z')
                            paramsgrd[num].w2 = DEFAULT_SIGMA_MULTIPLIER * shapeConvertor *
                                                STDcal(AllCellValues[num], paramsExtInfo[num].num, true,
                                                       paramsgrd[num].minTyp);
                    }
                }
                if (LoopNum >= 2 && LoopNum < MAX_LOOP_NUM_TYPLOC_SELECTION) {
                    if ((previousTypLocCountAll >= MAX_TYPLOC_NUM && TypLocCountAll <= MIN_TYPLOC_NUM)) {
                        LoopNum = MAX_LOOP_NUM_TYPLOC_SELECTION; /// do another loop
                    }
                    if ((previousTypLocCountAll <= MIN_TYPLOC_NUM && TypLocCountAll >= MAX_TYPLOC_NUM) ||
                        (previousTypLocCountAll == TypLocCountAll && TypLocCountAll != 0)) {
                        LoopNum = MAX_LOOP_NUM_TYPLOC_SELECTION + 1; /// do not do extra loop
                    }
                }
            }
            MPI_Bcast(&LoopNum, 1, MPI_INT, 0, MCW);
            for (num = 0; num < paramsNum; num++) {
                MPI_Bcast(&paramsgrd[num].shape, 1, MPI_CHAR, 0, MCW);
                MPI_Bcast(&paramsgrd[num].maxTyp, 1, MPI_FLOAT, 0, MCW);
                MPI_Bcast(&paramsgrd[num].minTyp, 1, MPI_FLOAT, 0, MCW);
                MPI_Bcast(&paramsgrd[num].w1, 1, MPI_FLOAT, 0, MCW);
                MPI_Bcast(&paramsgrd[num].k1, 1, MPI_FLOAT, 0, MCW);
                MPI_Bcast(&paramsgrd[num].r1, 1, MPI_FLOAT, 0, MCW);
                MPI_Bcast(&paramsgrd[num].w2, 1, MPI_FLOAT, 0, MCW);
                MPI_Bcast(&paramsgrd[num].k2, 1, MPI_FLOAT, 0, MCW);
                MPI_Bcast(&paramsgrd[num].r2, 1, MPI_FLOAT, 0, MCW);
            }
        }
		/// release temp variables to save memory 
		if(autoSel) Release2DArray(paramsNum, AllCellValues);
        /// End Finding typical locations and determining the parameters for fuzzy inference

        double computet = MPI_Wtime(); /// record computing time
		//printf("compute done.\n");
        /// write inference information into output configuration file, exclude RPI
        if (rank == 0 && autoSel) {
            fstream fs;
            fs.open(outconffile, ios_base::out);
            for (num = 0; num < paramsNum; num++) {
                if (num != RPIindex && paramsgrd[num].shape != 'D') {
                    fs << "Parameters\t" << paramsgrd[num].name << "\t" << paramsgrd[num].path << "\t" <<
                    paramsgrd[num].shape << "\t" << paramsgrd[num].w1 << "\t" << paramsgrd[num].r1 << "\t" <<
                    paramsgrd[num].k1 << "\t" << paramsgrd[num].w2 << "\t" << paramsgrd[num].r2 << "\t" <<
                    paramsgrd[num].k2 << endl;
                }
            }
            fs.close();
            // if inconfigfile's name has 'Initial', then remove it
            string inconf = inconfigfile;
            int pos = inconf.find("Initial");
            if (pos > -1) {
                inconf.erase(pos, 7);
            }
            fs.open(inconf.c_str(), ios_base::out);
            fs << "ProtoTag" << "\t" << prototag << endl;
            fs << "ParametersNUM" << "\t" << selectedNum << endl;
            for (num = 0; num < paramsNum; num++)
                if (paramsgrd[num].shape != 'D' && paramsgrd[num].maxTyp > paramsgrd[num].minTyp)
                    fs << "Parameters" << "\t" << paramsgrd[num].name << "\t" << paramsgrd[num].path << "\t" <<
                    paramsgrd[num].minTyp << "\t" << paramsgrd[num].maxTyp << endl;
            if (addparamsNum != 0) {
                fs << "AdditionalNUM" << "\t" << addparamsNum << endl;
                for (num = 0; num < addparamsNum; num++)
                    fs << "Additional" << "\t" << addparamgrd[num].name << "\t" << addparamgrd[num].path << "\t" <<
                    addparamgrd[num].minTyp << "\t" << addparamgrd[num].maxTyp << endl;
            }
            fs << "OUTPUT" << "\t" << typlocfile << endl;
            fs.close();
        }
        //// create and write tiff
        int nodata = MISSINGSHORT;
        tiffIO typlocf(typlocfile, SHORT_TYPE, nodata, RPIf);
        typlocf.write(xstart, ystart, ny, nx, typloc->getGridPointer());
        double writet = MPI_Wtime(); // record writing time
		//printf("write data done.\n");
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

        if (rank == 0) {
            printf("Typical location number is: %d\n", TypLocCountAll);
            printf("Processor:%d\n    Read time:%f\n    Compute time:%f\n    Write time:%f\n    Total time:%f\n", size,
                   dataRead, compute, write, total);
            fflush(stdout);
        }
		/// free memory

		/// release memory
		delete[] params;
		if (addparams != nullptr) delete[] addparams;
		delete typloc;
    }
    MPI_Finalize();
    return 0;
}
