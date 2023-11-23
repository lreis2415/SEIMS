#include "DepressionFS.h"
#include "text.h"

DepressionFS::DepressionFS() :
    m_nCells(-1), m_depCo(NODATA_VALUE), m_depCap(nullptr), m_pet(nullptr), m_ei(nullptr),
    m_ed(nullptr), m_sd(nullptr), m_sr(nullptr), m_checkInput(true) {
}

DepressionFS::~DepressionFS() {
    Release1DArray(m_sd);
    Release1DArray(m_sr);
    Release1DArray(m_storageCapSurplus);
    Release1DArray(m_ed);
}

bool DepressionFS::CheckInputData() {
    if (m_date == -1) {
        throw ModelException(M_DEP_FS[0], "CheckInputData", "You have not set the time.");
    }
    if (m_nCells <= 0) {
        throw ModelException(M_DEP_FS[0], "CheckInputData", "The cell number of the input can not be less than zero.");
    }
    if (m_depCo == NODATA_VALUE) {
        throw ModelException(M_DEP_FS[0], "CheckInputData",
                             "The parameter: initial depression storage coefficient has not been set.");
    }
    if (m_depCap == NULL) {
        throw ModelException(M_DEP_FS[0], "CheckInputData",
                             "The parameter: depression storage capacity has not been set.");
    }
#ifndef STORM_MODE
    if (m_pet == NULL) {
        throw ModelException(M_DEP_FS[0], "CheckInputData", "The parameter: PET has not been set.");
    }
    if (m_ei == NULL) {
        throw ModelException(M_DEP_FS[0], "CheckInputData",
                             "The parameter: evaporation from the interception storage has not been set.");
    }
    if (m_ed == NULL) {
        throw ModelException(M_DEP_FS[0], "CheckInputData",
                             "The parameter: DEET from the interception storage has not been set.");
    }
#endif /* not STORM_MODE */
    return true;
}

void DepressionFS::InitialOutputs() {
    if (m_nCells <= 0) {
        throw ModelException(M_DEP_FS[0], "InitialOutputs", "The cell number of the input can not be less than zero.");
    }
    if (m_sd == NULL) {
        m_sd = new float[m_nCells];
        m_sr = new float[m_nCells];
        m_storageCapSurplus = new float[m_nCells];
        m_ed = new float[m_nCells];
#pragma omp parallel for
        for (int i = 0; i < m_nCells; ++i) {
            m_sd[i] = m_depCo * m_depCap[i];// �ݵ���ˮ��� = �ݵس�ʼ��ˮϵ�� * �ݵ����
            m_sr[i] = 0.0f;
        }
    }
}

int DepressionFS::Execute() {
    InitialOutputs();
    if (m_checkInput) {
        CheckInputData();
        m_checkInput = false;
    }

//#pragma omp parallel for
    for (int i = 0; i < m_nCells; ++i) {

        // sr is temporarily used to stored the water depth including the depression storage
		// m_sr�ǵر�ˮ��ݵ�����֮ǰ��m_sr�������ݵ�ˮ�
        float hWater = m_sr[i];
		// ��� �ر�ˮ�� <= �ݵ����
		// �ݵ�����֮ǰ
        if (hWater <= m_depCap[i]) {
			// �ݵ�ˮ�� = �ر�ˮ��
            m_sd[i] = hWater;
			// �ر�ˮ�� = 0
            m_sr[i] = 0.f;
        } else {
			// �ݵ�����֮��
			// �ݵ�ˮ�� = �ݵ����
            m_sd[i] = m_depCap[i];
			// �ر�ˮ�� = �ر�ˮ�� - �ݵ���ȣ���ʱ�ĵر�ˮ��ָ�ݵ��ϱ������ϵ�ˮ��
            m_sr[i] = hWater - m_depCap[i];
        }
        // ʣ��洢���� = �ݵ���� - �ݵ�ˮ��
        m_storageCapSurplus[i] = m_depCap[i] - m_sd[i];
        if (m_sd[i] > 0) {
            //This section is taken from DEP_LINSLEY
            if (m_pet[i] - m_ei[i] < m_sd[i]) {
                m_ed[i] = m_pet[i] - m_ei[i];
            } else {
                m_ed[i] = m_sd[i];
            }
        } else {
            m_ed[i] = 0.f;
        }
    }
    return 0;
}

bool DepressionFS::CheckInputSize(const char* key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}

void DepressionFS::SetValue(const char* key, float data) {
    string sk(key);
    if (StringMatch(sk, VAR_DEPREIN[0])) {
        m_depCo = data;
    } else {
        throw ModelException(M_DEP_FS[0], "SetValue", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

void DepressionFS::Set1DData(const char* key, int n, float* data) {
    //check the input data
    string sk(key);
    if (StringMatch(sk, VAR_DEPRESSION[0])) {
        m_depCap = data;
		for (int i = 0; i < m_nCells; i++)
		{
			if (m_depCap[i] < 0.00001)
			{
				m_depCap[i] = 0.f;
			}
		}
    }
	else if (StringMatch(sk, VAR_PET[0])) {
	       m_pet = data;
    } else if (StringMatch(sk, VAR_INLO[0])) {
        m_ei = data;
	   }
	else {
        throw ModelException(M_DEP_FS[0], "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

void DepressionFS::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_DPST[0])) {
        *data = m_sd;
    } else if (StringMatch(sk, VAR_SURU[0])) {
        *data = m_sr;
    } else if (StringMatch(sk, VAR_STCAPSURPLUS[0])) {
        *data = m_storageCapSurplus;
    } else if (StringMatch(sk, VAR_DEET[0])) {
        *data = m_ed;
    } else {
        throw ModelException(M_DEP_FS[0], "Get1DData", "Output " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}
