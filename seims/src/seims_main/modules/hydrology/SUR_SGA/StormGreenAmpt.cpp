#include "StormGreenAmpt.h"

#include "text.h"

StormGreenAmpt::StormGreenAmpt() :
    m_dt(-1), m_nCells(-1), m_tSnow(0.0f), m_t0(1.0f), m_infilFactor(1.0f),
    m_activeDepthMax(0.0f),
    m_maxSoilLyrs(-1), m_nSoilLyrs(nullptr),
    m_soilDepth(nullptr), m_soilPor(nullptr),
    m_soilClay(nullptr), m_soilSand(nullptr), m_ks(nullptr),
    m_initSoilWtrStoRatio(nullptr), m_soilFC(nullptr),
    m_meanTmp(nullptr), m_netPcp(nullptr), m_sd(nullptr),
    m_snowMelt(nullptr), m_snowAccu(nullptr), m_surfRf(nullptr),
    m_capillarySuction(nullptr), m_accumuDepth(nullptr),
    m_soilWtrSto(nullptr), m_infil(nullptr), m_infilCapacitySurplus(nullptr),
    m_exsPcp(nullptr){

}

StormGreenAmpt::~StormGreenAmpt() {
    if (m_capillarySuction != nullptr) Release1DArray(m_capillarySuction);
    if (m_accumuDepth != nullptr) Release1DArray(m_accumuDepth);
    if (m_soilWtrSto != nullptr) Release2DArray(m_soilWtrSto);
    if (m_infil != nullptr) Release1DArray(m_infil);
    if (m_infilCapacitySurplus != nullptr) Release1DArray(m_infilCapacitySurplus);
    if (m_exsPcp != nullptr) Release1DArray(m_exsPcp);
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
        Initialize1DArray(m_nCells, m_exsPcp, 0.);

//#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) {
                if (m_initSoilWtrStoRatio[i] < 0.f || m_initSoilWtrStoRatio[i] > 1.f ||
                    m_soilFC[i][j] < 0.f) {
                    continue;
                }
                m_soilWtrSto[i][j] = m_initSoilWtrStoRatio[i] * m_soilFC[i][j];
            }
        }
        std::cout << "\n[DEBUG] StormGreenAmpt Initialization Check:" << std::endl;

        // debug
        /*int debug_count = 0;
        for (int i = 0; i < m_nCells && debug_count < 5; i++) {
            if (m_initSoilWtrStoRatio[i] <= 0.f) continue;
            debug_count++;
            std::cout << "  Cell ID: " << i << ", InitRatio: " << m_initSoilWtrStoRatio[i] << std::endl;
            for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) {
                std::cout << "    Layer " << j << ": FC=" << m_soilFC[i][j]
                    << " -> Calc Storage=" << m_soilWtrSto[i][j] << std::endl;
            }
        }*/
        /*//debug
        double total_storage = 0.0;
        double total_fc = 0.0;
        long valid_cells = 0;
        for (int i = 0; i < m_nCells; i++) {
            for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) {
                total_storage += m_soilWtrSto[i][j];
                total_fc += m_soilFC[i][j];
            }
            if (CVT_INT(m_nSoilLyrs[i]) > 0) valid_cells++;
        }*/

        /*if (total_fc > 0) {
            std::cout << "[DEBUG] Basin Statistics:" << std::endl;
            std::cout << "  Avg Init Storage: " << (total_storage / valid_cells) << " mm" << std::endl;
            std::cout << "  Basin Avg Saturation: " << (total_storage / total_fc) * 100.0 << "%" << std::endl;
        }
        else {
            std::cout << "[DEBUG] WARNING: Total Field Capacity is 0!" << std::endl;
        }*/
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
    } else if (StringMatch(sk, VAR_EXCP[0])) {
        *data = m_exsPcp; // excess precipitation
    }
    else {
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
    //CHECK_POINTER(M_SUR_SGA[0], m_deprSto);
    //CHECK_POINTER(M_SUR_SGA[0], m_surfRf);

    return true;
}

int StormGreenAmpt::Execute(void) {
    InitialOutputs();
# ifdef _DEBUG
    //string baseOutputPath = "G:\\program\\\seims\\\data\\log\\";
    std::ostringstream infiltOss;
    infiltOss << m_outpath << "infilt_" << counter << ".txt";
    string infiltFile = infiltOss.str();

    //if (counter == 0 && _access(infiltFile.c_str(), 0) == 0) {
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

//#pragma omp parallel for
        for (int i = 0; i < m_nCells; ++i) {
            if (m_nSoilLyrs[i] > 0) {
                m_capillarySuction[i] = CalculateCapillarySuction(m_soilPor[i][0],
                                                                  m_soilClay[i][0],
                                                                  m_soilSand[i][0]);
            }
        }
    }

//#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        // only calculate the first soillayer
        int j = 0;

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
            //the old depression water has been added.(m_deprSto)
            hWater = m_netPcp[i] + m_sd[i] + snowMelt;
        }

        //hWater += m_surfRf[i];

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

        const float activeDepth = CalculateWettingFrontDepth(i);
        const float infilCap = CalculateActiveInfilCap(i, activeDepth);

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
                float rawInfil = Min(infilRate * dt * 1000.f, infilCap); // mm
                rawInfil *= m_infilFactor;
                m_infil[i] = rawInfil;

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


                if (m_soilDepth != nullptr && m_infil[i] > 0.f) {
                    AddInfiltrationToSoil(i, m_infil[i], activeDepth);
                }
            }
            m_exsPcp[i] = hWater - m_infil[i];
            // xdw modify
            //m_surfRf[i] = hWater - m_infil[i];  // sr is temporarily used to stored the water depth including the depression storage
        } else {
            //m_surfRf[i] = 0.0f;
            m_infil[i] = 0.0f;
            m_infilCapacitySurplus[i] = Min(infilRate * dt * 1000.f, infilCap);

            m_exsPcp[i] = 0.f;
        }

        // debug
        //int target_cell = 2988;
        //if (i == target_cell && (hWater > 0 || m_infil[i] > 0)) {
        //    std::cout << "\n=== Cell " << i << " Step Debug ===" << std::endl;
        //    std::cout << "  [Inputs] NetPcp: " << m_netPcp[i] << " | DepStore: " << m_deprSto[i] << " | Prev_SurfRf: " << (m_surfRf ? m_surfRf[i] : 0) << std::endl;
        //    std::cout << "  [State]  hWater (Total Avail): " << hWater << " mm" << std::endl;
        //    std::cout << "  [Params] Por: " << m_soilPor[i][j] << " | InitMoist: " << m_soilWtrSto[i][j] << " | Suction: " << m_capillarySuction[i] << std::endl;

        //    std::cout << "  [Calc]   MatricPot: " << matricPotential << " m | Ks: " << ks << " m/s" << std::endl;
        //    std::cout << "  [Calc]   InfilRate: " << infilRate * 1000.0f * 3600.0f << " mm/h" << std::endl; // mm/h
        //    std::cout << "  [Result] Infil: " << m_infil[i] << " mm | Runoff: " << m_surfRf[i] << " mm" << std::endl;
        //    std::cout << "===============================" << std::endl;
        //}
        // --- DEBUG PRINT ---
        std::vector<int> targetCells = { 1304, 1193, 1192, 1191, 1190,
                                     1189, 1188, 1187, 1186, 1185,
                                     1294, 1404, 1403, 1513, 1623,
                                     1622, 1731, 1730, 1838, 1837, 1836,
                                     1944 };
        bool isDebugTarget = false;
        for (int id : targetCells) { if (i == id) { isDebugTarget = true; break; } }

        if (isDebugTarget) { // DEBUG_ID
            std::cout << "[TRACE_CSV],Step," << counter  
                << ",Module,GreenAmpt"
                << ",Cell," << i
                << ",NetRain," << m_netPcp[i]
                << ",Infiltration," << m_infil[i]
                << ",Excess_Pcp," << m_exsPcp[i]
                << ",Sat_Deficit," << (m_soilPor[i][0] - m_soilWtrSto[i][0])
                << std::endl;
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
    // debug
    /*if (cs < 0.00001f) {
        std::cout << "\n[ERROR] Capillary Suction Underflow Detected!" << std::endl;
        std::cout << "  -> Inputs: Por=" << por << ", Clay=" << clay << ", Sand=" << sand << std::endl;
        std::cout << "  -> Result CS: " << cs << " (Should not be 0)" << std::endl;
        std::cout << "  -> Hint: Check if Clay/Sand are Percent(0-100) or Fraction(0-1). Formula usually expects Percent." << std::endl;
    }*/
    return cs;
}

float StormGreenAmpt::CalculateWettingFrontDepth(const int cell) {
    const float minActiveDepth = 10.0f; // mm, keeps early storm infiltration numerically stable.
    if (m_nSoilLyrs[cell] <= 0 || m_soilDepth[cell][0] <= 0.f) {
        return minActiveDepth;
    }

    float activeDepth = m_activeDepthMax;
    if (activeDepth <= 0.f) {
        activeDepth = m_soilDepth[cell][CVT_INT(m_nSoilLyrs[cell]) - 1];
    }
    if (activeDepth < minActiveDepth) {
        activeDepth = minActiveDepth;
    }
    const float profileDepth = m_soilDepth[cell][CVT_INT(m_nSoilLyrs[cell]) - 1];
    if (profileDepth > 0.f && activeDepth > profileDepth) {
        activeDepth = profileDepth;
    }
    return activeDepth;
}

float StormGreenAmpt::CalculateActiveInfilCap(const int cell, const float activeDepth) {
    float activeStorage = 0.f;
    float upperDepth = 0.f;
    for (int k = 0; k < CVT_INT(m_nSoilLyrs[cell]); k++) {
        const float lowerDepth = m_soilDepth[cell][k];
        if (lowerDepth <= upperDepth) {
            continue;
        }
        if (activeDepth <= upperDepth) {
            break;
        }
        const float activeThickness = Min(activeDepth, lowerDepth) - upperDepth;
        if (activeThickness > 0.f) {
            const float initTheta = m_initSoilWtrStoRatio[cell] * m_soilFC[cell][k];
            const float deficit = m_soilPor[cell][k] - initTheta;
            if (deficit > 0.f) {
                activeStorage += deficit * activeThickness;
            }
        }
        upperDepth = lowerDepth;
    }
    const float remainingStorage = activeStorage - m_accumuDepth[cell];
    return Max(remainingStorage, 0.f);
}

void StormGreenAmpt::AddInfiltrationToSoil(const int cell, const float infiltration, const float activeDepth) {
    float remainInfil = infiltration;
    float upperDepth = 0.f;
    for (int k = 0; k < CVT_INT(m_nSoilLyrs[cell]); k++) {
        const float lowerDepth = m_soilDepth[cell][k];
        if (lowerDepth <= upperDepth) {
            continue;
        }
        if (activeDepth <= upperDepth) {
            break;
        }
        const float layerThickness = lowerDepth - upperDepth;
        const float activeThickness = Min(activeDepth, lowerDepth) - upperDepth;
        if (activeThickness > 0.f) {
            const float deficit = m_soilPor[cell][k] - m_soilWtrSto[cell][k];
            if (deficit > 0.f) {
                const float fill = Min(remainInfil, deficit * activeThickness);
                m_soilWtrSto[cell][k] += fill / layerThickness;
                remainInfil -= fill;
                if (remainInfil <= 0.f) {
                    break;
                }
            }
        }
        upperDepth = lowerDepth;
    }
}


void StormGreenAmpt::SetValue(const char *key, const int value) {
    string sk(key);
    if (m_stormMode && StringMatch(sk, Tag_HillSlopeTimeStep[0])) {
        m_dt = value;
    } else {
        throw ModelException(M_SUR_SGA[0], "SetValue",
                             "Parameter " + sk + " does not exist.");
    }

}

void StormGreenAmpt::SetValue(const char* key, const FLTPT value) {
    string sk(key);
    if (StringMatch(sk, "INFIL_FACTOR")) {
        m_infilFactor = CVT_FLT(value);
    } else if (StringMatch(sk, "ACTIVE_DEPTH_MAX")) {
        m_activeDepthMax = CVT_FLT(value);
    } else {
        throw ModelException(M_SUR_SGA[0], "SetValue",
                             "Parameter " + sk + " does not exist.");
    }
}

void StormGreenAmpt::Set1DData(const char *key, const int n, FLTPT *data) {
    CheckInputSize(M_SUR_SGA[0], key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_TMEAN[0])) {
        m_meanTmp = data;
    } else if (StringMatch(sk, VAR_NEPR[0])) {
        m_netPcp = data;
    } else if (StringMatch(sk, VAR_DPST[0])) {
        m_sd = data;
    } else if (StringMatch(sk, VAR_SURU[0])) {
        m_surfRf = data;
    } else if (StringMatch(sk, VAR_MOIST_IN[0])) {
        m_initSoilWtrStoRatio = data;
    } else {
        throw ModelException(M_SUR_SGA[0], "Set1DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void StormGreenAmpt::Set1DData(const char* key, const int n, int* data) {
    CheckInputSize(M_SUR_SGA[0], key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_SOILLAYERS[0])) {
        m_nSoilLyrs = data;
    }
    else {
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
