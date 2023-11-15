#include "PETPriestleyTaylor.h"

#include "text.h"
#include "ClimateParams.h"
#include "utils_time.h"

PETPriestleyTaylor::PETPriestleyTaylor() :
    m_meanTemp(nullptr), m_maxTemp(nullptr), m_minTemp(nullptr),
    m_sr(nullptr), m_rhd(nullptr), m_dem(nullptr),
    m_nCells(-1), m_petFactor(1.f), m_cellLat(nullptr),
    m_phuAnn(nullptr), m_snowTemp(NODATA_VALUE),
    m_dayLen(nullptr), m_phuBase(nullptr), m_pet(nullptr), m_vpd(nullptr) 
{
    SetModuleName(M_PET_PT[0]);
}

PETPriestleyTaylor::~PETPriestleyTaylor() {
    if (m_dayLen != nullptr) Release1DArray(m_dayLen);
    if (m_phuBase != nullptr) Release1DArray(m_phuBase);
    if (m_pet != nullptr) Release1DArray(m_pet);
    if (m_vpd != nullptr) Release1DArray(m_vpd);
}

void PETPriestleyTaylor::Get1DData(const char* key, int* n, FLTPT** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_DAYLEN[0])) *data = m_dayLen;
    else if (StringMatch(sk, VAR_VPD[0])) *data = m_vpd;
    else if (StringMatch(sk, VAR_PHUBASE[0])) *data = m_phuBase;
    else if (StringMatch(sk, VAR_PET[0])) *data = m_pet;
    else {
        throw ModelException(GetModuleName(), "Get1DData", "Parameter " + sk +
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
    if (nullptr == m_pet) Initialize1DArray(m_nCells, m_pet, 0.);
    if (nullptr == m_vpd) Initialize1DArray(m_nCells, m_vpd, 0.);
    if (nullptr == m_dayLen) Initialize1DArray(m_nCells, m_dayLen, 0.);
    if (nullptr == m_phuBase) Initialize1DArray(m_nCells, m_phuBase, 0.);
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
        if (m_dayOfYear == 1) m_phuBase[i] = 0.;
        if (m_meanTemp[i] > 0. && m_phuAnn[i] > 0.01) {
            m_phuBase[i] += m_meanTemp[i] / m_phuAnn[i];
        }
        /// compute net radiation
        /// net short-wave radiation for PET, etpot.f in SWAT src
        FLTPT raShortWave = m_sr[i] * (1. - 0.23);
        //if the mean T < T_snow, consider the snow depth is larger than 0.5mm.
        if (m_meanTemp[i] < m_snowTemp) {
            raShortWave = m_sr[i] * (1. - 0.8);
        }

        /// calculate the max solar radiation
        FLTPT srMax; /// maximum solar radiation of current day
        MaxSolarRadiation(m_dayOfYear, m_cellLat[i], m_dayLen[i], srMax);

        /// calculate net long-wave radiation
        /// net emissivity  equation 2.2.20 in SWAT manual
        FLTPT satVaporPressure = SaturationVaporPressure(m_meanTemp[i]);
        FLTPT actualVaporPressure = 0.;
        if (m_rhd[i] > 1) {
            /// IF percent unit.
            m_rhd[i] *= 0.01;
        } else if (m_rhd[i] < UTIL_ZERO) {
            m_rhd[i] = UTIL_ZERO;
        }
        actualVaporPressure = m_rhd[i] * satVaporPressure;
        if (actualVaporPressure < UTIL_ZERO) {
            actualVaporPressure = UTIL_ZERO;
        }
        m_vpd[i] = satVaporPressure - actualVaporPressure;
        if (m_vpd[i] < 0.) {
            m_vpd[i] = 0.;
        }
        FLTPT rbo = -(0.34 - 0.139 * CalSqrt(actualVaporPressure));
        //cloud cover factor
        FLTPT rto = 0.0;
        if (srMax >= 1.0e-4) {
            rto = 0.9 * (m_sr[i] / srMax) + 0.1;
        }
        //net long-wave radiation
        //it may be negative because the sky temperature is colder than the grass temperature
        //http://hyperphysics.phy-astr.gsu.edu/hbase/thermo/stefan.html
        //http://www.indiana.edu/~geog109/topics/04_radiation/stefan_bol.htm
        FLTPT tk = m_meanTemp[i] + 273.15;
        FLTPT raLongWave = rbo * rto * 4.9e-9 * CalPow(tk, 4.);

        FLTPT raNet = raShortWave + raLongWave;

        //////////////////////////////////////////////////////////////////////////
        //calculate the slope of the saturation vapor pressure curve
        FLTPT dlt = 4098. * satVaporPressure / CalPow(m_meanTemp[i] + 237.3, 2.);

        //calculate latent heat of vaporization(MJ/kg, from swat)
        FLTPT latentHeat = LatentHeatVapor(m_meanTemp[i]);

        //calculate mean barometric pressure(kPa)
        FLTPT pb = MeanBarometricPressure(m_dem[i]);
        //psychrometric constant(kPa/deg C)
        FLTPT gma = 1.013e-3 * pb / (0.622 * latentHeat);
        FLTPT pet_alpha = 1.28f;

        FLTPT petValue = pet_alpha * (dlt / (dlt + gma)) * raNet / latentHeat;
        m_pet[i] = m_petFactor * Max(0., petValue);
        if (m_pet[i] != m_pet[i]) {
            cout << "cell id: " << i << ", pet: " << m_pet[i] << ", meanT: " << m_meanTemp[i] <<
                    ", rhd: " << m_rhd[i] << ", rbo: " << rbo << ", sr: " << m_sr[i] << ", m_srMax: "
                    << srMax << ", rto: " << rto << ", satVaporPressure: " << satVaporPressure << endl;
            throw ModelException(GetModuleName(), "Execute", "Calculation error occurred!\n");
        }
    }
    return 0;
}

void PETPriestleyTaylor::Set1DData(const char* key, const int n, FLTPT* value) {
    CheckInputSize(key, n, m_nCells);
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
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk +
                             " does not exist in current module. Please contact the module developer.");
    }
}

void PETPriestleyTaylor::SetValue(const char* key, const FLTPT value) {
    string sk(key);
    if (StringMatch(sk, VAR_T_SNOW[0])) m_snowTemp = value;
    else if (StringMatch(sk, VAR_K_PET[0])) m_petFactor = value;
    else {
        throw ModelException(GetModuleName(), "SetValue", "Parameter " + sk +
                             " does not exist in current module. Please contact the module developer.");
    }
}
