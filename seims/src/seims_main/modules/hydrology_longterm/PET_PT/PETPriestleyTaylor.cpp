#include "PETPriestleyTaylor.h"
#include "MetadataInfo.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include "ModelException.h"
#include "util.h"
#include <omp.h>
#include "ClimateParams.h" /// added by LJ, 2016

using namespace std;

PETPriestleyTaylor::PETPriestleyTaylor(void) : m_tMin(NULL), m_tMax(NULL), m_sr(NULL), m_rhd(NULL), m_elev(NULL),
                                               m_phutot(NULL), 
											   m_dayLen(NULL), m_phuBase(NULL), m_pet(NULL), m_vpd(NULL),
                                               m_petFactor(1.f), m_nCells(-1)
{
}

PETPriestleyTaylor::~PETPriestleyTaylor(void)
{
    if (this->m_dayLen != NULL) Release1DArray(this->m_dayLen);
    if (this->m_phuBase != NULL) Release1DArray(this->m_phuBase);
    if (this->m_pet != NULL) Release1DArray(this->m_pet);
    if (this->m_vpd != NULL) Release1DArray(this->m_vpd);
}

void PETPriestleyTaylor::Get1DData(const char *key, int *n, float **data)
{
    // CheckInputData();// Plz avoid putting CheckInputData() in Get1DData, this may cause Set time error! By LJ
    initialOutputs();
    string sk(key);
	*n = this->m_nCells;
    if (StringMatch(sk, VAR_DAYLEN)) *data = this->m_dayLen;
    else if (StringMatch(sk, VAR_VPD)) *data = this->m_vpd; 
	else if (StringMatch(sk, VAR_PHUBASE)) *data = this->m_phuBase; 
	else if (StringMatch(sk, VAR_PET)) *data = this->m_pet;  
    else
        throw ModelException(MID_PET_PT, "Get1DData",
                             "Parameter " + sk + " does not exist. Please contact the module developer.");
}

bool PETPriestleyTaylor::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
        throw ModelException(MID_PET_PT, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    if (this->m_nCells != n)
    {
        if (this->m_nCells <= 0) this->m_nCells = n;
        else
            throw ModelException(MID_PET_PT, "CheckInputSize", "Input data for " + string(key) +
                                                               " is invalid. All the input data should have same size.");
    }
    return true;
}

bool PETPriestleyTaylor::CheckInputData()
{
    if (this->m_date < 0)
        throw ModelException(MID_PET_PT, "CheckInputData", "You have not set the time.");
    if (m_nCells <= 0)
        throw ModelException(MID_PET_PT, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    if (this->m_elev == NULL)
        throw ModelException(MID_PET_PT, "CheckInputData", "The elevation can not be NULL.");
    if (this->m_cellLat == NULL)
        throw ModelException(MID_PET_PT, "CheckInputData", "The latitude can not be NULL.");
    if (this->m_rhd == NULL)
        throw ModelException(MID_PET_PT, "CheckInputData", "The relative humidity can not be NULL.");
    if (this->m_sr == NULL)
        throw ModelException(MID_PET_PT, "CheckInputData", "The solar radiation can not be NULL.");
    if (this->m_tMin == NULL)
        throw ModelException(MID_PET_PT, "CheckInputData", "The min temperature can not be NULL.");
    if (this->m_tMax == NULL)
        throw ModelException(MID_PET_PT, "CheckInputData", "The max temperature can not be NULL.");
    if (this->m_tMean == NULL)
        throw ModelException(MID_PET_PT, "CheckInputData", "The mean temperature can not be NULL.");
    if (this->m_phutot == NULL)
        throw ModelException(MID_PET_PT, "CheckInputData", "The PHU0 can not be NULL.");
    return true;
}

void PETPriestleyTaylor::initialOutputs()
{
	if(this->m_pet == NULL) Initialize1DArray(m_nCells, m_pet, 0.f);
	if(this->m_vpd == NULL) Initialize1DArray(m_nCells, m_vpd, 0.f);
	if(this->m_dayLen == NULL) Initialize1DArray(m_nCells, m_dayLen, 0.f);
	if(this->m_phuBase == NULL) Initialize1DArray(m_nCells, m_phuBase, 0.f);
}

int PETPriestleyTaylor::Execute()
{
    CheckInputData();
    initialOutputs();
    m_jday = JulianDay(this->m_date);
#pragma omp parallel for
    for (int i = 0; i < m_nCells; ++i)
    {
        /// update phubase of the simulation year.
        /* update base zero total heat units, src code from SWAT, subbasin.f
        if (tmpav(j) > 0. .and. phutot(hru_sub(j)) > 0.01) then
            phubase(j) = phubase(j) + tmpav(j) / phutot(hru_sub(j))
        end if*/
        if (m_jday == 1) m_phuBase[i] = 0.f;
        if (m_tMean[i] > 0.f && m_phutot[i] > 0.01f)
            m_phuBase[i] += m_tMean[i] / m_phutot[i];

        /// compute net radiation
        /// net short-wave radiation for PET, etpot.f in SWAT src
        float raShortWave = m_sr[i] * (1.0f - 0.23f);
        //if the mean T < T_snow, consider the snow depth is larger than 0.5mm.
        if (m_tMean[i] < this->m_tSnow)
            raShortWave = m_sr[i] * (1.0f - 0.8f);

        /// calculate the max solar radiation
        MaxSolarRadiation(m_jday, this->m_cellLat[i], this->m_dayLen[i], m_srMax);

        /// calculate net long-wave radiation
        /// net emissivity  equation 2.2.20 in SWAT manual
        float satVaporPressure = SaturationVaporPressure(m_tMean[i]);
        float actualVaporPressure = 0.f;
        if (m_rhd[i] > 1)   /// IF percent unit.
            actualVaporPressure = m_rhd[i] * satVaporPressure * 0.01f;
        else
            actualVaporPressure = m_rhd[i] * satVaporPressure;
        m_vpd[i] = satVaporPressure - actualVaporPressure;
        float rbo = -(0.34f - 0.139f * sqrt(actualVaporPressure));
        //cloud cover factor
        float rto = 0.0f;
        if (m_srMax >= 1.0e-4f)
            rto = 0.9f * (m_sr[i] / m_srMax) + 0.1f;
        //net long-wave radiation
        float tk = m_tMean[i] + 273.15f;
        float raLongWave = rbo * rto * 4.9e-9f * pow(tk, 4.f);

        float raNet = raShortWave + raLongWave;

        //////////////////////////////////////////////////////////////////////////
        //calculate the slope of the saturation vapor pressure curve
        float dlt = 4098.f * satVaporPressure / pow((m_tMean[i] + 237.3f), 2.f);

        //calculate latent heat of vaporization(MJ/kg, from swat)
        float latentHeat = LatentHeatVapor(m_tMean[i]);

        //calculate mean barometric pressure(kPa)
        float pb = MeanBarometricPressure(m_elev[i]);
        //psychrometric constant(kPa/deg C)
        float gma = 1.013e-3f * pb / (0.622f * latentHeat);
        float pet_alpha = 1.28f;

        float petValue = pet_alpha * (dlt / (dlt + gma)) * raNet / latentHeat;
        m_pet[i] = m_petFactor * max(0.f, petValue);
    }
    return 0;
}

void PETPriestleyTaylor::Set1DData(const char *key, int n, float *value)
{
    if (!this->CheckInputSize(key, n)) return;
    string sk(key);
    if (StringMatch(sk, DataType_MeanTemperature)) this->m_tMean = value;
    else if (StringMatch(sk, DataType_MinimumTemperature)) this->m_tMin = value;
    else if (StringMatch(sk, DataType_MaximumTemperature)) this->m_tMax = value;
    else if (StringMatch(sk, DataType_RelativeAirMoisture)) this->m_rhd = value;
    else if (StringMatch(sk, DataType_SolarRadiation)) this->m_sr = value;
    else if (StringMatch(sk, VAR_DEM)) this->m_elev = value;
    else if (StringMatch(sk, VAR_CELL_LAT)) this->m_cellLat = value;
    else if (StringMatch(sk, VAR_PHUTOT)) this->m_phutot = value;
    else
        throw ModelException(MID_PET_PT, "Set1DData", "Parameter " + sk +
                                                      " does not exist in current module. Please contact the module developer.");
}

void PETPriestleyTaylor::SetValue(const char *key, float value)
{
    string sk(key);
    if (StringMatch(sk, VAR_T_SNOW)) this->m_tSnow = value;
    else if (StringMatch(sk, VAR_K_PET)) m_petFactor = value;
    else if (StringMatch(sk, VAR_OMP_THREADNUM)) omp_set_num_threads((int) value);
    else
        throw ModelException(MID_PET_PT, "SetValue", "Parameter " + sk +
                                                     " does not exist in current module. Please contact the module developer.");
}
