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
    CHECK_POSITIVE(M_PET_PM[0], m_date);
    CHECK_POSITIVE(M_PET_PM[0], m_nCells);
    CHECK_POINTER(M_PET_PM[0], m_canHgt);
    CHECK_POINTER(M_PET_PM[0], m_dem);
    CHECK_POINTER(M_PET_PM[0], m_igro);
    CHECK_POINTER(M_PET_PM[0], m_lai);
    CHECK_POINTER(M_PET_PM[0], m_alb);
    CHECK_POINTER(M_PET_PM[0], m_rhd);
    CHECK_POINTER(M_PET_PM[0], m_sr);
    CHECK_POINTER(M_PET_PM[0], m_maxTemp);
    CHECK_POINTER(M_PET_PM[0], m_meanTemp);
    CHECK_POINTER(M_PET_PM[0], m_minTemp);
    CHECK_POINTER(M_PET_PM[0], m_cellLat);
    CHECK_POINTER(M_PET_PM[0], m_ws);
    CHECK_POINTER(M_PET_PM[0], m_gsi);
    CHECK_POINTER(M_PET_PM[0], m_minTemp);
    CHECK_POINTER(M_PET_PM[0], m_vpdfr);
    CHECK_POINTER(M_PET_PM[0], m_frgmax);
    return true;
}

void PETPenmanMonteith::SetValue(const char* key, const FLTPT value) {
    string sk(key);
    if (StringMatch(sk, VAR_CO2[0])) m_co2Conc = value;
    else if (StringMatch(sk, VAR_T_SNOW[0])) m_snowTemp = value;
    else if (StringMatch(sk, VAR_K_PET[0])) m_petFactor = value;
    else {
        throw ModelException(M_PET_PM[0], "SetValue", "Parameter " + sk +
                             " does not exist in current module. Please contact the module developer.");
    }
}

void PETPenmanMonteith::Set1DData(const char* key, const int n, FLTPT* value) {
    CheckInputSize(M_PET_PM[0], key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_TMEAN[0])) m_meanTemp = value;
    else if (StringMatch(sk, VAR_TMAX[0])) m_maxTemp = value;
    else if (StringMatch(sk, VAR_TMIN[0])) m_minTemp = value;
    else if (StringMatch(sk, VAR_CELL_LAT[0])) m_cellLat = value;
    else if (StringMatch(sk, DataType_RelativeAirMoisture)) m_rhd = value;
    else if (StringMatch(sk, DataType_SolarRadiation)) m_sr = value;
    else if (StringMatch(sk, DataType_WindSpeed)) m_ws = value;
    else if (StringMatch(sk, VAR_DEM[0])) m_dem = value;
    else if (StringMatch(sk, VAR_CHT[0])) m_canHgt = value;
    else if (StringMatch(sk, VAR_ALBDAY[0])) m_alb = value;
    else if (StringMatch(sk, VAR_LAIDAY[0])) m_lai = value;
    else if (StringMatch(sk, VAR_PHUTOT[0])) m_phuAnn = value;
    else if (StringMatch(sk, VAR_GSI[0])) m_gsi = value;
    else if (StringMatch(sk, VAR_VPDFR[0])) m_vpdfr = value;
    else if (StringMatch(sk, VAR_FRGMAX[0])) m_frgmax = value;
    else {
        throw ModelException(M_PET_PM[0], "Set1DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void PETPenmanMonteith::Set1DData(const char* key, const int n, int* value) {
    CheckInputSize(M_PET_PM[0], key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_IGRO[0])) m_igro = value;
    else {
        throw ModelException(M_PET_PM[0], "Set1DData",
                             "Integer Parameter " + sk + " does not exist.");
    }
}

void PETPenmanMonteith::InitialOutputs() {
    CHECK_POSITIVE(M_PET_PM[0], m_nCells);
    if (nullptr == m_vpd) Initialize1DArray(m_nCells, m_vpd, 0.);
    if (nullptr == m_dayLen) Initialize1DArray(m_nCells, m_dayLen, 0.);
    if (nullptr == m_phuBase) Initialize1DArray(m_nCells, m_phuBase, 0.);
    if (nullptr == m_pet) Initialize1DArray(m_nCells, m_pet, 0.);
    if (nullptr == m_maxPltET) Initialize1DArray(m_nCells, m_maxPltET, 0.);
    if (nullptr == m_vpd2) Initialize1DArray(m_nCells, m_vpd2, 0.);

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (m_frgmax[i] > 0. && m_vpdfr[i] > 0.) {
            m_vpd2[i] = (1. - m_frgmax[i]) / (m_vpdfr[i] - 1.);
        }
        else {
            m_vpd2[i] = 0.;
        }
    }
}

int PETPenmanMonteith::Execute() {
    CheckInputData();
    InitialOutputs();
    //do the execute
#pragma omp parallel for
    for (int j = 0; j < m_nCells; j++) {
        /// update phubase of the simulation year.
        /* update base zero total heat units, src code from SWAT, subbasin.f
        if (tmpav(j) > 0. .and. phutot(hru_sub(j)) > 0.01) then
            phubase(j) = phubase(j) + tmpav(j) / phutot(hru_sub(j))
        end if*/
        if (m_dayOfYear == 1) m_phuBase[j] = 0.;
        if (m_meanTemp[j] > 0. && m_phuAnn[j] > 0.01) {
            m_phuBase[j] += m_meanTemp[j] / m_phuAnn[j];
        }

        //////////////////////////////////////////////////////////////////////////
        //               compute net radiation
        //net short-wave radiation for PET (from swat)
        FLTPT raShortWave = m_sr[j] * (1. - 0.23);
        if (m_meanTemp[j] < m_snowTemp) {
            //if the mean t < snow t, consider the snow depth is larger than 0.5mm.
            raShortWave = m_sr[j] * (1. - 0.8);
        }

        //calculate the max solar radiation
        FLTPT srMax = 0.f;
        MaxSolarRadiation(m_dayOfYear, m_cellLat[j], m_dayLen[j], srMax);
        //calculate net long-wave radiation
        //net emissivity  equation 2.2.20 in SWAT manual
        FLTPT satVaporPressure = SaturationVaporPressure(m_meanTemp[j]); //kPa
        FLTPT actualVaporPressure = 0.;
        if (m_rhd[j] > 1) {
            /// IF percent unit.
            actualVaporPressure = m_rhd[j] * satVaporPressure * 0.01;
        } else {
            actualVaporPressure = m_rhd[j] * satVaporPressure;
        }
        m_vpd[j] = satVaporPressure - actualVaporPressure;
        FLTPT rbo = -(0.34 - 0.139 * CalSqrt(actualVaporPressure)); //P37 1:1.2.22
        //cloud cover factor equation 2.2.19
        FLTPT rto = 0.;
        if (srMax >= 1.0e-4) {
            rto = 0.9 * (m_sr[j] / srMax) + 0.1;
        }
        //net long-wave radiation equation 2.2.21
        FLTPT tk = m_meanTemp[j] + 273.15;
        FLTPT raLongWave = rbo * rto * 4.9e-9 * CalPow(tk, 4.);
        // calculate net radiation
        FLTPT raNet = raShortWave + raLongWave;

        //////////////////////////////////////////////////////////////////////////
        //calculate the slope of the saturation vapor pressure curve
        FLTPT dlt = 4098. * satVaporPressure / CalPow(m_meanTemp[j] + 237.3, 2.);

        //calculate latent heat of vaporization(MJ/kg, from swat)
        FLTPT latentHeat = 2.501 - 0.002361 * m_meanTemp[j];

        //calculate mean barometric pressure(kPa)
        FLTPT pb = 101.3 - m_dem[j] * (0.01152 - 0.544e-6 * m_dem[j]); //p53 1:2.3.8 P
        //psychrometric constant(kPa/deg C)
        FLTPT gma = 1.013e-3 * pb / (0.622 * latentHeat); //p53 1:2.3.7

        FLTPT rho = 1710. - 6.85 * m_meanTemp[j]; //p127 2:2.2.19
        //aerodynamic resistance to sensible heat and vapor transfer(s/m)
        FLTPT rv = 114. / (m_ws[j] * CalPow(170. * 0.001, 0.2)); // P127 2:2.2.20  ??the wind speed need modified?
        //canopy resistance(s/m), 1. / 330. = 0.0030303030303030303
        FLTPT rc = 49. / (1.4 - 0.4 * m_co2Conc * 0.0030303030303030303); //P128 2:2.2.22
        FLTPT petValue = (dlt * raNet + gma * rho * m_vpd[j] / rv) /
                (latentHeat * (dlt + gma * (1. + rc / rv))); //P122 2:2.2.2
        petValue = m_petFactor * Max(0., petValue);
        m_pet[j] = petValue;
        //*********************************************************
        //The albedo would be obtained from plant growth module. But now it is assumed to be a constant.
        //After the plant growth module is completed, the following codes should be removed.
        //float albedo = 0.8f;
        //if(m_tMean[j] > m_snowTemp) albedo = 0.23f;
        //// m_albedo is calculated by PG_EPIC module. By LJ, 2016
        //*********************************************************
        // calculate net short-wave radiation for max plant PET
        FLTPT raShortWavePlant = m_sr[j] * (1. - m_alb[j]);
        FLTPT raNetPlant = raShortWavePlant + raLongWave;

        FLTPT epMax = 0.;
        if (m_igro[j] > 0) {
            //land cover growing
            //determine wind speed and height of wind speed measurement
            //adjust to 100 cm (1 m) above canopy if necessary
            FLTPT zz = 0.; //height at which wind speed is determined(cm)
            if (m_canHgt[j] <= 1.) {
                zz = 170.;
            } else {
                zz = m_canHgt[j] * 100. + 100.;
            }
            //wind speed at height zz(m/s)
            FLTPT uzz = m_ws[j] * CalPow(zz * 0.001, 0.2);

            //calculate canopy height in cm
            FLTPT chz = 0.;
            if (m_canHgt[j] < 0.01) {
                chz = 1.;
            } else {
                chz = m_canHgt[j] * 100.;
            }

            //calculate roughness length for momentum transfer
            FLTPT zom = 0.;
            if (chz <= 200.) {
                zom = 0.123 * chz;
            } else {
                zom = 0.058 * CalPow(chz, 1.19);
            }

            //calculate roughness length for vapor transfer
            FLTPT zov = 0.1 * zom;

            //calculate zero-plane displacement of wind profile
            FLTPT d = 0.667 * chz;

            //calculate aerodynamic resistance
            rv = CalLn((zz - d) / zom) * CalLn((zz - d) / zov);
            rv /= CalPow(0.41, 2.) * uzz;

            //adjust stomatal conductivity for low vapor pressure
            //this adjustment will lower maximum plant ET for plants
            //sensitive to very low vapor pressure
            FLTPT xx = m_vpd[j] - 1.; //difference between vpd and vpthreshold
            //amount of vapor pressure deficit over threshold value
            FLTPT fvpd = 1.;
            if (xx > 0.) {
                fvpd = Max(0.1, 1. - m_vpd2[j] * xx);
            }
            FLTPT gsi_adj = m_gsi[j] * fvpd;

            if (gsi_adj > 1.e-6) {
                //calculate canopy resistance
                rc = 1.f / gsi_adj; //single leaf resistance
                rc /= (0.5 * (m_lai[j] + 0.01) * (1.4 - 0.4 * m_co2Conc * 0.0030303030303030303));
                //calculate maximum plant ET
                epMax = (dlt * raNetPlant + gma * rho * m_vpd[j] / rv) / (latentHeat * (dlt + gma * (1. + rc / rv)));
                epMax = Max(0., epMax);
                epMax = Min(epMax, petValue);
            } else {
                epMax = 0.;
            }
        }
        m_maxPltET[j] = epMax;
    }
    return 0;
}

void PETPenmanMonteith::Get1DData(const char* key, int* n, FLTPT** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_PET[0])) *data = m_pet;
    else if (StringMatch(sk, VAR_PPT[0])) *data = m_maxPltET;
    else if (StringMatch(sk, VAR_VPD[0])) *data = m_vpd;
    else if (StringMatch(sk, VAR_DAYLEN[0])) *data = m_dayLen;
    else if (StringMatch(sk, VAR_PHUBASE[0])) *data = m_phuBase;
    else {
        throw ModelException(M_PET_PM[0], "Get1DData",
                             "Parameter " + sk + " does not exist.");
    }
}
