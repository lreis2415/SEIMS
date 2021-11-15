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
#include "data_raster.hpp"

using namespace ccgl;
using namespace db_mongoc;
using namespace data_raster;

#ifndef IntRaster
#define IntRaster   clsRasterData<int>
#endif
#ifndef FloatRaster
#define FloatRaster clsRasterData<float>
#endif
#ifndef FloatMaskedRaster
#define FloatMaskedRaster clsRasterData<float, int>
#endif
#ifndef FltMaskFltRaster
#define FltMaskFltRaster clsRasterData<float, float>
#endif


class SubbasinIUHCalculator: Interface {
public:
    SubbasinIUHCalculator(int t, FloatRaster* rsMask, FltMaskFltRaster* rsLanduse,
                          FltMaskFltRaster* rsTime, FltMaskFltRaster* rsDelta, MongoGridFs* grdfs);

private:
    vector<vector<double> > uhCell, uh1; //IUH from cell to watershed outlet

    float noDataValue;
    int nRows, nCols; //number of rows and columns
    int dt;           //time interval in hours
    int nCells;       //number of cells

    float* mask;      //value of subwatershed/subbasin
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
