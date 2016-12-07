/*!
 * \file clsPI_MSM.h
 * \brief Compute canopy interception of rainfall using Maximum Storage Method
 * \author Alex Storey
 * \date May 2011
 * \revised LiangJun Zhu
 * \date May 2016
 * 
 */
#pragma once

#include <string>
#include <vector>
#include "api.h"
#include "SimulationModule.h"

using namespace std;
/** \defgroup PI_MSM
 * \ingroup Hydrology_longterm
 * \brief Class for the Precipitation Interception module
 *
 */

/*!
 * \class clsPI_MSM
 * \ingroup PI_MSM
 * \brief Calculate the Precipitation Interception
 * 
 */

class clsPI_MSM : public SimulationModule
{
private:
    //five parameters
    
    float m_Pi_b;
    float m_Init_IS;
    float *m_maxSt;
    float *m_minSt;

    //two input variables

	//! Precipitation
    float *m_P;
	//! PET
    float *m_PET;

    //! state variable, the initial value equal to 0
    float *m_st;

    //three results

	//! Interception loss
    float *m_interceptionLoss;
	//! Evaporation loss
    float *m_evaporation;
	//! Net precipitation
    float *m_netPrecipitation;

    //!  number of valid cells
    int m_nCells;

    //! previous date
    time_t m_dateLastTimeStep; 
public:
    virtual void Set1DData(const char *key, int nRows, float *data);

    virtual void SetValue(const char *key, float data);

    virtual void Get1DData(const char *key, int *nRows, float **data);

    virtual int Execute(void);

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

	/*!
     * \brief Initialize output variables for the first run of the entire simulation
     */
    void initialOutputs();
public:
    //! Constructor
    clsPI_MSM(void);

    //! Destructor
    ~clsPI_MSM(void);
};

