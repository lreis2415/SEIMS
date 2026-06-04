#include "InterFlow_IKW.h"
#include "text.h"

#include <cmath>
#include <cstdlib>
#include <fstream>

// using namespace std;  // Avoid this statement! by lj.

InterFlow_IKW::InterFlow_IKW() :
    m_nCells(-1), m_dt(-1.0f), m_CellWidth(-1.0f), m_chWidth(nullptr),
    m_s0(nullptr), m_rootDepth(nullptr), m_ks(nullptr), m_landuseFactor(1.f),
    m_initSoilWtrStoRatio(nullptr), m_moistureReference(0.f),
    m_fastRatio(0.f), m_anisotropy(1.f), m_macroporeFactor(1.f),
    m_substeps(1),
    m_soilWtrSto(nullptr), m_initialSoilWtrSto(nullptr),
    m_porosity(nullptr), m_poreIndex(nullptr), m_fieldCapacity(nullptr),
    m_flowInIndex(nullptr), m_routingLayers(nullptr), m_nLayers(-1),m_nSoilLyrs(nullptr),
    m_qi(nullptr), m_h(nullptr), m_sr(nullptr), m_streamLink(nullptr), m_hReturnFlow(nullptr),
    m_subSurfQ(nullptr){
}

InterFlow_IKW::~InterFlow_IKW(void) {
    Release1DArray(m_h);
    Release1DArray(m_qi);
    Release1DArray(m_hReturnFlow);
    Release2DArray(m_subSurfQ);
    Release2DArray(m_initialSoilWtrSto);
}

bool InterFlow_IKW::CheckInputData(void) {
    if (m_date <= 0) {
        throw ModelException(M_IKW_IF[0], "CheckInputData", "You have not set the Date variable.");
        return false;
    }

    if (m_nCells <= 0) {
        throw ModelException(M_IKW_IF[0], "CheckInputData", "The cell number of the input can not be less than zero.");
        return false;
    }

    if (m_dt <= 0) {
        throw ModelException(M_IKW_IF[0], "CheckInputData", "You have not set the TimeStep variable.");
        return false;
    }

    if (m_CellWidth <= 0) {
        throw ModelException(M_IKW_IF[0], "CheckInputData", "You have not set the CellWidth variable.");
        return false;
    }

    if (m_chWidth == nullptr) {
        throw ModelException(M_IKW_CH[0], "CheckInputData", "The parameter: CHWIDTH has not been set.");
    }

    if (m_flowInIndex == nullptr) {
        throw ModelException(M_IKW_IF[0], "CheckInputData", "The parameter: flow in index has not been set.");
    }
    if (m_routingLayers == nullptr) {
        throw ModelException(M_IKW_IF[0], "CheckInputData", "The parameter: routingLayers has not been set.");
    }

    if (m_s0 == nullptr) {
        throw ModelException(M_IKW_IF[0], "CheckInputData", "The parameter: slope has not been set.");
    }
    if (m_rootDepth == nullptr) {
        throw ModelException(M_IKW_IF[0], "CheckInputData", "The parameter: soil depth has not been set.");
    }
    if (m_ks == nullptr) {
        throw ModelException(M_IKW_IF[0], "CheckInputData", "The parameter: Conductivity has not been set.");
    }

    if (m_porosity == nullptr) {
        throw ModelException(M_IKW_IF[0], "CheckInputData", "The porosity can not be nullptr.");
    }
    if (m_poreIndex == nullptr) {
        throw ModelException(M_IKW_IF[0], "CheckInputData", "The pore index can not be nullptr.");
    }
    if (m_fieldCapacity == nullptr) {
        throw ModelException(M_IKW_IF[0], "CheckInputData", "The field capacity can not be nullptr.");
    }
    if (m_initSoilWtrStoRatio == nullptr) {
        throw ModelException(M_IKW_IF[0], "CheckInputData", "The initial soil moisture can not be nullptr.");
    }
    if (m_soilWtrSto == nullptr) {
        throw ModelException(M_IKW_IF[0], "CheckInputData", "The soil moistrue can not be nullptr.");
    }
    if (m_streamLink == nullptr) {
        throw ModelException(M_IKW_IF[0], "CheckInputData", "The STREAM_LINK can not be nullptr.");
    }

    if (m_sr == nullptr) {
        throw ModelException(M_IKW_IF[0], "CheckInputData", "The parameter D_SURU is not set.");
    }
    if (m_nSoilLyrs == nullptr) {
        throw ModelException(M_IKW_IF[0], "CheckInputData", "The soil layers can not be nullptr.");
    }
    CHECK_POINTER(M_IKW_IF[0], m_nSoilLyrs);
    return true;
}

void InterFlow_IKW:: InitialOutputs() {
    if (m_nCells <= 0) {
        throw ModelException(M_IKW_IF[0], "InitialOutputs", "The cell number of the input can not be less than zero.");
    }

    if (m_qi == nullptr) {
        CheckInputData();

        m_qi = new float[m_nCells];
        m_h = new float[m_nCells];
        m_hReturnFlow = new float[m_nCells];
        m_subSurfQ = new float* [m_nCells];
        Initialize2DArray(m_nCells, m_maxSoilLyrs, m_initialSoilWtrSto, 0.f);
        for (int i = 0; i < m_nCells; ++i) {
            m_qi[i] = 0.0f;
            m_h[i] = 0.f;
            m_hReturnFlow[i] = 0.f;
            m_subSurfQ[i] = new float[m_nSoilLyrs[i]];
            for (int j = 0; j < m_nSoilLyrs[i]; ++j) {
                m_subSurfQ[i][j] = 0.0f;
                float ratio = std::isfinite(m_initSoilWtrStoRatio[i]) ?
                    Max(0.f, Min(m_initSoilWtrStoRatio[i], 1.f)) : 0.f;
                float fieldCapacity = std::isfinite(m_fieldCapacity[i][j]) ?
                    Max(m_fieldCapacity[i][j], 0.f) : 0.f;
                float porosity = std::isfinite(m_porosity[i][j]) ?
                    Max(m_porosity[i][j], fieldCapacity) : fieldCapacity;
                float baseTheta = m_moistureReference >= 0.5f ? porosity : fieldCapacity;
                m_initialSoilWtrSto[i][j] = Min(ratio * baseTheta, porosity);
            }

        }
    }
}

bool InterFlow_IKW::FlowInSoil(const int id) {
    m_hReturnFlow[id] = 0.0f;

    //Loop through all upstream cells
    vector<float> qUp(m_nSoilLyrs[id], 0.0f);
    for (int k = 1; k <= (int) m_flowInIndex[id][0]; ++k) {
        int flowInID = (int) m_flowInIndex[id][k];

        // Get the number of layers of the upstream cell
        // (Assuming upstream layers >= current layers, or we take the min)
        int commonLayers = Min(m_nSoilLyrs[id], m_nSoilLyrs[flowInID]);

        for (int j = 0; j < commonLayers; ++j) {
            // Add the outflow from the upstream cell's corresponding layer
            float upFlow = m_subSurfQ[flowInID][j];

            // Safety check for negative flow
            if (upFlow < 0.0f) {
                // Log error if needed, or just reset to 0
                upFlow = 0.0f;
            }
            qUp[j] += upFlow;
        }
        
    }

	float s0 = Max(m_s0[id], 0.01f);
    float flowWidth = m_CellWidth;

    // If this is a river cell, it collects water from all soil layers of upstream cells
    if (m_streamLink[id] > 0) {
        float total_river_q = 0.0f;

        // Sum up inflow from all layers
        for (int j = 0; j < m_nSoilLyrs[id]; ++j) {
            total_river_q += qUp[j];

            // Ensure no subsurface flow leaves the river cell to downstream soil
            m_subSurfQ[id][j] = 0.0f;
        }
        // Assign the total aggregated flow to the river channel variable
        m_qi[id] = total_river_q;

        flowWidth -= m_chWidth[id];
        if (flowWidth <= 0) {
            
            m_h[id] = 0.f;

        }
        //The river course fills the entire cell, and the soil width is 0.
        //Return directly to prevent m_qi on the river channel from being set as nodata
        return true;
    }

    m_qi[id] = 0.0f;
    float total_qi = 0.0f; // To sum up total interflow for this cell
    float total_h_vol = 0.0f; // To sum up total volume (for depth calculation)

	// adjust soil moisture
	for (int j = 0; j < m_nSoilLyrs[id]; j++) {
		//float s0 = m_s0[id];
        float rootDepth = std::isfinite(m_rootDepth[id][j]) ? Max(m_rootDepth[id][j], 0.f) : 0.f;
		float soilVolumn = rootDepth / 1000.f * m_CellWidth * flowWidth / cos(atan(s0)); //m3
        if (soilVolumn <= 1e-6) soilVolumn = 1e-6; // Avoid division by zero

        float fieldCapacity = std::isfinite(m_fieldCapacity[id][j]) ?
            Max(m_fieldCapacity[id][j], 1.e-6f) : 1.e-6f;
        float porosity = std::isfinite(m_porosity[id][j]) ?
            Max(m_porosity[id][j], fieldCapacity + 1.e-6f) : fieldCapacity + 1.e-6f;
        float soilTheta = std::isfinite(m_soilWtrSto[id][j]) ?
            Max(0.f, Min(m_soilWtrSto[id][j], porosity)) : 0.f;
        m_soilWtrSto[id][j] = soilTheta;

        float initialTheta = std::isfinite(m_initialSoilWtrSto[id][j]) ?
            Max(0.f, Min(m_initialSoilWtrSto[id][j], porosity)) : 0.f;
        const float eventThreshold = Max(0.f, Min(initialTheta, fieldCapacity));
        // Preferential paths are assumed to become connected by field capacity,
        // while porosity still limits the event-water storage volume.
        const float connectivityRange = Max(fieldCapacity - eventThreshold, 1.e-6f);
        float eventConnectivity = Max(0.0f, Min((soilTheta - eventThreshold) / connectivityRange, 1.0f));
        float effectiveFastRatio = Max(0.0f, Min(m_fastRatio * eventConnectivity, 0.95f));
        float upstreamFlow = Max(qUp[j], 0.0f);
        float fastInflow = upstreamFlow * effectiveFastRatio;
        float matrixInflow = upstreamFlow - fastInflow;

        // Add the matrix part of upstream inflow to soil moisture. The fast
        // part is routed laterally as preferential flow without filling storage.
        m_soilWtrSto[id][j] += matrixInflow * m_dt / soilVolumn;


		// the water exceeds the porosity is added to storage (return flow)
		if (m_soilWtrSto[id][j] > porosity) {
			float returnFlow = (m_soilWtrSto[id][j] - porosity) * rootDepth;
			m_hReturnFlow[id] += returnFlow;
			// Return flow is added to surface runoff in Execute() after all substeps,
			// to avoid double-counting when m_substeps > 1.
			m_soilWtrSto[id][j] = porosity; // Cap at porosity
		}

        soilTheta = Max(0.f, Min(m_soilWtrSto[id][j], porosity));

		// calculate effective hydraulic conductivity (mm/h -> m/s)
		//float k = m_ks[id]/1000/3600 * CalPow((m_soilMoistrue[id] - m_residual[id])/(m_porosity[id] - m_residual[id]), m_poreIndex[id]);
        // Fix: Use correct Campbell exponent (3 + 2/lambda) instead of lambda directly
        // Assuming m_poreIndex stores lambda (pore size distribution index) as used in Percolation module
        float poreIndex = std::isfinite(m_poreIndex[id][j]) ? Max(m_poreIndex[id][j], 1.e-6f) : 1.e-6f;
        float campbell_exponent = 3.0f + 2.0f / poreIndex;
        float relativeSaturation = Max(0.0f, Min(soilTheta / porosity, 1.0f));
        float ksat = std::isfinite(m_ks[id][j]) ? Max(m_ks[id][j], 0.0f) / 1000.f / 3600.f : 0.0f;
        float baseK = ksat * CalPow(relativeSaturation, campbell_exponent);
        float k = baseK * Max(m_anisotropy, 0.0f);
        
        // calculate interflow (m3/s)
		float layer_q = 0.0f;
		if (soilTheta > fieldCapacity) {
            layer_q = m_landuseFactor * rootDepth / 1000.f * s0 * k * m_CellWidth;
        }


		// available water
		float availableWater = (soilTheta - fieldCapacity) * soilVolumn;
        if (availableWater < 0.0f) {
            availableWater = 0.0f;
        }

		float potentialFlowVol = layer_q * m_dt; // m3
		if (potentialFlowVol > availableWater) {
            layer_q = availableWater / m_dt;
            potentialFlowVol = availableWater;
		}

		// adjust soil moisture
		m_soilWtrSto[id][j] -= potentialFlowVol / soilVolumn;
        availableWater -= potentialFlowVol;
        soilTheta = Max(0.f, Min(m_soilWtrSto[id][j], porosity));

        float macropore_q = 0.0f;
        float eventWater = (soilTheta - eventThreshold) * soilVolumn;
        if (m_macroporeFactor > 1.0f && eventWater > 0.0f && soilTheta > eventThreshold) {
            eventConnectivity = Max(0.0f, Min((soilTheta - eventThreshold) / connectivityRange, 1.0f));
            float macroporeK = k * (m_macroporeFactor - 1.0f) * eventConnectivity;
            macropore_q = m_landuseFactor * rootDepth / 1000.f * s0 * macroporeK * m_CellWidth;
            float macroporeWater = Max(eventWater, 0.0f);
            float macroporeVol = macropore_q * m_dt;
            if (macroporeVol > macroporeWater) {
                macropore_q = macroporeWater / m_dt;
                macroporeVol = macroporeWater;
            }
            m_soilWtrSto[id][j] -= macroporeVol / soilVolumn;
            m_soilWtrSto[id][j] = Max(eventThreshold, m_soilWtrSto[id][j]);
            potentialFlowVol += macroporeVol;
        }

        m_subSurfQ[id][j] = layer_q + macropore_q + fastInflow;

        total_h_vol += potentialFlowVol;
        total_qi += m_subSurfQ[id][j];
	}

    m_qi[id] = total_qi;
    m_h[id] = 1000 * total_h_vol / (m_CellWidth * m_CellWidth);
    // --- DEBUG PRINT ---
    std::vector<int> targetCells = { 1304, 1193, 1192, 1191, 1190,
                                     1189, 1188, 1187, 1186, 1185,
                                     1294, 1404, 1403, 1513, 1623,
                                     1622, 1731, 1730, 1838, 1837, 1836,
                                     1944 };
    bool isDebugTarget = false;
    for (int target : targetCells) { if (id == target) { isDebugTarget = true; break; } }

    if (false && isDebugTarget) { // DEBUG_ID
        
        std::cout << "[TRACE_CSV],Step,UNKNOWN"
            << ",Module,InterFlow"
            << ",Cell," << id
            << ",Total_Interflow_Qi," << m_qi[id]; 

        for (int j = 0; j < m_nSoilLyrs[id]; j++) {
            std::cout << ",LatFlow_L" << j << "," << m_subSurfQ[id][j]
                << ",SoilSto_L" << j << "," << m_soilWtrSto[id][j];
        }
        std::cout << std::endl;
    }
    return true;
}

int InterFlow_IKW::Execute() {

    InitialOutputs();

    // Substep routing: allow interflow to propagate multiple routing layers
    // within a single model timestep, reducing peak timing lag.
    float dtOriginal = m_dt;
    int substeps = Max(1, m_substeps);
    m_dt = dtOriginal / substeps;

    float* totalReturnFlow = nullptr;
    if (m_nCells > 0) {
        totalReturnFlow = new float[m_nCells];
        for (int i = 0; i < m_nCells; i++) {
            totalReturnFlow[i] = 0.f;
        }
    }

    for (int sub = 0; sub < substeps; sub++) {
        for (int iLayer = 0; iLayer < m_nLayers; ++iLayer) {
            // There are not any flow relationship within each routing layer.
            // So parallelization can be done here.
            int nCells = (int) m_routingLayers[iLayer][0];
            //SetOpenMPThread(2);
			int errCount = 0; //similar to SSR_DA, such that FlowInSoil(id) isn't called in omp loop
//#pragma omp parallel for
            for (int iCell = 1; iCell <= nCells; ++iCell) {
                int id = (int) m_routingLayers[iLayer][iCell];
                if (!FlowInSoil(id))
                {
                    errCount++;
                }
                if (totalReturnFlow != nullptr) {
                    totalReturnFlow[id] += m_hReturnFlow[id];
                }
            }
            if (errCount > 0) {
                throw ModelException(M_IKW_IF[0], "Execute:FlowInSoil",
                                     "Please check the error message for more information");
            }
        }
    }

    // Add total return flow to surface runoff once, after all substeps
    if (totalReturnFlow != nullptr) {
        for (int i = 0; i < m_nCells; i++) {
            if (totalReturnFlow[i] > 0.f) {
                m_sr[i] += totalReturnFlow[i];
            }
        }
        delete[] totalReturnFlow;
    }

    const char* diagEnv = std::getenv("SEIMS_IKW_IF_DIAG");
    if (diagEnv != nullptr && string(diagEnv) != "0" && !m_outpath.empty()) {
        double riverQi = 0.0;
        double allQi = 0.0;
        double returnFlow = 0.0;
        float maxRiverQi = 0.f;
        float maxCellQi = 0.f;
        float maxThetaMinusInitial = -MAXIMUMFLOAT;
        float maxThetaMinusFieldCapacity = -MAXIMUMFLOAT;
        int riverCells = 0;
        int wetCells = 0;
        int aboveInitialCells = 0;
        int aboveFieldCapacityCells = 0;
        for (int i = 0; i < m_nCells; i++) {
            const float qi = std::isfinite(m_qi[i]) ? Max(m_qi[i], 0.f) : 0.f;
            allQi += qi;
            if (qi > maxCellQi) {
                maxCellQi = qi;
            }
            if (m_streamLink[i] > 0) {
                riverQi += qi;
                riverCells++;
                if (qi > maxRiverQi) {
                    maxRiverQi = qi;
                }
            }
            if (qi > 0.f) {
                wetCells++;
            }
            returnFlow += std::isfinite(m_hReturnFlow[i]) ? Max(m_hReturnFlow[i], 0.f) : 0.f;
            for (int j = 0; j < m_nSoilLyrs[i]; j++) {
                const float theta = std::isfinite(m_soilWtrSto[i][j]) ? m_soilWtrSto[i][j] : 0.f;
                const float initialTheta = std::isfinite(m_initialSoilWtrSto[i][j]) ?
                    m_initialSoilWtrSto[i][j] : 0.f;
                const float fieldCapacity = std::isfinite(m_fieldCapacity[i][j]) ?
                    m_fieldCapacity[i][j] : MAXIMUMFLOAT;
                const float dInitial = theta - initialTheta;
                const float dFieldCapacity = theta - fieldCapacity;
                if (dInitial > maxThetaMinusInitial) {
                    maxThetaMinusInitial = dInitial;
                }
                if (dFieldCapacity > maxThetaMinusFieldCapacity) {
                    maxThetaMinusFieldCapacity = dFieldCapacity;
                }
                if (dInitial > 1.e-6f) {
                    aboveInitialCells++;
                }
                if (dFieldCapacity > 1.e-6f) {
                    aboveFieldCapacityCells++;
                }
            }
        }
        const string diagPath = m_outpath + SEP + "IKW_IF_diag.csv";
        std::ifstream existing(diagPath.c_str());
        const bool needHeader = !existing.good();
        existing.close();
        std::ofstream fs(diagPath.c_str(), std::ios::out | std::ios::app);
        if (fs.is_open()) {
            if (needHeader) {
                fs << "time,river_qi_sum,all_qi_sum,max_river_qi,max_cell_qi,"
                   << "returnflow_depth_sum,river_cells,wet_cells,"
                   << "max_theta_minus_initial,max_theta_minus_fc,"
                   << "above_initial_cells,above_fc_cells\n";
            }
            fs << ConvertToString2(m_date) << ","
               << riverQi << "," << allQi << ","
               << maxRiverQi << "," << maxCellQi << ","
               << returnFlow << "," << riverCells << "," << wetCells << ","
               << maxThetaMinusInitial << "," << maxThetaMinusFieldCapacity << ","
               << aboveInitialCells << "," << aboveFieldCapacityCells << "\n";
        }
    }

    m_dt = dtOriginal;
    return 0;
}

bool InterFlow_IKW::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        //StatusMsg("Input data for "+string(key) +" is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; }
        else {
            //StatusMsg("Input data for "+string(key) +" is invalid. All the input data should have same size.");
            std::ostringstream oss;
            oss << "Input data for " + string(key) << " is invalid with size: " << n <<
                    ". The origin size is " << m_nCells << ".\n";
            throw ModelException(M_IKW_IF[0], "CheckInputSize", oss.str());
        }
    }

    return true;
}

void InterFlow_IKW::SetValue(const char *key, FLTPT data) {
    string sk(key);
    if (StringMatch(sk, Tag_CellWidth[0])) {
        m_CellWidth = data;
    }
    else if (StringMatch(sk, VAR_KI[0])) {
        m_landuseFactor = Max(0.f, CVT_FLT(data));
    }
    else if (StringMatch(sk, "MOIST_IN_REF")) {
        m_moistureReference = CVT_FLT(data) >= 0.5f ? 1.f : 0.f;
    }
    else if (StringMatch(sk, VAR_FAST_RATIO[0])) {
        m_fastRatio = Max(0.f, Min(CVT_FLT(data), 1.f));
    }
    else if (StringMatch(sk, VAR_ANISOTROPY[0])) {
        m_anisotropy = Max(0.f, CVT_FLT(data));
    }
    else if (StringMatch(sk, VAR_MACROPORE_FACTOR[0])) {
        m_macroporeFactor = Max(1.f, CVT_FLT(data));
    }
    else if (StringMatch(sk, "IF_SUBSTEPS")) {
        m_substeps = Max(1, CVT_INT(data));
    } else {
        throw ModelException(M_IKW_IF[0], "SetSingleData", "Parameter " + sk
                             + " does not exist.");
    }

}

void InterFlow_IKW::SetValue(const char* key, int data) {
    string sk(key);
    if (StringMatch(sk, Tag_HillSlopeTimeStep[0])) {
        m_dt = data;
    }
    else if (StringMatch(sk, Tag_CellSize[0])) {
        m_nCells = (int)data;
    }
    else {
        throw ModelException(M_IKW_IF[0], "SetSingleData", "Parameter " + sk
            + " does not exist.");
    }

}

void InterFlow_IKW::Set1DData(const char *key, int n, FLTPT *data) {
    //check the input data
    CheckInputSize(key, n);
    string s(key);
    if (StringMatch(s, VAR_SLOPE[0])) {
        m_s0 = data;

    // } else if (StringMatch(s, VAR_SOILDEPTH[0])) {
    //     m_soilDepth = data;
    // } else if (StringMatch(s, VAR_FIELDCAP[0])) {
    //     m_fieldCapacity = data;
    // } else if (StringMatch(s, VAR_SOILDEPTH[0])) {
    //     m_rootDepth = data;
    // } else if (StringMatch(s, VAR_CONDUCT[0])) {
    //     m_ks = data;
    // } else if (StringMatch(s, VAR_POROST[0])) {
    //     m_porosity = data;
    // } else if (StringMatch(s, VAR_POREIDX[0])) {
    //     m_poreIndex = data;
    // } else if (StringMatch(s, VAR_SOL_ST[0])) {
    //     m_soilMoistrue = data;
    } else if (StringMatch(s, VAR_CHWIDTH[0])) {
        m_chWidth = data;
    } else if (StringMatch(s, VAR_SURU[0])) {
        m_sr = data;
    } else if (StringMatch(s, VAR_MOIST_IN[0])) {
        m_initSoilWtrStoRatio = data;
    } 
    else {
        throw ModelException(M_IKW_IF[0], "Set1DData", "Parameter " + s
                             + " does not exist.");
    }
}

void InterFlow_IKW::Set1DData(const char* key, int n, int* data) {
    //check the input data
    CheckInputSize(key, n);
    string s(key);
    if (StringMatch(s, VAR_STREAM_LINK[0])) {
        m_streamLink = data;
    }
    else if (StringMatch(s, VAR_SOILLAYERS[0])) {
        m_nSoilLyrs = data;
    }
    else {
        throw ModelException(M_IKW_IF[0], "Set1DData", "Parameter " + s
            + " does not exist.");
    }

}

void InterFlow_IKW::Get1DData(const char *key, int *n, float **data) {
    InitialOutputs();

    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_QSOIL[0])) {
        *data = m_qi;
    } else if (StringMatch(sk, VAR_RETURNFLOW[0])) {
        *data = m_hReturnFlow;
    } else {
        throw ModelException(M_IKW_IF[0], "Get1DData", "Output " + sk
                             + " does not exist.");
    }
}

void InterFlow_IKW::Get2DData(const char* key, int* nrows, int* ncols, FLTPT*** data)
{
}

void InterFlow_IKW::Set2DData(const char *key, int nrows, int ncols, FLTPT **data) {
    //check the input data

    string sk(key);
    /*if (StringMatch(sk, Tag_ROUTING_LAYERS[0])) {
        m_nLayers = nrows;
        m_routingLayers = data;
    } else if (StringMatch(sk, Tag_FLOWIN_INDEX[0])) {
		CheckInputSize(key, nrows);
		m_flowInIndex = data;
	}
	else*/

    if (StringMatch(sk, VAR_SOILDEPTH[0])) {
		CheckInputSize(key, nrows);
		m_maxSoilLyrs = ncols;
		m_rootDepth = data;
    }
	else if (StringMatch(sk, VAR_SOL_ST[0])) {
		CheckInputSize(key, nrows);
		m_maxSoilLyrs = ncols;
		m_soilWtrSto = data;
	}
	else if (StringMatch(sk, VAR_FIELDCAP[0])) {
		CheckInputSize(key, nrows);
		m_maxSoilLyrs = ncols;
		m_fieldCapacity = data;
	}
	else if (StringMatch(sk, VAR_POROST[0])) {
		CheckInputSize(key, nrows);
		m_maxSoilLyrs = ncols;
		m_porosity = data;
	}
	else if (StringMatch(sk, VAR_POREIDX[0])) {
		CheckInputSize(key, nrows);
		m_maxSoilLyrs = ncols;
		m_poreIndex = data;
	}
	else if (StringMatch(sk, VAR_CONDUCT[0])) {
		CheckInputSize(key, nrows);
		m_maxSoilLyrs = ncols;
		m_ks = data;
	}
	else {
        throw ModelException(M_IKW_IF[0], "Set2DData", "Parameter " + sk
            + " does not exist. Please contact the module developer.");
    }
}

void InterFlow_IKW::Set2DData(const char* key, int nrows, int ncols, int** data) {
    //check the input data

    string sk(key);
    if (StringMatch(sk, Tag_ROUTING_LAYERS[0])) {
        m_nLayers = nrows;
        m_routingLayers = data;
    }
    else if (StringMatch(sk, Tag_FLOWIN_INDEX[0])) {
        CheckInputSize(key, nrows);
        m_flowInIndex = data;
    }
    else {
        throw ModelException(M_IKW_IF[0], "Set2DData", "Parameter " + sk
            + " does not exist. Please contact the module developer.");
    }
}
