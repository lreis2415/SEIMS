/** 
*	@version	1.0
*	@author    Junzhi Liu
*	@date	2016-08-12
*
*	@brief	IUH overland method to calculate overland sediment routing
*	Revision: Liang-Jun Zhu
*	Date:     2018-3-22
*   Description:
*   1.  The length of subbasin related array should equal to the count of subbasins, for both mpi version and omp version.
*/
#ifndef SEIMS_MODULE_IUH_SED_OL_H
#define SEIMS_MODULE_IUH_SED_OL_H

#include "SimulationModule.h"

using namespace std;

/** \defgroup IUH_SED_OL
 * \ingroup Hydrology_longterm
 * \brief IUH overland method to calculate overland flow routing
 *
 */

/*!
 * \class IUH_SED_OL
 * \ingroup IUH_SED_OL
 * \brief IUH overland method to calculate overland flow routing
 * 
 */
class IUH_SED_OL : public SimulationModule {
public:
    IUH_SED_OL();

    ~IUH_SED_OL();

    virtual int Execute();

    virtual void SetValue(const char *key, float data);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Set2DData(const char *key, int nRows, int nCols, float **data);

    virtual void GetValue(const char *key, float *value);

    virtual void Get1DData(const char *key, int *n, float **data);

    bool CheckInputSize(const char *key, int n);

    bool CheckInputData();

private:
    void initialOutputs();

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
    /// current subbasin ID, 0 for the entire watershed
    int m_subbasinID;
    /// subbasin grid (subbasins ID)
    float *m_subbasin;

    /// IUH of each grid cell (1/s)
    float **m_iuhCell;
    /// the number of columns of Ol_iuh
    int m_iuhCols;
    /// sediment yield in each cell
    float *m_sedYield;

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
};
#endif /* SEIMS_MODULE_IUH_SED_OL_H */
