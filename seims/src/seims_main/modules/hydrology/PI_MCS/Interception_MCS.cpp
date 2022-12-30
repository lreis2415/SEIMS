
#include "text.h"
#include "Interception_MCS.h"

#include "utils_time.h"

clsPI_MCS::clsPI_MCS() :
    m_embnkFr(0.15f), m_pcp2CanalFr(0.5f), m_landUse(nullptr),
    m_intcpStoCapExp(-1.f), m_initIntcpSto(0.f), m_maxIntcpStoCap(nullptr),
    m_minIntcpStoCap(nullptr),
    m_pcp(nullptr), m_canSto(nullptr),
    m_intcpLoss(nullptr), m_netPcp(nullptr), m_nCells(-1){
#ifndef STORM_MODE
    m_IntcpET = nullptr;
	m_pet = nullptr;
#else
    m_hilldt = -1.f;
    m_slope = nullptr;
#endif
}

clsPI_MCS::~clsPI_MCS() {
    if (m_intcpLoss != nullptr) Release1DArray(m_intcpLoss);
    if (m_canSto != nullptr) Release1DArray(m_canSto);
    if (m_netPcp != nullptr) Release1DArray(m_netPcp);
#ifndef STORM_MODE
    if (m_IntcpET != nullptr) Release1DArray(m_IntcpET);
#endif
}

void clsPI_MCS::Set1DData(const char* key, int nrows, float* data) {
    CheckInputSize(MID_PI_MCS, key, nrows, m_nCells);
    string s(key);
	if (StringMatch(s, VAR_PCP)){
		m_pcp = data;
	}
    else if (StringMatch(s, VAR_PET)) {
#ifndef STORM_MODE
        m_pet = data;
#endif
    } else if (StringMatch(s, VAR_INTERC_MAX)) m_maxIntcpStoCap = data;
    else if (StringMatch(s, VAR_INTERC_MIN)) m_minIntcpStoCap = data;
    else if (StringMatch(s, VAR_LANDUSE)) m_landUse = data;
	else if (StringMatch(s, VAR_SLOPE)) m_slope = data;
    else {
        throw ModelException(MID_PI_MCS, "Set1DData", "Parameter " + s + " does not exist.");
    }
}

void clsPI_MCS::SetValue(const char* key, const float value) {
    string s(key);
    if (StringMatch(s, VAR_PI_B)) m_intcpStoCapExp = value;
    else if (StringMatch(s, VAR_INIT_IS)) m_initIntcpSto = value;
    else if (StringMatch(s, VAR_PCP2CANFR_PR)) m_pcp2CanalFr = value;
    else if (StringMatch(s, VAR_EMBNKFR_PR)) m_embnkFr = value;
#ifdef STORM_MODE
    else if (StringMatch(s, Tag_HillSlopeTimeStep)) m_hilldt = value;
#endif // STORM_MODE
    else {
        throw ModelException(MID_PI_MCS, "SetValue", "Parameter " + s + " does not exist.");
    }
}

void clsPI_MCS::Get1DData(const char* key, int* nRows, float** data) {
    InitialOutputs();
    string s = key;
    if (StringMatch(s, VAR_INLO)) {
        *data = m_intcpLoss;
    } else if (StringMatch(s, VAR_INET)) {
#ifndef STORM_MODE
        *data = m_IntcpET;
#endif
    } else if (StringMatch(s, VAR_CANSTOR)) {
        *data = m_canSto;
    } else if (StringMatch(s, VAR_NEPR)) {
        *data = m_netPcp;
    } else {
        throw ModelException(MID_PI_MCS, "Get1DData", "Result " + s + " does not exist.");
    }
    *nRows = m_nCells;
}

void clsPI_MCS::InitialOutputs() {
    if (m_canSto == nullptr) {
        Initialize1DArray(m_nCells, m_canSto, m_initIntcpSto);
    }
#ifndef STORM_MODE
    if (m_IntcpET == nullptr) {
        Initialize1DArray(m_nCells, m_IntcpET, 0.f);
    }
#endif
    if (m_netPcp == nullptr) {
        Initialize1DArray(m_nCells, m_netPcp, 0.f);
    }
    if (m_intcpLoss == nullptr) {
        Initialize1DArray(m_nCells, m_intcpLoss, 0.f);
    }
}

int clsPI_MCS::Execute() {
    //check input data
    CheckInputData();
    /// initialize outputs
    InitialOutputs();

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
		// 如果当前时间步长的降雨量>0
        if (m_pcp[i] > 0.f) {
#ifdef STORM_MODE
            /// correction for slope gradient, water spreads out over larger area
            /// 1. / 3600. = 0.0002777777777777778
            m_pcp[i] = m_pcp[i] * m_hilldt * 0.0002777777777777778f * cos(atan(m_slope[i]));
#endif // STORM_MODE
            //interception storage capacity, 1. / 365. = 0.0027397260273972603
            float degree = 2.f * PI * (m_dayOfYear - 87.f) * 0.0027397260273972603f;
            /// For water, min and max are both 0, then no need for specific handling.
            float min = m_minIntcpStoCap[i];
            float max = m_maxIntcpStoCap[i];
            float capacity = min + (max - min) * pow(0.5f + 0.5f * sin(degree), m_intcpStoCapExp);

			// capacity是最大截留能力,m_canSto[i]是截止当前累计的截留量(林冠截留能力),availableSpace是剩余的截留容量
            //interception, currently, m_st[i] is storage of (t-1) time step
            float availableSpace = capacity - m_canSto[i];
            if (availableSpace < 0) {
                availableSpace = 0.f;
            }
			// 如果剩余的截留容量 < 当前时间步长的降雨量
            if (availableSpace < m_pcp[i]) {
				// 令 截留损失 = 剩余的截留容量，即把剩余的截留容量耗光
                m_intcpLoss[i] = availableSpace;
				// 如果土地利用类型是稻田，就默认把15%的降雨分配给路堤区域
                // if the cell is paddy, by default 15% part of pcp will be allocated to embankment area
                if (CVT_INT(m_landUse[i]) == LANDUSE_ID_PADDY) {
					// m_embnkFr 落入路堤的降雨百分比(default 15%)
					// m_pcp2CanalFr 落入路堤的降雨中，进入沟渠的百分比(default 50%)
                    // water added into ditches from low embankment, should be added to somewhere else.
                    float pcp2canal = m_pcp[i] * m_pcp2CanalFr * m_embnkFr;
					// 净雨量 = 降雨 - 冠层截留损失 - 落入路堤且进入沟渠的降雨
                    m_netPcp[i] = m_pcp[i] - m_intcpLoss[i] - pcp2canal;
                } else {
                    //net precipitation
					//净雨量 = 降雨 - 冠层截留损失
                    m_netPcp[i] = m_pcp[i] - m_intcpLoss[i];
                }
            } else {
				// 如果剩余的截留容量 >= 当前时间步长的降雨量，令截留损失 = 当前时间步长的降雨量
                m_intcpLoss[i] = m_pcp[i];
				// 净雨量 = 0
                m_netPcp[i] = 0.f;
            }
			// 累计截留量 加上 截留损失
            m_canSto[i] += m_intcpLoss[i];
        } else {
			// 如果当前时间步长的降雨量=0,则截留损失=0，净雨量=0
            m_intcpLoss[i] = 0.f;
            m_netPcp[i] = 0.f;
        }
#ifndef STORM_MODE
        //evaporation
		//如果累计截留量 > 日潜在蒸散,则令蒸发损失=日潜在蒸散
        if (m_canSto[i] > m_pet[i]) {
            m_IntcpET[i] = m_pet[i];
        } else {
			//否则令蒸发损失=累计截留量
            m_IntcpET[i] = m_canSto[i];
        }
		m_IntcpET[i] = m_canSto[i];
        m_canSto[i] -= m_IntcpET[i];
		
#endif
    }
	//float total_netPcp = 0.0;
	//float ave_netPcp = 0.0;
	//// 计算当前时间步长上的平均净雨量
	//for (int i = 0; i < m_nCells; i++)
	//{
	//	total_netPcp += m_netPcp[i];
	//}
	//ave_netPcp = total_netPcp / m_nCells;
	//cout << "average net precipation: " << ave_netPcp << "mm" << endl;
    return 0;
}

bool clsPI_MCS::CheckInputData() {
    CHECK_POSITIVE(MID_PI_MCS, m_date);
    CHECK_POSITIVE(MID_PI_MCS, m_nCells);
    CHECK_POINTER(MID_PI_MCS, m_pcp);
#ifndef STORM_MODE
    CHECK_POINTER(MID_PI_MCS, m_pet);
#else
    //CHECK_POINTER(MID_PI_MCS, m_slope);
    //CHECK_POINTER(MID_PI_MCS, m_hilldt);
#endif
    CHECK_POINTER(MID_PI_MCS, m_maxIntcpStoCap);
    CHECK_POINTER(MID_PI_MCS, m_minIntcpStoCap);
    CHECK_DATA(MID_PI_MCS, m_intcpStoCapExp > 1.5f || m_intcpStoCapExp < 0.5f,
        "The interception storage capacity exponent "
        "can not be " + ValueToString(m_intcpStoCapExp) + ". It should between 0.5 and 1.5.");
    CHECK_DATA(MID_PI_MCS, m_initIntcpSto > 1.f || m_initIntcpSto < 0.f, "The Initial interception storage cannot "
        "be " + ValueToString(m_initIntcpSto) + ". It should between 0 and 1.");
    return true;
}
