/*!
 * \file CanopyEvaporation.h
 * \brief Canopy Evaporation calculation.
 * \author Your Name
 * \date 2023-11-03
 *
 */
#ifndef SEIMS_MODULE_CAN_EVAP_H
#define SEIMS_MODULE_CAN_EVAP_H

#include "SimulationModule.h"

class CanopyEvaporation : public SimulationModule {
public:
    CanopyEvaporation(); //! Constructor
    ~CanopyEvaporation(); //! Destructor

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
    FLTPT* m_canopyStorage;

    //output variables
    FLTPT* m_canopyEvaporation; ///< Canopy Evaporation
};

#endif /* SEIMS_MODULE_CAN_EVAP_H */