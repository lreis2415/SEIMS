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
    CHECK_POSITIVE(M_DEP_LINSLEY[0], m_date);
    CHECK_POSITIVE(M_DEP_LINSLEY[0], m_nCells);
    CHECK_NODATA(M_DEP_LINSLEY[0], m_depCo);
    CHECK_POINTER(M_DEP_LINSLEY[0], m_depCap);
    CHECK_POINTER(M_DEP_LINSLEY[0], m_pet);
    CHECK_POINTER(M_DEP_LINSLEY[0], m_ei);
    CHECK_POINTER(M_DEP_LINSLEY[0], m_pe);
    return true;
}

void DepressionFSDaily::InitialOutputs() {
    CHECK_POSITIVE(M_DEP_LINSLEY[0], m_nCells);
    if (nullptr == m_sd) {
        Initialize1DArray(m_nCells, m_sd, 0.);
        Initialize1DArray(m_nCells, m_ed, 0.);
        Initialize1DArray(m_nCells, m_sr, 0.);
    }
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        m_sd[i] = m_depCo * m_depCap[i];
    }
}

int DepressionFSDaily::Execute() {
    CheckInputData();
    InitialOutputs();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        //////////////////////////////////////////////////////////////////////////
        // runoff
		// ����ݵ����Ϊ0��m_sr = m_pe����������ȣ��ݵ���ˮ��� = 0
        if (m_depCap[i] < 0.001) {
            m_sr[i] = m_pe[i];
            m_sd[i] = 0.;
        } else if (m_pe[i] > 0.) {
			// ����ݵ����> 0������Ǳ����ɢ��� > 0���ر�������� = ���� - ���������ݵ���ˮ = ������
            FLTPT pc = m_pe[i] - m_depCap[i] * CalLn(1. - m_sd[i] / m_depCap[i]);
            FLTPT deltaSd = m_pe[i] * CalExp(-pc / m_depCap[i]);
            if (deltaSd > m_depCap[i] - m_sd[i]) {
                deltaSd = m_depCap[i] - m_sd[i];
            }
            m_sd[i] += deltaSd;
            m_sr[i] = m_pe[i] - deltaSd;
        } else {
			// ����ݵ����> 0������Ǳ����ɢ��� = 0����ر�������� = 0���ݵ���ˮ��� = �ݵ���ˮ��� + �����������
            m_sd[i] += m_pe[i];
            m_sr[i] = 0.;
        }

        //////////////////////////////////////////////////////////////////////////
        // evaporation
		// ����ݵ���ˮ��� > 0
        if (m_sd[i] > 0) {
            /// TODO: Is this logically right? PET is just potential, which include
            ///       not only ET from surface water, but also from plant and soil.
            ///       Please Check the corresponding theory. By LJ.
            // evaporation from depression storage
			// �����Ǳ����ɢ��� - ֲ�������������� < �ݵ���ˮ��ȣ��ݵ����� = ��Ǳ����ɢ��� - ֲ��������������
			// �����Ǳ����ɢ��� - ֲ�������������� > �ݵ���ˮ��ȣ��ݵ����� = �ݵ���ˮ���(ȫ������)
            if (m_pet[i] - m_ei[i] < m_sd[i]) {
                m_ed[i] = m_pet[i] - m_ei[i];
            } else {
                m_ed[i] = m_sd[i];
            }
			// �ݵ���ˮ��� - �ݵ��������
            m_sd[i] -= m_ed[i];
        } else {
            m_ed[i] = 0.;
            m_sd[i] = 0.;
        }
        if (m_impoundTriger != nullptr && FloatEqual(m_impoundTriger[i], 0.f)) {
            if (m_potVol != nullptr) {
                m_potVol[i] += m_sr[i];
                m_potVol[i] += m_sd[i];
                m_sr[i] = 0.;
                m_sd[i] = 0.;
            }
        }
    }
    return true;
}

void DepressionFSDaily::SetValue(const char* key, const FLTPT value) {
    string sk(key);
    if (StringMatch(sk, VAR_DEPREIN[0])) m_depCo = value;
    else {
        throw ModelException(M_DEP_LINSLEY[0], "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void DepressionFSDaily::Set1DData(const char* key, const int n, FLTPT* data) {
    CheckInputSize(M_DEP_LINSLEY[0], key, n, m_nCells);
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
    } else {
        throw ModelException(M_DEP_LINSLEY[0], "Set1DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void DepressionFSDaily::Set1DData(const char* key, const int n, int* data) {
    CheckInputSize(M_DEP_LINSLEY[0], key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_IMPOUND_TRIG[0])) {
        m_impoundTriger = data;
    } else {
        throw ModelException(M_DEP_LINSLEY[0], "Set1DData",
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
        throw ModelException(M_DEP_LINSLEY[0], "Get1DData",
                             "Output " + sk + " does not exist.");
    }
}
