/** 
*	@file
*	@version	1.0
*	@author    Junzhi Liu
*	@date	2016-08-12
*
*	@brief	IUH overland method to calculate overland sediment routing
*/

#pragma once

#include <string>
#include <ctime>
#include "api.h"

using namespace std;

#include "SimulationModule.h"
/** \defgroup IUH_NUTR_OL
 * \ingroup Hydrology_longterm
 * \brief IUH overland method to calculate overland flow routing
 *
 */

/*!
 * \class IUH_NUTR_OL
 * \ingroup IUH_NUTR_OL
 * \brief IUH overland method to calculate overland flow routing
 * 
 */
class IUH_NUTR_OL : public SimulationModule
{
public:
    IUH_NUTR_OL(void);

    ~IUH_NUTR_OL(void);

    virtual int Execute();

    virtual void SetValue(const char *key, float data);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Set2DData(const char *key, int nRows, int nCols, float **data);

	virtual void SetSubbasins(clsSubbasins *);

    virtual void Get1DData(const char *key, int *n, float **data);

    bool CheckInputSize(const char *key, int n);

    bool CheckInputData(void);

private:
    /// time step (sec)
    int m_TimeStep;
    /// validate cells number
    int m_nCells;
    /// cell width of the grid (m)
    float m_CellWidth;
	/// cell area
	float m_cellArea;
    /// the total number of subbasins
	int m_nSubbasins;
	//! subbasin IDs
	vector<int> m_subbasinIDs;
    /// subbasin grid (subbasins ID)
    float *m_subbasin;

	/// subbasins information
	clsSubbasins *m_subbasinsInfo;
    /// start time of IUH for each grid cell
    ///float* m_uhminCell;
    /// end time of IUH for each grid cell
    ///float* m_uhmaxCell;
    
    /// IUH of each grid cell (1/s)
    float **m_iuhCell;
    /// the number of columns of Ol_iuh
    int m_iuhCols;
	/// sediment yield in each cell
	float *m_sedYield;

	/// nutrient variables

	/// amount of nitrate transported with surface runoff
	float *m_surqno3ToCh;
	// amount of organic nitrogen in surface runoff
	float *m_sedorgnToCh;

	/// amount of soluble phosphorus in surface runoff
	float *m_surqsolpToCh;
	// amount of active mineral phosphorus absorbed to sediment in surface runoff
	float *m_sedminpaToCh;
	// amount of stable mineral phosphorus absorbed to sediment in surface runoff
	float *m_sedminpsToCh;
	// amount of organic phosphorus in surface runoff
	float *m_sedorgpToCh;

	/// cod to reach in surface runoff (kg)
	float *m_codToCh;

	/// amount of nitrate transported with lateral flow
	float *m_latno3ToCh;
	/// amount of ammonium transported with lateral flow
	float *m_ammoToCh;
	/// amount of nitrite transported with lateral flow
	float *m_nitriteToCh;

	/// nitrate loading to reach in groundwater
	float *m_no3gwToCh;
	/// soluble P loading to reach in groundwater
	float *m_minpgwToCh;

    /*/// length of rainfall series
    int m_nr;*/
    /// end time of simulation
    ///time_t m_EndDate;

    //temporary

	/// the maximum of second column of OL_IUH plus 1.
    int m_cellFlowCols;
	/// store the sediment of each cell in each day between min time and max time
	float **m_cellSed;

	//////////////////////////////////////////////////////////////////////
    //output
	/// sediment to streams 
	float *m_sedtoCh;
	/// sediment to channel at each cell at current time step
	float *m_sedOL;

	//! intial outputs
    void initialOutputs();
};

