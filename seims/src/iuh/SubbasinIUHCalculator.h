/**
*    @file IUHCalculator.h
*    @Version 1.0
*    @Author  Wu Hui
*    @Date 28-October-2010
*
*    @brief Class of Subbasin IUH calculation
*
*    Revision: Liangjun Zhu
*    Date: 6-May-2018
*/
#ifndef IUH_SUBBASIN_CALCULATOR_H
#define IUH_SUBBASIN_CALCULATOR_H

#include "db_mongoc.h"
#include "data_raster.h"

using namespace ccgl;
using namespace db_mongoc;
using namespace data_raster;

class SubbasinIUHCalculator: Interface {
public:
    SubbasinIUHCalculator(int t, clsRasterData<int>& rsMask, clsRasterData<float>& rsLanduse,
                          clsRasterData<float>& rsTime, clsRasterData<float>& rsDelta, MongoGridFs* grdfs);

private:
    vector<vector<double> > uhCell, uh1; //IUH from cell to watershed outlet

    int noDataValue;
    int nRows, nCols; //number of rows and columns
    int dt;           //time interval in hours
    int nCells;       //number of cells

    int* mask;        //value of subwatershed/subbasin
    float* landcover; //landcover map
    float* t0;        //flow time
    float* delta;     //standard deviation of flow time
    MongoGridFs* gfs;

    int mt;      //maximum length of IUH
    int maxtSub; //maximum length of subwatershed IUH

    //    void readData();

    double IUHti(double delta0, double t00, double ti);

public:
    int calCell(int id);

    static void adjustRiceField(int& mint0, int& maxt0, vector<double>& iuhRow);
};

#endif /* IUH_SUBBASIN_CALCULATOR_H */
