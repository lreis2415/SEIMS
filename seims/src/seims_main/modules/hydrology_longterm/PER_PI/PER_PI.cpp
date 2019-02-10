#include "PER_PI.h"

#include "text.h"

PER_PI::PER_PI() :
    m_maxSoilLyrs(-1), m_nSoilLyrs(nullptr), m_soilThk(nullptr),
    m_dt(-1), m_nCells(-1), m_soilFrozenTemp(NODATA_VALUE),
    m_ks(nullptr), m_soilSat(nullptr), m_soilFC(nullptr), m_soilWP(nullptr),
    m_poreIdx(nullptr), m_soilWtrSto(nullptr), m_soilWtrStoPrfl(nullptr),
    m_soilTemp(nullptr), m_infil(nullptr), m_impndTrig(nullptr),
    m_soilPerco(nullptr) {
}

PER_PI::~PER_PI() {
    if (m_soilPerco != nullptr) Release2DArray(m_nCells, m_soilPerco);
}

void PER_PI::InitialOutputs() {
    CHECK_POSITIVE(MID_PER_PI, m_nCells);
    if (nullptr == m_soilPerco) Initialize2DArray(m_nCells, m_maxSoilLyrs, m_soilPerco, NODATA_VALUE);
}

int PER_PI::Execute() {
    CheckInputData();
    InitialOutputs();

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        /// firstly, assume all infiltrated water is added to the first soil layer.
        // this step is removed to surface runoff and infiltration module. by LJ, 2016-9-2
        //m_soilStorage[i][0] += m_infil[i];
        /// secondly, model water percolation across layers
        for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) {
            float k = 0.f, swater = 0.f, maxSoilWater = 0.f, fcSoilWater = 0.f;
            // for the upper two layers, soil may be frozen
            // No movement if soil moisture is below field capacity
            if (j == 0 && m_soilTemp[i] <= m_soilFrozenTemp) {
                continue;
            }
            swater = m_soilWtrSto[i][j];
            maxSoilWater = m_soilSat[i][j];
            fcSoilWater = m_soilFC[i][j];

            //bool percAllowed = true;
            //if (j < (int)m_nSoilLayers[i] -1 ){
            //	float nextSoilWater = 0.f;
            //	nextSoilWater = m_soilStorage[i][j+1];
            //	if (nextSoilWater >= m_fc[i][j+1])
            //		percAllowed = false;
            //}

            if (swater > fcSoilWater) {
                //if (i == 1762)
                //	cout<<"PER_PI, layer: "<<j<<", swater: "<<swater<<", max: "<<maxSoilWater<<", fc: "<<fcSoilWater<<endl;

                //the moisture content can exceed the porosity in the way the algorithm is implemented
                if (swater > maxSoilWater) {
                    k = m_ks[i][j];
                } else {
                    /// Using Clapp and Hornberger (1978) equation to calculate unsaturated hydraulic conductivity.
                    float dcIndex = 2.f * m_poreIdx[i][j] + 3.f;          // pore disconnectedness index
                    k = m_ks[i][j] * pow(swater / maxSoilWater, dcIndex); // mm/h
                }

                m_soilPerco[i][j] = k * m_dt / 3600.f; // mm

                if (swater - m_soilPerco[i][j] > maxSoilWater) {
                    m_soilPerco[i][j] = swater - maxSoilWater;
                } else if (swater - m_soilPerco[i][j] < fcSoilWater) {
                    m_soilPerco[i][j] = swater - fcSoilWater;
                }

                if (m_soilPerco[i][j] < 0.f) {
                    m_soilPerco[i][j] = 0.f;
                }
                //Adjust the moisture content in the current layer, and the layer immediately below it
                m_soilWtrSto[i][j] -= m_soilPerco[i][j];
                if (j < m_nSoilLyrs[i] - 1) {
                    m_soilWtrSto[i][j + 1] += m_soilPerco[i][j];
                }

                //if (m_soilStorage[i][j] != m_soilStorage[i][j] || m_soilStorage[i][j] < 0.f)
                //{
                //    cout << MID_PER_PI << " CELL:" << i << ", Layer: " << j << "\tPerco:" << swater << "\t" <<
                //    fcSoilWater << "\t" << m_perc[i][j] << "\t" << m_soilThick[i][j] << "\tValue:" << m_soilStorage[i][j] <<
                //    endl;
                //    throw ModelException(MID_PER_PI, "Execute", "moisture is less than zero.");
                //}
            } else {
                m_soilPerco[i][j] = 0.f;
            }
        }
        //if (i == 1762)
        //{
        //	for (int j = 0; j < (int)m_nSoilLayers[i]; j++)
        //		cout<<"after, infil: "<<m_infil[i]<<", perco: "<<m_perc[i][j]<<", soilStorage: "<<m_soilStorage[i][j]<<endl;
        //}
        /// update total soil water content
        m_soilWtrStoPrfl[i] = 0.f;
        for (int ly = 0; ly < CVT_INT(m_nSoilLyrs[i]); ly++) {
            m_soilWtrStoPrfl[i] += m_soilWtrSto[i][ly];
        }
    }
    return 0;
}

void PER_PI::Get2DData(const char* key, int* nRows, int* nCols, float*** data) {
    InitialOutputs();
    string sk(key);
    *nRows = m_nCells;
    *nCols = m_maxSoilLyrs;
    if (StringMatch(sk, VAR_PERCO)) *data = m_soilPerco;
    else {
        throw ModelException(MID_PER_PI, "Get2DData", "Output " + sk + " does not exist.");
    }
}

void PER_PI::Set1DData(const char* key, const int nrows, float* data) {
    CheckInputSize(MID_PER_PI, key, nrows, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_SOTE)) m_soilTemp = data;
    else if (StringMatch(sk, VAR_INFIL)) m_infil = data;
    else if (StringMatch(sk, VAR_SOILLAYERS)) m_nSoilLyrs = data;
    else if (StringMatch(sk, VAR_SOL_SW)) m_soilWtrStoPrfl = data;
    else if (StringMatch(sk, VAR_IMPOUND_TRIG)) m_impndTrig = data;
    else {
        throw ModelException(MID_PER_PI, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void PER_PI::Set2DData(const char* key, const int nrows, const int ncols, float** data) {
    CheckInputSize2D(MID_PER_PI, key, nrows, ncols, m_nCells, m_maxSoilLyrs);
    string sk(key);
    if (StringMatch(sk, VAR_CONDUCT)) m_ks = data;
    else if (StringMatch(sk, VAR_SOILTHICK)) m_soilThk = data;
    else if (StringMatch(sk, VAR_SOL_AWC)) m_soilFC = data;
    else if (StringMatch(sk, VAR_SOL_WPMM)) m_soilWP = data;
    else if (StringMatch(sk, VAR_POREIDX)) m_poreIdx = data;
    else if (StringMatch(sk, VAR_SOL_UL)) m_soilSat = data;
    else if (StringMatch(sk, VAR_SOL_ST)) m_soilWtrSto = data;
    else {
        throw ModelException(MID_PER_PI, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void PER_PI::SetValue(const char* key, const float value) {
    string s(key);
    if (StringMatch(s, Tag_TimeStep)) m_dt = CVT_INT(value);
    else if (StringMatch(s, VAR_T_SOIL)) m_soilFrozenTemp = value;
    else {
        throw ModelException(MID_PER_PI, "SetValue", "Parameter " + s + " does not exist.");
    }
}

bool PER_PI::CheckInputData() {
    CHECK_POSITIVE(MID_PER_PI, m_date);
    CHECK_POSITIVE(MID_PER_PI, m_nCells);
    CHECK_POSITIVE(MID_PER_PI, m_dt);
    CHECK_POINTER(MID_PER_PI, m_ks);
    CHECK_POINTER(MID_PER_PI, m_soilSat);
    CHECK_POINTER(MID_PER_PI, m_poreIdx);
    CHECK_POINTER(MID_PER_PI, m_soilFC);
    CHECK_POINTER(MID_PER_PI, m_soilWP);
    CHECK_POINTER(MID_PER_PI, m_soilThk);
    CHECK_POINTER(MID_PER_PI, m_soilTemp);
    CHECK_POINTER(MID_PER_PI, m_infil);
    CHECK_NODATA(MID_PER_PI, m_soilFrozenTemp);
    CHECK_POINTER(MID_PER_PI, m_soilWtrSto);
    CHECK_POINTER(MID_PER_PI, m_soilWtrStoPrfl);
    return true;
}
