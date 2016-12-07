/*!
 * \file Percolation.h
 * \brief
 * \author Junzhi Liu
 * \date May 2013
 */
#pragma once

#include <string>
#include <vector>
#include <string>
#include <sstream>
#include "SimulationModule.h"

using namespace std;
/** \defgroup PERCO_DARCY
 * \ingroup Hydrology
 * \brief Calculate percolation using Darcy law
 */
/*!
 * \class Percolation_DARCY
 * \ingroup PERCO_DARCY
 *
 * \brief Calculate percolation using Darcy law
 *
 */
class Percolation_DARCY : public SimulationModule
{
private:
    int m_timestep;
    //! Valid cells number
    int m_nCells;
    //! Width of cell (m)
    float m_CellWidth;

    float *m_Conductivity;
    float *m_Porosity;
    //float* m_Residual;
    float *m_Poreindex;
    float *m_Moisture;
    float *m_FieldCapacity;
    float *m_rootDepth;
    //float* m_SoilT;
    //float  m_ForzenT;

    float *m_recharge;


public:
    //! Constructor
    Percolation_DARCY(void);

    //! Destructor
    ~Percolation_DARCY(void);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

    virtual void SetValue(const char *key, float value);

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

};

