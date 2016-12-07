/**
*    @file IUHCalculator.h
*    @Version 1.0
*    @Author  Wu Hui
*    @Date 28-October-2010
*
*    @brief Class of IUH calculation
*
*    Revision:
*    Date:
*/
#pragma once

#include "IUHCalculator.h"

#include "Raster.cpp"
#include "gridfs.h"

class SubbasinIUHCalculator
{
public:
    SubbasinIUHCalculator(int t, Raster<int> &rsMask, Raster<float> &rsLanduse, Raster<float> &rsTime, Raster<float> &rsDelta, gridfs *grdfs);

    virtual ~SubbasinIUHCalculator(void);


private:
    vector<vector<double> > uhCell, uh1;      //IUH from cell to watershed outlet

    int noDataValue;
    int nRows, nCols;    //number of rows and columns
    int dt;              //time interval in hours
    int nCells;          //number of cells

    int **mask;  //value of subwatershed/subbasin
	float **landcover; //landcover map
    float **t0;          //flow time
    float **delta;       //standard deviation of flow time
    gridfs *gfs;

    int mt;          //maximum length of IUH
    int maxtSub;                     //maximum length of subwatershed IUH

    void readData();

    double IUHti(double delta0, double t00, double ti);

public:
    int calCell(int id);
	void adjustRiceField(int& mint0, int& maxt0, vector<double>& iuhRow);
};
