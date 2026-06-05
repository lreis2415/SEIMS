#include "StormGreenAmpt.h"

#include "text.h"

#include <cmath>
#include <cstdlib>
#include <fstream>

StormGreenAmpt::StormGreenAmpt() :
    m_dt(-1), m_nCells(-1), m_tSnow(0.0f), m_t0(1.0f), m_infilFactor(1.0f),
    m_activeDepthMax(0.0f), m_moistureReference(0.0f),
    m_accumuRecoveryRate(0.0f), m_accumuRecoveryDelay(0.0f),
    m_stateRecoveryFactor(0.0f),
    m_maxSoilLyrs(-1), m_nSoilLyrs(nullptr),
    m_soilDepth(nullptr), m_soilPor(nullptr),
    m_soilClay(nullptr), m_soilSand(nullptr), m_ks(nullptr),
    m_initSoilWtrStoRatio(nullptr), m_soilFC(nullptr),
    m_meanTmp(nullptr), m_netPcp(nullptr), m_sd(nullptr),
    m_snowMelt(nullptr), m_snowAccu(nullptr), m_surfRf(nullptr),
    m_capillarySuction(nullptr), m_accumuDepth(nullptr), m_dryDuration(nullptr),
    m_soilWtrSto(nullptr), m_infil(nullptr), m_infilCapacitySurplus(nullptr),
    m_exsPcp(nullptr){

}

StormGreenAmpt::~StormGreenAmpt() {
    if (m_capillarySuction != nullptr) Release1DArray(m_capillarySuction);
    if (m_accumuDepth != nullptr) Release1DArray(m_accumuDepth);
    if (m_dryDuration != nullptr) Release1DArray(m_dryDuration);
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
                m_soilWtrSto[i][j] = CalculateInitialSoilWater(i, j);
            }
        }

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
        Initialize1DArray(m_nCells, m_dryDuration, 0.f);

//#pragma omp parallel for
        for (int i = 0; i < m_nCells; ++i) {
            if (m_nSoilLyrs[i] > 0) {
                m_capillarySuction[i] = CalculateCapillarySuction(m_soilPor[i][0],
                                                                  m_soilClay[i][0],
                                                                  m_soilSand[i][0]);
            }
        }
    }

    const char* diagEnv = std::getenv("SEIMS_WB_DIAG");
    const bool writeDiag = diagEnv != nullptr && string(diagEnv) != "0" && !m_outpath.empty();
    double diagNetPcp = 0.0;
    double diagDepOld = 0.0;
    double diagSurfOld = 0.0;
    double diagSnowMelt = 0.0;
    double diagHWater = 0.0;
    double diagInfil = 0.0;
    double diagExcp = 0.0;
    double diagInfilSurplus = 0.0;
    double diagPotentialInfil = 0.0;
    double diagRawPotentialInfil = 0.0;
    double diagActiveDepth = 0.0;
    double diagInfilCap = 0.0;
    double diagAccumuDepth = 0.0;
    double diagAccumuRecovery = 0.0;
    double diagSoilDeficit = 0.0;
    double diagTheta = 0.0;
    double diagPorosity = 0.0;
    double diagClosure = 0.0;
    int diagWetCells = 0;
    int diagRedistributionCells = 0;
    int diagSaturatedCells = 0;
    int diagCapZeroCells = 0;
    int diagPotentialZeroCells = 0;
    int diagFactorZeroCells = 0;
    int diagOtherZeroCells = 0;
    int diagPositiveInfilCells = 0;

//#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        // only calculate the first soillayer
        int j = 0;

        float snowMelt = 0.f;
        float snowAcc = 0.f;
        if (m_snowMelt != nullptr) {
            snowMelt = std::isfinite(m_snowMelt[i]) ? Max(m_snowMelt[i], 0.f) : 0.f;
        }
        if (m_snowAccu != nullptr) {
            snowAcc = std::isfinite(m_snowAccu[i]) ? Max(m_snowAccu[i], 0.f) : 0.f;
        }

        const float netPcp = std::isfinite(m_netPcp[i]) ? Max(m_netPcp[i], 0.f) : 0.f;
        const float depWater = (m_sd != nullptr && std::isfinite(m_sd[i])) ? Max(m_sd[i], 0.f) : 0.f;
        const float surfWater = (m_surfRf != nullptr && std::isfinite(m_surfRf[i])) ? Max(m_surfRf[i], 0.f) : 0.f;
        const float meanTmp = std::isfinite(m_meanTmp[i]) ? m_meanTmp[i] : m_t0 + 1.f;
        float hWater = netPcp;
        //account for the effects of snow melt and soil temperature
        if (meanTmp <= m_tSnow) {
            // snow, without snow melt
            hWater = 0.0f;
        }
        else if (meanTmp > m_tSnow && meanTmp <= m_t0 && snowAcc > hWater) {
            // rain on snow, no snow melt, but rainfall is still liquid water
            hWater = netPcp + depWater;
        }
        else {
            //the old depression water has been added.(m_deprSto)
            hWater = netPcp + depWater + snowMelt;
        }
        if (!std::isfinite(hWater) || hWater < 0.f) {
            hWater = 0.f;
        }

        float dt = m_dt;
        if (!std::isfinite(dt) || dt <= 0.f) {
            dt = 1.f;
        }
        const float dryThreshold = 1.e-6f;
        const bool canRedistribute = netPcp <= dryThreshold &&
                                     snowMelt <= dryThreshold &&
                                     depWater <= dryThreshold;
        if (canRedistribute) {
            m_dryDuration[i] += dt;
        } else {
            m_dryDuration[i] = 0.f;
        }
        const float activeDepth = CalculateWettingFrontDepth(i);
        const float accumuRecovery = RedistributeAccumulatedInfiltration(i, activeDepth, dt, canRedistribute);

        float por = std::isfinite(m_soilPor[i][j]) ? Max(m_soilPor[i][j], 0.f) : 0.f;
        float theta = std::isfinite(m_soilWtrSto[i][j]) ? m_soilWtrSto[i][j] : 0.f;
        theta = Max(0.f, Min(theta, por));
        m_soilWtrSto[i][j] = theta;

        const float suction = std::isfinite(m_capillarySuction[i]) ?
            Max(m_capillarySuction[i], 0.f) : 0.f;
        const float soilDeficit = Max(por - theta, 0.f);
        // effective matric potential (m)
        float matricPotential = soilDeficit * suction / 1000.f;
        // algorithm of Li, 1996, uesd in C2SC2D
        float ks = std::isfinite(m_ks[i][j]) ? Max(m_ks[i][j], 0.f) / 1000.f / 3600.f : 0.f; // mm/h -> m/s
        float infilDepth = std::isfinite(m_accumuDepth[i]) ? Max(m_accumuDepth[i], 0.f) / 1000.f : 0.f; // mm ->m

        float p1 = ks * dt - 2.f * infilDepth;
        if (!std::isfinite(p1)) {
            p1 = 0.f;
        }
        float p2 = ks * (infilDepth + matricPotential);
        if (!std::isfinite(p2) || p2 < 0.f) {
            p2 = 0.f;
        }
        float radicand = CalPow(p1, 2.f) + 8.f * p2 * dt;
        if (!std::isfinite(radicand) || radicand < 0.f) {
            radicand = 0.f;
        }
        // infiltration rate (m/s)
        float infilRate = (p1 + CalSqrt(radicand)) / (2.f * dt);
        if (!std::isfinite(infilRate) || infilRate < 0.f) {
            infilRate = 0.f;
        }

        float infilCap = CalculateActiveInfilCap(i, activeDepth);
        if (!std::isfinite(infilCap) || infilCap < 0.f) {
            infilCap = 0.f;
        }
        const float accumuDepthMm = std::isfinite(m_accumuDepth[i]) ? Max(m_accumuDepth[i], 0.f) : 0.f;
        float rawPotentialInfil = infilRate * dt * 1000.f;
        if (!std::isfinite(rawPotentialInfil) || rawPotentialInfil < 0.f) {
            rawPotentialInfil = 0.f;
        }
        float potentialInfil = rawPotentialInfil;
        potentialInfil = Min(potentialInfil, infilCap);

        if (hWater > 0) {
            // for frozen soil
            //if (m_soilTemp[i] <= m_tSoilFrozen && m_soilMoisture[i] >= m_sFrozen*m_porosity[i])
            //{
            //	m_pe[i] = pNet;
            //	m_infil[i] = 0.0f;
            //}
            //for saturation overland flow
            if (theta >= por) {
                m_infil[i] = 0.0f;
                m_infilCapacitySurplus[i] = 0.f;
            }
            else {
                const float infilFactor = std::isfinite(m_infilFactor) ? Max(m_infilFactor, 0.f) : 0.f;
                float rawInfil = Min(potentialInfil * infilFactor, infilCap); // mm
                if (!std::isfinite(rawInfil) || rawInfil < 0.f) {
                    rawInfil = 0.f;
                }
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
                if (std::isfinite(m_infil[i]) && m_infil[i] > 0.f) {
                    m_accumuDepth[i] = Max(m_accumuDepth[i], 0.f) + m_infil[i];
                }


                if (m_soilDepth != nullptr && m_infil[i] > 0.f) {
                    AddInfiltrationToSoil(i, m_infil[i], activeDepth);
                }
            }
            m_exsPcp[i] = Max(hWater - m_infil[i], 0.f);
            // xdw modify
            //m_surfRf[i] = hWater - m_infil[i];  // sr is temporarily used to stored the water depth including the depression storage
        } else {
            //m_surfRf[i] = 0.0f;
            m_infil[i] = 0.0f;
            m_infilCapacitySurplus[i] = potentialInfil;

            m_exsPcp[i] = 0.f;
        }

        if (writeDiag) {
            const bool hasWater = hWater > 1.e-6f;
            diagNetPcp += netPcp;
            diagDepOld += depWater;
            diagSurfOld += surfWater;
            diagSnowMelt += snowMelt;
            diagHWater += hWater;
            diagInfil += m_infil[i];
            diagExcp += m_exsPcp[i];
            diagInfilSurplus += m_infilCapacitySurplus[i];
            diagPotentialInfil += potentialInfil;
            diagRawPotentialInfil += rawPotentialInfil;
            diagActiveDepth += activeDepth;
            diagInfilCap += infilCap;
            diagAccumuDepth += accumuDepthMm;
            diagAccumuRecovery += accumuRecovery;
            diagSoilDeficit += soilDeficit;
            diagTheta += theta;
            diagPorosity += por;
            diagClosure += hWater - m_infil[i] - m_exsPcp[i];
            if (hasWater || m_infil[i] > 1.e-6f || m_exsPcp[i] > 1.e-6f) {
                diagWetCells++;
            }
            if (accumuRecovery > 1.e-6f) {
                diagRedistributionCells++;
            }
            if (hasWater) {
                if (theta >= por) {
                    diagSaturatedCells++;
                } else if (infilCap <= 1.e-6f) {
                    diagCapZeroCells++;
                } else if (potentialInfil <= 1.e-6f) {
                    diagPotentialZeroCells++;
                } else if (m_infilFactor <= 1.e-6f) {
                    diagFactorZeroCells++;
                } else if (m_infil[i] <= 1.e-6f) {
                    diagOtherZeroCells++;
                } else {
                    diagPositiveInfilCells++;
                }
            }
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
#ifdef IS_DEBUG
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
#endif

# ifdef IS_DEBUG
        if ((counter >= printInfilMinT && counter <= printInfilMaxT)) {
            if (i >= output_icell_min && i <= output_icell_max && infiltFileFptr.is_open()) {
                infiltFileFptr << "icell: " << i << " infil: " << m_infil[i] << " acc: " << m_accumuDepth[i] << " pNet: " << m_netPcp[i] << " m_sr: " << m_surfRf[i] << endl;
            }
        }
# endif
    }
    infiltFileFptr.close();
    if (writeDiag) {
        const string diagPath = m_outpath + SEP + "SUR_SGA_balance.csv";
        std::ifstream existing(diagPath.c_str());
        const bool needHeader = !existing.good();
        existing.close();
        std::ofstream fs(diagPath.c_str(), std::ios::out | std::ios::app);
        if (fs.is_open()) {
            if (needHeader) {
                fs << "time,ncells,wet_cells,net_pcp_mm_cell,dep_old_mm_cell,"
                   << "surf_old_mm_cell,snowmelt_mm_cell,hwater_mm_cell,"
                   << "raw_potential_infil_mm_cell,potential_infil_mm_cell,"
                   << "active_depth_mm_cell,infil_cap_mm_cell,accumu_depth_mm_cell,"
                   << "accumu_recovery_mm_cell,soil_deficit_cell,theta_cell,porosity_cell,"
                   << "infil_mm_cell,excp_mm_cell,infil_capacity_surplus_mm_cell,"
                   << "redistribution_cells,saturated_cells,cap_zero_cells,potential_zero_cells,"
                   << "factor_zero_cells,other_zero_cells,positive_infil_cells,"
                   << "closure_mm_cell\n";
            }
            fs << ConvertToString2(m_date) << ","
               << m_nCells << "," << diagWetCells << ","
               << diagNetPcp << "," << diagDepOld << ","
               << diagSurfOld << "," << diagSnowMelt << ","
               << diagHWater << "," << diagRawPotentialInfil << ","
               << diagPotentialInfil << "," << diagActiveDepth << ","
               << diagInfilCap << "," << diagAccumuDepth << ","
               << diagAccumuRecovery << "," << diagSoilDeficit << "," << diagTheta << ","
               << diagPorosity << "," << diagInfil << "," << diagExcp << ","
               << diagInfilSurplus << "," << diagRedistributionCells << ","
               << diagSaturatedCells << ","
               << diagCapZeroCells << "," << diagPotentialZeroCells << ","
               << diagFactorZeroCells << "," << diagOtherZeroCells << ","
               << diagPositiveInfilCells << "," << diagClosure << "\n";
        }
    }
    return 0;
}

//this function calculated the wetting front matric potential (mm)
float StormGreenAmpt::CalculateCapillarySuction(float por, float clay, float sand) {
    float cs = 10.0f * CalExp(6.5309f - 7.32561f * por + 0.001583f * CalPow(clay, 2) + 3.809479f * CalPow(por, 2)
                              + 0.000344f * sand * clay - 0.049837f * por * sand
                              + 0.001608f * CalPow(por, 2) * CalPow(sand, 2)
                              + 0.001602f * CalPow(por, 2) * CalPow(clay, 2) - 0.0000136f * CalPow(sand, 2) * clay -
                              0.003479f * CalPow(clay, 2) * por - 0.000799f * CalPow(sand, 2) * por);
    if (!std::isfinite(cs) || cs < 0.f) {
        return 0.f;
    }
    // debug
    /*if (cs < 0.00001f) {
        std::cout << "\n[ERROR] Capillary Suction Underflow Detected!" << std::endl;
        std::cout << "  -> Inputs: Por=" << por << ", Clay=" << clay << ", Sand=" << sand << std::endl;
        std::cout << "  -> Result CS: " << cs << " (Should not be 0)" << std::endl;
        std::cout << "  -> Hint: Check if Clay/Sand are Percent(0-100) or Fraction(0-1). Formula usually expects Percent." << std::endl;
    }*/
    return cs;
}

float StormGreenAmpt::CalculateInitialSoilWater(const int cell, const int layer) {
    float ratio = std::isfinite(m_initSoilWtrStoRatio[cell]) ? m_initSoilWtrStoRatio[cell] : 0.f;
    ratio = Max(0.f, Min(ratio, 1.f));

    const float por = std::isfinite(m_soilPor[cell][layer]) ? Max(m_soilPor[cell][layer], 0.f) : 0.f;
    if (por <= 0.f) {
        return 0.f;
    }

    float baseTheta = m_soilFC[cell][layer];
    if (m_moistureReference >= 0.5f) {
        baseTheta = m_soilPor[cell][layer];
    }
    if (!std::isfinite(baseTheta) || baseTheta < 0.f) {
        baseTheta = 0.f;
    }
    return Min(ratio * baseTheta, por);
}

float StormGreenAmpt::CalculateWettingFrontDepth(const int cell) {
    const float minActiveDepth = 10.0f; // mm, keeps early storm infiltration numerically stable.
    if (m_nSoilLyrs[cell] <= 0 || !std::isfinite(m_soilDepth[cell][0]) ||
        m_soilDepth[cell][0] <= 0.f) {
        return minActiveDepth;
    }

    float activeDepth = m_activeDepthMax;
    if (!std::isfinite(activeDepth) || activeDepth <= 0.f) {
        activeDepth = m_soilDepth[cell][CVT_INT(m_nSoilLyrs[cell]) - 1];
    }
    if (activeDepth < minActiveDepth) {
        activeDepth = minActiveDepth;
    }
    const float profileDepth = m_soilDepth[cell][CVT_INT(m_nSoilLyrs[cell]) - 1];
    if (std::isfinite(profileDepth) && profileDepth > 0.f && activeDepth > profileDepth) {
        activeDepth = profileDepth;
    }
    return activeDepth;
}

float StormGreenAmpt::CalculateActiveInfilCap(const int cell, const float activeDepth) {
    if (!std::isfinite(activeDepth) || activeDepth <= 0.f || m_nSoilLyrs[cell] <= 0) {
        return 0.f;
    }
    float dynamicStorage = 0.f;
    float eventStorage = 0.f;
    CalculateActiveStorage(cell, activeDepth, dynamicStorage, eventStorage);

    // Cumulative Green-Ampt memory is reduced only by dry-period redistribution.
    const float effectiveAccumuDepth = std::isfinite(m_accumuDepth[cell]) ? Max(m_accumuDepth[cell], 0.f) : 0.f;
    const float remainingEventStorage = Max(eventStorage - effectiveAccumuDepth, 0.f);
    return Min(Max(dynamicStorage, 0.f), remainingEventStorage);
}

void StormGreenAmpt::CalculateActiveStorage(const int cell, const float activeDepth,
                                            float& dynamicStorage, float& eventStorage) {
    dynamicStorage = 0.f;
    eventStorage = 0.f;
    if (!std::isfinite(activeDepth) || activeDepth <= 0.f || m_nSoilLyrs[cell] <= 0) {
        return;
    }
    float upperDepth = 0.f;
    for (int k = 0; k < CVT_INT(m_nSoilLyrs[cell]); k++) {
        const float lowerDepth = m_soilDepth[cell][k];
        if (!std::isfinite(lowerDepth) || lowerDepth <= upperDepth) {
            continue;
        }
        if (activeDepth <= upperDepth) {
            break;
        }
        const float activeThickness = Min(activeDepth, lowerDepth) - upperDepth;
        if (activeThickness > 0.f) {
            const float por = std::isfinite(m_soilPor[cell][k]) ? Max(m_soilPor[cell][k], 0.f) : 0.f;
            const float theta = std::isfinite(m_soilWtrSto[cell][k]) ?
                Max(0.f, Min(m_soilWtrSto[cell][k], por)) : 0.f;
            const float dynamicDeficit = por - theta;
            if (dynamicDeficit > 0.f) {
                dynamicStorage += dynamicDeficit * activeThickness;
            }
            const float initTheta = CalculateInitialSoilWater(cell, k);
            const float eventDeficit = por - initTheta;
            if (eventDeficit > 0.f) {
                eventStorage += eventDeficit * activeThickness;
            }
        }
        upperDepth = lowerDepth;
    }
}

float StormGreenAmpt::RedistributeAccumulatedInfiltration(const int cell, const float activeDepth,
                                                          const float dt, const bool canRedistribute) {
    if (!canRedistribute || !std::isfinite(dt) || dt <= 0.f ||
        !std::isfinite(activeDepth) || activeDepth <= 0.f ||
        m_nSoilLyrs[cell] <= 0 || m_accumuDepth[cell] <= 0.f) {
        return 0.f;
    }

    const float recoveryDelay = Max(0.f, m_accumuRecoveryDelay) * 3600.f;
    if (m_dryDuration[cell] < recoveryDelay) {
        return 0.f;
    }

    float dynamicStorage = 0.f;
    float eventStorage = 0.f;
    CalculateActiveStorage(cell, activeDepth, dynamicStorage, eventStorage);

    // Water still stored above the event-reference state must keep limiting Green-Ampt capacity.
    const float retainedEventWater = Max(eventStorage - dynamicStorage, 0.f);
    const float memoryAboveRetained = Max(m_accumuDepth[cell] - retainedEventWater, 0.f);
    if (memoryAboveRetained <= 0.f) {
        return 0.f;
    }

    const float stateRecoveryFactor = std::isfinite(m_stateRecoveryFactor) ?
        Max(0.f, Min(m_stateRecoveryFactor, 1.f)) : 0.f;
    const float stateRecovery = stateRecoveryFactor * memoryAboveRetained;
    const float rateRecovery = m_accumuRecoveryRate > 0.f ? m_accumuRecoveryRate * dt / 3600.f : 0.f;

    float recovery = 0.f;
    if (stateRecovery > 0.f && rateRecovery > 0.f) {
        recovery = Min(stateRecovery, rateRecovery);
    } else {
        recovery = Max(stateRecovery, rateRecovery);
    }
    recovery = Min(memoryAboveRetained, recovery);
    if (!std::isfinite(recovery) || recovery <= 0.f) {
        return 0.f;
    }

    const float oldAccumu = m_accumuDepth[cell];
    m_accumuDepth[cell] = Max(retainedEventWater, m_accumuDepth[cell] - recovery);
    return Max(oldAccumu - m_accumuDepth[cell], 0.f);
}

void StormGreenAmpt::AddInfiltrationToSoil(const int cell, const float infiltration, const float activeDepth) {
    if (!std::isfinite(infiltration) || infiltration <= 0.f ||
        !std::isfinite(activeDepth) || activeDepth <= 0.f) {
        return;
    }
    float remainInfil = infiltration;
    float upperDepth = 0.f;
    for (int k = 0; k < CVT_INT(m_nSoilLyrs[cell]); k++) {
        const float lowerDepth = m_soilDepth[cell][k];
        if (!std::isfinite(lowerDepth) || lowerDepth <= upperDepth) {
            continue;
        }
        if (activeDepth <= upperDepth) {
            break;
        }
        const float layerThickness = lowerDepth - upperDepth;
        const float activeThickness = Min(activeDepth, lowerDepth) - upperDepth;
        if (activeThickness > 0.f) {
            const float por = std::isfinite(m_soilPor[cell][k]) ? Max(m_soilPor[cell][k], 0.f) : 0.f;
            const float theta = std::isfinite(m_soilWtrSto[cell][k]) ?
                Max(0.f, Min(m_soilWtrSto[cell][k], por)) : 0.f;
            const float deficit = por - theta;
            if (deficit > 0.f) {
                const float fill = Min(remainInfil, deficit * activeThickness);
                m_soilWtrSto[cell][k] = Min(theta + fill / layerThickness, por);
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
        m_infilFactor = Max(0.f, CVT_FLT(value));
    } else if (StringMatch(sk, "ACTIVE_DEPTH_MAX")) {
        m_activeDepthMax = CVT_FLT(value);
    } else if (StringMatch(sk, "MOIST_IN_REF")) {
        m_moistureReference = CVT_FLT(value) >= 0.5f ? 1.f : 0.f;
    } else if (StringMatch(sk, "GA_ACC_RECOVERY_RATE")) {
        m_accumuRecoveryRate = Max(0.f, CVT_FLT(value));
    } else if (StringMatch(sk, "GA_ACC_RECOVERY_DELAY")) {
        m_accumuRecoveryDelay = Max(0.f, CVT_FLT(value));
    } else if (StringMatch(sk, "GA_STATE_RECOVERY_FACTOR")) {
        m_stateRecoveryFactor = Max(0.f, Min(CVT_FLT(value), 1.f));
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
