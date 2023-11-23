#include "PETHargreaves.h"

#include "utils_time.h"
#include "ClimateParams.h"
#include "text.h"

PETHargreaves::PETHargreaves() :
    m_nCells(-1), m_HCoef_pet(0.0023), m_petFactor(1.),
    m_cellLat(nullptr), m_phuAnn(nullptr),
    m_meanTemp(nullptr), m_maxTemp(nullptr), m_minTemp(nullptr), m_rhd(nullptr),
    m_dayLen(nullptr), m_phuBase(nullptr), m_pet(nullptr), m_vpd(nullptr) 
{
    SetModuleName(M_PET_H[0]);
}

PETHargreaves::~PETHargreaves() {
    if (m_dayLen != nullptr) Release1DArray(m_dayLen);
    if (m_phuBase != nullptr) Release1DArray(m_phuBase);
    if (m_pet != nullptr) Release1DArray(m_pet);
    if (m_vpd != nullptr) Release1DArray(m_vpd);
}

void PETHargreaves::SetValue(const char* key, const FLTPT value) {
    string sk(key);
    if (StringMatch(sk, VAR_K_PET[0])) m_petFactor = value;
    else if (StringMatch(sk, VAR_PET_HCOEF[0])) m_HCoef_pet = value;
    else {
        throw ModelException(GetModuleName(), "SetValue", "Parameter " + sk +
                             " does not exist in current module.");
    }
}

void PETHargreaves::Set1DData(const char* key, const int n, FLTPT* value) {
    CheckInputSize(key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_TMEAN[0])) m_meanTemp = value;
    else if (StringMatch(sk, VAR_TMAX[0])) m_maxTemp = value;
    else if (StringMatch(sk, VAR_TMIN[0])) m_minTemp = value;
    else if (StringMatch(sk, DataType_RelativeAirMoisture)) m_rhd = value;
    else if (StringMatch(sk, VAR_CELL_LAT[0])) m_cellLat = value;
    else if (StringMatch(sk, VAR_PHUTOT[0])) m_phuAnn = value;
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk +
                             " does not exist in current module.");
    }
}

bool PETHargreaves::CheckInputData() {
    CHECK_POSITIVE(GetModuleName(), m_date);
    CHECK_POSITIVE(GetModuleName(), m_nCells);
    CHECK_POINTER(GetModuleName(), m_maxTemp);
    CHECK_POINTER(GetModuleName(), m_meanTemp);
    CHECK_POINTER(GetModuleName(), m_minTemp);
    CHECK_POINTER(GetModuleName(), m_rhd);
    CHECK_POINTER(GetModuleName(), m_cellLat);
    CHECK_POINTER(GetModuleName(), m_phuAnn);
    return true;
}

void PETHargreaves::InitialOutputs() {
    CHECK_POSITIVE(GetModuleName(), m_nCells);
    if (nullptr == m_pet) Initialize1DArray(m_nCells, m_pet, 0.);
    if (nullptr == m_vpd) Initialize1DArray(m_nCells, m_vpd, 0.);
    if (nullptr == m_dayLen) Initialize1DArray(m_nCells, m_dayLen, 0.);
    if (nullptr == m_phuBase) Initialize1DArray(m_nCells, m_phuBase, 0.);
}

int PETHargreaves::Execute() {
    CheckInputData();
    InitialOutputs();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        /// update phubase of the simulation year.
        /* update base zero total heat units, src code from SWAT, subbasin.f
        if (tmpav(j) > 0. .and. phutot(hru_sub(j)) > 0.01) then
            phubase(j) = phubase(j) + tmpav(j) / phutot(hru_sub(j))
        end if*/
        if (m_dayOfYear == 1) {
            m_phuBase[i] = 0.;
        }
        if (m_meanTemp[i] > 0. && m_phuAnn[i] > 0.01) {
            m_phuBase[i] += m_meanTemp[i] / m_phuAnn[i];
        }
        /// calculate the max solar radiation
        FLTPT srMax; /// maximum solar radiation of current day
        MaxSolarRadiation(m_dayOfYear, m_cellLat[i], m_dayLen[i], srMax);
        ///calculate latent heat of vaporization(from swat)
        FLTPT latentHeat = 2.501 - 0.002361 * m_meanTemp[i];
        /// extraterrestrial radiation
        /// equation 1:1.1.6 in SWAT Theory 2009, p33
        FLTPT h0 = srMax * 1.253; // 37.59f / 30.0f = 1.253f;
        /// calculate potential evapotranspiration, equation 2:2.2.24 in SWAT Theory 2009, p133
        /// Hargreaves et al., 1985. In SWAT Code, 0.0023 is replaced by harg_petco, which range from 0.0019 to 0.0032. by LJ
        FLTPT petValue = m_HCoef_pet * h0 * CalPow(Abs(m_maxTemp[i] - m_minTemp[i]), 0.5)
                * (m_meanTemp[i] + 17.8) / latentHeat;
        m_pet[i] = m_petFactor * Max(0.0, petValue);
        /// calculate m_vpd
        FLTPT satVaporPressure = SaturationVaporPressure(m_meanTemp[i]);
        FLTPT actualVaporPressure = 0.;
        if (m_rhd[i] > 1) {
            /// IF percent unit.
            actualVaporPressure = m_rhd[i] * satVaporPressure * 0.01;
        } else {
            actualVaporPressure = m_rhd[i] * satVaporPressure;
        }
        m_vpd[i] = satVaporPressure - actualVaporPressure;
    }
    return 0;
}

void PETHargreaves::Get1DData(const char* key, int* n, FLTPT** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_PET[0])) *data = m_pet;
    else if (StringMatch(sk, VAR_VPD[0])) *data = m_vpd;
    else if (StringMatch(sk, VAR_DAYLEN[0])) *data = m_dayLen;
    else if (StringMatch(sk, VAR_PHUBASE[0])) *data = m_phuBase;
    else {
        throw ModelException(GetModuleName(), "Get1DData", "Parameter " + sk + " does not exist.");
    }
}
