#ifndef SEIMS_MODULE_GMELT_HBV_H
#define SEIMS_MODULE_GMELT_HBV_H

#include "SimulationModule.h"
#include "basic.h"

class GMELT_HBV : public SimulationModule {
public:
    GMELT_HBV(); //! Constructor
    ~GMELT_HBV(); //! Destructor

    ///////////// SetData series functions /////////////
    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;
    void Set1DData(const char* key, int n, int* data) OVERRIDE;

    ///////////// GetData series functions /////////////
    void SetValue(const char* key, FLTPT value) OVERRIDE;
    void Get1DData(const char* key, int* n, FLTPT** data) OVERRIDE;

    ///////////// CheckInputData and InitialOutputs /////////////
    bool CheckInputData() OVERRIDE;
    void InitialOutputs() OVERRIDE;
    bool CheckInputSize(const char* key, int n);

    ///////////// Main control structure of execution code /////////////
    int Execute() OVERRIDE;

private:
    int m_nCells; ///< valid cells number

    //input variables
    FLTPT* m_potentialMelt; ///< Potential Melt
    FLTPT* m_snowLiq; ///< liquid snow

    int* m_landuse; ///< Landuse
    FLTPT m_hbv_glacier_melt_correction; ///< HBV glacier melt correction

    //output variables
    FLTPT* m_glacierMelt;
};

#endif /* SEIMS_MODULE_GMELT_HBV_H */
