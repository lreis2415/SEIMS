/*!
 * \brief Percolation calculated by Darcy's law and Brooks-Corey equation.
 * \author Junzhi Liu
 * \date May 2011
 * \revised LiangJun Zhu
 * \date 2016-5-29
 */
#pragma once

#include "api.h"
#include "SimulationModule.h"
#include <string>
#include <vector>
#include <string>
#include <sstream>

using namespace std;
/** \defgroup PER_PI
 * \ingroup Hydrology_longterm
 * \brief Calculate the amount of water percolated from the soil water reservoir
 *
 */

/*!
 * \class PER_PI
 * \ingroup PER_PI
 * \brief Calculate water percolated from the soil water reservoir
 * 
 */
class PER_PI : public SimulationModule
{
private:
    /// maximum number of soil layers
    int m_soilLayers;
	/// soil layers
	float *m_nSoilLayers;
    ///// soil depth
    //float **m_soilDepth;
	/// soil thickness
	float **m_soilThick;
    ///// depth of the up soil layer
    //float *m_upSoilDepth;
 
    /// time step
    int m_dt;
    /// valid cells number
    int m_nCells;
    /// threshold soil freezing temperature
    float m_frozenT;
    /// saturated conductivity, mm/h
    float **m_ks;
    ///// soil porosity
    //float **m_porosity;

	/// amount of water held in the soil layer at saturation (sat - wp water), mm
	float **m_sat;    
	/// amount of water held in the soil layer at field capacity (fc - wp water) mm H2O
	float **m_fc;
	/// water content of soil at -1.5 MPa (wilting point) mm H2O
	float **m_wp;
    /// pore size distribution index
    float **m_poreIndex;
	/// amount of water stored in soil layers on current day, sol_st in SWAT
	float **m_soilStorage;
	/// amount of water stored in soil profile on current day, sol_sw in SWAT
	float *m_soilStorageProfile;
    /// soil temperature
    float *m_soilT;
    /// infiltration (mm)
    float *m_infil;
	/// impound/release 
	float *m_impoundTriger;
    /// Output
	
	///percolation (mm)
    float **m_perc;

public:
    //! Constructor
    PER_PI(void);

    //! Destructor
    ~PER_PI(void);

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


