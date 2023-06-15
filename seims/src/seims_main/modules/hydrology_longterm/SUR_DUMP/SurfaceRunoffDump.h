/*!
 * \file SurfaceRunoffDump.h
 * \brief 
 * \author Yujing Wang
 * \date 2023-06-15
 *
 */
#ifndef SEIMS_SURFACE_RUNOFF_DUMP_H
#define SEIMS_SURFACE_RUNOFF_DUMP_H

#include "SimulationModule.h"

class SurfaceRunoffDump : public SimulationModule {
public:
    SurfaceRunoffDump(); //! Constructor
    ~SurfaceRunoffDump(); //! Destructor

    ///////////// SetData series functions /////////////

    void SetValue(const char* key, int value) OVERRIDE;
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
    FLTPT* m_pcp; ///< precipitation

    //output variables
    FLTPT* m_surfaceRunoff;


};

#endif
