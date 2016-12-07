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

#include <string>
#include <vector>

using namespace std;

#define IUHZERO 0.000000001

/** 
*    @brief This class is the base class of IUH calculation.
*/
class IUHCalculator
{
public:
    IUHCalculator(string inputfile, string watershedfile,
                  string stream_networkfile, string t0file, string deltafile);

    virtual ~IUHCalculator(void);

/**
     @brief Calculate IUH
*/
    virtual int calIUH(void) = 0;

    void setDt(int t) { dt = t; }

protected:
    string inputFile;
    string watershedFile;    // = subbasin file
    string strnetworkFile;
    string t0File;
    string deltaFile;
    //string runoffCoFile;   // used to calculate subbasin IUH

    int mt;          //maximum length of IUH
    int nRows, nCols;    //number of rows and columns
    int dt;              //time interval in hours
    int nSubs;           //number of sub-watersheds
    int nCells;          //number of cells
    int nLinks;          //number of links

    vector<vector<int> > watershed;  //value of subwatershed/subbasin
    vector<vector<int> > strnetwork;  // value of stream network, used to define stream link
    vector<vector<int> > link;   //calculated from watershed and stream network
    vector<vector<double> > t0;          //flow time
    vector<vector<double> > delta;       //standard deviation of flow time
    //vector< vector<double> > runoffCo;    //potential runoff coefficient

/**
     @brief Read data 
*/
    virtual void readData();

/**
     @brief Calculate IUH for time step ti

	 @param double delta0:standard deviation of t00
	 @param double t00:flow time to the watershed outlet from each grid cell
	 @param double ti: time step, eg. hourly
*/
    double IUHti(double delta0, double t00, double ti);  //calculate IUH for time step ti
};
