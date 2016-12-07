/** 
*	@file
*	@version	1.0
*	@author    Wu Hui
*	@date	24-January-2011
*
*	@brief	IUH overland method to calculate overland flow routing
*
*	Revision:	Zhiqiang YU	
*   Date:		2011-2-22
*	Description:
*	1.	Add parameter CellWidth.
*	2.	Delete parameter uhminCell and uhmaxCell because the parameter Ol_iuh
*		contains these information. The first and second column of Ol_iuh is 
*		min time and max time.
*	3.	The number of subbasins (m_nsub) should get from m_subbasin rather than
*		from main program. So does variable m_nCells.
*	4.	Add variable m_iuhCols to store the number of columns of Ol_iuh. In the 
*		meantime, add one parameter nCols to function SetIUHCell.
*	5.	Add variable m_cellFlow to store the flow of each cell in each day between
*		min time and max time. Its number of columns equals to the maximum of second
*		column of Ol_iuh add 1.
*	6.  Add function initial to initialize some variables.
*	7.	Modify function Execute.
*	
*	Revision: Liang-Jun Zhu
*	Date:	  2016-7-29
*	Description:
*	1.	Unify the code style.
*	2.	Replace VAR_SUBBASIN with VAR_SUBBASIN_PARAM, which is common used by several modules.
*	3.	
*/
#pragma once

#include <string>
#include <ctime>
#include "api.h"

using namespace std;

#include "SimulationModule.h"
/** \defgroup IUH_OL
 * \ingroup Hydrology_longterm
 * \brief IUH overland method to calculate overland flow routing
 *
 */

/*!
 * \class IUH_OL
 * \ingroup IUH_OL
 * \brief IUH overland method to calculate overland flow routing
 * 
 */
class IUH_OL : public SimulationModule
{
public:
    IUH_OL(void);

    ~IUH_OL(void);

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
	/// cell area, BE CAUTION, the unit is m^2, NOT ha!!!
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
    
	/// landcover code
	//float *m_landcover;
    /// IUH of each grid cell (1/s)
    float **m_iuhCell;
    /// the number of columns of Ol_iuh
    int m_iuhCols;
    /// surface runoff from depression module
    float *m_rs;
    /*/// length of rainfall series
    int m_nr;*/
    /// end time of simulation
    ///time_t m_EndDate;

    //temporary

	/// store the flow of each cell in each day between min time and max time
    float **m_cellFlow;
	/// the maximum of second column of OL_IUH plus 1.
    int m_cellFlowCols;

    //output
    
    /// overland flow to streams for each subbasin (m3/s)
    float *m_Q_SBOF;
	// overland flow in each cell (mm) //added by Gao, as intermediate variable, 29 Jul 2016
	float *m_OL_Flow;
	//! initial outputs
    void initialOutputs();
};

