#include "PETPriestleyTaylor.h"

#include "text.h"
#include "ClimateParams.h"
#include "utils_time.h"

PETPriestleyTaylor::PETPriestleyTaylor() :
    m_meanTemp(nullptr), m_maxTemp(nullptr), m_minTemp(nullptr),
    m_sr(nullptr), m_rhd(nullptr), m_dem(nullptr),
    m_nCells(-1), m_petFactor(1.f), m_cellLat(nullptr),
    m_phuAnn(nullptr), m_snowTemp(NODATA_VALUE),
    m_dayLen(nullptr), m_phuBase(nullptr), m_pet(nullptr), m_vpd(nullptr) {
}

PETPriestleyTaylor::~PETPriestleyTaylor() {
    if (m_dayLen != nullptr) Release1DArray(m_dayLen);
    if (m_phuBase != nullptr) Release1DArray(m_phuBase);
    if (m_pet != nullptr) Release1DArray(m_pet);
    if (m_vpd != nullptr) Release1DArray(m_vpd);
}

void PETPriestleyTaylor::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_DAYLEN[0])) *data = m_dayLen;
    else if (StringMatch(sk, VAR_VPD[0])) *data = m_vpd;
    else if (StringMatch(sk, VAR_PHUBASE[0])) *data = m_phuBase;
    else if (StringMatch(sk, VAR_PET[0])) *data = m_pet;
    else {
        throw ModelException(M_PET_PT[0], "Get1DData", "Parameter " + sk +
                             " does not exist. Please contact the module developer.");
    }
}

bool PETPriestleyTaylor::CheckInputData() {
    CHECK_POSITIVE(M_PET_H[0], m_date);
    CHECK_POSITIVE(M_PET_H[0], m_nCells);
    CHECK_POINTER(M_PET_H[0], m_dem);
    CHECK_POINTER(M_PET_H[0], m_cellLat);
    CHECK_POINTER(M_PET_H[0], m_maxTemp);
    CHECK_POINTER(M_PET_H[0], m_meanTemp);
    CHECK_POINTER(M_PET_H[0], m_minTemp);
    CHECK_POINTER(M_PET_H[0], m_rhd);
    CHECK_POINTER(M_PET_H[0], m_sr);
    CHECK_POINTER(M_PET_H[0], m_phuAnn);
    return true;
}

void PETPriestleyTaylor::InitialOutputs() {
    CHECK_POSITIVE(M_PET_H[0], m_nCells);
    if (nullptr == m_pet) Initialize1DArray(m_nCells, m_pet, 0.f);
    if (nullptr == m_vpd) Initialize1DArray(m_nCells, m_vpd, 0.f);
    if (nullptr == m_dayLen) Initialize1DArray(m_nCells, m_dayLen, 0.f);
    if (nullptr == m_phuBase) Initialize1DArray(m_nCells, m_phuBase, 0.f);
}

int PETPriestleyTaylor::Execute() {
    CheckInputData();
    InitialOutputs();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        /// update phubase of the simulation year.
        /* update base zero total heat units, src code from SWAT, subbasin.f
        if (tmpav(j) > 0. .and. phutot(hru_sub(j)) > 0.01) then
            phubase(j) = phubase(j) + tmpav(j) / phutot(hru_sub(j))
        end if*/
        if (m_dayOfYear == 1) m_phuBase[i] = 0.f;
        if (m_meanTemp[i] > 0.f && m_phuAnn[i] > 0.01f) {
            m_phuBase[i] += m_meanTemp[i] / m_phuAnn[i];
        }
        /// compute net radiation
        /// net short-wave radiation for PET, etpot.f in SWAT src
        float raShortWave = m_sr[i] * (1.0f - 0.23f);
        //if the mean T < T_snow, consider the snow depth is larger than 0.5mm.
        if (m_meanTemp[i] < m_snowTemp) {
            raShortWave = m_sr[i] * (1.0f - 0.8f);
        }

        /// calculate the max solar radiation
        float srMax; /// maximum solar radiation of current day
        MaxSolarRadiation(m_dayOfYear, m_cellLat[i], m_dayLen[i], srMax);

        /// calculate net long-wave radiation
        /// net emissivity  equation 2.2.20 in SWAT manual
        float satVaporPressure = SaturationVaporPressure(m_meanTemp[i]);
        float actualVaporPressure = 0.f;
        if (m_rhd[i] > 1) {
            /// IF percent unit.
            m_rhd[i] *= 0.01f;
        } else if (m_rhd[i] < UTIL_ZERO) {
            m_rhd[i] = UTIL_ZERO;
        }
        actualVaporPressure = m_rhd[i] * satVaporPressure;
        if (actualVaporPressure < UTIL_ZERO) {
            actualVaporPressure = UTIL_ZERO;
        }
        m_vpd[i] = satVaporPressure - actualVaporPressure;
        if (m_vpd[i] < 0.f) {
            m_vpd[i] = 0.f;
        }
        float rbo = -(0.34f - 0.139f * sqrt(actualVaporPressure));
        //cloud cover factor
        float rto = 0.0f;
        if (srMax >= 1.0e-4f) {
            rto = 0.9f * (m_sr[i] / srMax) + 0.1f;
        }
        //net long-wave radiation
		//it may be negative because the sky temperature is colder than the grass temperature
        //http://hyperphysics.phy-astr.gsu.edu/hbase/thermo/stefan.html
        //http://www.indiana.edu/~geog109/topics/04_radiation/stefan_bol.htm
        float tk = m_meanTemp[i] + 273.15f;
        float raLongWave = rbo * rto * 4.9e-9f * pow(tk, 4.f);

        float raNet = raShortWave + raLongWave;

        //////////////////////////////////////////////////////////////////////////
        //calculate the slope of the saturation vapor pressure curve
        float dlt = 4098.f * satVaporPressure / pow(m_meanTemp[i] + 237.3f, 2.f);

        //calculate latent heat of vaporization(MJ/kg, from swat)
        float latentHeat = LatentHeatVapor(m_meanTemp[i]);

        //calculate mean barometric pressure(kPa)
        float pb = MeanBarometricPressure(m_dem[i]);
        //psychrometric constant(kPa/deg C)
        float gma = 1.013e-3f * pb / (0.622f * latentHeat);
        float pet_alpha = 1.28f;

        float petValue = pet_alpha * (dlt / (dlt + gma)) * raNet / latentHeat;
        m_pet[i] = m_petFactor * Max(0.f, petValue);
        if (m_pet[i] != m_pet[i]) {
            cout << "cell id: " << i << ", pet: " << m_pet[i] << ", meanT: " << m_meanTemp[i] <<
                    ", rhd: " << m_rhd[i] << ", rbo: " << rbo << ", sr: " << m_sr[i] << ", m_srMax: "
                    << srMax << ", rto: " << rto << ", satVaporPressure: " << satVaporPressure << endl;
            throw ModelException(M_PET_PT[0], "Execute", "Calculation error occurred!\n");
        }
    }
    return 0;
}

void PETPriestleyTaylor::Set1DData(const char* key, const int n, float* value) {
    CheckInputSize(M_PET_PT[0], key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_TMEAN[0])) m_meanTemp = value;
    else if (StringMatch(sk, VAR_TMAX[0])) m_maxTemp = value;
    else if (StringMatch(sk, VAR_TMIN[0])) m_minTemp = value;
    else if (StringMatch(sk, DataType_RelativeAirMoisture)) m_rhd = value;
    else if (StringMatch(sk, DataType_SolarRadiation)) m_sr = value;
    else if (StringMatch(sk, VAR_DEM[0])) m_dem = value;
    else if (StringMatch(sk, VAR_CELL_LAT[0])) m_cellLat = value;
    else if (StringMatch(sk, VAR_PHUTOT[0])) m_phuAnn = value;
    else {
        throw ModelException(M_PET_PT[0], "Set1DData", "Parameter " + sk +
                             " does not exist in current module. Please contact the module developer.");
    }
}

void PETPriestleyTaylor::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, VAR_T_SNOW[0])) m_snowTemp = value;
    else if (StringMatch(sk, VAR_K_PET[0])) m_petFactor = value;
    else {
        throw ModelException(M_PET_PT[0], "SetValue", "Parameter " + sk +
                             " does not exist in current module. Please contact the module developer.");
    }
}
