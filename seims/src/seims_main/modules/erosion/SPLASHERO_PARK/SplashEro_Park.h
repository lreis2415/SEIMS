/*!
 * \brief Park Equation for splash erosion, and Foster Equation for overland flow soil detachment
 *           use the USLE_C, USLE_K in the calculation of splash erosion.
 *           PARK S, MITCHELL J, BUBENZER G,. Rainfall characteristics and their relation to splash erosion[J].
 *                 Trans. ASAE, 1983, 26(3): 795–804.
 * \author Hui Wu
 * \date Feb. 2012
 * \revised Liang-Jun Zhu
 * \date Mar. 2017
 * \description:  1.
 */
#ifndef SEIMS_SPLASHERO_PARK_H
#define SEIMS_SPLASHERO_PARK_H

#include "SimulationModule.h"

// using namespace std;  // Avoid this statement! by lj.

/** \defgroup SplashEro_Park
 * \ingroup Erosion
 * \brief Park Equation for splash erosion
 */
/*!
 * \class SplashEro_Park
 * \ingroup SplashEro_Park
 *
 * \brief Park Equation for splash erosion, and Foster Equation for overland flow soil detachment
 *           use the USLE_C, USLE_K in the calculation of splash erosion.//
 *
 */
class SplashEro_Park : public SimulationModule {
public:
    //! Constructor
    SplashEro_Park(void);

    //! Destructor
    ~SplashEro_Park(void);

    virtual int Execute(void);

    virtual void SetValue(const char *key, FLTPT value);

    virtual void Set1DData(const char *key, int n, FLTPT *data);

    virtual void Get1DData(const char *key, int *n, FLTPT **data);

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

private:

    //static string toString(FLTPT value);

    void InitialOutputs(void);

private:
    //Parameters
    
    /// number of cells
    int m_nCells;
    /// length of time step (s)
    FLTPT m_TimeStep;
    /// cell area of the unit (m^2)
    FLTPT* m_cellArea;

    ///parameter calibration coefficient of splash erosion (-)
    FLTPT m_Omega;
    /// slope of map, to calculate slope gradient.
    FLTPT *m_Slope;
    /// fraction of stones on the surface, affects splash [-]
    //FLTPT* m_GrassFrac;
    /// fraction of vegetation canopy cover [-]
    //FLTPT* m_coverFrac;
    /// channel width [m]
    //FLTPT* m_ChWidth;
    /// crop management factor
    FLTPT *m_USLE_C;
    /// soil erodibility factor
    FLTPT *m_USLE_K;

    //input from modules
    /// the depth of the surface water layer (mm), after kinematic wave model
    /// WaterDepth = Depression + SurfaceRunoffDepth
    FLTPT *m_sr;
    FLTPT *m_depression;

    /// water flux after kinematic wave model, m3/s
    FLTPT *m_Q;
    /// the amount of rainfall (mm) (pNet?)
    FLTPT *m_Rain;
    /// snowmelt cover map, value 1.0 if there is snowcover, 0 without [-], from distribution of snow accumulation from snow water balance module
    //FLTPT* m_SnowCover;

    //output
    /// the distribution of splash detachment, kg/cell
    FLTPT *m_DETSplash;

};
///parameter calibration coefficient of splash erosion (-)  /// NOT USED, deleted? LJ
//FLTPT m_Ccoe;
#endif /* SEIMS_SPLASHERO_PARK_H */
