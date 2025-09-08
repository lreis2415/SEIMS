/*!
 * \file template.h
 * \brief Brief description of this module
 *        Detail description about the implementation.
 * \author Liangjun Zhu
 * \date 2018-02-07
 *
 * Changelog:
 *   - 1. 2018-02-07 - lj - Initial implementaition
 *   - 2. 2019-01-30 - lj - Add (or update) all available APIs
 */
#ifndef SEIMS_MODULE_TEMPLATE_H
#define SEIMS_MODULE_TEMPLATE_H

#include "SimulationModule.h"

// using namespace std;  // Avoid this statement! by lj.

class ModuleTemplate: public SimulationModule {
public:
    ModuleTemplate(); //! Constructor

    ~ModuleTemplate(); //! Destructor

    ///////////// SetData series functions /////////////

    void SetValue(const char* key, int value) OVERRIDE;

    void SetValue(const char* key, FLTPT value) OVERRIDE;

    void SetValueByIndex(const char* key, int index, int value) OVERRIDE;

    void SetValueByIndex(const char* key, int index, FLTPT value) OVERRIDE;

    void Set1DData(const char* key, int n, int* data) OVERRIDE;

    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;

    void Set2DData(const char* key, int n, int col, int** data) OVERRIDE;

    void Set2DData(const char* key, int n, int col, FLTPT** data) OVERRIDE;

    void SetReaches(clsReaches* rches) OVERRIDE;

    void SetSubbasins(clsSubbasins* subbsns) OVERRIDE;

    void SetScenario(Scenario* sce) OVERRIDE;

    ///////////// CheckInputData and InitialOutputs /////////////

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    ///////////// Main control structure of execution code /////////////

    int Execute() OVERRIDE;

    ///////////// GetData series functions /////////////

    TimeStepType GetTimeStepType() OVERRIDE;

    void GetValue(const char* key, int* value) OVERRIDE;

    void GetValue(const char* key, FLTPT* value) OVERRIDE;

    void Get1DData(const char* key, int* n, int** data) OVERRIDE;

    void Get1DData(const char* key, int* n, FLTPT** data) OVERRIDE;

    void Get2DData(const char* key, int* n, int* col, int*** data) OVERRIDE;

    void Get2DData(const char* key, int* n, int* col, FLTPT*** data) OVERRIDE;

private:
    int m_nCells; ///< valid cells number
};

#endif /* SEIMS_MODULE_TEMPLATE_H */
