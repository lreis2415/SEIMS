/*!
 * \brief Potential evapotranspiration using Hargreaves method
 * \author Junzhi Liu
 * \date Nov. 2010
 * \revised LiangJun Zhu
 * \date May. 2016
 * \note: 1. Add m_tMean from database, which may be measurement value or the mean of tMax and tMin;
			  2. The PET calculate is changed from site-based to cell-based, because PET is not only dependent on Climate site data;
			  3. Add m_VPD, m_dayLen as outputs, which will be used in PG_EPIC module
			  4. Add m_phuBase as outputs, which will be used in MGT_SWAT module
 */
#ifndef SEIMS_MODULE_PET_H_H
#define SEIMS_MODULE_PET_H_H

#include "SimulationModule.h"

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
class PETHargreaves: public SimulationModule {
public:
    PETHargreaves();

    ~PETHargreaves();

    void SetValue(const char* key, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* value) OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

    int Execute() OVERRIDE;

private:
    /*!
    * \brief check the input data. Make sure all the input data is available.
    * \return bool The validity of the input data.
    */
    bool CheckInputData();

    /*!
    * \brief check the input size. Make sure all the input data have same dimension.
    * \param[in] key The key of the input data
    * \param[in] n The input data dimension
    * \return bool The validity of the dimension
    */
    bool CheckInputSize(const char* key, int n);

    //! Initialize of output variables
    void InitialOutputs();
private:
    /// Parameters from Database
    /// mean air temperature for a given day(degree)
    float* m_meanTemp;
    /// maximum air temperature for a given day(degree)
    float* m_maxTemp;
    /// minimum air temperature for a given day(degree)
    float* m_minTemp;
    /// relative humidity(%)
    float* m_rhd;
    /// latitude of each valid cells
    float* m_cellLat;
    /// annual PHU
    float* m_phuAnn;
    /// valid cell number
    int m_nCells;
    /// coefficient related to radiation used in Hargreaves method
    float m_HCoef_pet;
    /// Correction Factor for PET
    float m_petFactor;

    /// output

    /// day length (hr)
    float* m_dayLen;
    /// base zero total heat units (used when no land cover is growing)
    float* m_phuBase;
    /// pet
    float* m_pet;
    /// vapor pressure deficit
    float* m_vpd;
};
#endif /* SEIMS_MODULE_PET_H_H */
