/*
 * 
 * \review Liang-Jun Zhu
 * \date 2016-7-24
 * \note: 1. Add support of multi soil layers of each cells.
 *        2. 
 */
#pragma once
#include <string>
#include <vector>
#include <string>
#include <sstream>
#include "api.h"

using namespace std;

#include "SimulationModule.h"
/** \defgroup SSR_DA
 * \ingroup Hydrology_longterm
 * \brief --
 *
 */

/*!
 * \class SSR_DA
 * \ingroup SSR_DA
 * \brief --
 * 
 */
class SSR_DA : public SimulationModule
{
private:
	/// valid cell numbers
	int m_nCells;
	/// width of cell (m)
	float m_CellWidth;
    /// max number of soil layers
    int m_nSoilLayers;
	/// number of soil layers of each cell
	float *m_soilLayers;
	/// soil thickness
	float **m_soilThick;

    ///// depth of the up soil layer
    //float m_upSoilDepth;
	/// 
    //float *m_rootDepth;

	/// timestep
    int m_dt;
	/// Interflow scale factor
    float m_ki;
	/// soil freezing temperature threshold, deg C
    float m_frozenT;
	/// slope (tan)
    float *m_slope;
	/// conductivity
    float **m_ks;
	///// porosity (mm/mm)
    //float **m_porosity;

	/// amount of water held in the soil layer at saturation (sat - wp water), mm
	float **m_satmm;
	/// pore size distribution index
    float **m_poreIndex;

	/// amount of water available to plants in soil layer at field capacity (AWC=FC-WP), mm
	float **m_fcmm;
	/// water content of soil at -1.5 MPa (wilting point) mm H2O
	float **m_wpmm;
	/// soil water storage (mm)
	float **m_soilStorage;
	/// soil water storage in soil profile (mm)
	float *m_soilStorageProfile;
	/// soil temperature, deg C
    float *m_soilT;

    /// channel width, m
    float *m_chWidth;
	/// stream link 
    float *m_streamLink;

    /**
    *	@brief 2d array of flow in cells
    *
    *	The first element in each sub-array is the number of flow in cells in this sub-array
    */
    float **m_flowInIndex;

    /**
    *	@brief percentage of flow out to current cell from each upstream cells, this used for MFD flow direction algorithms
    *
    *	It has the same data structure as m_flowInIndex.
    */
    float **m_flowInPercentage;

    /**
    *	@brief Routing layers according to the flow direction
    *
    *	There are not flow relationships within each layer.
    *	The first element in each layer is the number of cells in the layer
    */
    float **m_routingLayers;
	/// number of routing layers
    int m_nRoutingLayers;
	/// number of subbasin
    int m_nSubbasin;
    /// subbasin grid (ID of subbasin)
    float *m_subbasin;

    // outputs

	/// subsurface runoff (mm), VAR_SSRU
    float **m_qi;
	/// subsurface runoff volume (m3), VAR_SSRUVOL
    float **m_qiVol;
	/// subsurface to streams from each subbasin, the first element is the whole watershed, m3, VAR_SBIF
    float *m_qiSubbasin;

public:
	/// constructor
    SSR_DA(void);
	/// destructor
    ~SSR_DA(void);

    virtual int Execute();

    virtual void SetValue(const char *key, float data);

    virtual void Set1DData(const char *key, int nRows, float *data);

    virtual void Set2DData(const char *key, int nrows, int ncols, float **data);

	virtual void SetSubbasins(clsSubbasins *subbasins);

    virtual void Get1DData(const char *key, int *n, float **data);

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

    void FlowInSoil(int id);
};

