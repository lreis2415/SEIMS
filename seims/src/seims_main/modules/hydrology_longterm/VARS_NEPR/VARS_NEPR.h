#ifndef SEIMS_HYDRO_NEPR_H
#define SEIMS_HYDRO_NEPR_H

#include "SimulationModule.h"

// using namespace std;  // Avoid this statement! by lj.

class VARS_NEPR : public SimulationModule {
private:

    int m_nCells;

    FLTPT* m_NEPR;
    FLTPT* m_DPST;
    FLTPT* m_SNME;
    FLTPT* m_GREL;

public:
    VARS_NEPR();
    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;
    bool CheckInputSize(const char* key, int n);

    int Execute(void) OVERRIDE;
};
#endif /* SEIMS_HYDRO_NEPR_H */
