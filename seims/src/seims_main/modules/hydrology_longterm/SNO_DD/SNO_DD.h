/*!
 * \file SNO_DD.h
 * \brief
 * \author Chunping Ou
 * \date May 2011
 * \revised LiangJun Zhu
 * \date 2016-7-13
 *  1. Remove m_isInitial and add initialOutputs()
 *  2. Add VAR_SWE as output, so this module will be not dependent on others
 *  3. Add net precipitation as output, and added water amount of snow melt
 * 
 */
#pragma once

#include <string>
#include <ctime>
#include "api.h"

using namespace std;

#include "SimulationModule.h"
/** \defgroup SNO_DD
 * \ingroup Hydrology_longterm
 * \brief Calculate snow melt by Degree-Day method
 *
 */

/*!
 * \class SNO_DD
 * \ingroup SNO_DD
 * \brief Calculate snow melt by Degree-Day method
 * 
 */
class SNO_DD : public SimulationModule
{
public:
    //! Constructor
    SNO_DD(void);

    //! Destructor
    ~SNO_DD(void);

    virtual int Execute();

    virtual void SetValue(const char *key, float data);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

    bool CheckInputSize(const char *key, int n);

	bool CheckInputData(void);

	void initialOutputs();

private:
    //! Valid cells number
    int m_nCells;
	//! Mean air temperature at which snow melt will occur, sub_smtmp
	float m_t0;
	//! fraction coefficient of precipitation as snow
	float m_kblow;
	//! Snowfall temperature, i.e., precipitation as snow
	float m_tsnow;
	//! temperature impact factor
    float m_csnow;
	//! Rainfall impact factor
    float m_crain;

	//float m_swe0;
	//float m_swe;
	//float m_lastSWE;
	
	//! mean temperature
    float *m_tMean;
	//! net precipitation
    float *m_Pnet;

	//! snow redistribution
	float *m_SR;
	//! snow sublimation, snoev in SWAT in etact.f
	float *m_SE;

    //result
    /// Snow melt
    float *m_SM;
    /// Snow accumulation
    float *m_SA;
};

