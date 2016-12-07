/*!
 * \file Muskingum.h
 * \brief Routing in the channel cells using Muskingum-Cunge method.
 * \author Junzhi Liu
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
/** \defgroup CH_MSK
 * \ingroup Hydrology
 * \brief Routing in the channel cells using Muskingum method
 */
/*!
 * \class Muskingum
 * \ingroup CH_MSK
 *
 * \brief Channel routing using Muskingum-Cunge method.
 *
 */
/*!
 * \struct MuskWeights
 * \ingroup CH_MSK
 * \brief 
 */
struct MuskWeights
{
    float c1;
    float c2;
    float c3;
    float c4;
    float dt;
};

class Muskingum : public SimulationModule
{
public:
    //! Constructor
    Muskingum(void);

    //! Destructor
    ~Muskingum(void);

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

    void GetCofficients(float reachLength, float waterDepth, float s0, float v0, MuskWeights &weights);

    float GetDelta_t(float timeStep, float fmin, float fmax);

private:
    /// deal with positive and negative float numbers
    float Power(float a, float n)
    {
        if (a >= 0)
            return pow(a, n);
        else
            return -pow(-a, n);
    }

    void ChannelFlow(int iReach, int iCell, int id, float qgEachCell);

    void initialOutputs();

    /// valid cells number
    int m_nCells;

    /// Muskingum weighting factor
    float m_msk_x;
    /// initial channel storage m3/m
    float m_chS0;

    /// cell width of the grid (m)
    float m_CellWidth;
    /// time step (second)
    float m_dt;

    /// slope (percent)
    float *m_s0;
    /// channel width (raster type to keep consistent with the one in IKW_CH, zero for overland cells)
    float *m_chWidth;
    /// stream link
    float *m_streamLink;
    /// v scaling factor used for calibration
    float m_vScalingFactor;

    /**
    *	@brief Flow direction by the rule of TauDEM
    *
        The value of direction is as following (TauDEM):
        4  3  2
        5     1
        6  7  8
        The value of direction is as following (ArcGIS):
        32 64 128
        64     1
        8   4  2
    */
    float *m_direction;

    /// precipitation
    float *m_prec;
    /// Flux in the downslope boundary of cells(output)
    float *m_qs;
    /// interflow to channel
    float *m_qi;
    /// groundwater flow to channel
    float *m_qg;

    /**
    *	@brief 2d array of flow in cells
    *
    *	The first element in each sub-array is the number of flow in cells in this sub-array
    */
    float **m_flowInIndex;
    /// flow out index
    float *m_flowOutIndex;

    /// channel storage
    float **m_chStorage;
    /// Flux in the downslope boundary of channel cells(output)
    float **m_qCh;
    /// flow in the up boundary last step
    float **m_qUpCh;
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
    map<int, int> m_diagonal;

    /// beta in manning equation
    float m_beta;
    /// threshold for Newton iteration method
    float m_delta;
    /// sqrt(2.f)
    float SQ2;
    /// 2/3
    float TWO_THIRDS;

    /// flow length
    float **m_flowLen;
    /// alpha in manning equation
    float **m_alpha;

    /// channel information
    /// channel number
    int m_chNumber;
    /// downstream id (The value is 0 if there if no downstream reach)
    float *m_reachDownStream;
    /// upstream id (The value is -1 if there if no upstream reach)
    vector<vector<int> > m_reachUpStream;

    /// stream order
    float *m_streamOrder;
    /// bankful flow velocity (m/s)
    float *m_v0;

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

