#include "PER_STR.h"

#include "text.h"

PER_STR::PER_STR() :
    m_maxSoilLyrs(-1), m_nSoilLyrs(nullptr), m_soilThk(nullptr), m_dt(-1),
    m_nCells(-1), m_soilFrozenTemp(NODATA_VALUE), m_ks(nullptr),
    m_soilSat(nullptr), m_soilFC(nullptr),
    m_soilWtrSto(nullptr), m_soilWtrStoPrfl(nullptr), m_soilTemp(nullptr), m_infil(nullptr),
    m_surfRf(nullptr), m_potVol(nullptr), m_impoundTrig(nullptr),
    m_soilPerco(nullptr) {
}

PER_STR::~PER_STR() {
    if (m_soilPerco != nullptr) Release2DArray(m_soilPerco);
}

void PER_STR::InitialOutputs() {
    CHECK_POSITIVE(M_PER_STR[0], m_nCells);
    if (nullptr == m_soilPerco) Initialize2DArray(m_nCells, m_maxSoilLyrs, m_soilPerco, 0.);
}

int PER_STR::Execute() {
    CheckInputData();
    InitialOutputs();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        // Note that, infiltration, pothole seepage, irrigation etc. have been added to
        // the first soil layer in other modules. By LJ
        FLTPT excessWater = 0., maxSoilWater = 0., fcSoilWater = 0.;
        for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) {
            excessWater = 0.;
            maxSoilWater = m_soilSat[i][j];
            fcSoilWater = m_soilFC[i][j];
            // determine gravity drained water in layer
            excessWater += m_soilWtrSto[i][j] - fcSoilWater;
            //if (i == 100)
            //	cout<<"lyr: "<<j<<", soil storage: "<<m_soilStorage[i][j]<<", fc: "<<fcSoilWater<<", excess: "<<excessWater<<endl;
            // for the upper two layers, soil may be frozen
            if (j == 0 && m_soilTemp[i] <= m_soilFrozenTemp) {
                continue;
            }
            m_soilPerco[i][j] = 0.;
            // No movement if soil moisture is below field capacity
            if (excessWater > 1.e-5) {
                FLTPT maxPerc = maxSoilWater - fcSoilWater;
                if (maxPerc < 0.) maxPerc = 0.1;
                FLTPT tt = 3600. * maxPerc / m_ks[i][j];                  // secs
                m_soilPerco[i][j] = excessWater * (1. - CalExp(-m_dt / tt)); // secs

                if (m_soilPerco[i][j] > maxPerc) {
                    m_soilPerco[i][j] = maxPerc;
                }
                //Adjust the moisture content in the current layer, and the layer immediately below it
                m_soilWtrSto[i][j] -= m_soilPerco[i][j];
                excessWater -= m_soilPerco[i][j];
                m_soilWtrSto[i][j] = Max(UTIL_ZERO, m_soilWtrSto[i][j]);
                // redistribute soil water if above field capacity (high water table), rewrite from sat_excess.f of SWAT
                //float qlyr = m_soilStorage[i][j];
                if (j < CVT_INT(m_nSoilLyrs[i]) - 1) {
                    m_soilWtrSto[i][j + 1] += m_soilPerco[i][j];
                    if (m_soilWtrSto[i][j] - m_soilSat[i][j] > 1.e-4) {
                        m_soilWtrSto[i][j + 1] += m_soilWtrSto[i][j] - m_soilSat[i][j];
                        m_soilWtrSto[i][j] = m_soilSat[i][j];
                    }
                } else {
                    /// for the last soil layer
                    if (m_soilWtrSto[i][j] - m_soilSat[i][j] > 1.e-4) {
                        FLTPT ul_excess = m_soilWtrSto[i][j] - m_soilSat[i][j];
                        m_soilWtrSto[i][j] = m_soilSat[i][j];
                        for (int ly = CVT_INT(m_nSoilLyrs[i]) - 2; ly >= 0; ly--) {
                            m_soilWtrSto[i][ly] += ul_excess;
                            if (m_soilWtrSto[i][ly] > m_soilSat[i][ly]) {
                                ul_excess = m_soilWtrSto[i][ly] - m_soilSat[i][ly];
                                m_soilWtrSto[i][ly] = m_soilSat[i][ly];
                            } else {
                                ul_excess = 0.;
                                break;
                            }
                            if (ly == 0 && ul_excess > 0.) {
                                // add ul_excess to depressional storage and then to surfq
                                if (m_potVol != nullptr && m_impoundTrig == 0) {
                                    m_potVol[i] += ul_excess;
                                } else {
                                    m_surfRf[i] += ul_excess;
                                }
                                m_infil[i] -= ul_excess;
                            }
                        }
                    }
                }
            } else {
                m_soilPerco[i][j] = 0.;
            }
        }
        /// update soil profile water
        m_soilWtrStoPrfl[i] = 0.;
        for (int ly = 0; ly < CVT_INT(m_nSoilLyrs[i]); ly++) {
            m_soilWtrStoPrfl[i] += m_soilWtrSto[i][ly];
        }
    }
    // DEBUG
    //cout << "PER_STR, cell id 14377, m_soilStorage: ";
    //for (int i = 0; i < (int)m_soilLayers[14377]; i++)
    //    cout << m_soilStorage[14377][i] << ", ";
    //cout << endl;
    // END OF DEBUG
    return 0;
}

void PER_STR::Get2DData(const char* key, int* nrows, int* ncols, FLTPT*** data) {
    InitialOutputs();
    string sk(key);
    *nrows = m_nCells;
    *ncols = m_maxSoilLyrs;
    if (StringMatch(sk, VAR_PERCO[0])) *data = m_soilPerco;
    else {
        throw ModelException(M_PER_STR[0], "Get2DData",
                             "Output " + sk + " does not exist.");
    }
}

void PER_STR::Set1DData(const char* key, const int nrows, FLTPT* data) {
    CheckInputSize(M_PER_STR[0], key, nrows, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_SOTE[0])) m_soilTemp = data;
    else if (StringMatch(sk, VAR_INFIL[0])) m_infil = data;
    else if (StringMatch(sk, VAR_SOL_SW[0])) m_soilWtrStoPrfl = data;
    else if (StringMatch(sk, VAR_POT_VOL[0])) m_potVol = data;
    else if (StringMatch(sk, VAR_SURU[0])) m_surfRf = data;
    else {
        throw ModelException(M_PER_STR[0], "Set1DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void PER_STR::Set1DData(const char* key, const int nrows, int* data) {
    CheckInputSize(M_PER_STR[0], key, nrows, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_SOILLAYERS[0])) m_nSoilLyrs = data;
    else if (StringMatch(sk, VAR_IMPOUND_TRIG[0])) m_impoundTrig = data;
    else {
        throw ModelException(M_PER_STR[0], "Set1DData",
                             "Integer Parameter " + sk + " does not exist.");
    }
}

void PER_STR::Set2DData(const char* key, const int nrows, const int ncols, FLTPT** data) {
    CheckInputSize2D(M_PER_STR[0], key, nrows, ncols, m_nCells, m_maxSoilLyrs);
    string sk(key);
    if (StringMatch(sk, VAR_CONDUCT[0])) m_ks = data;
    else if (StringMatch(sk, VAR_SOILTHICK[0])) m_soilThk = data;
    else if (StringMatch(sk, VAR_SOL_UL[0])) m_soilSat = data;
    else if (StringMatch(sk, VAR_SOL_AWC[0])) m_soilFC = data;
    else if (StringMatch(sk, VAR_SOL_ST[0])) m_soilWtrSto = data;
    else {
        throw ModelException(M_PER_STR[0], "Set2DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void PER_STR::SetValue(const char* key, const FLTPT value) {
    string s(key);
    if (StringMatch(s, VAR_T_SOIL[0])) m_soilFrozenTemp = value;
    else {
        throw ModelException(M_PER_STR[0], "SetValue",
                             "Parameter " + s + " does not exist in current module.");
    }
}

void PER_STR::SetValue(const char* key, const int value) {
    string s(key);
    if (StringMatch(s, Tag_TimeStep[0])) m_dt = value;
    else {
        throw ModelException(M_PER_STR[0], "SetValue",
                             "Integer Parameter " + s + " does not exist in current module.");
    }
}

bool PER_STR::CheckInputData() {
    CHECK_POSITIVE(M_PER_STR[0], m_date);
    CHECK_POSITIVE(M_PER_STR[0], m_nCells);
    CHECK_POSITIVE(M_PER_STR[0], m_dt);
    CHECK_POINTER(M_PER_STR[0], m_ks);
    CHECK_POINTER(M_PER_STR[0], m_soilSat);
    CHECK_NODATA(M_PER_STR[0], m_soilFrozenTemp);
    CHECK_POINTER(M_PER_STR[0], m_soilFC);
    CHECK_POINTER(M_PER_STR[0], m_soilWtrSto);
    CHECK_POINTER(M_PER_STR[0], m_soilWtrStoPrfl);
    CHECK_POINTER(M_PER_STR[0], m_soilThk);
    CHECK_POINTER(M_PER_STR[0], m_soilTemp);
    CHECK_POINTER(M_PER_STR[0], m_infil);
    return true;
}
