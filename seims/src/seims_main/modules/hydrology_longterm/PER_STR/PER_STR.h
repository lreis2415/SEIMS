/*!
 * \brief Percolation calculated by storage routing method
 * reference SWAT theory manual, p151-152
 * \author Junzhi Liu
 * \date May 2011
 * \revised LiangJun Zhu
 * \date 2016-5-29
 * \date 2016-9-8
 * \description: 1. ReWrite according to percmain.f and sat_excess.f of SWAT
 */
#pragma once

#include "api.h"
#include "SimulationModule.h"
#include <string>
#include <vector>
#include <string>
#include <sstream>

using namespace std;
/** \defgroup PER_STR
 * \ingroup Hydrology_longterm
 * \brief Percolation calculated by storage routing method.
 *
 */

/*!
 * \class PER_STR
 * \ingroup PER_STR
 * \brief Percolation calculated by storage routing method.
 * 
 */
class PER_STR : public SimulationModule
{
private:
    /// number of soil layers
    int m_nSoilLayers;
	/// soil layers
	float *m_soilLayers;
    /// soil thickness
    float **m_soilThick;
    /// time step
    int m_dt;
    /// valid cells number
    int m_nCells;
    /// threshold soil freezing temperature
    float m_frozenT;
    /// saturated conductivity
    float **m_ks;
	/// amount of water held in the soil layer at saturation (sat - wp water), mm
	float **m_sat;    
	/// amount of water held in the soil layer at field capacity (fc - wp water) mm H2O
	float **m_fc;
    /// soil moisture, mm H2O
    float **m_soilStorage;
	/// amount of water stored in soil profile on current day, sol_sw in SWAT
	float *m_soilStorageProfile;
    /// soil temperature
    float *m_soilT;
    /// infiltration, mm
    float *m_infil;
	/// surface runoff, mm
	float *m_surfQmm;
	/// pothole volume, mm
	float *m_potVol;
    /// Output: percolation
    float **m_perc;

public:
    /// Constructor
    PER_STR(void);

    /// Destructor
    ~PER_STR(void);

    virtual int Execute();

    virtual void SetValue(const char *key, float data);

    virtual void Set1DData(const char *key, int nRows, float *data);

    virtual void Set2DData(const char *key, int nrows, int ncols, float **data);

    virtual void Get2DData(const char *key, int *nRows, int *nCols, float ***data);

private:
    /**
    *	@brief check the input data. Make sure all the input data is available.
    *
    *	@return bool The validity of the input data.
    */
    bool CheckInputData(void);

    /**
    *	@brief check the input size. Make sure all the input data have same dimension.
    *
    *	@param key The key of the input data
    *	@param n The input data dimension
    *	@return bool The validity of the dimension
    */
    bool CheckInputSize(const char *, int);

	void initialOutputs();
};

