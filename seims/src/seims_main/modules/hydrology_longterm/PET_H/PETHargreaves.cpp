#include "PETHargreaves.h"
#include "MetadataInfo.h"
#include <vector>
#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
#include "util.h"
#include "ClimateParams.h"
#include "ModelException.h"
#include <omp.h>

using namespace std;

PETHargreaves::PETHargreaves(void) : m_nCells(-1), m_petFactor(1.f), m_HCoef_pet(0.0023f),
                                     m_tMean(NULL), m_tMin(NULL), m_tMax(NULL), m_rhd(NULL), m_phutot(NULL),
                                     m_dayLen(NULL), m_phuBase(NULL), m_pet(NULL), m_vpd(NULL)
{
}

PETHargreaves::~PETHargreaves(void)
{
    if (this->m_dayLen != NULL) Release1DArray(this->m_dayLen);
    if (this->m_phuBase != NULL) Release1DArray(this->m_phuBase);
    if (this->m_pet != NULL) Release1DArray(this->m_pet);
    if (this->m_vpd != NULL) Release1DArray(this->m_vpd);
}

void PETHargreaves::SetValue(const char *key, float value)
{
    string sk(key);
    if (StringMatch(sk, VAR_K_PET)) m_petFactor = value;
    else if (StringMatch(sk, VAR_PET_HCOEF)) m_HCoef_pet = value;
    else if (StringMatch(sk, VAR_OMP_THREADNUM)) omp_set_num_threads((int) value);
    else
        throw ModelException(MID_PET_H, "SetValue", "Parameter " + sk +
                                                    " does not exist in current module. Please contact the module developer.");
}

void PETHargreaves::Set1DData(const char *key, int n, float *value)
{
    if (!this->CheckInputSize(key, n)) return;
    string sk(key);
    if (StringMatch(sk, DataType_MeanTemperature))
        this->m_tMean = value;
    else if (StringMatch(sk, DataType_MaximumTemperature))
        this->m_tMax = value;
    else if (StringMatch(sk, DataType_MinimumTemperature))
        this->m_tMin = value;
    else if (StringMatch(sk, DataType_RelativeAirMoisture))
        this->m_rhd = value;
    else if (StringMatch(sk, VAR_CELL_LAT))
        this->m_cellLat = value;
    else if (StringMatch(sk, VAR_PHUTOT))
        this->m_phutot = value;
    else
        throw ModelException(MID_PET_H, "Set1DValue", "Parameter " + sk +
                                                      " does not exist in current module. Please contact the module developer.");
}

void PETHargreaves::initialOutputs()
{
	if(this->m_pet == NULL) Initialize1DArray(m_nCells, m_pet, 0.f);
	if(this->m_vpd == NULL) Initialize1DArray(m_nCells, m_vpd, 0.f);
	if(this->m_dayLen == NULL) Initialize1DArray(m_nCells, m_dayLen, 0.f);
	if(this->m_phuBase == NULL) Initialize1DArray(m_nCells, m_phuBase, 0.f);
}

int PETHargreaves::Execute()
{
    if (!this->CheckInputData()) return false;
    initialOutputs();
    m_jday = JulianDay(this->m_date);
    //cout<<m_jday<<","m_tMean[0]<<","<<m_tMax[0]<<","<<m_tMin[0]<<endl;
#pragma omp parallel for
    for (int i = 0; i < m_nCells; ++i)
    {
        /// update phubase of the simulation year.
        /* update base zero total heat units, src code from SWAT, subbasin.f
        if (tmpav(j) > 0. .and. phutot(hru_sub(j)) > 0.01) then
            phubase(j) = phubase(j) + tmpav(j) / phutot(hru_sub(j))
        end if*/
        if (m_jday == 1)
			m_phuBase[i] = 0.f;
        if (m_tMean[i] > 0.f && m_phutot[i] > 0.01f)
            m_phuBase[i] += m_tMean[i] / m_phutot[i];
        MaxSolarRadiation(m_jday, m_cellLat[i], m_dayLen[i], m_srMax);
        ///calculate latent heat of vaporization(from swat)
        float latentHeat = 2.501f - 0.002361f * m_tMean[i];
        /// extraterrestrial radiation
        /// equation 1:1.1.6 in SWAT Theory 2009, p33
        float h0 = m_srMax * 37.59f / 30.0f;
        /// calculate potential evapotranspiration, equation 2:2.2.24 in SWAT Theory 2009, p133
        /// Hargreaves et al., 1985. In SWAT Code, 0.0023 is replaced by harg_petco, which range from 0.0019 to 0.0032. by LJ
        float petValue = m_HCoef_pet * h0 * pow(abs(m_tMax[i] - m_tMin[i]), 0.5f)
                         * (m_tMean[i] + 17.8f) / latentHeat;
        m_pet[i] = m_petFactor * max(0.0f, petValue);
        /// calculate m_vpd
        float satVaporPressure = SaturationVaporPressure(m_tMean[i]);
        float actualVaporPressure = 0.f;
        if (m_rhd[i] > 1)   /// IF percent unit.
            actualVaporPressure = m_rhd[i] * satVaporPressure * 0.01f;
        else
            actualVaporPressure = m_rhd[i] * satVaporPressure;
        m_vpd[i] = satVaporPressure - actualVaporPressure;
    }
    return 0;
}


void PETHargreaves::Get1DData(const char *key, int *n, float **data)
{
    //CheckInputData(); // Plz avoid putting CheckInputData() in Get1DData, this may cause Set time error! By LJ
    initialOutputs();
    string sk(key);
    *n = this->m_nCells;
    if (this->m_pet == NULL)
        throw ModelException(MID_PET_H, "Get1DData", "The result is NULL. Please first execute the module.");
    if (StringMatch(sk, VAR_PET)) *data = this->m_pet;
    else if (StringMatch(sk, VAR_VPD)) *data = this->m_vpd;
    else if (StringMatch(sk, VAR_DAYLEN)) *data = this->m_dayLen;
    else if (StringMatch(sk, VAR_PHUBASE)) *data = this->m_phuBase;
    else
        throw ModelException(MID_PET_H, "Get1DData", "Parameter " + sk + " does not exist.");
}

bool PETHargreaves::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
        throw ModelException(MID_PET_H, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    if (this->m_nCells != n)
    {
        if (this->m_nCells <= 0) this->m_nCells = n;
        else
            throw ModelException(MID_PET_H, "CheckInputSize", "Input data for " + string(key) +
                                                              " is invalid. All the input data should have same size.");
    }
    return true;
}

bool PETHargreaves::CheckInputData()
{
    if (this->m_date < 0)
        throw ModelException(MID_PET_H, "CheckInputData", "You have not set the time.");
    if (m_nCells <= 0)
        throw ModelException(MID_PET_H, "CheckInputData", "The dimension of the input data can not be less than zero.");
    if (this->m_tMax == NULL)
        throw ModelException(MID_PET_H, "CheckInputData", "The maximum temperature can not be NULL.");
    if (this->m_tMin == NULL)
        throw ModelException(MID_PET_H, "CheckInputData", "The minimum temperature can not be NULL.");
    if (this->m_tMean == NULL)
        throw ModelException(MID_PET_H, "CheckInputData", "The mean temperature can not be NULL.");
    if (this->m_rhd == NULL)
        throw ModelException(MID_PET_H, "CheckInputData", "The relative humidity can not be NULL.");
    if (this->m_cellLat == NULL)
        throw ModelException(MID_PET_H, "CheckInputData", "The latitude can not be NULL.");
    if (this->m_phutot == NULL)
        throw ModelException(MID_PET_H, "CheckInputData", "The PHU0 can not be NULL.");
    return true;
}