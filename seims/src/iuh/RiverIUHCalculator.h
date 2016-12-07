#pragma once

#include "iuhcalculator.h"

class RiverIUHCalculator :
        public IUHCalculator
{
public:
    RiverIUHCalculator(string inputfile, string watershedfile,
                       string stream_networkfile, string t0file, string deltafile,
                       string uhfile);

    ~RiverIUHCalculator(void);

    virtual int calIUH(void);

    int get_New_T0andDelta(void);

protected:
    virtual void readData();

private:
    string uhFile;
    vector<int> linkCellNum;
    vector<vector<double> > uhCell, uhRiver, uhSub;

    vector<vector<double> > t0_new, t0_h_network, temp_t0;          //new flow time =t0_reach_cell - t0_reach_outlet
    vector<vector<double> > delta_new, delta_h_network, temp_d;       //new standard deviation of flow time = delta_reach_cell - delta_reach_outlet
};
