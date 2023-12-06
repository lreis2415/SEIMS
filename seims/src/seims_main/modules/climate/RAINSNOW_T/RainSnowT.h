#ifndef SEIMS_MODULE_RAINSNOW_T_H
#define SEIMS_MODULE_RAINSNOW_T_H

#include "SimulationModule.h"

class RainSnowT : public SimulationModule {
public:
    RainSnowT(); //! Constructor
    ~RainSnowT(); //! Destructor

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
    FLTPT* m_pcp; ///< 

    //parameters
    FLTPT m_t_rain_snow; ///< Temperature for snowfall

    //output variables
    FLTPT* m_snowfall; ///< Snow liquid
};

#endif /* SEIMS_MODULE_RAINSNOW_T_H */
