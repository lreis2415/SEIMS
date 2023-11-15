#include "PMELT_HBV.h"
#include "text.h"

PMELT_HBV::PMELT_HBV() :
    m_nCells(-1),
    m_t_mean(nullptr),
    m_lat(nullptr),
    m_melt_factor(nullptr),
    m_melt_factor_min(nullptr),
    m_aspect_corr(0),
    m_forest_corr(nullptr),
    m_landuse(nullptr),
    m_melt_temperature(0),
    m_potentialMelt(nullptr) {
    SetModuleName(M_PMELT_HBV[0]);
}

PMELT_HBV::~PMELT_HBV() {
    Release1DArray(m_potentialMelt);
}

void PMELT_HBV::InitialOutputs() {
    Initialize1DArray(m_nCells, m_potentialMelt, 0.);
}

bool PMELT_HBV::CheckInputData() {
    if (m_nCells <= 0) {
        throw ModelException(GetModuleName(), "CheckInputData", "Input data is invalid. The size could not be less than zero.");
    }
    CHECK_POINTER(GetModuleName(),m_t_mean);
    CHECK_POINTER(GetModuleName(),m_lat);
    CHECK_POINTER(GetModuleName(),m_melt_factor);
    CHECK_POINTER(GetModuleName(),m_melt_factor_min);
    CHECK_POINTER(GetModuleName(),m_forest_corr);
    CHECK_POINTER(GetModuleName(),m_landuse);
    CHECK_POINTER(GetModuleName(),m_t_mean);
    return true;
}

void PMELT_HBV::SetValue(const char* key, FLTPT value) {
    string sk(key);
    if (StringMatch(sk, VAR_T0[0])) {m_melt_temperature = value;}
    else if (StringMatch(sk, VAR_HBV_MELT_ASPECT_CORR[0])) { m_aspect_corr = value; }
    else {
        throw ModelException(GetModuleName(), "SetValue",
                             "Parameter " + sk + " does not exist.");
    }
}

void PMELT_HBV::Set1DData(const char* key, int n, FLTPT* data) {
    string sk(key);

    if (StringMatch(sk, VAR_TMEAN[0])) { m_t_mean = data; }
    else if (StringMatch(sk, VAR_CELL_LAT[0])) { m_lat = data; }
    else if (StringMatch(sk, VAR_MELT_FACTOR[0])) { m_melt_factor = data; }
    else if (StringMatch(sk, VAR_MIN_MELT_FACTOR[0])) { m_melt_factor_min = data; }
    else if (StringMatch(sk, VAR_HBV_MELT_FOREST_CORR[0])) { m_forest_corr = data; }
    else if (StringMatch(sk, VAR_LANDUSE[0])) { m_landuse = data; }
    else if (StringMatch(sk, VAR_FOREST_COV[0])) { m_forest_cov = data; }
    else if (StringMatch(sk, VAR_SLOPE[0])) { m_slope = data; }
    else if (StringMatch(sk, VAR_ASPECT[0])) { m_aspect = data; }
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

void PMELT_HBV::Get1DData(const char* key, int* n, FLTPT** data) {
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_POTENTIAL_MELT[0])) { *data = m_potentialMelt; }
    else {
        throw ModelException(GetModuleName(), "Get1DData", "Output " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

bool PMELT_HBV::CheckInputSize(const char* key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}
// This function is adapted from the Raven project
int PMELT_HBV::Execute() {
    CheckInputData();
    InitialOutputs();

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        FLTPT Ma = m_melt_factor[i];
        FLTPT Ma_min = m_melt_factor_min[i];
        FLTPT AM = m_aspect_corr;
        FLTPT MRF = m_forest_corr[i];
        FLTPT Fc = m_forest_cov[i];
        FLTPT melt_temp = m_melt_temperature;
        FLTPT slope_corr;

        // annual variation
        FLTPT adj = 0;
        FLTPT latRad = m_lat[i] * PI / 180.0; // convert latitude from degrees to radians
        if (m_lat[i] < 0.0) { adj = PI; } // southern hemisphere phase shift
        const FLTPT WINTER_SOLSTICE_ANG = 6.111043;
        Ma = Ma_min + (Ma - Ma_min) * 0.5 * (1.0 - cos(getDayAngle() - WINTER_SOLSTICE_ANG - adj));

        // forest correction
        Ma *= ((1.0 - Fc) * 1.0 + (Fc) * MRF);

        // slope correction (forest only, for some reason)
        slope_corr = ((1.0 - Fc) * 1.0 + (Fc) * sin(m_slope[i]));

        // aspect corrections
        Ma *= Max(1.0 - AM * slope_corr * cos(m_aspect[i] - adj), 0.0); // north facing slopes (aspect=0) have lower melt rates

        m_potentialMelt[i] = Max(Ma * (m_t_mean[i] - melt_temp),0);
    }

    return 0;
}
