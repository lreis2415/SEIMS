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

class WatershedIUHCalculator : public IUHCalculator
{
public:
    WatershedIUHCalculator(string inputfile, string watershedfile,
                           string stream_networkfile, string t0file, string deltafile,
                           string runoffcofile,
                           string uhcellfile, string uhsubfile, string uhwatershedfile = "");

    WatershedIUHCalculator(string inputfile, string watershedfile,
                           string stream_networkfile, string t0file, string deltafile,
                           string runoffcofile,
                           string uhwatershedfile = "");

    virtual ~WatershedIUHCalculator(void);


private:
    string runoffCoFile;
    string uhCellFile, uhSubFile, uhWatershedFile;

    vector<double> runoffSub;             //subwatershed runoff coefficient
    vector<vector<double> > runoffCo;    //potential runoff coefficient

    vector<vector<double> > uhCell, uh1;      //IUH from cell to watershed outlet
    vector<vector<double> > uhSub, uh2;       //IUH from each subwatershed
    vector<double> uhWatershed, uh3; //IUH of watershed

    int maxtSub;                     //maximum length of subwatershed IUH
    double sumRunCo;

protected:
    virtual void readData();

private:
    int calCell(void);

    int calSub(void);

    int calWatershed(void);

public:
    virtual int calIUH(void);

    int calIUHSubwatershed(void);

    int calIUHCell(void);
};
