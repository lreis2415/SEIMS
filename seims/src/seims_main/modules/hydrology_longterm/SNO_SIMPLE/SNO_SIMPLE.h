/*!
 * \file SNO_SIMPLE.h
 * \brief Calculate snow melt directly from potential melt.
 * \author Yujing Wang
 * \date 2023-11-11
 *
 */
#ifndef SEIMS_MODULE_SNO_SIMPLE_H
#define SEIMS_MODULE_SNO_SIMPLE_H

#include "SimulationModule.h"

class SNO_SIMPLE : public SimulationModule {
public:
    SNO_SIMPLE(); //! Constructor
    ~SNO_SIMPLE(); //! Destructor

    ///////////// SetData series functions /////////////
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
    FLTPT* m_snowAcc; ///< Snow Melt

    //output variables
    FLTPT* m_snowMelt; ///< Snow Melt
};

#endif /* SEIMS_MODULE_SNOW_MELT_SIMPLE_H */
