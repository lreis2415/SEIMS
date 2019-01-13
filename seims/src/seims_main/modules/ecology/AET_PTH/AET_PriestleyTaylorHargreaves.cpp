#include "AET_PriestleyTaylorHargreaves.h"

#include "utils_math.h"
#include "text.h"

using namespace utils_math;

AET_PT_H::AET_PT_H() :

    m_nCells(-1),
    m_esco(nullptr), m_nSoilLyrs(nullptr),
    m_landuse(nullptr), m_cropsta(nullptr),
    m_impoundTrig(nullptr), m_potVol(nullptr),

    m_maxSoilLyrs(-1),
    m_soilDepth(nullptr), m_soilThk(nullptr),
    /// input from other modules
    m_solFC(nullptr), m_rsdCovSoil(nullptr), m_solNo3(nullptr), m_tMean(nullptr),
    m_lai(nullptr), m_pet(nullptr), m_IntcpET(nullptr),
    m_snowAccum(nullptr), m_snowSublim(nullptr),
    m_soilWtrSto(nullptr), m_soilWtrStoPrfl(nullptr),
    /// output
    m_maxPltET(nullptr), m_soilET(nullptr) {
}

AET_PT_H::~AET_PT_H() {
    if (m_maxPltET != nullptr) Release1DArray(m_maxPltET);
    if (m_soilET != nullptr) Release1DArray(m_soilET);
}

void AET_PT_H::Set1DData(const char* key, const int n, float* data) {
    CheckInputSize(key, n);
    string sk(key);
    if (StringMatch(sk, VAR_ESCO)) {
        m_esco = data;
    } else if (StringMatch(sk, VAR_SOILLAYERS)) {
        m_nSoilLyrs = data;
    } else if (StringMatch(sk, DataType_MeanTemperature)) {
        m_tMean = data;
    } else if (StringMatch(sk, VAR_LAIDAY)) {
        m_lai = data;
    } else if (StringMatch(sk, VAR_PET)) {
        m_pet = data;
    } else if (StringMatch(sk, VAR_INET)) {
        m_IntcpET = data;
    } else if (StringMatch(sk, VAR_SNAC)) {
        m_snowAccum = data;
    } else if (StringMatch(sk, VAR_SNSB)) {
        m_snowSublim = data;
    } else if (StringMatch(sk, VAR_SOL_COV)) {
        m_rsdCovSoil = data;
    } else if (StringMatch(sk, VAR_SOL_SW)) {
        m_soilWtrStoPrfl = data;
    } else if (StringMatch(sk, VAR_LANDUSE)){
        m_landuse = data;
    } else if (StringMatch(sk, VAR_CROPSTA)) { 
        m_cropsta = data; 
    } else if (StringMatch(sk, VAR_IMPOUND_TRIG)) { 
        m_impoundTrig = data; 
    } else if (StringMatch(sk, VAR_POT_VOL)) { 
        m_potVol = data; 
    } else {
        throw ModelException(MID_AET_PTH, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void AET_PT_H::Set2DData(const char* key, const int n, const int col, float** data) {
    CheckInputSize(key, n);
    string sk(key);
    m_maxSoilLyrs = col;
    if (StringMatch(sk, VAR_SOILDEPTH)) m_soilDepth = data;
    else if (StringMatch(sk, VAR_SOILTHICK)) m_soilThk = data;
    else if (StringMatch(sk, VAR_SOL_AWC)) m_solFC = data;
    else if (StringMatch(sk, VAR_SOL_NO3)) m_solNo3 = data;
    else if (StringMatch(sk, VAR_SOL_ST)) m_soilWtrSto = data;
    else {
        throw ModelException(MID_AET_PTH, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

bool AET_PT_H::CheckInputSize(const char* key, const int n) {
    if (n <= 0) {
        throw ModelException(MID_AET_PTH, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) {
            m_nCells = n;
        } else {
            throw ModelException(MID_AET_PTH, "CheckInputSize", "Input data for " + string(key) +
                                 " is invalid. All the input data should have same size.");
        }
    }
    return true;
}

bool AET_PT_H::CheckInputData() {
    CHECK_POSITIVE(MID_AET_PTH, m_nCells);
    CHECK_POSITIVE(MID_AET_PTH, m_maxSoilLyrs);
    CHECK_POINTER(MID_AET_PTH, m_esco);
    CHECK_POINTER(MID_AET_PTH, m_nSoilLyrs);
    CHECK_POINTER(MID_AET_PTH, m_tMean);
    CHECK_POINTER(MID_AET_PTH, m_lai);
    CHECK_POINTER(MID_AET_PTH, m_pet);
    CHECK_POINTER(MID_AET_PTH, m_snowAccum);
    /// If m_snowSB is not provided, it will be initialized in  InitialOutputs().
    // CHECK_POINTER(MID_AET_PTH, m_snowSB);
    CHECK_POINTER(MID_AET_PTH, m_rsdCovSoil);
    CHECK_POINTER(MID_AET_PTH, m_soilDepth);
    CHECK_POINTER(MID_AET_PTH, m_soilThk);
    CHECK_POINTER(MID_AET_PTH, m_solFC);
    CHECK_POINTER(MID_AET_PTH, m_solNo3);
    CHECK_POINTER(MID_AET_PTH, m_soilWtrSto);
    CHECK_POINTER(MID_AET_PTH, m_soilWtrStoPrfl);
    return true;
}

void AET_PT_H::InitialOutputs() {
    CHECK_POSITIVE(MID_AET_PTH, m_nCells);
    if (nullptr == m_maxPltET) Initialize1DArray(m_nCells, m_maxPltET, 0.f);
    if (nullptr == m_soilET) Initialize1DArray(m_nCells, m_soilET, 0.f);
    if (nullptr == m_snowSublim) Initialize1DArray(m_nCells, m_snowSublim, 0.f);
}

float AET_PT_H::SWAT_maxPET(float pet, int i){
    /// compute potential plant evapotranspiration (PPT) other than Penman-Monteith method
    if (m_lai[i] <= 3.f) {
        m_maxPltET[i] = m_lai[i] * pet * _1div3;
    }
    else {
        m_maxPltET[i] = pet;
    }
    if (m_maxPltET[i] < 0.f) m_maxPltET[i] = 0.f;
    /// compute potential soil evaporation
    float cej = -5.e-5f;
    float eaj = 0.f;
    float es_max = 0.f;  ///maximum amount of evaporation (soil et)
    float eos1 = 0.f;
    if (m_snowAccum[i] >= 0.5f) {
        eaj = 0.5f;
    }
    else {
        eaj = exp(cej * (m_rsdCovSoil[i] + 0.1f));
    }
    es_max = pet * eaj;
    eos1 = pet / (es_max + m_maxPltET[i] + 1.e-10f);
    eos1 = es_max * eos1;
    es_max = min(es_max, eos1);
    es_max = max(es_max, 0.f);
    /// make sure maximum plant and soil ET doesn't exceed potential ET
    if (pet < es_max + m_maxPltET[i] && !FloatEqual(es_max + m_maxPltET[i], 0.f)) {
        es_max = pet * es_max / (es_max + m_maxPltET[i]);
        m_maxPltET[i] = pet * m_maxPltET[i] / (es_max + m_maxPltET[i]);
    }
    if (pet < es_max + m_maxPltET[i]) {
        es_max = pet - m_maxPltET[i] - UTIL_ZERO;
    }
    return es_max;
}

float AET_PT_H::ORYZA_maxPET(float pet, int i){
    // split the total pet to the radiation-driven part and drying power part
    float etrd = 0.f, etae = 0.f;
    etrd = 0.75f * pet;
    etae = pet - etrd;

    float es_max = 0.f;
    es_max = exp(-0.5f * m_lai[i]) * pet;

    m_maxPltET[i] = etrd * (1.f - exp(-0.5f * m_lai[i])) + etae * min(2.f, m_lai[i]);
    return es_max;
}

int AET_PT_H::Execute() {
    CheckInputData();
    InitialOutputs();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        /// define intermediate variables
        float esd = 0.f, etco = 0.f, effnup = 0.f;
        float no3up = 0.f, es_max = 0.f, eos1 = 0.f, xx = 0.f;
        float cej = 0.f, eaj = 0.f, pet = 0.f, esleft = 0.f;
        float evzp = 0.f, eosl = 0.f, dep = 0.f, evz = 0.f, sev = 0.f;
        pet = m_pet[i] - m_IntcpET[i]; /// Actually, m_IntcpET has been compared with m_pet in other module.
        esd = 500.f;
        etco = 0.8f;
        effnup = 0.1f;

        if (pet < UTIL_ZERO) {
            m_maxPltET[i] = 0.f; // i.e., ep_max in SWAT
            m_soilET[i] = 0.f;   // i.e., es_max in SWAT
            continue;
        }
        
        if (CVT_INT(m_landuse[i]) == LANDUSE_ID_PADDY && m_cropsta[i] == 4.f){
			// if the cell is paddy and rice in main field
			// add oryza method to compute soil evaporation and crop transpiration, by sf 2017.11.29
			es_max = ORYZA_maxPET(pet, i);
		}
		else{
			es_max = SWAT_maxPET(pet, i);
		}        

        /// initialize soil evaporation variables
        esleft = es_max;
        /// compute sublimation, using the input m_snowSB from snow sublimation module, if not provided, initialized as 0
        if (m_tMean[i] > 0.f) {
            if (m_snowAccum[i] >= esleft) {
                /// take all soil evap from snow cover
                m_snowAccum[i] -= esleft;
                m_snowSublim[i] += esleft;
                esleft = 0.f;
            } else {
                /// take all soil evap from snow cover then start taking from soil
                esleft -= m_snowAccum[i];
                m_snowSublim[i] += m_snowAccum[i];
                m_snowAccum[i] = 0.f;
            }
        }
        // take soil evap from each soil layer
        evzp = 0.f;
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
                        (m_soilDepth[i][ly] + exp(2.374f - 0.00713f * m_soilDepth[i][ly]));
                sev = evz - evzp * m_esco[i];
                evzp = evz;
                if (m_soilWtrSto[i][ly] < m_solFC[i][ly]) {
                    xx = 2.5f * (m_soilWtrSto[i][ly] - m_solFC[i][ly]) / m_solFC[i][ly]; /// non dimension
                    sev *= Expo(xx);
                }
                sev = Min(sev, m_soilWtrSto[i][ly] * etco);
                if (sev < 0.f || sev != sev) sev = 0.f;
                if (sev > esleft) sev = esleft;
                /// adjust soil storage, potential evap
                if (m_soilWtrSto[i][ly] > sev) {
                    esleft -= sev;
                    m_soilWtrSto[i][ly] = Max(UTIL_ZERO, m_soilWtrSto[i][ly] - sev);
                } else {
                    esleft -= m_soilWtrSto[i][ly];
                    m_soilWtrSto[i][ly] = 0.f;
                }
            }
            /// compute no3 flux from layer 2 to 1 by soil evaporation
            if (ly == 1) {
                /// index of layer 2 is 1 (soil surface, 10mm)
                no3up = 0.f;
                no3up = effnup * sev * m_solNo3[i][ly] / (m_soilWtrSto[i][ly] + UTIL_ZERO);
                no3up = Min(no3up, m_solNo3[i][ly]);
                m_solNo3[i][ly] -= no3up;
                m_solNo3[i][ly - 1] += no3up;
            }
        }
        /// update total soil water content
        m_soilWtrStoPrfl[i] = 0.f;
        for (int ly = 0; ly < CVT_INT(m_nSoilLyrs[i]); ly++) {
            m_soilWtrStoPrfl[i] += m_soilWtrSto[i][ly];
        }
        /// calculate actual amount of evaporation from soil
        //if (esleft != esleft || es_max != es_max)
        //	cout<<"esleft: "<<esleft<<", es_max: "<<es_max<<endl;
        if (es_max > esleft) {
            m_soilET[i] = es_max - esleft;
        } else {
            m_soilET[i] = 0.f;
        }
    }
    return 0;
}

void AET_PT_H::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_PPT)) *data = m_maxPltET;
    else if (StringMatch(sk, VAR_SOET)) *data = m_soilET;
    else {
        throw ModelException(MID_AET_PTH, "Get1DData", "Result " + sk + " does not exist.");
    }
    *n = m_nCells;
}
