#include "seims.h"
#include "PER_PI.h"

PER_PI::PER_PI() : m_soilLayers(-1), m_dt(-1), m_nCells(-1), m_frozenT(NODATA_VALUE),
                   m_ks(nullptr), m_sat(nullptr), m_poreIndex(nullptr), m_fc(nullptr),
                   m_wp(nullptr), m_soilThick(nullptr), m_impoundTriger(nullptr),
                   m_infil(nullptr), m_soilT(nullptr), m_soilStorage(nullptr),
                   m_perc(nullptr) {
}

PER_PI::~PER_PI() {
    if (m_perc != nullptr) Release2DArray(m_nCells, m_perc);
}

void PER_PI::initialOutputs() {
    CHECK_POSITIVE(MID_PER_PI, m_nCells);
    if (nullptr == m_perc) { Initialize2DArray(m_nCells, m_soilLayers, m_perc, NODATA_VALUE); }
}

int PER_PI::Execute() {
    CheckInputData();
    initialOutputs();

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        /// firstly, assume all infiltrated water is added to the first soil layer.
        // this step is removed to surface runoff and infiltration module. by LJ, 2016-9-2
        //m_soilStorage[i][0] += m_infil[i];
        /// secondly, model water percolation across layers
        for (int j = 0; j < (int) m_nSoilLayers[i]; j++) {
            float k = 0.f, swater = 0.f, maxSoilWater = 0.f, fcSoilWater = 0.f;
            // for the upper two layers, soil may be frozen
            // No movement if soil moisture is below field capacity
            if (j == 0 && m_soilT[i] <= m_frozenT) {
                continue;
            }
            swater = m_soilStorage[i][j];
            maxSoilWater = m_sat[i][j];
            fcSoilWater = m_fc[i][j];

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
                    float dcIndex = 2.f * m_poreIndex[i][j] + 3.f; // pore disconnectedness index
                    k = m_ks[i][j] * pow(swater / maxSoilWater, dcIndex); // mm/h
                }

                m_perc[i][j] = k * m_dt / 3600.f;  // mm

                if (swater - m_perc[i][j] > maxSoilWater) {
                    m_perc[i][j] = swater - maxSoilWater;
                } else if (swater - m_perc[i][j] < fcSoilWater) {
                    m_perc[i][j] = swater - fcSoilWater;
                }

                if (m_perc[i][j] < 0.f) {
                    m_perc[i][j] = 0.f;
                }
                //Adjust the moisture content in the current layer, and the layer immediately below it
                m_soilStorage[i][j] -= m_perc[i][j];
                if (j < m_nSoilLayers[i] - 1) {
                    m_soilStorage[i][j + 1] += m_perc[i][j];
                }

                //if (m_soilStorage[i][j] != m_soilStorage[i][j] || m_soilStorage[i][j] < 0.f)
                //{
                //    cout << MID_PER_PI << " CELL:" << i << ", Layer: " << j << "\tPerco:" << swater << "\t" <<
                //    fcSoilWater << "\t" << m_perc[i][j] << "\t" << m_soilThick[i][j] << "\tValue:" << m_soilStorage[i][j] <<
                //    endl;
                //    throw ModelException(MID_PER_PI, "Execute", "moisture is less than zero.");
                //}
            } else {
                m_perc[i][j] = 0.f;
            }
        }
        //if (i == 1762)
        //{
        //	for (int j = 0; j < (int)m_nSoilLayers[i]; j++)
        //		cout<<"after, infil: "<<m_infil[i]<<", perco: "<<m_perc[i][j]<<", soilStorage: "<<m_soilStorage[i][j]<<endl;
        //}
        /// update total soil water content
        m_soilStorageProfile[i] = 0.f;
        for (int ly = 0; ly < (int) m_nSoilLayers[i]; ly++) {
            m_soilStorageProfile[i] += m_soilStorage[i][ly];
        }
    }
    return 0;
}

void PER_PI::Get2DData(const char *key, int *nRows, int *nCols, float ***data) {
    initialOutputs();
    string sk(key);
    *nRows = m_nCells;
    *nCols = m_soilLayers;
    if (StringMatch(sk, VAR_PERCO)) { *data = m_perc; }
    else {
        throw ModelException(MID_PER_PI, "Get2DData", "Output " + sk + " does not exist.");
    }
}

void PER_PI::Set1DData(const char *key, int nRows, float *data) {
    CheckInputSize(key, nRows);
    string sk(key);
    if (StringMatch(sk, VAR_SOTE)) { m_soilT = data; }
    else if (StringMatch(sk, VAR_INFIL)) { m_infil = data; }
    else if (StringMatch(sk, VAR_SOILLAYERS)) { m_nSoilLayers = data; }
    else if (StringMatch(sk, VAR_SOL_SW)) { m_soilStorageProfile = data; }
    else if (StringMatch(sk, VAR_IMPOUND_TRIG)) { m_impoundTriger = data; }
    else {
        throw ModelException(MID_PER_PI, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void PER_PI::Set2DData(const char *key, int nrows, int ncols, float **data) {
    CheckInputSize(key, nrows);
    string sk(key);
    m_soilLayers = ncols;
    if (StringMatch(sk, VAR_CONDUCT)) { m_ks = data; }
    else if (StringMatch(sk, VAR_SOILTHICK)) { m_soilThick = data; }
    else if (StringMatch(sk, VAR_SOL_AWC)) { m_fc = data; }
    else if (StringMatch(sk, VAR_SOL_WPMM)) { m_wp = data; }
    else if (StringMatch(sk, VAR_POREIDX)) { m_poreIndex = data; }
    else if (StringMatch(sk, VAR_SOL_UL)) { m_sat = data; }
    else if (StringMatch(sk, VAR_SOL_ST)) { m_soilStorage = data; }
    else {
        throw ModelException(MID_PER_PI, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void PER_PI::SetValue(const char *key, float data) {
    string s(key);
    if (StringMatch(s, Tag_TimeStep)) { m_dt = int(data); }
    else if (StringMatch(s, VAR_T_SOIL)) { m_frozenT = data; }
    else {
        throw ModelException(MID_PER_PI, "SetValue", "Parameter " + s + " does not exist.");
    }
}

bool PER_PI::CheckInputData() {
    CHECK_POSITIVE(MID_PER_PI, m_date);
    CHECK_POSITIVE(MID_PER_PI, m_nCells);
    CHECK_POSITIVE(MID_PER_PI, m_dt);
    CHECK_POINTER(MID_PER_PI, m_ks);
    CHECK_POINTER(MID_PER_PI, m_sat);
    CHECK_POINTER(MID_PER_PI, m_poreIndex);
    CHECK_POINTER(MID_PER_PI, m_fc);
    CHECK_POINTER(MID_PER_PI, m_wp);
    CHECK_POINTER(MID_PER_PI, m_soilThick);
    CHECK_POINTER(MID_PER_PI, m_soilT);
    CHECK_POINTER(MID_PER_PI, m_infil);
    CHECK_DATA(MID_PER_PI, FloatEqual(m_frozenT, NODATA_VALUE), "The threshold soil freezing temperature has not been set.");
    CHECK_POINTER(MID_PER_PI, m_soilStorage);
    CHECK_POINTER(MID_PER_PI, m_soilStorageProfile);
    return true;
}

bool PER_PI::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_PER_PI, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; }
        else {
            throw ModelException(MID_PER_PI, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
        }
    }
    return true;
}
