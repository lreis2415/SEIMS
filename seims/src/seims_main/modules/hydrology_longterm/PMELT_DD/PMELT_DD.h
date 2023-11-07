/*!
 * \file PMELT_DD.h
 * \brief The method of Potential Melt by degree-day method.
 * \author Yujing Wang
 * \date 2023-11-03
 *
 */
#ifndef SEIMS_MODULE_PMELT_DD_H
#define SEIMS_MODULE_PMELT_DD_H

#include "SimulationModule.h"

class PMELT_DD : public SimulationModule {
public:
    PMELT_DD(); //! Constructor
    ~PMELT_DD(); //! Destructor

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
    FLTPT m_Ma; ///< Melt factor
    FLTPT m_t_melt; ///< Melt temperature
    // FLTPT m_subdaily_corr; ///< Subdaily correction factor

    //output variables
    FLTPT* m_potentialMelt; ///< Potential Melt
};

#endif /* SEIMS_MODULE_PMELT_DD_H */