#ifndef SEIMS_MODULE_PMELT_HBV_H
#define SEIMS_MODULE_PMELT_HBV_H

#include "SimulationModule.h"
#include "basic.h"

class PMELT_HBV : public SimulationModule {
public:
    PMELT_HBV(); //! Constructor
    ~PMELT_HBV(); //! Destructor

    ///////////// SetData series functions /////////////

    void SetValue(const char* key, FLTPT value) OVERRIDE;
    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;

    ///////////// GetData series functions /////////////
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
    FLTPT* m_t_mean; ///< daily average temperature

    //parameters
    FLTPT* m_lat; ///< Latitude
    FLTPT* m_melt_factor; ///< Melt factor
    FLTPT* m_melt_factor_min; ///< Melt factor min
    FLTPT m_aspect_corr; ///< Aspect correction
    FLTPT* m_forest_corr; ///< Forest correction
    FLTPT* m_forest_cov; ///< coverage
    FLTPT* m_landuse; ///< Landuse
    FLTPT m_melt_temperature; ///< Melt temperature

    FLTPT* m_slope{}; ///< Slope
    FLTPT* m_aspect{}; ///< Aspect

    //output variables
    FLTPT* m_potentialMelt; ///< Potential Melt
};

#endif /* SEIMS_MODULE_PMELT_HBV_H */
