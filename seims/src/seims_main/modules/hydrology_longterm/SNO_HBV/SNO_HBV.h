/*!
 * \file SNO_HBV.h
 * \brief Calculate snow melt directly from potential melt.
 * \author Yujing Wang
 * \date 2023-11-11
 *
 */
#ifndef SEIMS_MODULE_SNO_HBV_H
#define SEIMS_MODULE_SNO_HBV_H

#include "SimulationModule.h"

class SNO_HBV : public SimulationModule {
public:
    SNO_HBV(); //! Constructor
    ~SNO_HBV(); //! Destructor

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
    FLTPT* m_potentialMelt;
    FLTPT* m_snowfall;
    FLTPT* m_tMean; ///< daily average temperature

    //parameters
    FLTPT m_dt_day;             ///< time step (days)
    FLTPT* m_snowRefreezeFactor; ///< Snow refreeze factor
    FLTPT m_tMelt; ///< Melt temperature
    FLTPT m_swi; ///< Maximum liquid water content of snow

    //intermediate variables
    FLTPT* m_snowAcc; ///< Snow accumulation

    //output variables
    FLTPT* m_snowMelt; ///< Snow Melt
};

#endif /* SEIMS_MODULE_SNOW_MELT_SIMPLE_H */
