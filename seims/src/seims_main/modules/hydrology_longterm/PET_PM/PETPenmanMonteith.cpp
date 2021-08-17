#include "PETPenmanMonteith.h"

#include "utils_time.h"
#include "text.h"
#include "ClimateParams.h"

PETPenmanMonteith::PETPenmanMonteith() :
    m_meanTemp(nullptr), m_minTemp(nullptr),
    m_maxTemp(nullptr), m_sr(nullptr), m_rhd(nullptr),
    m_ws(nullptr), m_dem(nullptr), m_phuAnn(nullptr),
    m_igro(nullptr), m_canHgt(nullptr), m_lai(nullptr),
    m_alb(nullptr), m_nCells(-1), m_cellLat(nullptr), m_co2Conc(NODATA_VALUE),
    m_vpd2(nullptr), m_gsi(nullptr), m_vpdfr(nullptr), m_frgmax(nullptr),
    m_snowTemp(-1),
    m_petFactor(1.f),
    m_pet(nullptr), m_maxPltET(nullptr), m_vpd(nullptr), m_dayLen(nullptr),
    m_phuBase(nullptr) {
}

PETPenmanMonteith::~PETPenmanMonteith() {
    if (m_dayLen != nullptr) Release1DArray(m_dayLen);
    if (m_phuBase != nullptr) Release1DArray(m_phuBase);
    if (m_pet != nullptr) Release1DArray(m_pet);
    if (m_vpd != nullptr) Release1DArray(m_vpd);
}

bool PETPenmanMonteith::CheckInputData() {
    CHECK_POSITIVE(MID_PET_PM, m_date);
    CHECK_POSITIVE(MID_PET_PM, m_nCells);
    CHECK_POINTER(MID_PET_PM, m_canHgt);
    CHECK_POINTER(MID_PET_PM, m_dem);
    CHECK_POINTER(MID_PET_PM, m_igro);
    CHECK_POINTER(MID_PET_PM, m_lai);
    CHECK_POINTER(MID_PET_PM, m_alb);
    CHECK_POINTER(MID_PET_PM, m_rhd);
    CHECK_POINTER(MID_PET_PM, m_sr);
    CHECK_POINTER(MID_PET_PM, m_maxTemp);
    CHECK_POINTER(MID_PET_PM, m_meanTemp);
    CHECK_POINTER(MID_PET_PM, m_minTemp);
    CHECK_POINTER(MID_PET_PM, m_cellLat);
    CHECK_POINTER(MID_PET_PM, m_ws);
    CHECK_POINTER(MID_PET_PM, m_gsi);
    CHECK_POINTER(MID_PET_PM, m_minTemp);
    CHECK_POINTER(MID_PET_PM, m_vpdfr);
    CHECK_POINTER(MID_PET_PM, m_frgmax);
    return true;
}

void PETPenmanMonteith::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, VAR_CO2)) m_co2Conc = value;
    else if (StringMatch(sk, VAR_T_SNOW)) m_snowTemp = value;
    else if (StringMatch(sk, VAR_K_PET)) m_petFactor = value;
    else {
        throw ModelException(MID_PET_PM, "SetValue", "Parameter " + sk +
                             " does not exist in current module. Please contact the module developer.");
    }
}

void PETPenmanMonteith::Set1DData(const char* key, const int n, float* value) {
    CheckInputSize(MID_PET_PM, key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_TMEAN)) m_meanTemp = value;
    else if (StringMatch(sk, VAR_TMAX)) m_maxTemp = value;
    else if (StringMatch(sk, VAR_TMIN)) m_minTemp = value;
    else if (StringMatch(sk, VAR_CELL_LAT)) m_cellLat = value;
    else if (StringMatch(sk, DataType_RelativeAirMoisture)) m_rhd = value;
    else if (StringMatch(sk, DataType_SolarRadiation)) m_sr = value;
    else if (StringMatch(sk, DataType_WindSpeed)) m_ws = value;
    else if (StringMatch(sk, VAR_DEM)) m_dem = value;
    else if (StringMatch(sk, VAR_CHT)) m_canHgt = value;
    else if (StringMatch(sk, VAR_ALBDAY)) m_alb = value;
    else if (StringMatch(sk, VAR_LAIDAY)) m_lai = value;
    else if (StringMatch(sk, VAR_PHUTOT)) m_phuAnn = value;
    else if (StringMatch(sk, VAR_IGRO)) m_igro = value;
    else if (StringMatch(sk, VAR_GSI)) m_gsi = value;
    else if (StringMatch(sk, VAR_VPDFR)) m_vpdfr = value;
    else if (StringMatch(sk, VAR_FRGMAX)) m_frgmax = value;
    else {
        throw ModelException(MID_PET_PM, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void PETPenmanMonteith::InitialOutputs() {
    CHECK_POSITIVE(MID_PET_PM, m_nCells);
    if (nullptr == m_vpd) Initialize1DArray(m_nCells, m_vpd, 0.f);
    if (nullptr == m_dayLen) Initialize1DArray(m_nCells, m_dayLen, 0.f);
    if (nullptr == m_phuBase) Initialize1DArray(m_nCells, m_phuBase, 0.f);
    if (nullptr == m_pet) Initialize1DArray(m_nCells, m_pet, 0.f);
    if (nullptr == m_maxPltET) Initialize1DArray(m_nCells, m_maxPltET, 0.f);
    if (nullptr == m_vpd2) m_vpd2 = new(nothrow) float[m_nCells];
}

void PETPenmanMonteith::InitializeIntermediateVariables(){
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (m_frgmax[i] > 0.f && m_vpdfr[i] > 0.f) {
            m_vpd2[i] = (1.f - m_frgmax[i]) / (m_vpdfr[i] - 1.f);
        }
        else {
            m_vpd2[i] = 0.f;
        }
    }

    m_needReCalIntermediateParams = false;
}

int PETPenmanMonteith::Execute() {
    CheckInputData();
    InitialOutputs();
    if (m_needReCalIntermediateParams) InitializeIntermediateVariables();
    //do the execute
#pragma omp parallel for
    for (int j = 0; j < m_nCells; j++) {
        /// update phubase of the simulation year.
        /* update base zero total heat units, src code from SWAT, subbasin.f
        if (tmpav(j) > 0. .and. phutot(hru_sub(j)) > 0.01) then
            phubase(j) = phubase(j) + tmpav(j) / phutot(hru_sub(j))
        end if*/
        if (m_dayOfYear == 1) m_phuBase[j] = 0.f;
        if (m_meanTemp[j] > 0.f && m_phuAnn[j] > 0.01f) {
            m_phuBase[j] += m_meanTemp[j] / m_phuAnn[j];
        }

        //////////////////////////////////////////////////////////////////////////
        //               compute net radiation
        //net short-wave radiation for PET (from swat)
        float raShortWave = m_sr[j] * (1.f - 0.23f);
        if (m_meanTemp[j] < m_snowTemp) {
            //if the mean t < snow t, consider the snow depth is larger than 0.5mm.
            raShortWave = m_sr[j] * (1.f - 0.8f);
        }

        //calculate the max solar radiation
        float srMax = 0.f;
        MaxSolarRadiation(m_dayOfYear, m_cellLat[j], m_dayLen[j], srMax);
        //calculate net long-wave radiation
        //net emissivity  equation 2.2.20 in SWAT manual
        float satVaporPressure = SaturationVaporPressure(m_meanTemp[j]); //kPa
        float actualVaporPressure = 0.f;
        if (m_rhd[j] > 1) {
            /// IF percent unit.
            actualVaporPressure = m_rhd[j] * satVaporPressure * 0.01f;
        } else {
            actualVaporPressure = m_rhd[j] * satVaporPressure;
        }
        m_vpd[j] = satVaporPressure - actualVaporPressure;
        float rbo = -(0.34f - 0.139f * sqrt(actualVaporPressure)); //P37 1:1.2.22
        //cloud cover factor equation 2.2.19
        float rto = 0.0f;
        if (srMax >= 1.0e-4f) {
            rto = 0.9f * (m_sr[j] / srMax) + 0.1f;
        }
        //net long-wave radiation equation 2.2.21
        float tk = m_meanTemp[j] + 273.15f;
        float raLongWave = rbo * rto * 4.9e-9f * pow(tk, 4.f);
        // calculate net radiation
        float raNet = raShortWave + raLongWave;

        //////////////////////////////////////////////////////////////////////////
        //calculate the slope of the saturation vapor pressure curve
        float dlt = 4098.f * satVaporPressure / pow(m_meanTemp[j] + 237.3f, 2.f);

        //calculate latent heat of vaporization(MJ/kg, from swat)
        float latentHeat = 2.501f - 0.002361f * m_meanTemp[j];

        //calculate mean barometric pressure(kPa)
        float pb = 101.3f - m_dem[j] * (0.01152f - 0.544e-6f * m_dem[j]); //p53 1:2.3.8 P
        //psychrometric constant(kPa/deg C)
        float gma = 1.013e-3f * pb / (0.622f * latentHeat); //p53 1:2.3.7

        float rho = 1710.f - 6.85f * m_meanTemp[j]; //p127 2:2.2.19
        //aerodynamic resistance to sensible heat and vapor transfer(s/m)
        float rv = 114.f / (m_ws[j] * pow(170.f * 0.001f, 0.2f)); // P127 2:2.2.20  ??the wind speed need modified?
        //canopy resistance(s/m), 1. / 330. = 0.0030303030303030303
        float rc = 49.f / (1.4f - 0.4f * m_co2Conc * 0.0030303030303030303f); //P128 2:2.2.22
        float petValue = (dlt * raNet + gma * rho * m_vpd[j] / rv) /
                (latentHeat * (dlt + gma * (1.f + rc / rv))); //P122 2:2.2.2
        petValue = m_petFactor * Max(0.f, petValue);
        m_pet[j] = petValue;
        //*********************************************************
        //The albedo would be obtained from plant growth module. But now it is assumed to be a constant.
        //After the plant growth module is completed, the following codes should be removed.
        //float albedo = 0.8f;
        //if(m_tMean[j] > m_snowTemp) albedo = 0.23f;
        //// m_albedo is calculated by PG_EPIC module. By LJ, 2016
        //*********************************************************
        // calculate net short-wave radiation for max plant PET
        float raShortWavePlant = m_sr[j] * (1.0f - m_alb[j]);
        float raNetPlant = raShortWavePlant + raLongWave;

        float epMax = 0.f;
        if (m_igro[j] > 0) {
            //land cover growing
            //determine wind speed and height of wind speed measurement
            //adjust to 100 cm (1 m) above canopy if necessary
            float zz = 0.f; //height at which wind speed is determined(cm)
            if (m_canHgt[j] <= 1.f) {
                zz = 170.f;
            } else {
                zz = m_canHgt[j] * 100.f + 100.f;
            }
            //wind speed at height zz(m/s)
            float uzz = m_ws[j] * pow(zz * 0.001f, 0.2f);

            //calculate canopy height in cm
            float chz = 0.f;
            if (m_canHgt[j] < 0.01f) {
                chz = 1.0f;
            } else {
                chz = m_canHgt[j] * 100.f;
            }

            //calculate roughness length for momentum transfer
            float zom = 0.f;
            if (chz <= 200.f) {
                zom = 0.123f * chz;
            } else {
                zom = 0.058f * pow(chz, 1.19f);
            }

            //calculate roughness length for vapor transfer
            float zov = 0.1f * zom;

            //calculate zero-plane displacement of wind profile
            float d = 0.667f * chz;

            //calculate aerodynamic resistance
            rv = log((zz - d) / zom) * log((zz - d) / zov);
            rv /= pow(0.41f, 2.f) * uzz;

            //adjust stomatal conductivity for low vapor pressure
            //this adjustment will lower maximum plant ET for plants
            //sensitive to very low vapor pressure
            float xx = m_vpd[j] - 1.f; //difference between vpd and vpthreshold
            //amount of vapor pressure deficit over threshold value
            float fvpd = 1.f;
            if (xx > 0.f) {
                fvpd = Max(0.1f, 1.f - m_vpd2[j] * xx);
            }
            float gsi_adj = m_gsi[j] * fvpd;

            if (gsi_adj > 1.e-6f) {
                //calculate canopy resistance
                rc = 1.f / gsi_adj; //single leaf resistance
                rc /= (0.5f * (m_lai[j] + 0.01f) * (1.4f - 0.4f * m_co2Conc * 0.0030303030303030303f));
                //calculate maximum plant ET
                epMax = (dlt * raNetPlant + gma * rho * m_vpd[j] / rv) / (latentHeat * (dlt + gma * (1.f + rc / rv)));
                epMax = Max(0.f, epMax);
                epMax = Min(epMax, petValue);
            } else {
                epMax = 0.f;
            }
        }
        m_maxPltET[j] = epMax;
    }
    return 0;
}

void PETPenmanMonteith::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_PET)) *data = m_pet;
    else if (StringMatch(sk, VAR_PPT)) *data = m_maxPltET;
    else if (StringMatch(sk, VAR_VPD)) *data = m_vpd;
    else if (StringMatch(sk, VAR_DAYLEN)) *data = m_dayLen;
    else if (StringMatch(sk, VAR_PHUBASE)) *data = m_phuBase;
    else {
        throw ModelException(MID_PET_PM, "Get1DData", "Parameter " + sk + " does not exist.");
    }
}
