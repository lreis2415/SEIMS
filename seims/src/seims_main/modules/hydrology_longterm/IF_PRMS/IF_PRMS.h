/*!
 * \file IF_PRMS.h
 * \author Your Name
 * \date 2023-11-03
 *
 */
#ifndef SEIMS_MODULE_IF_PRMS_H
#define SEIMS_MODULE_IF_PRMS_H

#include "SimulationModule.h"

class IF_PRMS : public SimulationModule {
public:
    IF_PRMS(); //! Constructor
    ~IF_PRMS(); //! Destructor

    ///////////// SetData series functions /////////////
    void Set1DData(const char* key, int n, int* data) OVERRIDE;
    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;
    void SetValue(const char* key, int value) OVERRIDE;
    void Set2DData(const char* key, const int nrows, const int ncols, FLTPT** data) OVERRIDE;
    ///////////// GetData series functions /////////////
    void Get1DData(const char* key, int* n, FLTPT** data) OVERRIDE;

    ///////////// CheckInputData and InitialOutputs /////////////
    bool CheckInputData() OVERRIDE;
    void InitialOutputs() OVERRIDE;
    bool CheckInputSize(const char* key, int n);

    ///////////// Main control structure of execution code /////////////
    int Execute() OVERRIDE;

private:
    bool m_isInitialized;

    int m_nCells;

    //! number of subbasins
    int m_nSubbasins;
    //! subbasin IDs
    vector<int> m_subbasinIDs;
    //! All subbasins information
    clsSubbasins* m_subbasins;
    /// subbasin grid (subbasins ID)
    int* m_cellsMappingToSubbasinId;
    
    FLTPT* m_cellArea;
    int* m_soilLayers;

    int m_maxSoilLyrs;

    FLTPT** m_maxIfRate;
    FLTPT** m_soilThickness;
    FLTPT** m_porosity;
    FLTPT** m_awcAmount;
    FLTPT** m_soilWaterStorage;

    FLTPT* m_Q_SBIF;
};

#endif /* SEIMS_MODULE_IF_PRMS_H */
