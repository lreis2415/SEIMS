#ifndef SEIMS_HYDRO_VARS_H
#define SEIMS_HYDRO_VARS_H

#include "SimulationModule.h"

// using namespace std;  // Avoid this statement! by lj.

class HydroVarsUpdate : public SimulationModule {
private:

    int m_nCells;

    //! number of subbasins
    int m_nSubbasins;
    /// current subbasin ID, 0 for the entire watershed
    int m_inputSubbsnId;
    //! subbasin IDs
    vector<int> m_subbasinIds;
    //! All subbasins information
    clsSubbasins* m_subbasins;
    /// subbasin grid (subbasins ID)
    int* m_cellsMappingToSubbasinId;

    FLTPT* m_pet;

    FLTPT* m_petSubbsn;
    FLTPT* m_gwSto;

public:
    HydroVarsUpdate();
    ~HydroVarsUpdate();

    void InitialOutputs() OVERRIDE;

    // void Set1DData(const char *key, int nRows, FLTPT *data) OVERRIDE;
    // void SetValue(const char *key, FLTPT value) OVERRIDE;
    void SetValue(const char *key, int value) OVERRIDE;
    void SetSubbasins(clsSubbasins* subbsns) OVERRIDE;
    void Set1DData(const char* key, int n, int* data) OVERRIDE;
    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;
    void Get1DData(const char *key, int *nRows, FLTPT **data) OVERRIDE;

    int Execute(void) OVERRIDE;
};
#endif /* SEIMS_HYDRO_VARS_H */
