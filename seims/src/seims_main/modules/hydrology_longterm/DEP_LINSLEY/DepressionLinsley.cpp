#include "DepressionLinsley.h"

#include "text.h"

DepressionFSDaily::DepressionFSDaily() :
    m_nCells(-1), m_impoundTriger(nullptr),
    m_potVol(nullptr),
    m_depCo(NODATA_VALUE), m_depCap(nullptr), m_pet(nullptr),
    m_ei(nullptr), m_pe(nullptr), m_sd(nullptr),
    m_ed(nullptr), m_sr(nullptr) {
    SetModuleName(M_DEP_LINSLEY[0]);
}

DepressionFSDaily::~DepressionFSDaily() {
    Release1DArray(m_sd);
    Release1DArray(m_ed);
    Release1DArray(m_sr);
}

bool DepressionFSDaily::CheckInputData() {
    CHECK_POSITIVE(GetModuleName(), m_date);
    CHECK_POSITIVE(GetModuleName(), m_nCells);
    CHECK_NODATA(GetModuleName(), m_depCo);
    CHECK_POINTER(GetModuleName(), m_depCap);
    CHECK_POINTER(GetModuleName(), m_pet);
    CHECK_POINTER(GetModuleName(), m_ei);
    CHECK_POINTER(GetModuleName(), m_pe);
    return true;
}

void DepressionFSDaily::InitialOutputs() {
    CHECK_POSITIVE(GetModuleName(), m_nCells);
    if (nullptr == m_sd) {
        Initialize1DArray(m_nCells, m_sd, 0.);
        Initialize1DArray(m_nCells, m_ed, 0.);
        Initialize1DArray(m_nCells, m_sr, 0.);
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            m_sd[i] = m_depCo * m_depCap[i];
        }
    }
}

int DepressionFSDaily::Execute() {
    CheckInputData();
    InitialOutputs();
    FLTPT deltaSdSum = 0;
    FLTPT sdSum0 = 0;
    FLTPT sdSum1 = 0;
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        //////////////////////////////////////////////////////////////////////////
        // runoff
        // if depression depth is 0, all excess precipitation is runoff, no depression storage
        if (m_depCap[i] < 0.001) {
            m_sr[i] = m_pe[i];
            m_sd[i] = 0.;
        } else if (m_pe[i] > 0.) {
			// if depression depth > 0 and excess precp >0, runoff = excess precp - depression storage
            FLTPT pc = m_pe[i] - m_depCap[i] * CalLn(1. - m_sd[i] / m_depCap[i]);
            FLTPT deltaSd = m_pe[i] * CalExp(-pc / m_depCap[i]);
#ifdef PRINT_DEBUG
            if (isnan(pc)) {
                printf("[DEP_LINS] pc %f = %f - %f * CalLn(1 - %f / %f)\n", pc, m_pe[i], m_depCap[i], m_sd[i], m_depCap[i]);
                printf("[DEP_LINS] deltaSd %f = %f * CalExp(%f / %f)\n", deltaSd, m_pe[i], -pc, m_depCap[i]);
            }
#endif

            if (deltaSd > m_depCap[i] - m_sd[i]) {
                deltaSd = m_depCap[i] - m_sd[i];
            }
            deltaSdSum += deltaSd;
            m_sd[i] += deltaSd;
            m_sr[i] = m_pe[i] - deltaSd;
        } else {
			m_sd[i] += m_pe[i];
            m_sr[i] = 0.;
        }
        sdSum0 += m_sd[i];
        //////////////////////////////////////////////////////////////////////////
        // evaporation only if has depression storage
        /// TODO: Is this logically right? PET is just potential, which include
        ///       not only ET from surface water, but also from plant and soil.
        ///       Please Check the corresponding theory. By LJ.
        /// 2023.12.06: Not sure if this is theoretically right.
        ///             But I refactored the code while keeping its original logic. -- WYJ.
        m_ed[i] = 0.;
        FLTPT restET = m_pet[i] - m_ei[i];
        if (restET<0) {
            printf("[DepressionLinsley] Warning! restET(%f) < 0! m_pet[%d](%f) m_ei[%d](%f)\n", restET, i, m_pet[i], i, m_ei[i]);
            restET = 0;
        }
        Convey(m_sd[i], m_ed[i], restET, 1);
        if (m_impoundTriger != nullptr && FloatEqual(m_impoundTriger[i], 0.f)) {
            if (m_potVol != nullptr) {
                Flush(m_sr[i], m_potVol[i]);
                Flush(m_sd[i], m_potVol[i]);
            }
        }
        sdSum1 += m_sd[i];

    }
#ifdef PRINT_DEBUG
    FLTPT s1=0;
    FLTPT s2=0;
    FLTPT s3=0;
    FLTPT s4=0;
    for (int i = 0; i < m_nCells; i++) {
        s1 += m_pe[i];
        s2 += m_sd[i];
        s3 += m_ed[i];
        s4 += m_sr[i];
    }
    printf("[DEP_LINSLEY] deltaSdSum(%f), sdSum@runoff(%f), sdSum@evap(%f) \n", deltaSdSum, sdSum0, sdSum1);
    printf("[DEP_LINSLEY] pe->dep+E+SURU: %f -> %f + %f + %f\n", s1, s2, s3, s4);
    fflush(stdout);
#endif

    return true;
}

void DepressionFSDaily::SetValue(const char* key, const FLTPT value) {
    string sk(key);
    if (StringMatch(sk, VAR_DEPREIN[0])) m_depCo = value;
    else {
        throw ModelException(GetModuleName(), "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void DepressionFSDaily::Set1DData(const char* key, const int n, FLTPT* data) {
    CheckInputSize(key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_DEPRESSION[0])) {
        m_depCap = data;
    } else if (StringMatch(sk, VAR_INET[0])) {
        m_ei = data;
    } else if (StringMatch(sk, VAR_PET[0])) {
        m_pet = data;
    } else if (StringMatch(sk, VAR_EXCP[0])) {
        m_pe = data;
    } else if (StringMatch(sk, VAR_POT_VOL[0])) {
        m_potVol = data;
    } else if (StringMatch(sk, VAR_SURU[0])) {
        m_sr = data;
    } else {
        throw ModelException(GetModuleName(), "Set1DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void DepressionFSDaily::Set1DData(const char* key, const int n, int* data) {
    CheckInputSize(key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_IMPOUND_TRIG[0])) {m_impoundTriger = data;}

    else {
        throw ModelException(GetModuleName(), "Set1DData",
                             "Integer Parameter " + sk + " does not exist.");
    }
}

void DepressionFSDaily::Get1DData(const char* key, int* n, FLTPT** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_DPST[0])) {
        *data = m_sd;
    } else if (StringMatch(sk, VAR_DEET[0])) {
        *data = m_ed;
    } else if (StringMatch(sk, VAR_SURU[0])) {
        *data = m_sr;
    } else {
        throw ModelException(GetModuleName(), "Get1DData",
                             "Output " + sk + " does not exist.");
    }
}
