#include "seims.h"
#include "SUR_MR.h"

SUR_MR::SUR_MR() : m_nCells(-1), m_dt(-1), m_nSoilLayers(-1), m_tFrozen(NODATA_VALUE),
                   m_kRunoff(NODATA_VALUE), m_pMax(NODATA_VALUE),
                   m_sFrozen(NODATA_VALUE), m_runoffCo(nullptr), m_initSoilStorage(nullptr), m_tMean(nullptr),
                   m_sol_awc(nullptr), m_sol_ul(nullptr), m_sol_sumsat(nullptr), m_soilLayers(nullptr),
                   m_pNet(nullptr), m_sd(nullptr), m_soilTemp(nullptr), m_potVol(nullptr), m_impoundTrig(nullptr),
                   m_pe(nullptr), m_infil(nullptr), m_soilStorage(nullptr), m_soilStorageProfile(nullptr) {
}

SUR_MR::~SUR_MR() {
    if (m_pe != nullptr) Release1DArray(m_pe);
    if (m_infil != nullptr) Release1DArray(m_infil);
    if (m_soilStorage != nullptr) Release2DArray(m_nCells, m_soilStorage);
    if (m_soilStorageProfile != nullptr) Release1DArray(m_soilStorageProfile);
}

void SUR_MR::CheckInputData() {
    CHECK_POSITIVE(MID_SUR_MR, m_date);
    CHECK_POSITIVE(MID_SUR_MR, m_dt);
    CHECK_POSITIVE(MID_SUR_MR, m_nCells);
    CHECK_NODATA(MID_SUR_MR, m_tFrozen);
    CHECK_NODATA(MID_SUR_MR, m_kRunoff);
    CHECK_NODATA(MID_SUR_MR, m_pMax);
    CHECK_NODATA(MID_SUR_MR, m_sFrozen);
    CHECK_POINTER(MID_SUR_MR, m_initSoilStorage);
    CHECK_POINTER(MID_SUR_MR, m_runoffCo);
    CHECK_POINTER(MID_SUR_MR, m_sol_awc);
    CHECK_POINTER(MID_SUR_MR, m_tMean);
    CHECK_POINTER(MID_SUR_MR, m_soilTemp);
    CHECK_POINTER(MID_SUR_MR, m_pNet);
    CHECK_POINTER(MID_SUR_MR, m_sd);
}

void SUR_MR::initialOutputs() {
    CHECK_POSITIVE(MID_SUR_MR, m_nCells);
    // allocate the output variables
    if (nullptr == m_pe) {
        Initialize1DArray(m_nCells, m_pe, 0.f);
        Initialize1DArray(m_nCells, m_infil, 0.f);
        Initialize1DArray(m_nCells, m_soilStorageProfile, 0.f);
        Initialize2DArray(m_nCells, m_nSoilLayers, m_soilStorage, NODATA_VALUE);
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            for (int j = 0; j < (int) m_soilLayers[i]; j++) { /// mm
                if (m_initSoilStorage[i] >= 0.f && m_initSoilStorage[i] <= 1.f && m_sol_awc[i][j] >= 0.f) {
                    m_soilStorage[i][j] = m_initSoilStorage[i] * m_sol_awc[i][j];
                } else {
                    m_soilStorage[i][j] = 0.f;
                }
                m_soilStorageProfile[i] += m_soilStorage[i][j];
            }
        }
    }
    /// update (sol_sumul) amount of water held in soil profile at saturation
    if (m_sol_sumsat == NULL && m_sol_ul != NULL) {
        m_sol_sumsat = new float[m_nCells];
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            m_sol_sumsat[i] = 0.f;
            for (int j = 0; j < (int)m_soilLayers[i]; j++) { /// mm
                m_sol_sumsat[i] += m_sol_ul[i][j];
            }
        }
    }
}

int SUR_MR::Execute() {
    CheckInputData();
    initialOutputs();
    m_pMax = m_pMax * m_dt / 86400.f;
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        float hWater = 0.f;
        hWater = m_pNet[i] + m_sd[i];
        if (hWater > 0.f) {
            /// update total soil water content
            m_soilStorageProfile[i] = 0.f;
            for (int ly = 0; ly < (int) m_soilLayers[i]; ly++) {
                m_soilStorageProfile[i] += m_soilStorage[i][ly];
            }
            float smFraction = min(m_soilStorageProfile[i] / m_sol_sumsat[i], 1.f);
            // for frozen soil, no infiltration will occur
            if (m_soilTemp[i] <= m_tFrozen && smFraction >= m_sFrozen) {
                m_pe[i] = m_pNet[i];
                m_infil[i] = 0.f;
            } else {
                float alpha = m_kRunoff - (m_kRunoff - 1.f) * hWater / m_pMax;
                if (hWater >= m_pMax) {
                    alpha = 1.f;
                }

                //runoff percentage
                float runoffPercentage;
                if (m_runoffCo[i] > 0.99f) {
                    runoffPercentage = 1.f;
                } else {
                    runoffPercentage = m_runoffCo[i] * pow(smFraction, alpha);
                }

                float surfq = hWater * runoffPercentage;
                if (surfq > hWater) surfq = hWater;
                m_infil[i] = hWater - surfq;
                m_pe[i] = surfq;

                /// TODO: Why calculate surfq first, rather than infiltration first?
                ///       I think we should calculate infiltration first, until saturation,
                ///       then surface runoff should be calculated. By LJ.
            }
        } else {
            m_pe[i] = 0.f;
            m_infil[i] = 0.f;
        }
        if (m_infil[i] > 0.f) {
            m_soilStorage[i][0] += m_infil[i];
        }
    }
    return 0;
}

bool SUR_MR::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_SUR_MR, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; }
        else {
            throw ModelException(MID_SUR_MR, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
        }
    }
    return true;
}

void SUR_MR::SetValue(const char *key, float value) {
    string sk(key);
    if (StringMatch(key, VAR_OMP_THREADNUM)) { SetOpenMPThread((int) value); }
    else if (StringMatch(sk, Tag_HillSlopeTimeStep)) { m_dt = value; }
    else if (StringMatch(sk, VAR_T_SOIL)) { m_tFrozen = value; }
    else if (StringMatch(sk, VAR_K_RUN)) { m_kRunoff = value; }
    else if (StringMatch(sk, VAR_P_MAX)) { m_pMax = value; }
    else if (StringMatch(sk, VAR_S_FROZEN)) { m_sFrozen = value; }
    else {
        throw ModelException(MID_SUR_MR, "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void SUR_MR::Set1DData(const char *key, int n, float *data) {
    CheckInputSize(key, n);
    string sk(key);
    if (StringMatch(sk, VAR_RUNOFF_CO)) { m_runoffCo = data; }
    else if (StringMatch(sk, VAR_NEPR)) { m_pNet = data; }
    else if (StringMatch(sk, VAR_TMEAN)) { m_tMean = data; }
    else if (StringMatch(sk, VAR_MOIST_IN)) { m_initSoilStorage = data; }
    else if (StringMatch(sk, VAR_DPST)) { m_sd = data; }
    else if (StringMatch(sk, VAR_SOTE)) { m_soilTemp = data; }
    else if (StringMatch(sk, VAR_SOILLAYERS)) { m_soilLayers = data; }
    else if (StringMatch(sk, VAR_POT_VOL)) { m_potVol = data; }
    else if (StringMatch(sk, VAR_IMPOUND_TRIG)) { m_impoundTrig = data; } 
    else {
        throw ModelException(MID_SUR_MR, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void SUR_MR::Set2DData(const char *key, int nrows, int ncols, float **data) {
    string sk(key);
    CheckInputSize(key, nrows);
    m_nSoilLayers = ncols;
    if (StringMatch(sk, VAR_SOL_AWC)) { m_sol_awc = data; }
    else if (StringMatch(sk, VAR_SOL_UL)) { m_sol_ul = data; }
    else {
        throw ModelException(MID_SUR_MR, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void SUR_MR::Get1DData(const char *key, int *n, float **data) {
    initialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_INFIL)) {
        *data = m_infil;     //infiltration
    } else if (StringMatch(sk, VAR_EXCP)) {
        *data = m_pe; // excess precipitation
    } else if (StringMatch(sk, VAR_SOL_SW)) { *data = m_soilStorageProfile; }
    else {
        throw ModelException(MID_SUR_MR, "Get1DData", "Result " + sk + " does not exist.");
    }
    *n = m_nCells;
}

void SUR_MR::Get2DData(const char *key, int *nRows, int *nCols, float ***data) {
    initialOutputs();
    string sk(key);
    *nRows = m_nCells;
    *nCols = m_nSoilLayers;
    if (StringMatch(sk, VAR_SOL_ST)) {
        *data = m_soilStorage;
    } else {
        throw ModelException(MID_SUR_MR, "Get2DData", "Output " + sk + " does not exist.");
    }
}
