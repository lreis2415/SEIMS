/*!
 * \brief Routing in the overland cells using 1D kinematic wave method in LISEM model
 * \author Junzhi Liu
 * \date Feb. 2011
 */
#ifndef SEIMS_IKW_OL_H
#define SEIMS_IKW_OL_H

#include "SimulationModule.h"

// using namespace std;  // Avoid this statement! by lj.


/** \defgroup IKW_OL
 * \ingroup Hydrology
 * \brief Routing in the overland cells using implicit finite difference method
 */
/*!
 * \class ImplicitKinematicWave_OL
 * \ingroup IKW_OL
 *
 * \brief kinematic wave method in LISEM model
 *
 */
class ImplicitKinematicWave_OL : public SimulationModule {
public:
    //! Constructor
    ImplicitKinematicWave_OL(void);

    //! Destructor
    ~ImplicitKinematicWave_OL(void);

    virtual int Execute(void);

    virtual void SetValue(const char *key, float data);

    virtual void GetValue(const char *key, float *data);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

    virtual void Set2DData(const char *key, int nrows, int ncols, float **data);

    bool CheckInputSize(const char *key, int n);

    bool CheckInputData(void);

private:
    float GetNewQ(float qIn, float qLast, float surplus, float alpha, float dt, float dx);

    void OverlandFlow(int id);

    void InitialOutputs(void);

    //! valid cells number
    int m_nCells;

    /// cell width of the grid (m)
    float m_CellWidth;
    /// time step (second)
    float m_dtStorm;

    /// slope (percent)
    float *m_s0;
    /// manning's roughness
    float *m_n;
    /// channel width (zero for non-channel cells)
    float *m_chWidth;

    /**
    *	@brief flow direction by the rule of ArcGIS
    *
    *	The value of direction is as following:
        4  3  2
        5     1
        6  7  8
    */
    float *m_direction;
    /**
    *	@brief 2d array of flow in cells
    *
    *	The first element in each sub-array is the number of flow in cells in this sub-array
    */
    float **m_flowInIndex;

    /// flow out index
    float *m_flowOutIndex;

    /**
    *	@brief Routing layers according to the flow direction
    *
    *	There are not flow relationships within each layer.
    *	The first element in each layer is the number of cells in the layer
    */
    float **m_routingLayers;
    int m_nLayers;

    /// water height available for runoff (surface runoff)
    float *m_sr;
    /// discharge to the downslope cell
    float *m_q;
    /// flow velocity
    float *m_vel;

    /// id of outlet
    int m_idOutlet;

    /////////////////////////////////////////////////////////////////////////
    // reinfiltration
    // if pNet > infilPotential, surplus = 0
    // else surplus = infilPotential - pNet
    float *m_infilCapacitySurplus;
    // twice infiltration
    /// cumulative infiltration depth (m)
    float *m_accumuDepth;
    /// infiltration map of watershed (mm) of the total nCells
    float *m_infil;
    /// reinfiltration
    float *m_reInfil;

    //////////////////////////////////////////////////////////////////////////
    // the following are intermediate variables

    /// flow width of each cell
    float *m_flowWidth;
    /// stream link
    float *m_streamLink;
    /// flow length of each cell
    float *m_flowLen;
    /// alpha in manning equation
    float *m_alpha;
    /// slope (radian)
    float *m_sRadian;
};
#endif /* SEIMS_IKW_OL_H */
