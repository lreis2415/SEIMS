#include "seims.h"
#include "ClimateParams.h"

#include "PETHargreaves.h"

using namespace std;

PETHargreaves::PETHargreaves() : m_nCells(-1), m_petFactor(1.f), m_HCoef_pet(0.0023f),
                                 m_tMean(nullptr), m_tMin(nullptr), m_tMax(nullptr), m_rhd(nullptr), m_phutot(nullptr),
                                 m_dayLen(nullptr), m_phuBase(nullptr), m_pet(nullptr), m_vpd(nullptr) {
}

PETHargreaves::~PETHargreaves() {
    if (m_dayLen != nullptr) Release1DArray(m_dayLen);
    if (m_phuBase != nullptr) Release1DArray(m_phuBase);
    if (m_pet != nullptr) Release1DArray(m_pet);
    if (m_vpd != nullptr) Release1DArray(m_vpd);
}

void PETHargreaves::SetValue(const char *key, float value) {
    string sk(key);
    if (StringMatch(sk, VAR_K_PET)) { m_petFactor = value; }
    else if (StringMatch(sk, VAR_PET_HCOEF)) { m_HCoef_pet = value; }
    else if (StringMatch(sk, VAR_OMP_THREADNUM)) { SetOpenMPThread((int) value); }
    else {
        throw ModelException(MID_PET_H, "SetValue", "Parameter " + sk +
            " does not exist in current module. Please contact the module developer.");
    }
}

void PETHargreaves::Set1DData(const char *key, int n, float *value) {
    CheckInputSize(key, n);
    string sk(key);
    if (StringMatch(sk, DataType_MeanTemperature)) { m_tMean = value; } 
    else if (StringMatch(sk, DataType_MaximumTemperature)) { m_tMax = value; } 
    else if (StringMatch(sk, DataType_MinimumTemperature)) { m_tMin = value; } 
    else if (StringMatch(sk, DataType_RelativeAirMoisture)) { m_rhd = value; } 
    else if (StringMatch(sk, VAR_CELL_LAT)) { m_cellLat = value; } 
    else if (StringMatch(sk, VAR_PHUTOT)) { m_phutot = value; } 
    else {
        throw ModelException(MID_PET_H, "Set1DValue", "Parameter " + sk +
            " does not exist in current module. Please contact the module developer.");
    }
}

void PETHargreaves::initialOutputs() {
    CHECK_POSITIVE(MID_PET_H, m_nCells);
    if (nullptr == m_pet) Initialize1DArray(m_nCells, m_pet, 0.f);
    if (nullptr == m_vpd) Initialize1DArray(m_nCells, m_vpd, 0.f);
    if (nullptr == m_dayLen) Initialize1DArray(m_nCells, m_dayLen, 0.f);
    if (nullptr == m_phuBase) Initialize1DArray(m_nCells, m_phuBase, 0.f);
}

int PETHargreaves::Execute() {
    CheckInputData();
    initialOutputs();
    m_jday = JulianDay(m_date);
    //cout<<m_jday<<","m_tMean[0]<<","<<m_tMax[0]<<","<<m_tMin[0]<<endl;
#pragma omp parallel for
    for (int i = 0; i < m_nCells; ++i) {
        /// update phubase of the simulation year.
        /* update base zero total heat units, src code from SWAT, subbasin.f
        if (tmpav(j) > 0. .and. phutot(hru_sub(j)) > 0.01) then
            phubase(j) = phubase(j) + tmpav(j) / phutot(hru_sub(j))
        end if*/
        if (m_jday == 1) {
            m_phuBase[i] = 0.f;
        }
        if (m_tMean[i] > 0.f && m_phutot[i] > 0.01f) {
            m_phuBase[i] += m_tMean[i] / m_phutot[i];
        }
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
        if (m_rhd[i] > 1) {   /// IF percent unit.
            actualVaporPressure = m_rhd[i] * satVaporPressure * 0.01f;
        } else {
            actualVaporPressure = m_rhd[i] * satVaporPressure;
        }
        m_vpd[i] = satVaporPressure - actualVaporPressure;
    }
    return 0;
}

void PETHargreaves::Get1DData(const char *key, int *n, float **data) {
    initialOutputs();
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_PET)) { *data = m_pet; }
    else if (StringMatch(sk, VAR_VPD)) { *data = m_vpd; }
    else if (StringMatch(sk, VAR_DAYLEN)) { *data = m_dayLen; }
    else if (StringMatch(sk, VAR_PHUBASE)) { *data = m_phuBase; }
    else {
        throw ModelException(MID_PET_H, "Get1DData", "Parameter " + sk + " does not exist.");
    }
}

bool PETHargreaves::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_PET_H, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; }
        else {
            throw ModelException(MID_PET_H, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
        }
    }
    return true;
}

bool PETHargreaves::CheckInputData() {
    CHECK_POSITIVE(MID_PET_H, m_date);
    CHECK_POSITIVE(MID_PET_H, m_nCells);
    CHECK_POINTER(MID_PET_H, m_tMax);
    CHECK_POINTER(MID_PET_H, m_tMean);
    CHECK_POINTER(MID_PET_H, m_tMin);
    CHECK_POINTER(MID_PET_H, m_rhd);
    CHECK_POINTER(MID_PET_H, m_cellLat);
    CHECK_POINTER(MID_PET_H, m_phutot);
    return true;
}