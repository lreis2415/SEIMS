/*!
 * \file DepressionFS.h
 * \brief A simple fill and spill method method to calculate depression storage
 * \author Junzhi Liu
 * \date Feb. 2011
 * \revised	Zhiqiang Yu	
 * \date		2011-2-15
 *
 * \note 1.	Modify the name of some parameters, input and output variables.
 *		Please see the metadata rules for the names.
 *	2.	Depre_in would be DT_Single. Add function SetSingleData() to 
 *		set its value.
 *	3.	This module will be called by infiltration module to get the 
 *		depression storage. And this module will also use the outputs
 *		of infiltration module. The sequence of this two modules is 
 *		infiltration->depression. When infiltration first calls the 
 *		depression module, the execute function of depression module
 *		is not executed before getting the outputs. So, the output 
 *		variables should be initial in the Get1DData function. This 
 *		initialization is realized by function initalOutputs. 
*/

#pragma once

#include <string>
#include <ctime>
#include "api.h"
#include "SimulationModule.h"

using namespace std;
/** \defgroup DEP_FS
 * \ingroup Hydrology
 * \brief A simple fill and spill method method to calculate depression storage
 */
/*!
 * \class DepressionFS
 * \ingroup DEP_FS
 *
 * \brief A simple fill and spill method method to calculate depression storage
 *
 */
class DepressionFS : public SimulationModule
{
public:
    //! Constructor
    DepressionFS(void);

    //! Destructor
    ~DepressionFS(void);

    virtual int Execute();

    virtual void SetValue(const char *key, float data);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

private:
    bool CheckInputSize(const char *key, int n);

    bool CheckInputData(void);

    /// valid cells number
    int m_nCells;

    /// initial depression storage coefficient
    float m_depCo;
    /// depression storage capacity
    float *m_depCap;

    /// pet
    float *m_pet;
    /// evaporation from the interception storage
    float *m_ei;

    // state variables (output)
    /// depression storage
    float *m_sd;
    /// surface runoff
    float *m_sr;
    /// surplus of storage capacity
    float *m_storageCapSurplus;

    void initialOutputs();

    /// whether check inputs, TODO Is it useless? By LJ
    bool m_checkInput;
};

