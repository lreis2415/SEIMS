/*!
 * \file SnowRefreeze.h
 * \brief The method of Potential Melt by degree-day method.
 * \author Yujing Wang
 * \date 2023-11-03
 *
 */
#ifndef SEIMS_MODULE_SNOW_REFREEZE_H
#define SEIMS_MODULE_SNOW_REFREEZE_H

#include "SimulationModule.h"
#include "basic.h"

class SnowRefreeze : public SimulationModule {
public:
    SnowRefreeze(); //! Constructor
    ~SnowRefreeze(); //! Destructor

    ///////////// SetData series functions /////////////
    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;

    ///////////// GetData series functions /////////////
    void Get1DData(const char* key, int* n, FLTPT** data) OVERRIDE;

    ///////////// CheckInputData and InitialOutputs /////////////
    bool CheckInputData() OVERRIDE;
    bool CheckInputSize(const char* key, int n);
    void InitialOutputs() OVERRIDE;

    ///////////// Main control structure of execution code /////////////
    int Execute() OVERRIDE;

private:
    int m_nCells; ///< valid cells number

    //input variables
    FLTPT* m_snowLiq;

    //output variables
    FLTPT* m_snowAcc; ///< Potential Melt
};

#endif /* SEIMS_MODULE_SnowRefreeze_H */
