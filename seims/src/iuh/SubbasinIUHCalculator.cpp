#include "SubbasinIUHCalculator.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sstream>
#include "utilities.h"

using namespace std;

SubbasinIUHCalculator::SubbasinIUHCalculator(int t, clsRasterData<int> &rsMask, clsRasterData<float> &rsLanduse,
                                             clsRasterData<float> &rsTime, clsRasterData<float> &rsDelta,
                                             MongoGridFS* grdfs)
    : dt(t), gfs(grdfs), mt(30) {
    nRows = rsMask.getRows();
    nCols = rsMask.getCols();
    mask = rsMask.getRasterDataPointer();
    landcover = rsLanduse.getRasterDataPointer();
    noDataValue = rsMask.getNoDataValue();

    nCells = rsMask.getValidNumber();
//    for (int i = 0; i < nRows; i++)
//    {
//        for (int j = 0; j < nCols; j++)
//        {
//            if (mask[i][j] != noDataValue)
//                nCells++;
//        }
//    }

    uhCell.resize(nCells);
    uh1.resize(nCells);
    for (int i = 0; i < nCells; ++i) {
        //uhCell[i].resize(mt+1);
        uhCell[i].resize(301);
        uh1[i].resize(mt + 1);
        for (int j = 0; j <= mt; ++j) {
            //uhCell[i][j] = 0.0;
            uh1[i][j] = 0.0;
        }
        for (int j = 0; j <= 300; ++j) {
            uhCell[i][j] = 0.0;
        }
    }

    t0 = rsTime.getRasterDataPointer();
    delta = rsDelta.getRasterDataPointer();
}

SubbasinIUHCalculator::~SubbasinIUHCalculator(void) {

}

int SubbasinIUHCalculator::calCell(int id) {
    bson_t p = BSON_INITIALIZER;
    BSON_APPEND_INT32(&p, "SUBBASIN", id);
    const char *type = "OL_IUH";
    BSON_APPEND_UTF8(&p, "TYPE", type);

//    bson *p = (bson *) malloc(sizeof(bson));
//    bson_init(p);
//    bson_append_int(p, "SUBBASIN", id);
//    const char *type = "OL_IUH";
//    bson_append_string(p, "TYPE", type);

    ostringstream oss;
    oss << id << "_" << type;
    string remoteFilename = oss.str();
    BSON_APPEND_UTF8(&p, "ID", remoteFilename.c_str());
    BSON_APPEND_UTF8(&p, "DESCRIPTION", type);
    BSON_APPEND_DOUBLE(&p, "NUMBER", nCells);

//    bson_append_string(p, "ID", remoteFilename.c_str());
//    bson_append_string(p, "DESCRIPTION", type);
//    bson_append_double(p, "NUMBER", nCells);
//    bson_finish(p);
//	gridfile gfile[1];

	/// If the file is already existed in MongoDB, if existed, then delete it!
    gfs->removeFile(remoteFilename);

//    gridfs_remove_filename(gfs, remoteFilename.c_str());
//	/// create a new one
//    gridfile_writer_init(gfile, gfs, remoteFilename.c_str(), type);
//
//    float nCellsFloat = nCells;
//    gridfile_write_buffer(gfile, (const char *) &nCellsFloat, 4);
    //////////////////////////////////////////////////////////////////////////
    vector<float> storeddata_vector;
    storeddata_vector.push_back((float) nCells);
//    int nc = 0;                       //number of cell
    maxtSub = 0;                  //maximum length of uhSub

    // Output to txt file for debugging
//	ofstream iuhf;
//	char iuhfile[200];
//	strcpy(iuhfile,"E:\\iuh.txt");
//	iuhf.open(iuhfile,ios_base::app|ios_base::out);
//	iuhf<<"SubbasinID: "<<id<<endl;

    for (int i = 0; i < nCells; i++) {
        //this part is the same as the corresponding part in the RiverIUHCalculator
        //start
        int mint = int(max(0.0f, t0[i] - 3.f * delta[i]) + 0.5f);    //start time of IUH
        int maxt = min(int(t0[i] + 5.f * delta[i] + 0.5f), mt);       //end time
        maxt = max(maxt, 1);
        double sumUh = 0.0;
        for (int m = mint; m <= maxt; ++m) {
            double delta0 = max(0.01f, delta[i]);
            double t00 = max(0.01f, t0[i]);
            double ti = max(0.01f, float(m));
            uhCell[i][m] = IUHti(delta0, t00, ti);
            //double test = uhCell[nc][m];
            //cout<<"uhCell:"<<nc<<" = " <<test<<"\n";   // test
            sumUh += uhCell[i][m];
            //cout<<m<<endl;
        }

        if (abs(sumUh) < IUHZERO) {
            uhCell[i][0] = 1.0;
            mint = 0;
            maxt = 1;
        } else {
            for (int m = mint; m <= maxt; ++m) {
                uhCell[i][m] = uhCell[i][m] / sumUh;  //make sum of uhCell to 1
                if (uhCell[i][m] < 0.001 || uhCell[i][m] > 1) {
                    uhCell[i][m] = 0.0;
                }
            }
        }
        //define start and end time of uh_cell
        int mint0 = 0;
        for (int m = mint; m <= maxt; ++m) {
            if (uhCell[i][m] > 0.0005) {
                mint0 = m;
                break;
            }
        }
        mint = mint0;   //!actual start time

        int maxt0 = 0;
        for (int m = mint; m <= maxt; ++m) {
            if (uhCell[i][m] < 0.0005) {
                maxt0 = m - 1;
                break;
            }
        }
        maxt = max(mint, maxt0);  //!actual end time
        //end

        maxtSub = max(maxtSub, maxt);
        //cell IUH integration
        if (dt >= 1) {
            double uhSum = 0.0;
            for (int k = 0; k <= int(maxt / dt); ++k) {
                for (int x = 1; x <= dt; ++x) {
                    uh1[i][k] += uhCell[i][k * dt + x - 1];
                }
                uhSum += uh1[i][k];
            }
            mint0 = 0;
            maxt0 = int(maxt / dt);

            for (int k = mint0; k <= maxt0; ++k) {
                uh1[i][k] /= uhSum;
            }
        } else {
            for (int m = mint; m <= maxt; ++m) {
                uh1[i][m] = uhCell[i][m];
            }
            mint0 = mint;
            maxt0 = maxt;
        }

        // if landcover if rice paddy, adjust the iuh according to experience knowledge
        //if(landcover[i] == 33)
            adjustRiceField(mint0, maxt0, uh1[i]);

//        int nTemp = maxt0 - mint0 + 3;
//        float *pTemp = new float[nTemp];
//        pTemp[0] = mint0;
//        pTemp[1] = maxt0;
//        int index = 2;
//        for (int k = mint0; k <= maxt0; ++k)
//        {
//            pTemp[index] = uh1[i][k];
//            index++;
//        }

        storeddata_vector.push_back((float) mint0);
        storeddata_vector.push_back((float) maxt0);
        int index = 2;
        for (int k = mint0; k <= maxt0; k++) {
            storeddata_vector.push_back((float) uh1[i][k]);
        }
//        iuhf<<i<<",";
//        for (int k = 0; k < nTemp; k++)
//        {
//            iuhf<<pTemp[k]<<",";
//        }
        //cout<<endl;
//        iuhf<<endl;
//        gridfile_write_buffer(gfile, (const char *) pTemp, 4 * nTemp);
//        delete pTemp;

    }
    vector<float>(storeddata_vector).swap(storeddata_vector);
    int valuenumber = storeddata_vector.size();
    float *data = new float[valuenumber];
    size_t datalength = valuenumber * sizeof(float);
    for (int i = 0; i < valuenumber; i++) {
        data[i] = storeddata_vector[i];
    }
    char *databuf = (char *) data;
    gfs->writeStreamData(remoteFilename, databuf, datalength, &p);
    delete[] data;
    data = NULL;
    databuf = NULL;
//    for (int i = 0; i < nRows; ++i)
//    {
//        for (int j = 0; j < nCols; ++j)
//        {
//            if (mask[i][j] == noDataValue)
//                continue;
//            //this part is the same as the corresponding part in the RiverIUHCalculator
//            //start
//            int mint = int(max(0.0f, t0[i][j] - 3.f * delta[i][j]) + 0.5f);    //start time of IUH
//            int maxt = min(int(t0[i][j] + 5.f * delta[i][j] + 0.5f), mt);       //end time
//            maxt = max(maxt, 1);
//            double sumUh = 0.0;
//            for (int m = mint; m <= maxt; ++m)
//            {
//                double delta0 = max(0.01f, delta[i][j]);
//                double t00 = max(0.01f, t0[i][j]);
//                double ti = max(0.01f, float(m));
//                uhCell[nc][m] = IUHti(delta0, t00, ti);
//                //double test = uhCell[nc][m];
//                //cout<<"uhCell:"<<nc<<" = " <<test<<"\n";   // test
//                sumUh += uhCell[nc][m];
//                //cout<<m<<endl;
//            }
//
//            if (abs(sumUh) < IUHZERO)
//            {
//                uhCell[nc][0] = 1.0;
//                mint = 0;
//                maxt = 1;
//            }
//            else
//            {
//                for (int m = mint; m <= maxt; ++m)
//                {
//                    uhCell[nc][m] = uhCell[nc][m] / sumUh;  //make sum of uhCell to 1
//                    if (uhCell[nc][m] < 0.001 || uhCell[nc][m] > 1)
//                        uhCell[nc][m] = 0.0;
//                }
//            }
//            //define start and end time of uh_cell
//            int mint0 = 0;
//            for (int m = mint; m <= maxt; ++m)
//            {
//                if (uhCell[nc][m] > 0.0005)
//                {
//                    mint0 = m;
//                    break;
//                }
//            }
//            mint = mint0;   //!actual start time
//
//            int maxt0 = 0;
//            for (int m = mint; m <= maxt; ++m)
//            {
//                if (uhCell[nc][m] < 0.0005)
//                {
//                    maxt0 = m - 1;
//                    break;
//                }
//            }
//            maxt = max(mint, maxt0);  //!actual end time
//            //end
//
//            maxtSub = max(maxtSub, maxt);
//            //cell IUH integration
//            if (dt >= 1)
//            {
//                double uhSum = 0.0;
//                for (int k = 0; k <= int(maxt / dt); ++k)
//                {
//                    for (int x = 1; x <= dt; ++x)
//                    {
//                        uh1[nc][k] += uhCell[nc][k * dt + x - 1];
//                    }
//                    uhSum += uh1[nc][k];
//                }
//                mint0 = 0;
//                maxt0 = int(maxt / dt);
//
//                for (int k = mint0; k <= maxt0; ++k)
//                    uh1[nc][k] /= uhSum;
//            }
//            else
//            {
//                for (int m = mint; m <= maxt; ++m)
//                    uh1[nc][m] = uhCell[nc][m];
//                mint0 = mint;
//                maxt0 = maxt;
//            }
//
//			// if landcover if rice paddy, adjust the iuh according to experience knowledge
//			//if(landcover[i][j] == 33)
//			    adjustRiceField(mint0, maxt0, uh1[nc]);
//
//            int nTemp = maxt0 - mint0 + 3;
//            float *pTemp = new float[nTemp];
//            pTemp[0] = mint0;
//            pTemp[1] = maxt0;
//            int index = 2;
//            for (int k = mint0; k <= maxt0; ++k)
//            {
//                pTemp[index] = uh1[nc][k];
//                index++;
//            }
//            iuhf<<i<<","<<j<<",";
//			for (int k = 0; k < nTemp; k++)
//			{
//				iuhf<<pTemp[k]<<",";
//			}
//			//cout<<endl;
//            iuhf<<endl;
//            gridfile_write_buffer(gfile, (const char *) pTemp, 4 * nTemp);
//            delete pTemp;
//
//            //ostringstream oss;
//            //oss << mint0 << "\t" << maxt0 << "\t";
//            //for (int k = mint0; k <= maxt0; ++k )
//            //	oss << setprecision(8) << uh1[nc][k] << "\t";
//            //oss << "\n";
//
//            //gridfile_write_buffer(gfile, oss.str().c_str(), oss.str().length());
//
//            nc += 1;
//        }
//    }

    //iuhf.close();

//    gridfile_set_metadata(gfile, p);
//    int flag = gridfile_writer_done(gfile);
//    gridfile_destroy(gfile);

//    bson_destroy(p);
//    free(p);
    bson_destroy(&p);

    return 0;
}

double SubbasinIUHCalculator::IUHti(double delta0, double t00, double ti) {
    double tmp1 = 1 / (delta0 * sqrt(2 * 3.1416 * pow(ti, 3.0) / pow(t00, 3.0)));
    double tmp2 = pow(ti - t00, 2.0) / (2.0 * pow(delta0, 2.0) * ti / t00);
    return tmp1 * exp(-tmp2);
}

void SubbasinIUHCalculator::adjustRiceField(int &mint0, int &maxt0, vector<double> &iuhRow) {
    if (maxt0 - mint0 == 0) // if water will flow to channel within one day
    {
        maxt0 = 1;
        iuhRow[0] = 0.6f;  //0.6 and 0.4 are calibrated for youwuzhen, 2013-2015, daily. by lj
        iuhRow[1] = 0.4f;  //must make sure m_iuhCell has at least 4 columns in the readin codes
        //maxt0 = 5;
        //iuhRow[0] = 0.2f;
        //iuhRow[1] = 0.46f;
        //iuhRow[2] = 0.28f;
        //iuhRow[3] = 0.054f;
        //iuhRow[4] = 0.005f;
        //iuhRow[5] = 0.001f;
    } else {
        iuhRow[1] += 0.8f * iuhRow[0];
        iuhRow[0] = 0.2f * iuhRow[0];
    }
}
