/*!
 * \file CanopySublimation.h
 * \brief Canopy Sublimation calculation.
 * \author Your Name
 * \date 2023-11-03
 *
 */
#ifndef SEIMS_MODULE_CAN_SUBLIMATION_H
#define SEIMS_MODULE_CAN_SUBLIMATION_H

#include "SimulationModule.h"

class CanopySublimation : public SimulationModule {
public:
    CanopySublimation(); //! Constructor
    ~CanopySublimation(); //! Destructor

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
    FLTPT* m_canopySnowStorage;

    //output variables
    FLTPT* m_canopySublimation; ///< Canopy Sublimation
};

#endif /* SEIMS_MODULE_CAN_SUBLIMATION_H */
