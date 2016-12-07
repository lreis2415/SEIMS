/*!
 * \brief Potential plant transpiration for Priestley-Taylor and Hargreaves ET methods
 * and potential and actual soil evaporation.
 * Code from SWAT, etact.f
 * \author Liang-Jun Zhu
 * \date May 2016
 *
 * \revision: 1.1
 * \date: 2016-7-15
 * \description: 1. Code reformat with common functions, such as Release1DArray.
 *               2. VAR_SNSB should be output other than input.
 *               3. 
 * 
 */
#pragma once

#include <string>
#include "api.h"
#include "util.h"
#include "SimulationModule.h"

using namespace std;
/** \defgroup AET_PTH
 * \ingroup Ecology
 * \brief Potential plant transpiration for Priestley-Taylor and Hargreaves ET methods
 *Actual soil evaporation is also calculated.
 */
/*!
 * \class AET_PT_H
 * \ingroup AET_PTH
 *
 * \brief Potential plant transpiration for Priestley-Taylor and Hargreaves ET methods
 * Actual soil evaporation is also calculated.
 */
class AET_PT_H : public SimulationModule
{
private:
    /// valid cells number
    int m_nCells;
    /// leaf area index(m^2/m^2)
    float *m_lai;
    /// potential evapotranspiration on current day
    float *m_pet;
	/// Evaporation loss from canopy storage
	float *m_canEvp;
    /// depression storage capacity
    float *m_depSt;
    /// soil evaporation compensation factor, if not set or existed, it will be assigned 0.95 as default.
    /// esco should be vary from 0.01 to 1.0
    float *m_esco;
    /// soil layers
    float *m_nSoilLayers;
    /// maximum soil layers, mlyr in SWAT
    int m_soilLayers;
    /// soil depth
    float **m_soilDepth;
	/// soil thickness
	float **m_soilThick;
    /// amount of water available to plants in soil layer at field capacity (FC-WP)
    float **m_solFC;
    /// amount of residue on soil surface (kg/ha)
    float *m_solCov;
    /// amount of nitrogen stored in the nitrate pool
    float **m_solNo3;
    /// mean air temperature (deg C)
    float *m_tMean;
    /// amount of water in snow on current day
    float *m_snowAcc;
    /// snow sublimation on current day
    float *m_snowSB;
    /// soil storage of each soil layer, mm H2O
    float **m_soilStorage;
	/// soil water storage in soil profile (mm)
	float *m_soilStorageProfile;
    /// add output variables

    /// maximum amount of transpiration (plant et)  that can occur on current day in HRU, ep_max in SWAT
    float *m_ppt;
    /// actual amount of evaporation (soil et) that occurs on day, es_day in SWAT
    float *m_soilESDay;
    /// amount of nitrate moving upward in the soil profile in watershed
    float m_no3Up;

public:
    //! Constructor
    AET_PT_H(void);

    //! Destructor
    ~AET_PT_H(void);

    virtual int Execute();

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

    virtual void Set2DData(const char *key, int n, int col, float **data);

    virtual void GetValue(const char *key, float *value);

private:
    /*!
     * \brief check the input data. Make sure all the input data is available.
     * \return bool The validity of the input data.
     */
    bool CheckInputData(void);

    /*!
     * \brief check the input size. Make sure all the input data have same dimension.
     *
     *
     * \param[in] key The key of the input data
     * \param[in] n The input data dimension
     * \return bool The validity of the dimension
     */
    bool CheckInputSize(const char *, int);

	//! initialize outputs
	void initialOutputs();
};

