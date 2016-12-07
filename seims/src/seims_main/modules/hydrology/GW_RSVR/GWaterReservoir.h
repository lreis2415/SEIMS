/*!
 * \file GWaterReservoir.h
 * \brief Calculate groundwater using reservoir method
 * \author Junzhi Liu
 * \date Oct. 2011
 * \revesed LiangJun Zhu
 * \note Change the module name from GWATER_RESERVOIR to GW_RSVR
 */

#pragma once

#include <string>
#include "SimulationModule.h"

using namespace std;
/** \defgroup GW_RSVR
 * \ingroup Hydrology
 * \brief Calculate groundwater using reservoir method
 *
 */
/*!
 * \class GWaterReservoir
 * \ingroup GW_RSVR
 *
 * \brief Calculate groundwater using reservoir method
 *
 */
class GWaterReservoir : public SimulationModule
{
public:
    //! Constructor
    GWaterReservoir(void);

    //! Destructor
    ~GWaterReservoir(void);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Set2DData(const char *key, int nrows, int ncols, float **data);

    virtual void SetValue(const char *key, float value);

    //virtual void GetValue(const char* key, float* value);
    virtual void Get1DData(const char *key, int *n, float **data);

    virtual int Execute(void);

    //virtual TimeStepType GetTimeStepType()
    //{
    //	return TIMESTEP_CHANNEL;
    //};

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
    *	@param key: The key of the input data
    *	@param n: The input data dimension
    *	@return bool The validity of the dimension
    */
    bool CheckInputSize(const char *, int n);

    void InitOutputs(void);

private:

    /// time step(seconds)
    float m_dt;

    /// count of valid cells
    int m_nCells;

    /// count of reaches
    int m_nReaches;

    /// cell size
    float m_CellWidth;

    float *m_subbasin;

    /// percolation from soil layer (mm)
    float *m_recharge;
    /// the amount of water recharged to the deep groundwater reservoir
    float m_deepCoefficient;

    /// baseflow recession coefficient
    float m_recessionCoefficient;

    /// baseflow recession exponent
    float m_recessionExponent;

    /// ground water storage (mm)
    float *m_storage;
    /// Maximum groundwater storage
    float m_storageMax;

    /// outflow at basin outlet
    float m_qOutlet;

    float *m_qg;

    /// temporary variables
    float *m_percSubbasin;
    int *m_nCellsSubbasin;
    float m_initStorage;
};

