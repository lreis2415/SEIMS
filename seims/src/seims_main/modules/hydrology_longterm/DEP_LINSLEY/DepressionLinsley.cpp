#include "DepressionLinsley.h"

#include "text.h"

DepressionFSDaily::DepressionFSDaily() :
    m_nCells(-1), m_impoundTriger(nullptr),
    m_potVol(nullptr),
    m_depCo(NODATA_VALUE), m_depCap(nullptr), m_pet(nullptr),
    m_ei(nullptr), m_pe(nullptr), m_sd(nullptr),
    m_ed(nullptr), m_sr(nullptr) {
}

DepressionFSDaily::~DepressionFSDaily() {
    if (m_sd != nullptr) Release1DArray(m_sd);
    if (m_ed != nullptr) Release1DArray(m_ed);
    if (m_sr != nullptr) Release1DArray(m_sr);
}

bool DepressionFSDaily::CheckInputData() {
    CHECK_POSITIVE(MID_DEP_LINSLEY, m_date);
    CHECK_POSITIVE(MID_DEP_LINSLEY, m_nCells);
    CHECK_NODATA(MID_DEP_LINSLEY, m_depCo);
    CHECK_POINTER(MID_DEP_LINSLEY, m_depCap);
    CHECK_POINTER(MID_DEP_LINSLEY, m_pet);
    CHECK_POINTER(MID_DEP_LINSLEY, m_ei);
    CHECK_POINTER(MID_DEP_LINSLEY, m_pe);
    return true;
}

void DepressionFSDaily::InitialOutputs() {
    CHECK_POSITIVE(MID_DEP_LINSLEY, m_nCells);
    if (nullptr == m_sd) {
        Initialize1DArray(m_nCells, m_sd, 0.f);
        Initialize1DArray(m_nCells, m_ed, 0.f);
        Initialize1DArray(m_nCells, m_sr, 0.f);
    }
}

void DepressionFSDaily::InitializeIntermediateVariables(){
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        m_sd[i] = m_depCo * m_depCap[i];
    }

    m_needReCalIntermediateParams = false;
}

int DepressionFSDaily::Execute() {
    CheckInputData();
    InitialOutputs();
    if (m_needReCalIntermediateParams) InitializeIntermediateVariables();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        //////////////////////////////////////////////////////////////////////////
        // runoff
        if (m_depCap[i] < 0.001f) {
            m_sr[i] = m_pe[i];
            m_sd[i] = 0.f;
        } else if (m_pe[i] > 0.f) {
            float pc = m_pe[i] - m_depCap[i] * log(1.f - m_sd[i] / m_depCap[i]);
            float deltaSd = m_pe[i] * exp(-pc / m_depCap[i]);
            if (deltaSd > m_depCap[i] - m_sd[i]) {
                deltaSd = m_depCap[i] - m_sd[i];
            }
            m_sd[i] += deltaSd;
            m_sr[i] = m_pe[i] - deltaSd;
        } else {
            m_sd[i] += m_pe[i];
            m_sr[i] = 0.f;
        }

        //////////////////////////////////////////////////////////////////////////
        // evaporation
        if (m_sd[i] > 0) {
            /// TODO: Is this logically right? PET is just potential, which include
            ///       not only ET from surface water, but also from plant and soil.
            ///       Please Check the corresponding theory. By LJ.
            // evaporation from depression storage
            if (m_pet[i] - m_ei[i] < m_sd[i]) {
                m_ed[i] = m_pet[i] - m_ei[i];
            } else {
                m_ed[i] = m_sd[i];
            }
            m_sd[i] -= m_ed[i];
        } else {
            m_ed[i] = 0.f;
            m_sd[i] = 0.f;
        }
        if (m_impoundTriger != nullptr && FloatEqual(m_impoundTriger[i], 0.f)) {
            if (m_potVol != nullptr) {
                m_potVol[i] += m_sr[i];
                m_potVol[i] += m_sd[i];
                m_sr[i] = 0.f;
                m_sd[i] = 0.f;
            }
        }
    }
    return true;
}

void DepressionFSDaily::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, VAR_DEPREIN)) m_depCo = value;
    else {
        throw ModelException(MID_DEP_LINSLEY, "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void DepressionFSDaily::Set1DData(const char* key, const int n, float* data) {
    CheckInputSize(MID_DEP_LINSLEY, key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_DEPRESSION)) {
        m_depCap = data;
    } else if (StringMatch(sk, VAR_INET)) {
        m_ei = data;
    } else if (StringMatch(sk, VAR_PET)) {
        m_pet = data;
    } else if (StringMatch(sk, VAR_EXCP)) {
        m_pe = data;
    } else if (StringMatch(sk, VAR_IMPOUND_TRIG)) {
        m_impoundTriger = data;
    } else if (StringMatch(sk, VAR_POT_VOL)) {
        m_potVol = data;
    } else {
        throw ModelException(MID_DEP_LINSLEY, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void DepressionFSDaily::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_DPST)) {
        *data = m_sd;
    } else if (StringMatch(sk, VAR_DEET)) {
        *data = m_ed;
    } else if (StringMatch(sk, VAR_SURU)) {
        *data = m_sr;
    } else {
        throw ModelException(MID_DEP_LINSLEY, "Get1DData", "Output " + sk + " does not exist.");
    }
}
