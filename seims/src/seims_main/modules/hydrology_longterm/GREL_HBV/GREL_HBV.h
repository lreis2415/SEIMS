#ifndef SEIMS_MODULE_GREL_HBV_H
#define SEIMS_MODULE_GREL_HBV_H

#include "SimulationModule.h"
#include "basic.h"

class GREL_HBV : public SimulationModule {
public:
    GREL_HBV(); //! Constructor
    ~GREL_HBV(); //! Destructor

    ///////////// SetData series functions /////////////
    void SetValue(const char* key, FLTPT value) OVERRIDE;
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

    //input
    FLTPT* m_glacMelt; ///< melted glacier
    FLTPT* m_snowAcc; ///< Snow accumulation
    FLTPT* m_snowLiq; ///< Snow liquid water content

    //parameters
    int* m_landuse; ///< Landuse
    FLTPT m_glacStorageCoef; ///< Glacier storage coefficient
    FLTPT m_kMin; ///< Minimum glacier release coefficient
    FLTPT m_ag; ///< extinction coefficient for diminishing storage coefficient

    //output variables
    FLTPT* m_glacRelease; ///< released glacier melt, to join the runoff



};

#endif /* SEIMS_MODULE_GREL_HBV_H */
