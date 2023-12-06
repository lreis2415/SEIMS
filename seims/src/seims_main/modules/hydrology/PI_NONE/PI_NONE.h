
#ifndef SEIMS_MODULE_PI_NONE_H
#define SEIMS_MODULE_PI_NONE_H

#include "SimulationModule.h"

class PI_NONE: public SimulationModule {
public:
    PI_NONE();

    ~PI_NONE();

    void Set1DData(const char* key, int nRows, FLTPT* data) OVERRIDE;
    
    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void Get1DData(const char* key, int* nrows, FLTPT** data) OVERRIDE;

private:
    //!  number of valid cells
    int m_nCells;

    FLTPT* m_pcp;
    FLTPT* m_snowfall;

    FLTPT* m_netPcp;
    FLTPT* m_snowAcc;

};
#endif /* SEIMS_MODULE_PI_NONE_H */
