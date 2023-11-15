#ifndef SEIMS_MODULE_GMELT_SIMPLE_H
#define SEIMS_MODULE_GMELT_SIMPLE_H

#include "SimulationModule.h"
#include "basic.h"

class GMELT_SIMPLE : public SimulationModule {
public:
    GMELT_SIMPLE(); //! Constructor
    ~GMELT_SIMPLE(); //! Destructor

    ///////////// SetData series functions /////////////
    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;
    void Set1DData(const char* key, int n, int* data) OVERRIDE;

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
    FLTPT* m_potentialMelt; ///< Potential Melt
    FLTPT* m_excessPcp; ///< Excess Precipitation

    int* m_landuse; ///< Landuse

    //output variables
    FLTPT* m_glacMelt;
};

#endif /* SEIMS_MODULE_GMELT_HBV_H */
