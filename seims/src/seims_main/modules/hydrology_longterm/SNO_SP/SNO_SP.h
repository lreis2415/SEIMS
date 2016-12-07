/*!
 * \file SNO_SP.h
 * \brief
 * \author Zhiqiang Yu
 * \date May 2011
 * \revised LiangJun Zhu
 * \date 2016-5-29
 *  1. Remove m_isInitial and add initialOutputs()
 *  2. Add m_snowCoverMax and m_snowCover50 to adjust for areal extent of snow cover.
 *  3. ReWrite the execute code according to snom.f of SWAT.
 *  4. In this version, snow melt is added to net precipitation.
 * 
 */
#pragma once

#include <string>
#include <ctime>
#include "api.h"

using namespace std;

#include "SimulationModule.h"
/** \defgroup SNO_SP
 * \ingroup Hydrology_longterm
 * \brief Calculate snow melt by snowpack daily method from SWAT
 *
 */

/*!
 * \class SNO_SP
 * \ingroup SNO_SP
 * \brief Calculate snow melt by snowpack daily method from SWAT
 * 
 */
class SNO_SP : public SimulationModule
{
public:
    //! Constructor
    SNO_SP(void);

    //! Destructor
    ~SNO_SP(void);

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
	//! Initial snow water equivalent
    //float m_swe0;

	/*Snow pack temperature lag factor (0-1), sub_timp in SWAT
	* 1 = no lag (snow pack temp=current day air temp) 
	* as the lag factor goes to zero, the snow pack's 
	* temperature will be less influenced by the current day's 
	* air temperature
	*/
    float m_lagSnow;
	//! Maximum melt rate for snow during year, sub_smfmx
    float m_csnow6;
	//! Minimum melt rate for snow during year, sub_smfmn
    float m_csnow12;
	//! Minimum snow water content that corresponds to 100% snow cover, mm H2O, SNOCOVMX
	float m_snowCoverMax;
	//! Fraction of SNOCOVMX that corresponds to 50% snow cover, SNO50COV
	float m_snowCover50;
	//! 1st shape parameter for snow cover equation
	float m_snowCoverCoef1;
	//! 2nd shape parameter for snow cover equation
	float m_snowCoverCoef2;

	//! average snow accumulation of the watershed
    //float m_swe;
	//! snow water equivalent of last time step
    //float m_lastSWE;

	//! Mean temperature
    float *m_tMean;
	//! Max temperature
    float *m_tMax;
	//! Net precipitation
    float *m_Pnet;
	
	//! snow redistribution
    float *m_SR;
	//! snow sublimation, snoev in SWAT in etact.f
    float *m_SE;
	

    //! temperature of snow pack, snotmp in SWAT
    float *m_packT;

	/// outputs

    //! amount of water in snow melt, snomlt in SWAT
    float *m_SM;
	//! snow accumulation, sno_hru in SWAT
    float *m_SA;
};

