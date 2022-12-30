#include "StormGreenAmpt.h"
#include "text.h"

using namespace std;

StormGreenAmpt::StormGreenAmpt(void) : m_infil(NULL), m_capillarySuction(NULL), m_accumuDepth(NULL),
                                       m_dt(-1), m_nCells(-1), m_pNet(NULL), m_porosity(NULL), m_soilMoisture(NULL),
                                       m_rootDepth(NULL), m_fieldCap(NULL),
                                       m_sd(NULL), m_sr(NULL), m_tMax(NULL), m_tMin(NULL),
                                       m_tSnow(0.0f), m_t0(1.0f), m_snowAccu(NULL), m_snowMelt(NULL),
                                       m_tSoilFrozen(-5.0f), m_sFrozen(0.5f), m_soilTemp(NULL),
                                       m_ks(NULL), m_clay(NULL), m_sand(NULL), m_initSoilMoisture(NULL) {

}

StormGreenAmpt::~StormGreenAmpt(void) {
    if (m_infil != NULL) Release1DArray(m_infil);
    if (m_infilCapacitySurplus != NULL) Release1DArray(m_infilCapacitySurplus);
    if (m_capillarySuction != NULL) Release1DArray(m_capillarySuction);
    if (m_accumuDepth != NULL) Release1DArray(m_accumuDepth);
    if (m_soilMoisture != NULL) Release1DArray(m_soilMoisture);
}

void StormGreenAmpt:: InitialOutputs() {
    // allocate the output variable
    if (m_infil == NULL) {
        CheckInputData();

        m_infil = new float[m_nCells];
        m_infilCapacitySurplus = new float[m_nCells];
        m_soilMoisture = new float[m_nCells];
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
			//printf("i=%d, m_fieldCap[i]=%lf\n", i, m_fieldCap[0][i]);
			//if (abs(m_fieldCap[0][i]) < 0.00001)
			//{
			//	m_fieldCap[0][i] = 0.0f;
			//}
            m_initSoilMoisture[i] = m_initSoilMoisture[i] * m_fieldCap[i][0];
            m_soilMoisture[i] = m_initSoilMoisture[i];
        }
    }
}

void StormGreenAmpt::Get1DData(const char *key, int *n, float **data) {
    InitialOutputs();

    *n = m_nCells;
    string sk(key);
    if (StringMatch(sk, VAR_INFIL))   //infiltration
    {
        *data = m_infil;
    } else if (StringMatch(sk, VAR_INFILCAPSURPLUS)) {
        *data = m_infilCapacitySurplus;
    } else if (StringMatch(sk, VAR_ACC_INFIL)) {
        *data = m_accumuDepth;
    } else if (StringMatch(sk, VAR_SOL_ST)) // soil moisture
    {
        *data = m_soilMoisture;
    } else {
        throw ModelException(MID_SUR_SGA, "Get1DData",
                             "Parameter " + sk + " does not exist. Please contact the module developer.");

    }
}

void StormGreenAmpt::clearInputs() {
    //m_date = -1;
}

bool StormGreenAmpt::CheckInputData() {
    if (m_date < 0) {
        throw ModelException(MID_SUR_SGA, "CheckInputData", "You have not set the time.");
        return false;
    }
    if (m_dt < 0) {
        throw ModelException(MID_SUR_SGA, "CheckInputData", "You have not set the time step.");
        return false;
    }
    if (m_nCells <= 0) {
        throw ModelException(MID_SUR_SGA, "CheckInputData", "The cell number can not be less than zero.");
        return false;
    }

    if (m_pNet == NULL) {
        throw ModelException(MID_SUR_SGA, "CheckInputData", "The net precipitation can not be NULL.");
        return false;
    }

    if (m_porosity == NULL) {
        throw ModelException(MID_SUR_SGA, "CheckInputData", "The soil porosity can not be NULL.");
        return false;
    }
    //if (m_soilMoisture == NULL)
    //{
    //	throw ModelException(MID_SUR_SGA,"CheckInputData","The soil moisture can not be NULL.");
    //	return false;
    //}
    if (m_sd == NULL) {
        throw ModelException(MID_SUR_SGA, "CheckInputData", "The depression storage can not be NULL.");
        return false;
    }

    //if (m_t == NULL)
    //{
    //	throw ModelException(MID_SUR_SGA,"CheckInputData","The temperature can not be NULL.");
    //	return false;
    //}
    //if (m_snowAccu == NULL)
    //{
    //	throw ModelException(MID_SUR_SGA,"CheckInputData","The snow accumulation data can not be NULL.");
    //	return false;
    //}
    //if (m_snowMelt == NULL)
    //{
    //	throw ModelException(MID_SUR_SGA,"CheckInputData","The snow melt can not be NULL.");
    //	return false;
    //}
    //if (m_soilTemp == NULL)
    //{
    //	throw ModelException(MID_SUR_SGA,"CheckInputData","The soil temperature can not be NULL.");
    //	return false;
    //}

    if (m_ks == NULL) {
        throw ModelException(MID_SUR_SGA, "CheckInputData", "The hydraulic conductivity can not be NULL.");
        return false;
    }

    if (m_clay == NULL) {
        throw ModelException(MID_SUR_SGA, "CheckInputData", "The clay percentage can not be NULL.");
        return false;
    }
    if (m_sand == NULL) {
        throw ModelException(MID_SUR_SGA, "CheckInputData", "The sand percentage can not be NULL.");
        return false;
    }
    if (m_initSoilMoisture == NULL) {
        throw ModelException(MID_SUR_SGA, "CheckInputData", "The initial soil temperature can not be NULL.");
        return false;
    }
    if (m_fieldCap == NULL) {
        throw ModelException(MID_SUR_SGA, "CheckInputData", "The field capacity can not be NULL.");
        return false;
    }

    return true;
}

bool StormGreenAmpt::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_SUR_SGA, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; }
        else {
            throw ModelException(MID_SUR_SGA, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
        }
    }

    return true;
}

int StormGreenAmpt::Execute(void) {
    InitialOutputs();


    // allocate intermediate variables
    if (m_capillarySuction == NULL) {
		// 土壤毛细吸头
        m_capillarySuction = new float[m_nCells];
		// 累积渗透深度
        m_accumuDepth = new float[m_nCells];

#pragma omp parallel for
        for (int i = 0; i < m_nCells; ++i) {
            m_accumuDepth[i] = 0.0f;
			// 根据第1层土壤的孔隙度、粘土占比、沙土占比计算土壤毛细吸头
            m_capillarySuction[i] = CalculateCapillarySuction(m_porosity[i][0], m_clay[i][0] * 100, m_sand[i][0] * 100);
        }
    }

//#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        float t = 10.f;
        /// TODO change to m_tMean
		// 最高温度、最低温度, 计算平均温度
        if (m_tMax != NULL && m_tMin != NULL) {
            t = (m_tMax[i] + m_tMin[i]) / 2.f;
        }
		// 融雪径流，optional
        float snowMelt = 0.f;
        float snowAcc = 0.f;
        if (m_snowMelt != NULL) {
            snowMelt = m_snowMelt[i];
        }
		// 融雪积累，optional
        if (m_snowAccu != NULL) {
            snowAcc = m_snowAccu[i];
        }
		// 水深=降雨深度
        float hWater = m_pNet[i];
        //account for the effects of snow melt and soil temperature
        // snow, without snow melt
        if (t <= m_tSnow) {
            hWater = 0.0f;
        }
        // rain on snow, no snow melt
        else if (t > m_tSnow && t <= m_t0 && snowAcc > hWater) {
            hWater = 0.0f;
        } else {
            hWater = m_pNet[i] + m_sd[i] + snowMelt;
        }
		// 水深=降雨深度+地表径流水深
        hWater += m_sr[i];

        // effective matric potential (m)
        float matricPotential = (m_porosity[i][0] - m_initSoilMoisture[i]) * m_capillarySuction[i] / 1000.f;
        // algorithm of Li, 1996, uesd in C2SC2D
        float ks = m_ks[i][0] / 1000.f / 3600.f; // mm/h -> m/s
        float dt = m_dt;
        float infilDepth = m_accumuDepth[i] / 1000.f; // mm ->m

        float p1 = (float) (ks * dt - 2.0 * infilDepth);
        float p2 = ks * (infilDepth + matricPotential);
        // infiltration rate (m/s) 根据累计入渗深度计算出来的当前时间步长的下渗能力（最大下渗速率）
        float infilRate = (float) ((p1 + sqrt(pow(p1, 2.0f) + 8.0f * p2 * dt)) / (2.0f * dt));

		//infilCap是当前时间步长土壤还能容纳的最大入渗最大深度,这里m_rootDepth取第4层，因为第4层的深度比较合适，第1层的深度太小了
        float infilCap = (m_porosity[i][0] - m_soilMoisture[i]) * m_rootDepth[i][4];
		//cout << "m_ks: " << m_ks[i][0] << " ks: " << ks << " infilDepth: " << infilDepth << " infilRate: " << infilRate << " infilCap: " << infilCap << endl;

        if (hWater > 0) {
            // for frozen soil
            //if (m_soilTemp[i] <= m_tSoilFrozen && m_soilMoisture[i] >= m_sFrozen*m_porosity[i])
            //{
            //	m_pe[i] = pNet;
            //	m_infil[i] = 0.0f;
            //}
            //for saturation overland flow
			// 当前的土壤湿度 > 土壤孔隙度，代表土壤水饱和了，则不下渗了；土壤湿度会随着下渗量的增加而增加
            if (m_soilMoisture[i] > m_porosity[i][0]) {
                m_infil[i] = 0.0f;
                m_infilCapacitySurplus[i] = 0.f;
            } else {
				// 取较小值的意思是，如果入渗能力*时间步长 < 最大入渗深度，就代表在当前时间步长下，土壤还能容纳这么多水的下渗（入渗能力*时间步长）
				// 如果入渗能力*时间步长 > 最大入渗深度，就代表在当前时间步长下，土壤已经容纳不了这么多水的下渗（入渗能力*时间步长）
                m_infil[i] = min(infilRate * dt * 1000.f, infilCap); // mm

                //check if the infiltration potential exceeds the available water
                if (m_infil[i] > hWater) {
                    m_infilCapacitySurplus[i] = m_infil[i] - hWater;
                    //limit infiltration rate to available water supply
                    m_infil[i] = hWater;
                } else {
                    m_infilCapacitySurplus[i] = 0.f;
                }

                //Compute the cumulative depth of infiltration
                m_accumuDepth[i] += m_infil[i];

                if (m_rootDepth != NULL) {
                    m_soilMoisture[i] += m_infil[i] / m_rootDepth[i][0];
                }
            }
			// xdw modify, 把对地表水深的更新挪到这里，因为如果土壤湿度>土壤孔隙度，也需要更新地表水深
			m_sr[i] = hWater -
				m_infil[i];  // sr is temporarily used to stored the water depth including the depression storage
        } else {
            m_sr[i] = 0.0f;
            m_infil[i] = 0.0f;
            m_infilCapacitySurplus[i] = min(infilRate * dt * 1000.f, infilCap);
        }
    }

    return 0;
}

//this function calculated the wetting front matric potential (mm)
float StormGreenAmpt::CalculateCapillarySuction(float por, float clay, float sand) {
    float cs = 10.0f * exp(6.5309f - 7.32561f * por + 0.001583f * pow(clay, 2) + 3.809479f * pow(por, 2)
                               + 0.000344f * sand * clay - 0.049837f * por * sand
                               + 0.001608f * pow(por, 2) * pow(sand, 2)
                               + 0.001602f * pow(por, 2) * pow(clay, 2) - 0.0000136f * pow(sand, 2) * clay -
        0.003479f * pow(clay, 2) * por - 0.000799f * pow(sand, 2) * por);

    return cs;
}

// set value
void StormGreenAmpt::SetValue(const char *key, float value) {
    string sk(key);

    //if (StringMatch(sk,"T_snow"))
    //{
    //	m_tSnow = value;
    //}
    //else if (StringMatch(sk,"t_soil"))
    //{
    //	m_tSoilFrozen = value;
    //}
    //else if (StringMatch(sk,"T0"))
    //{
    //	m_t0 = value;
    //}
    //else if (StringMatch(sk,"s_frozen"))
    //{
    //	m_sFrozen = value;
    //}
    //else
    if (StringMatch(sk, Tag_HillSlopeTimeStep)) {
        m_dt = value;
    } else {
        throw ModelException(MID_SUR_SGA, "SetValue", "Parameter " + sk + " does not exist in SetValue method.");
    }

}

void StormGreenAmpt::Set2DData(const char* key, const int n, const int col, float** data) {
	string sk(key);
	if (StringMatch(sk, VAR_CONDUCT)) {
		m_ks = data;
	}else if (StringMatch(sk, VAR_POROST)) {
		m_porosity = data;
	}else if (StringMatch(sk, VAR_CLAY)) {
		m_clay = data;
	}else if (StringMatch(sk, VAR_SAND)) {
		m_sand = data;
	}else if (StringMatch(sk, VAR_FIELDCAP)) {
		m_fieldCap = data;
	}else if (StringMatch(sk, VAR_SOILDEPTH)) {
		m_rootDepth = data;
	}
}

void StormGreenAmpt::Set1DData(const char *key, int n, float *data) {
    if (data == NULL) {
        return;
    }

    //check the input data
    if (!CheckInputSize(key, n)) return;

    //set the value
    string sk(key);
    if (StringMatch(sk, VAR_NEPR)) {
        m_pNet = data;
    }
        //else if (StringMatch(sk,"D_TEMP"))
        //{
        //	m_t = data;
        //}
        //else if (StringMatch(sk,"D_SOMO"))
        //{
        //	m_soilMoisture = data;
        //}
    else if (StringMatch(sk, VAR_DPST)) {
        m_sd = data;
    } else if (StringMatch(sk, VAR_SURU)) {
        m_sr = data;
    }
	//else if (StringMatch(sk, VAR_SOILDEPTH)) {
 //       m_rootDepth = data;
 //   }
	//else if (StringMatch(sk, VAR_FIELDCAP)) {
 //       m_fieldCap = data;
 //   }
        //else if (StringMatch(sk,"D_SOTE"))
        //{
        //	m_soilTemp = data;
        //}
        //else if (StringMatch(sk,"D_SNAC"))
        //{
        //	m_snowAccu = data;
        //}
        //else if (StringMatch(sk,"D_SNME"))
        //{
        //	m_snowMelt = data;
        //}
    else if (StringMatch(sk, VAR_MOIST_IN)) {
        m_initSoilMoisture = data;
    }
	//else if (StringMatch(sk, VAR_POROST)) {
 //       m_porosity = data;
 //   }
	//else if (StringMatch(sk, VAR_CONDUCT)) {
 //       m_ks = data;
 //   }
	//else if (StringMatch(sk, VAR_CLAY)) {
 //       m_clay = data;
 //   }
	//else if (StringMatch(sk, VAR_SAND)) {
 //       m_sand = data;
 //   }
	else {
        throw ModelException(MID_SUR_SGA, "SetValue",
                             "Parameter " + sk + " does not exist. Please contact the module developer.");
    }
}
