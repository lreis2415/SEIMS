/*!
 * \brief routing in the channel cells using 4-point implicit finite difference method
 * \author Junzhi Liu
 * \version 1.0
 * \date Feb. 2011
 */
#pragma once

#include <string>
#include <ctime>
#include <cmath>
#include <map>
#include <vector>
#include "SimulationModule.h"

using namespace std;
/** \defgroup CH_DW
 * \ingroup Hydrology
 * \brief Channel routing using diffusive wave equation.
 */
/*!
 * \ingroup CH_DW
 * \class DiffusiveWave
 *
 * \brief Routing in the channel
 *
 */
class DiffusiveWave : public SimulationModule
{
public:
    //! Constructor
    DiffusiveWave(void);

    //! Destructor
    ~DiffusiveWave(void);

    virtual int Execute();

    virtual void SetValue(const char *key, float data);

    virtual void GetValue(const char *key, float *value);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

    virtual void Set2DData(const char *key, int nrows, int ncols, float **data);

    virtual void Get2DData(const char *key, int *nRows, int *nCols, float ***data);

    bool CheckInputSize(const char *key, int n);

    bool CheckInputSizeChannel(const char *key, int n);

    bool CheckInputData(void);

private:
    ///// deal with positive and negative float numbers /// Defined in util.h
    //float Power(float a, float n)
    //{
    //	if (a >= 0)
    //		return pow(a, n);
    //	else
    //		return -pow(-a, n);
    //}
    void ChannelFlow(int iReach, int iCell, int id);

    void initialOutputs();

    ///< Valid cells number
    int m_nCells;

    ///< cell width of the grid (m)
    float m_CellWidth;

    float m_dt;///< time step (second)

    float *m_s0;
    ///< slope (percent)
    /// channel width (raster type to keep consistent with the one in IKW_CH, zero for overland cells)
    float *m_chWidth;
    float *m_elevation;

    /// stream link
    float *m_streamLink;
    /// manning scaling factor
    float m_manningScalingFactor;

    /**
    *	@brief flow direction by the rule of ArcGIS
    *
    *	The value of direction is as following:
        32 64 128
        64     1
        8   4  2
    */
    float *m_direction;

    /// precipitation
    float *m_prec;
    /// overland flow to channel (m3/s)
    float *m_qs;
    /// interflow to channel (m3/s)
    float *m_qi;

    /**
    *	@brief 2d array of flow in cells
    *
    *	The first element in each sub-array is the number of flow in cells in this sub-array
    */
    float **m_flowInIndex;
    /// flow out index
    float *m_flowOutIndex;

    /// Water depth in the downslope boundary of channel cells(output)
    float **m_hCh;
    /// Flux in the downslope boundary of channel cells(output)
    float **m_qCh;

    /// discharge at subasin outlet
    float *m_qSubbasin;


    // id of the outlet
    int m_idOutlet;
    /// id of the cell which the water of the upstream subbasin flows into
    int m_idUpReach;
    /// channel flow from upstream subbasin(assuming there is only channel flow between subbasins)
    float m_qUpReach;

    //////////////////////////////////////////////////////////////////////////
    // the following are intermediate variables
    /**
    *	@brief convert direction code to whether diagonal
    *
    *	derived from flow direction
        1  0  1
        0     0
        1  0  1
    */
    std::map<int, int> m_diagonal;

    /// flow length
    float **m_flowLen;

    /// channel information
    /// channel number
    int m_chNumber;
    /// downstream id (The value is 0 if there if no downstream reach)
    float *m_reachDownStream;
    /// upstream id (The value is -1 if there if no upstream reach)
    vector<vector<int> > m_reachUpStream;
    /// reach manning's n
    float *m_reachN;
    /// stream order
    float *m_streamOrder;

    // id the reaches
    float *m_reachId;
    /// map from subbasin id to index of the array
    map<int, int> m_idToIndex;

    map<int, vector<int> > m_reachLayers;

    /**
    *	@brief reach links
    *
    *	key: index of the reach
    *	value: vector of cell index
    */
    map<int, vector<int> > m_reachs;

    /// id of source cells of reaches
    int *m_sourceCellIds;
};

