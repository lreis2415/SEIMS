/*!
 * \brief routing in the channel cells using 4-point implicit finite difference method
 * \author Junzhi Liu, Liangjun Zhu
 * \version 1.1
 * \date May 2017
 * \revised LJ - Replace Tag_ReachParameter by VAR_REACH_PARAM
 */
#ifndef SEIMS_MODULE_CH_DW_H
#define SEIMS_MODULE_CH_DW_H

#include "SimulationModule.h"

using namespace std;
/*! \defgroup CH_DW
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
class DiffusiveWave : public SimulationModule {
public:
    //! Constructor
    DiffusiveWave();

    //! Destructor
    ~DiffusiveWave();

    virtual int Execute();

    virtual void SetReaches(clsReaches *reaches);

    virtual void SetValue(const char *key, float data);

    virtual void GetValue(const char *key, float *value);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

    virtual void Set2DData(const char *key, int nrows, int ncols, float **data);

    virtual void Get2DData(const char *key, int *nRows, int *nCols, float ***data);

    bool CheckInputSize(const char *key, int n);

    bool CheckInputSizeChannel(const char *key, int n);

    bool CheckInputData();

private:
    void ChannelFlow(int iReach, int iCell, int id);

    void initialOutputs();

    ///< Valid cells number
    int m_nCells;
    ///< cell width of the grid (m)
    float m_CellWidth;
    ///< layering method, 0 means UP_DOWN, 1 means DOWN_UP
    LayeringMethod m_layeringMethod;
    ///< time step (second)
    float m_dt;
    ///< slope (percent)
    float *m_s0;
    /// channel width (raster type to keep consistent with the one in IKW_CH, zero for overland cells)
    float *m_chWidth;
    /// elevation
    float *m_elevation;
    /// stream link
    float *m_streamLink;
    /**
    *	@brief flow direction by the rule of TauDEM
    *   4  3  2
    *   5     1
    *   6  7  8
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
    ///** Deprecated, and m_diagonal is defined as DiagonalCCW in seims.h
    //*	@brief convert direction code to whether diagonal
    //*
    //*	derived from flow direction
    //    1  0  1
    //    0     0
    //    1  0  1
    //*/
    //std::map<int, int> m_diagonal;

    /// flow length
    float **m_flowLen;

    /// channel information
    /// channel number
    int m_chNumber;
    /// downstream id (The value is 0 if there if no downstream reach)
    float *m_reachDownStream;
    /// upstream id (The value is -1 if there if no upstream reach)
    vector <vector<int>> m_reachUpStream;
    /// reach manning's n
    float *m_reachN;
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

#endif /* SEIMS_MODULE_CH_DW_H */
