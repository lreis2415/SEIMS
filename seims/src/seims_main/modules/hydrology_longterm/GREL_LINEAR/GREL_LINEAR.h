#ifndef SEIMS_MODULE_GREL_LINEAR_H
#define SEIMS_MODULE_GREL_LINEAR_H

#include "SimulationModule.h"
#include "basic.h"

class GREL_LINEAR : public SimulationModule {
public:
    GREL_LINEAR(); //! Constructor
    ~GREL_LINEAR(); //! Destructor

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
    FLTPT* m_glacMelt; ///< Potential Melt

    //parameters
    FLTPT* m_landuse;
    FLTPT m_glacStorageCoef; ///< Glacier storage coefficient

    //output variables
    FLTPT* m_glacRelease; ///< released glacier melt, to join the runoff
};

#endif /* SEIMS_MODULE_GREL_LINEAR_H */
