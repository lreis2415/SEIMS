/*!
 * \brief Potential evapotranspiration using Hargreaves method
 * \author Junzhi Liu
 * \date Nov. 2010
 * \revised LiangJun Zhu
 * \date May. 2016
 * \note: 1. Add m_tMean from database, which may be measurement value or the mean of tMax and tMin;
			  2. The PET calculate is changed from site-based to cell-based, because PET is not only dependent on Climate site data;
			  3. Add m_VPD, m_dayLen as outputs, which will be used in BIO_EPIC module
			  4. Add m_phuBase as outputs, which will be used in MGT_SWAT module
 */
#pragma once

#include "SimulationModule.h"

using namespace std;
/** \defgroup PET_H
 * \ingroup Hydrology_longterm
 * \brief Calculate potential evapotranspiration using Hargreaves method
 */
/*!
 * \class PETHargreaves
 * \ingroup PET_H
 *
 * \brief Hargreaves method to Compute PET
 *
 */
class PETHargreaves : public SimulationModule {
public:
    //! Constructor
    PETHargreaves(void);

    //! Destructor
    ~PETHargreaves(void);

    virtual void SetValue(const char *key, float value);

    virtual void Set1DData(const char *key, int n, float *value);

    virtual void Get1DData(const char *key, int *n, float **data);

    virtual int Execute();

private:
    /// Parameters from Database
    /// mean air temperature for a given day(degree)
    float *m_tMean;
    /// maximum air temperature for a given day(degree)
    float *m_tMax;
    /// minimum air temperature for a given day(degree)
    float *m_tMin;
    /// relative humidity(%)
    float *m_rhd;
    /// latitude of each valid cells
    float *m_cellLat;
    /// annual PHU
    float *m_phutot;
    /// valid cell number
    int m_nCells;
    /// coefficient related to radiation used in Hargreaves method
    float m_HCoef_pet;
    /// Correction Factor for PET
    float m_petFactor;

    /// temporary variables and output

    /// maximum solar radiation of current day
    float m_srMax;
    /// Julian day
    int m_jday;
    /// output

    /// day length (hr)
    float *m_dayLen;
    /// base zero total heat units (used when no land cover is growing)
    float *m_phuBase;
    /// pet
    float *m_pet;
    /// vapor pressure deficit
    float *m_vpd;
private:

    /*!
     * \brief check the input data. Make sure all the input data is available.
     * \return bool The validity of the input data.
     */
    bool CheckInputData(void);

    /*!
     * \brief check the input size. Make sure all the input data have same dimension.
     *
     *
     * \param[in] key The key of the input data
     * \param[in] n The input data dimension
     * \return bool The validity of the dimension
     */
    bool CheckInputSize(const char *, int);

    //! Initialize of output variables
    void initialOutputs();
};
