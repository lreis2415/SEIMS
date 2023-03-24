#include "StormGreenAmpt.h"

#include "text.h"

StormGreenAmpt::StormGreenAmpt() :
    m_dt(-1), m_nCells(-1), m_tSnow(0.0f), m_t0(1.0f),
    m_maxSoilLyrs(-1), m_nSoilLyrs(nullptr),
    m_soilDepth(nullptr), m_soilPor(nullptr),
    m_soilClay(nullptr), m_soilSand(nullptr), m_ks(nullptr),
    m_initSoilWtrStoRatio(nullptr), m_soilFC(nullptr),
    m_meanTmp(nullptr), m_netPcp(nullptr), m_deprSto(nullptr),
    m_snowMelt(nullptr), m_snowAccu(nullptr), m_surfRf(nullptr),
    m_capillarySuction(nullptr), m_accumuDepth(nullptr),
    m_soilWtrSto(nullptr), m_infil(nullptr), m_infilCapacitySurplus(nullptr) {

}

StormGreenAmpt::~StormGreenAmpt() {
    if (m_capillarySuction != nullptr) Release1DArray(m_capillarySuction);
    if (m_accumuDepth != nullptr) Release1DArray(m_accumuDepth);
    if (m_soilWtrSto != nullptr) Release2DArray(m_soilWtrSto);
    if (m_infil != nullptr) Release1DArray(m_infil);
    if (m_infilCapacitySurplus != nullptr) Release1DArray(m_infilCapacitySurplus);
}

void StormGreenAmpt:: InitialOutputs() {
    // Only check necessary variables
    CHECK_POSITIVE(M_SUR_MR[0], m_nCells);
    CHECK_POINTER(M_SUR_SGA[0], m_nSoilLyrs);
    CHECK_POINTER(M_SUR_SGA[0], m_initSoilWtrStoRatio);
    CHECK_POINTER(M_SUR_SGA[0], m_soilFC);

    if (nullptr == m_infil) {
        output_icell_min = 0;
        output_icell_max = 10000000;
        printInfilMinT = 39000;
        printInfilMaxT = 39090;
        counter = 0;
        CheckInputData();
        Initialize1DArray(m_nCells, m_infil, 0.f);
        Initialize1DArray(m_nCells, m_infilCapacitySurplus, 0.f);
        Initialize2DArray(m_nCells, m_maxSoilLyrs, m_soilWtrSto, 0.f);

#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) {
                if (m_initSoilWtrStoRatio[i] < 0.f || m_initSoilWtrStoRatio[i] > 1.f ||
                    m_soilFC[i][j] < 0.f) {
                    continue;
                }
                m_soilWtrSto[i][j] = m_initSoilWtrStoRatio[i] * m_soilFC[i][j];
            }
        }
    }
}

void StormGreenAmpt::Get1DData(const char *key, int *n, float **data) {
    InitialOutputs();
    *n = m_nCells;
    string sk(key);
    if (StringMatch(sk, VAR_INFIL[0])) { //infiltration
        *data = m_infil;
    } else if (StringMatch(sk, VAR_INFILCAPSURPLUS[0])) {
        *data = m_infilCapacitySurplus;
    } else if (StringMatch(sk, VAR_ACC_INFIL[0])) {
        *data = m_accumuDepth;
    } else {
        throw ModelException(M_SUR_SGA[0], "Get1DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void StormGreenAmpt::Get2DData(const char* key, int* nrows, int* ncols, float*** data) {
    InitialOutputs();
    string sk(key);
    *nrows = m_nCells;
    *ncols = m_maxSoilLyrs;

    if (StringMatch(sk, VAR_SOL_ST[0])) {
        *data = m_soilWtrSto;
    }
    else {
        throw ModelException(M_SUR_SGA[0], "Get2DData",
                             "Output " + sk + " does not exist.");
    }
}

bool StormGreenAmpt::CheckInputData() {
    CHECK_POSITIVE(M_SUR_SGA[0], m_dt);
    CHECK_POSITIVE(M_SUR_SGA[0], m_nCells);
    CHECK_POSITIVE(M_SUR_SGA[0], m_maxSoilLyrs);
    CHECK_POINTER(M_SUR_SGA[0], m_nSoilLyrs);
    CHECK_POINTER(M_SUR_SGA[0], m_soilDepth);
    CHECK_POINTER(M_SUR_SGA[0], m_soilPor);
    CHECK_POINTER(M_SUR_SGA[0], m_soilClay);
    CHECK_POINTER(M_SUR_SGA[0], m_soilSand);
    CHECK_POINTER(M_SUR_SGA[0], m_ks);
    CHECK_POINTER(M_SUR_SGA[0], m_initSoilWtrStoRatio);
    CHECK_POINTER(M_SUR_SGA[0], m_soilFC);
    CHECK_POINTER(M_SUR_SGA[0], m_meanTmp);
    CHECK_POINTER(M_SUR_SGA[0], m_netPcp);
    CHECK_POINTER(M_SUR_SGA[0], m_deprSto);
    CHECK_POINTER(M_SUR_SGA[0], m_surfRf);

    return true;
}

int StormGreenAmpt::Execute(void) {
    InitialOutputs();
# ifdef _DEBUG
    string baseOutputPath = "G:\\program\\\seims\\\data\\log\\";
    std::ostringstream infiltOss;
    infiltOss << baseOutputPath << "infilt_" << counter << ".txt";
    string infiltFile = infiltOss.str();

    //if (counter == 0 && _access(infiltFile.c_str(), 0) == 0) {//�ļ�����ɾ��
    //	if (remove(infiltFile.c_str()) == 0) {
    //		cout << "succeed to delete infiltration  file.  " << endl;
    //	}
    //	else {
    //		cout << "failed to delete infiltration file.  " << endl;
    //	}
    //}
    counter++;
    if ((counter >= printInfilMinT && counter <= printInfilMaxT) && !infiltFileFptr.is_open()) {
        if (DeleteExistedFile(infiltFile) == 0) {
            cout << "Deleted " << infiltFile << endl;
        }
        infiltFileFptr.open(infiltFile.c_str(), std::ios::out | std::ios::app);
        infiltFileFptr << "timestamp " << counter << endl;
    }
#endif

    // allocate intermediate variables
    if (nullptr == m_capillarySuction) {
        Initialize1DArray(m_nCells, m_capillarySuction, 0.f);
        Initialize1DArray(m_nCells, m_accumuDepth, 0.f);

#pragma omp parallel for
        for (int i = 0; i < m_nCells; ++i) {
            for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) {
                m_capillarySuction[i] = CalculateCapillarySuction(m_soilPor[i][j],
                                                                  m_soilClay[i][j] * 100,
                                                                  m_soilSand[i][j] * 100);
            }
        }
    }

//#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) {
            float snowMelt = 0.f;
            float snowAcc = 0.f;
            if (m_snowMelt != nullptr) {
                snowMelt = m_snowMelt[i];
            }
            if (m_snowAccu != nullptr) {
                snowAcc = m_snowAccu[i];
            }

            float hWater = m_netPcp[i];
            //account for the effects of snow melt and soil temperature
            if (m_meanTmp[i] <= m_tSnow) {
                // snow, without snow melt
                hWater = 0.0f;
            }
            else if (m_meanTmp[i] > m_tSnow && m_meanTmp[i] <= m_t0 && snowAcc > hWater) {
                // rain on snow, no snow melt
                hWater = 0.0f;
            }
            else {
                hWater = m_netPcp[i] + m_deprSto[i] + snowMelt;
            }

            hWater += m_surfRf[i];

            // effective matric potential (m)
            float matricPotential = (m_soilPor[i][j] - m_soilWtrSto[i][j]) * m_capillarySuction[i] / 1000.f;
            // algorithm of Li, 1996, uesd in C2SC2D
            float ks = m_ks[i][j] / 1000.f / 3600.f; // mm/h -> m/s
            float dt = m_dt;
            float infilDepth = m_accumuDepth[i] / 1000.f; // mm ->m

            float p1 = ks * dt - 2.f * infilDepth;
            float p2 = ks * (infilDepth + matricPotential);
            // infiltration rate (m/s)
            float infilRate = (p1 + CalSqrt(CalPow(p1, 2.f) + 8.f * p2 * dt)) / (2.f * dt);

            float infilCap = (m_soilPor[i][j] - m_soilWtrSto[i][j]) * m_soilDepth[i][j];

            if (hWater > 0) {
                // for frozen soil
                //if (m_soilTemp[i] <= m_tSoilFrozen && m_soilMoisture[i] >= m_sFrozen*m_porosity[i])
                //{
                //	m_pe[i] = pNet;
                //	m_infil[i] = 0.0f;
                //}
                //for saturation overland flow
                if (m_soilWtrSto[i][j] > m_soilPor[i][j]) {
                    m_infil[i] = 0.0f;
                    m_infilCapacitySurplus[i] = 0.f;
                }
                else {
                    m_infil[i] = Min(infilRate * dt * 1000.f, infilCap); // mm

                    //cout << m_infil[i] << endl;
                    //check if the infiltration potential exceeds the available water
                    if (m_infil[i] > hWater) {
                        m_infilCapacitySurplus[i] = m_infil[i] - hWater;
                        //limit infiltration rate to available water supply
                        m_infil[i] = hWater;
                    }
                    else {
                        m_infilCapacitySurplus[i] = 0.f;
                    }

                    //Compute the cumulative depth of infiltration
                    m_accumuDepth[i] += m_infil[i];
                    // sr is temporarily used to stored the water depth including the depression storage
                    m_surfRf[i] = hWater - m_infil[i];

                    if (m_soilDepth != nullptr) {
                        m_soilWtrSto[i][j] += m_infil[i] / m_soilDepth[i][j];
                    }
                }
                // xdw modify
                m_surfRf[i] = hWater - m_infil[i];  // sr is temporarily used to stored the water depth including the depression storage
            } else {
                m_surfRf[i] = 0.0f;
                m_infil[i] = 0.0f;
                m_infilCapacitySurplus[i] = Min(infilRate * dt * 1000.f, infilCap);
            }
        }
# ifdef IS_DEBUG
        if ((counter >= printInfilMinT && counter <= printInfilMaxT)) {
            if (i >= output_icell_min && i <= output_icell_max && infiltFileFptr.is_open()) {
                infiltFileFptr << "icell: " << i << " infil: " << m_infil[i] << " acc: " << m_accumuDepth[i] << " pNet: " << m_netPcp[i] << " m_sr: " << m_surfRf[i] << endl;
            }
        }
# endif
    }
    infiltFileFptr.close();
    return 0;
}

//this function calculated the wetting front matric potential (mm)
float StormGreenAmpt::CalculateCapillarySuction(float por, float clay, float sand) {
    float cs = 10.0f * CalExp(6.5309f - 7.32561f * por + 0.001583f * CalPow(clay, 2) + 3.809479f * CalPow(por, 2)
                              + 0.000344f * sand * clay - 0.049837f * por * sand
                              + 0.001608f * CalPow(por, 2) * CalPow(sand, 2)
                              + 0.001602f * CalPow(por, 2) * CalPow(clay, 2) - 0.0000136f * CalPow(sand, 2) * clay -
                              0.003479f * CalPow(clay, 2) * por - 0.000799f * CalPow(sand, 2) * por);

    return cs;
}


void StormGreenAmpt::SetValue(const char *key, const float value) {
    string sk(key);
    if (StringMatch(sk, Tag_HillSlopeTimeStep[0])) {
        m_dt = value;
    } else {
        throw ModelException(M_SUR_SGA[0], "SetValue",
                             "Parameter " + sk + " does not exist.");
    }

}

void StormGreenAmpt::Set1DData(const char *key, const int n, float *data) {
    CheckInputSize(M_SUR_SGA[0], key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_TMEAN[0])) {
        m_meanTmp = data;
    } else if (StringMatch(sk, VAR_NEPR[0])) {
        m_netPcp = data;
    } else if (StringMatch(sk, VAR_DPST[0])) {
        m_deprSto = data;
    } else if (StringMatch(sk, VAR_SURU[0])) {
        m_surfRf = data;
    } else if (StringMatch(sk, VAR_MOIST_IN[0])) {
        m_initSoilWtrStoRatio = data;
    } else {
        throw ModelException(M_SUR_SGA[0], "Set1DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void StormGreenAmpt::Set2DData(const char* key, int nrows, int ncols, float** data) {
    string sk(key);
    CheckInputSize2D(M_SUR_SGA[0], key, nrows, ncols, m_nCells, m_maxSoilLyrs);
    if (StringMatch(sk, VAR_CONDUCT[0])) {
        m_ks = data;
    } else if (StringMatch(sk, VAR_CLAY[0])) {
        m_soilClay = data;
    } else if (StringMatch(sk, VAR_SAND[0])) {
        m_soilSand = data;
    } else if (StringMatch(sk, VAR_FIELDCAP[0])) {
        m_soilFC = data;
    } else if (StringMatch(sk, VAR_POROST[0])) {
        m_soilPor = data;
    } else if (StringMatch(sk, VAR_SOILDEPTH[0])) {
        m_soilDepth = data;
    } else {
        throw ModelException(M_SUR_SGA[0], "Set2DData",
                             "Parameter " + sk + " does not exist.");
    }
}
