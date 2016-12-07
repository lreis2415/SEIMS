#pragma once
/*
 * Revision:    Liang-Jun Zhu
 * Date:        2016-7-28
 * Description: 1. Move subbasin class to base/data/clsSubbasin, to keep consistent with other modules
 *              2. Code cleanup
 *              3. 
 *
 */
#include <string>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include "clsSubbasin.h"
#include "SimulationModule.h"
#include "api.h"

using namespace std;

/** \defgroup SOL_WB
 * \ingroup Hydrology_longterm
 * \brief Soil water balance calculation
 *
 */

/*!
 * \class SOL_WB
 * \ingroup SOL_WB
 * \brief Soil water balance calculation
 * 
 */

class SOL_WB : public SimulationModule
{
private:
	//! valid cells number
    int m_nCells;
	//! maximum soil layers number
    int m_nSoilLayers;
	//! soil layers number of each cell
	float *m_soilLayers;
	//! soil thickness of each layer
	float **m_soilThick; 
	//! the maximum soil depth
	float *m_soilZMX;

	//! Net precipitation (include snow melt if stated) (mm)
    float *m_pNet;
	//! infiltration water (mm)
    float *m_Infil;
	//! evaporation from the soil water storage, es_day in SWAT (mm)
    float *m_ES;
	//! revaporization from groundwater to the last soil layer (mm)
    float *m_Revap; 
	//! subsurface runoff 
    float **m_RI;
	//! percolation (mm)
    float **m_Perco;
	//! soil storage (mm)
    float **m_soilStorage;
	// Outputs
    // used to output time series result for soil water balance

	//! precipitation on the current day (mm)
    float *m_PCP;
	//! interception loss (mm)
	float *m_Interc;
	//! evaporation from the interception storage (mm)
	float *m_EI;
	//! depression (mm)
    float *m_Dep;
	//! evaporation from depression storage (mm)
    float *m_ED;
	//! surface runoff generated (mm)
    float *m_RS;
	//! groundwater runoff
    float *m_RG;
	//! snow sublimation
    float *m_SE;
	//! mean temperature
	float *m_tMean;
	//! soil temperature
	float *m_SoilT;
	//! subbasins number
	int m_nSubbasins;
	//! subbasin IDs
	vector<int> m_subbasinIDs;
	//! All subbasins information,\sa clsSubbasins, \sa Subbasin
	clsSubbasins *m_subbasinsInfo;
	/* soil water balance, time series result
	 * the row index is subbasinID
	 */
	float **m_soilWaterBalance;
public:
    SOL_WB(void);

    ~SOL_WB(void);

    virtual void SetValue(const char *key, float data);

    virtual void Set1DData(const char *key, int nRows, float *data);

    virtual void Set2DData(const char *key, int nrows, int ncols, float **data);

	virtual void SetSubbasins(clsSubbasins *subbasins);

    virtual void Get2DData(const char *key, int *nRows, int *nCols, float ***data);

    virtual int Execute(void);


private:
    /**
    *	@brief check the input data. Make sure all the input data is available.
    */
    void CheckInputData(void);

    /**
    *	@brief check the input size. Make sure all the input data have same dimension.
    *
    *	@param key The key of the input data
    *	@param n The input data dimension
    *	@return bool The validity of the dimension
    */
    bool CheckInputSize(const char *, int);
	//! 
	void initialOutputs();
	/*
	 * \brief Set parameter values to subbasins
	 */
    void setValueToSubbasins(void);
};


	// previously, RootDepth (of plant) is confused with the sol_zmx, now change m_RootDepth to m_soilZMX
    //float *m_RootDepth;
    //float m_upSoilDepth; /// not needed any more. By LJ.

	//time_t m_Date; there is no need to define date here. By LJ.