#include "AET_PriestleyTaylorHargreaves.h"

#include "utils_math.h"
#include "text.h"
// #include "Logging.h"

using namespace utils_math;

// INITIALIZE_NULL_EASYLOGGINGPP

AET_PT_H::AET_PT_H() :
    m_nCells(-1), m_maxSoilLyrs(-1),
    m_esco(nullptr), m_nSoilLyrs(nullptr), m_soilDepth(nullptr), m_soilThk(nullptr),
    /// input from other modules
    m_solFC(nullptr), m_rsdCovSoil(nullptr), m_solNo3(nullptr), m_tMean(nullptr),
    m_lai(nullptr), m_pet(nullptr), m_IntcpET(nullptr),
    m_snowAccum(nullptr), m_snowSublim(nullptr),
    m_soilWtrSto(nullptr), m_soilWtrStoPrfl(nullptr),
    /// output
    m_maxPltET(nullptr), m_soilET(nullptr) 
{
    SetModuleName(M_AET_PTH[0]);
}

AET_PT_H::~AET_PT_H() {
    if (m_maxPltET != nullptr) Release1DArray(m_maxPltET);
    if (m_soilET != nullptr) Release1DArray(m_soilET);
}

void AET_PT_H::Set1DData(const char* key, const int n, FLTPT* data) {
    string sk(key);
    if (StringMatch(sk, VAR_ESCO[0])) m_esco = data;
    else if (StringMatch(sk, DataType_MeanTemperature)) m_tMean = data;
    else if (StringMatch(sk, VAR_LAIDAY[0])) m_lai = data;
    else if (StringMatch(sk, VAR_PET[0])) m_pet = data;
    else if (StringMatch(sk, VAR_INET[0])) m_IntcpET = data;
    else if (StringMatch(sk, VAR_SNAC[0])) m_snowAccum = data;
    else if (StringMatch(sk, VAR_SNSB[0])) m_snowSublim = data;
    else if (StringMatch(sk, VAR_SOL_COV[0]))m_rsdCovSoil = data;
    else if (StringMatch(sk, VAR_SOL_SW[0]))m_soilWtrStoPrfl = data;
    else {
        throw ModelException(GetModuleName(), "Set1DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void AET_PT_H::Set1DData(const char* key, const int n, int* data) {
    string sk(key);
    if (StringMatch(sk, VAR_SOILLAYERS[0])) m_nSoilLyrs = data;
    else {
        throw ModelException(GetModuleName(), "Set1DData",
                             "Integer Parameter " + sk + " does not exist.");
    }
}

void AET_PT_H::Set2DData(const char* key, const int n, const int col, FLTPT** data) {
    CheckInputSize2D(key, n, col, m_nCells, m_maxSoilLyrs);
    string sk(key);
    if (StringMatch(sk, VAR_SOILDEPTH[0])) m_soilDepth = data;
    else if (StringMatch(sk, VAR_SOILTHICK[0])) m_soilThk = data;
    else if (StringMatch(sk, VAR_SOL_AWC_AMOUNT[0])) m_solFC = data;
    else if (StringMatch(sk, VAR_SOL_NO3[0])) m_solNo3 = data;
    else if (StringMatch(sk, VAR_SOL_ST[0])) m_soilWtrSto = data;
    else {
        throw ModelException(GetModuleName(), "Set2DData",
                             "Parameter " + sk + " does not exist.");
    }
}
bool AET_PT_H::CheckInputSize(const char* key, const int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}

bool AET_PT_H::CheckInputData() {
    CHECK_POSITIVE(GetModuleName(), m_nCells);
    CHECK_POSITIVE(GetModuleName(), m_maxSoilLyrs);
    CHECK_POINTER(GetModuleName(), m_esco);
    CHECK_POINTER(GetModuleName(), m_nSoilLyrs);
    CHECK_POINTER(GetModuleName(), m_tMean);
    CHECK_POINTER(GetModuleName(), m_lai);
    CHECK_POINTER(GetModuleName(), m_pet);
    /// If m_snowSB is not provided, it will be initialized in  InitialOutputs().
    // CHECK_POINTER(GetModuleName(), m_snowSB);
    CHECK_POINTER(GetModuleName(), m_rsdCovSoil);
    CHECK_POINTER(GetModuleName(), m_soilDepth);
    CHECK_POINTER(GetModuleName(), m_soilThk);
    CHECK_POINTER(GetModuleName(), m_solFC);
    CHECK_POINTER(GetModuleName(), m_solNo3);
    CHECK_POINTER(GetModuleName(), m_soilWtrSto);
    CHECK_POINTER(GetModuleName(), m_soilWtrStoPrfl);
    return true;
}

void AET_PT_H::InitialOutputs() {
    CHECK_POSITIVE(GetModuleName(), m_nCells);
    Initialize1DArray(m_nCells, m_maxPltET, 0.);
    Initialize1DArray(m_nCells, m_soilET, 0.);
    Initialize1DArray(m_nCells, m_snowSublim, 0.);
}

int AET_PT_H::Execute() {
    CheckInputData();
    InitialOutputs();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        FLTPT esd = 0.;
        FLTPT etco = 0.;
        FLTPT effnup = 0.;
        FLTPT no3up = 0.;
        FLTPT es_max = 0.;
        FLTPT eos1 = 0.;
        FLTPT xx = 0.;
        FLTPT cej = 0.;
        FLTPT eaj = 0.;
        FLTPT pet = 0.;
        FLTPT esleft = 0.;
        FLTPT evzp = 0.;
        FLTPT eosl = 0.;
        FLTPT dep = 0.;
        FLTPT evz = 0.;
        FLTPT sev = 0.;
        pet = m_pet[i] - m_IntcpET[i]; /// Actually, m_IntcpET has been compared with m_pet in other module.
        esd = 500.;
        etco = 0.8;
        effnup = 0.1;

        if (pet < UTIL_ZERO) {
            m_maxPltET[i] = 0.; // i.e., ep_max in SWAT
            m_soilET[i] = 0.;   // i.e., es_max in SWAT
            continue;
        }
        /// compute potential plant evapotranspiration (PPT) other than Penman-Monteith method
        if (m_lai[i] <= 3.) {
            m_maxPltET[i] = m_lai[i] * pet * _1div3;
        } else {
            m_maxPltET[i] = pet;
        }
        if (m_maxPltET[i] < 0.) m_maxPltET[i] = 0.;
        /// compute potential soil evaporation
        cej = -5.e-5;
        es_max = 0.; ///maximum amount of evaporation (soil et)
        eos1 = 0.;
        if (m_snowAccum!=nullptr && m_snowAccum[i] >= 0.5) {
            eaj = 0.5;
        } else {
            eaj = CalExp(cej * (m_rsdCovSoil[i] + 0.1));
        }
        es_max = pet * eaj;
        eos1 = pet / (es_max + m_maxPltET[i] + 1.e-10);
        eos1 = es_max * eos1;
        es_max = Min(es_max, eos1);
        es_max = Max(es_max, 0.);
        /// make sure maximum plant and soil ET doesn't exceed potential ET
        if (pet < es_max + m_maxPltET[i] && !FloatEqual(es_max + m_maxPltET[i], 0.)) {
            es_max = pet * es_max / (es_max + m_maxPltET[i]);
            m_maxPltET[i] = pet * m_maxPltET[i] / (es_max + m_maxPltET[i]);
        }
        if (pet < es_max + m_maxPltET[i]) {
            es_max = pet - m_maxPltET[i] - UTIL_ZERO;
        }

        /// initialize soil evaporation variables
        esleft = es_max;
        /// compute sublimation, using the input m_snowSB from snow sublimation module, if not provided, initialized as 0
        if (m_snowAccum != nullptr && m_tMean[i] > 0.) {
            if (m_snowAccum[i] >= esleft) {
                /// take all soil evap from snow cover
                m_snowAccum[i] -= esleft;
                m_snowSublim[i] += esleft;
                esleft = 0.;
            }
            else {
                /// take all soil evap from snow cover then start taking from soil
                esleft -= m_snowAccum[i];
                m_snowSublim[i] += m_snowAccum[i];
                m_snowAccum[i] = 0.;
            }
        }
        // take soil evap from each soil layer
        evzp = 0.;
        eosl = esleft;
        for (int ly = 0; ly < CVT_INT(m_nSoilLyrs[i]); ly++) {
            /// depth exceeds max depth for soil evap (esd, by default 500 mm)
            if (ly == 0) {
                dep = m_soilDepth[i][ly];
            } else {
                dep = m_soilDepth[i][ly - 1];
            }
            if (dep < esd) {
                /// calculate evaporation from soil layer
                evz = eosl * m_soilDepth[i][ly] /
                        (m_soilDepth[i][ly] + CalExp(2.374 - 0.00713 * m_soilDepth[i][ly]));
                sev = evz - evzp * m_esco[i];
                evzp = evz;
                if (m_soilWtrSto[i][ly] < m_solFC[i][ly]) {
                    xx = 2.5 * (m_soilWtrSto[i][ly] - m_solFC[i][ly]) / m_solFC[i][ly]; /// non dimension
                    sev *= CalExp(xx);
                }
                sev = Min(sev, m_soilWtrSto[i][ly] * etco);
                if (sev < 0. || sev != sev) sev = 0.;
                if (sev > esleft) sev = esleft;
                /// adjust soil storage, potential evap
                if (m_soilWtrSto[i][ly] > sev) {
                    esleft -= sev;
                    m_soilWtrSto[i][ly] = Max(UTIL_ZERO, m_soilWtrSto[i][ly] - sev);
                } else {
                    esleft -= m_soilWtrSto[i][ly];
                    m_soilWtrSto[i][ly] = 0.;
                }
            }
            /// compute no3 flux from layer 2 to 1 by soil evaporation
            if (ly == 1) {
                /// index of layer 2 is 1 (soil surface, 10mm)
                no3up = 0.;
                no3up = effnup * sev * m_solNo3[i][ly] / (m_soilWtrSto[i][ly] + UTIL_ZERO);
                no3up = Min(no3up, m_solNo3[i][ly]);
                m_solNo3[i][ly] -= no3up;
                m_solNo3[i][ly - 1] += no3up;
            }
        }
        /// update total soil water content
        m_soilWtrStoPrfl[i] = 0.;
        for (int ly = 0; ly < CVT_INT(m_nSoilLyrs[i]); ly++) {
            m_soilWtrStoPrfl[i] += m_soilWtrSto[i][ly];
        }
        /// calculate actual amount of evaporation from soil
        //if (esleft != esleft || es_max != es_max)
        //	cout<<"esleft: "<<esleft<<", es_max: "<<es_max<<endl;
        if (es_max > esleft) {
            m_soilET[i] = es_max - esleft;
        } else {
            m_soilET[i] = 0.;
        }
    }
    return 0;
}

void AET_PT_H::Get1DData(const char* key, int* n, FLTPT** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_PPT[0])) *data = m_maxPltET;
    else if (StringMatch(sk, VAR_SOET[0])) *data = m_soilET;
    else {
        throw ModelException(GetModuleName(), "Get1DData",
                             "Result " + sk + " does not exist.");
    }
    *n = m_nCells;
}
