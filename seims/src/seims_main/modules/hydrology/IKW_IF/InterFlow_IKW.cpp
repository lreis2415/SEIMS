#include "InterFlow_IKW.h"
#include "text.h"

// using namespace std;  // Avoid this statement! by lj.

InterFlow_IKW::InterFlow_IKW() :
    m_nCells(-1), m_dt(-1.0f), m_CellWidth(-1.0f), m_chWidth(nullptr),
    m_s0(nullptr), m_rootDepth(nullptr), m_ks(nullptr), m_landuseFactor(1.f),
    m_soilWtrSto(nullptr), m_porosity(nullptr), m_poreIndex(nullptr), m_fieldCapacity(nullptr),
    m_flowInIndex(nullptr), m_routingLayers(nullptr), m_nLayers(-1),m_nSoilLyrs(nullptr),
    m_qi(nullptr), m_h(nullptr), m_sr(nullptr), m_streamLink(nullptr), m_hReturnFlow(nullptr),
    m_subSurfQ(nullptr){
}

InterFlow_IKW::~InterFlow_IKW(void) {
    Release1DArray(m_h);
    Release1DArray(m_qi);
    Release1DArray(m_hReturnFlow);
    Release2DArray(m_subSurfQ);
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
        for (int i = 0; i < m_nCells; ++i) {
            m_qi[i] = 0.0f;
            m_h[i] = 0.f;
            m_hReturnFlow[i] = 0.f;
            m_subSurfQ[i] = new float[m_nSoilLyrs[i]];
            for (int j = 0; j < m_nSoilLyrs[i]; ++j) {
                m_subSurfQ[i][j] = 0.0f;
            }

        }
    }
}

bool InterFlow_IKW::FlowInSoil(const int id) {

    //debug
    const int TARGET_ID = 100;
    bool is_debug = (id == TARGET_ID);

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
		float soilVolumn = m_rootDepth[id][j] / 1000 * m_CellWidth * flowWidth / cos(atan(s0)); //m3
        if (soilVolumn <= 1e-6) soilVolumn = 1e-6; // Avoid division by zero

        // Add upstream inflow to the current layer's soil moisture
        // Note: qUp_layers is in m3/s, so multiply by dt to get volume
        m_soilWtrSto[id][j] += qUp[j] * m_dt / soilVolumn;


		// the water exceeds the porosity is added to storage (return flow)
		if (m_soilWtrSto[id][j] > m_porosity[id][j]) {
			m_hReturnFlow[id] = (m_soilWtrSto[id][j] - m_porosity[id][j]) * m_rootDepth[id][j];
			m_sr[id] += m_hReturnFlow[id]; // Add to surface runoff
			m_soilWtrSto[id][j] = m_porosity[id][j]; // Cap at porosity
		}

		// if soil moisture is below the field capacity, no interflow will be generated
		if (m_soilWtrSto[id][j] < m_fieldCapacity[id][j]) {
            m_subSurfQ[id][j] = 0.0f; // No flow generated

            continue; //--fanxy
			//return;
		}

		// calculate effective hydraulic conductivity (mm/h -> m/s)
		//float k = m_ks[id]/1000/3600 * CalPow((m_soilMoistrue[id] - m_residual[id])/(m_porosity[id] - m_residual[id]), m_poreIndex[id]);
		float k = m_ks[id][j] / 1000 / 3600 * CalPow(m_soilWtrSto[id][j] / m_porosity[id][j], m_poreIndex[id][j]);
        
        // calculate interflow (m3/s)
		float layer_q = m_landuseFactor * m_rootDepth[id][j] / 1000.f * s0 * k * m_CellWidth;


		// available water
		float availableWater = (m_soilWtrSto[id][j] - m_fieldCapacity[id][j]) * soilVolumn;
        if (availableWater < 0.0f) {
            availableWater = 0.0f;
        }

		float potentialFlowVol = layer_q * (int)m_dt; // m3
		if (potentialFlowVol > availableWater) {
            layer_q = availableWater / (int)m_dt;
            potentialFlowVol = availableWater;
		}

        m_subSurfQ[id][j] = layer_q;
        

		// adjust soil moisture
		m_soilWtrSto[id][j] -= potentialFlowVol / soilVolumn;

        total_h_vol += potentialFlowVol;
        total_qi += m_subSurfQ[id][j];
	}

    m_qi[id] = total_qi;
    m_h[id] = 1000 * total_h_vol / (m_CellWidth * m_CellWidth);

    return true;
}

int InterFlow_IKW::Execute() {

    InitialOutputs();

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
        }
        if (errCount > 0) {
            throw ModelException(M_IKW_IF[0], "Execute:FlowInSoil",
                                 "Please check the error message for more information");
        }
    }
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
        m_landuseFactor = data;
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
