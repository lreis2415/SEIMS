/*!
 * \file InterFlow_IKW.h
 * \brief Interflow routing using implicit finite difference method
 * kinematic wave method in LISEM model
 * \author Junzhi Liu
 * \date Feb. 2011 
 */
#pragma once

#include <string>
#include <ctime>
#include <cmath>
#include <map>
#include "SimulationModule.h"

using namespace std;
/** \defgroup IKW_IF
 * \ingroup Hydrology
 * \brief Interflow routing using implicit finite difference method
 */
/*!
 * \class InterFlow_IKW
 * \ingroup IKW_IF
 *
 * \brief Interflow routing using implicit finite difference method
 *
 */
class InterFlow_IKW : public SimulationModule
{
public:
    //! Constructor
    InterFlow_IKW(void);

    //! Destructor
    ~InterFlow_IKW(void);

    virtual int Execute();

    virtual void SetValue(const char *key, float data);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

    virtual void Set2DData(const char *key, int nrows, int ncols, float **data);

    bool CheckInputSize(const char *key, int n);

    bool CheckInputData(void);

private:
    ///// deal with positive and negative float numbers
    //float Power(float a, float n)
    //{
    //	if (a >= 0)
    //		return pow(a, n);
    //	else
    //		return -pow(-a, n);
    //}

    void FlowInSoil(int id);

    void initialOutputs();

    /// size
    int m_nCells;

    /// cell width of the grid (m)
    float m_CellWidth;
    /// time step (second)
    float m_dt;
    /// channel width
    float *m_chWidth;

    /// slope (percent)
    float *m_s0;
    /// root depth (m)  /// mm? LJ
    float *m_rootDepth;
    /// soil depth (mm)
    float *m_soilDepth;
    /// conductivity (mm/h)
    float *m_ks;
    /// scaling factor depending on land use (Ki)
    float m_landuseFactor;

    float *m_soilMoistrue;
    //float* m_residual;
    float *m_porosity;
    float *m_poreIndex;
    float *m_fieldCapacity;

    float *m_streamLink;
    /**
    *	@brief 2d array of flow in cells
    *
    *	The first element in each sub-array is the number of flow in cells in this sub-array
    */
    float **m_flowInIndex;

    /**
    *	@brief Routing layers according to the flow direction
    *
    *	There are not flow relationships within each layer.
    *	The first element in each layer is the number of cells in the layer
    */
    float **m_routingLayers;
    int m_nLayers;

    /// depression storage
    float *m_sr;

    /// output flow to downstream cells (output)
    float *m_q;
    /// interflow depth
    float *m_h;
    /// return flow
    float *m_hReturnFlow;

    //////////////////////////////////////////////////////////////////////////
    // the following are intermediate variables

    /// sqrt(2.0f)
    float SQ2;
};

