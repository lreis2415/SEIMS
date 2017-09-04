#include "seims.h"
#include "SSR_DA.h"

SSR_DA::SSR_DA(void) : m_nSoilLayers(-1), m_dt(-1), m_nCells(-1), m_CellWidth(-1.f), m_nSubbasin(-1),
                       m_frozenT(NODATA_VALUE), m_ki(NODATA_VALUE),
                       m_soilLayers(NULL), m_soilThick(NULL), m_ks(NULL), m_satmm(NULL), m_poreIndex(NULL),
                       m_fcmm(NULL), m_wpmm(NULL),
                       m_slope(NULL), m_chWidth(NULL), m_streamLink(NULL), m_subbasin(NULL),
                       m_flowInIndex(NULL), m_flowInPercentage(NULL), m_routingLayers(NULL), m_nRoutingLayers(-1),
    /// input from other modules
                       m_soilStorage(NULL), m_soilStorageProfile(NULL), m_soilT(NULL),
    /// outputs
                       m_qi(NULL), m_qiVol(NULL), m_qiSubbasin(NULL) {
}

SSR_DA::~SSR_DA(void) {
    if (m_qi != NULL) Release2DArray(m_nCells, m_qi);
    if (m_qiVol != NULL) Release2DArray(m_nCells, m_qiVol);
    if (m_qiSubbasin != NULL) Release1DArray(m_qiSubbasin);
}

void SSR_DA::FlowInSoil(int id) {
    float s0 = max(m_slope[id], 0.01f);
    float flowWidth = m_CellWidth;
    // there is no land in this cell
    if (m_streamLink[id] > 0) {
        flowWidth -= m_chWidth[id];
    }
    // initialize for current cell of current timestep
    for (int j = 0; j < (int) m_soilLayers[id]; j++) {
        m_qi[id][j] = 0.f;
        m_qiVol[id][j] = 0.f;
    }
    // return with initial values if flowWidth is less than 0
    if (flowWidth <= 0) return;
    // number of flow-in cells
    int nUpstream = (int) m_flowInIndex[id][0];
    m_soilStorageProfile[id] = 0.f; // update soil storage on profile
    for (int j = 0; j < (int) m_soilLayers[id]; j++) {
        float smOld = m_soilStorage[id][j];
        //sum the upstream subsurface flow, m3
        float qUp = 0.f;
        for (int upIndex = 1; upIndex <= nUpstream; ++upIndex) {
            int flowInID = (int) m_flowInIndex[id][upIndex];
            //cout << id << "\t" << flowInID << "\t" << m_nCells << endl;
            if (m_qi[flowInID][j] > 0.f) {
                qUp += m_qi[flowInID][j];
            }// * m_flowInPercentage[id][upIndex];
            //cout << id << "\t" << flowInID << "\t" << m_nCells << "\t" << m_qi[flowInID][j] << endl;
        }
        // add upstream water to the current cell
        if (qUp < 0.f) qUp = 0.f;
        m_soilStorage[id][j] += qUp; // mm
        //TEST
        if (m_soilStorage[id][j] != m_soilStorage[id][j] || m_soilStorage[id][j] < 0.f) {
            ostringstream oss;
            oss << "cell id: " << id << "\t layer: " << j << "\nmoisture is less than zero: " << m_soilStorage[id][j]
                << "\t"
                << smOld << "\nqUp: " << qUp << "\t depth:" << m_soilThick[id][j] << endl;
            cout << oss.str() << endl;
            throw ModelException(MID_SSR_DA, "Execute:FlowInSoil", oss.str());
        }

        // if soil moisture is below the field capacity, no interflow will be generated, otherwise:
        if (m_soilStorage[id][j] > m_fcmm[id][j]) {
            // for the upper two layers, soil may be frozen
            // also check if there are upstream inflow
            if (j == 0 && m_soilT[id] <= m_frozenT && qUp <= 0.f) {
                continue;
            }

            float k = 0.f, maxSoilWater = 0.f, soilWater = 0.f, fcSoilWater = 0.f;
            soilWater = m_soilStorage[id][j];
            maxSoilWater = m_satmm[id][j];
            fcSoilWater = m_fcmm[id][j];
            //the moisture content can exceed the porosity in the way the algorithm is implemented
            if (m_soilStorage[id][j] > maxSoilWater) {
                k = m_ks[id][j];
            } else {
                /// Using Clapp and Hornberger (1978) equation to calculate unsaturated hydraulic conductivity.
                float dcIndex = 2.f * m_poreIndex[id][j] + 3.f; // pore disconnectedness index
                k = m_ks[id][j] * pow(m_soilStorage[id][j] / maxSoilWater, dcIndex);
                if (k <= UTIL_ZERO) k = 0.f;
                //cout << id << "\t" << j << "\t" << k << endl;
            }
            m_qi[id][j] = m_ki * s0 * k * m_dt / 3600.f * m_soilThick[id][j] / 1000.f / flowWidth; // the unit is mm

            if (soilWater - m_qi[id][j] > maxSoilWater) {
                m_qi[id][j] = soilWater - maxSoilWater;
            } else if (soilWater - m_qi[id][j] < fcSoilWater) {
                m_qi[id][j] = soilWater - fcSoilWater;
            }
            m_qi[id][j] = max(0.f, m_qi[id][j]);

            m_qiVol[id][j] = m_qi[id][j] / 1000.f * m_CellWidth * flowWidth; //m3
            m_qiVol[id][j] = max(UTIL_ZERO, m_qiVol[id][j]);
            //Adjust the moisture content in the current layer, and the layer immediately below it
            m_soilStorage[id][j] -= m_qi[id][j];
            m_soilStorageProfile[id] += m_soilStorage[id][j];
            if (m_soilStorage[id][j] != m_soilStorage[id][j] || m_soilStorage[id][j] < 0.f) {
                ostringstream oss;
                oss << id << "\t" << j << "\nmoisture is less than zero: " << m_soilStorage[id][j] << "\t"
                    << m_soilThick[id][j];
                throw ModelException(MID_SSR_DA, "Execute", oss.str());
            }
        }
    }
}

//Execute module
int SSR_DA::Execute() {
    CheckInputData();
    initialOutputs();

    for (int iLayer = 0; iLayer < m_nRoutingLayers; ++iLayer) {
        // There are not any flow relationship within each routing layer.
        // So parallelization can be done here.
        int nCells = (int) m_routingLayers[iLayer][0];

#pragma omp parallel for
        for (int iCell = 1; iCell <= nCells; ++iCell) {
            int id = (int) m_routingLayers[iLayer][iCell];
            FlowInSoil(id);
        }
    }

    //cout << "end flowinsoil" << endl;
    for (int i = 0; i <= m_nSubbasin; i++) {
        m_qiSubbasin[i] = 0.f;
    }
    /// using openmp for reduction an array should be pay much more attention.
    /// here is an solution. https://stackoverflow.com/questions/20413995/reducing-on-array-in-openmp
    /// #pragma omp parallel for reduction(+:myArray[:6]) is supported with OpenMP 4.5.
    /// However, MSVS are using OpenMP 2.0.
    /// Added by lj, 2017-8-23
#pragma omp parallel
    {
        float *tmp_qiSubbsn = new float[m_nSubbasin + 1];
        for (int i = 0; i <= m_nSubbasin; i++) {
            tmp_qiSubbsn[i] = 0.f;
        }
#pragma omp for
        for (int i = 0; i < m_nCells; i++) {
            if (m_streamLink[i] > 0) {
                float qiAllLayers = 0.f;
                for (int j = 0; j < (int)m_soilLayers[i]; j++) {
                    if (m_qiVol[i][j] > UTIL_ZERO) {
                        qiAllLayers += m_qiVol[i][j] / m_dt;
                    } /// m^3/s
                }
                //cout << m_nSubbasin << "\tsubbasin:" << tmp_qiSubbsn[i] << "\t" << qiAllLayers << endl;
                tmp_qiSubbsn[int(m_subbasin[i])] += qiAllLayers;
            }
        }
#pragma omp critical
        {
            for (int i = 1; i <= m_nSubbasin; i++) {
                m_qiSubbasin[i] += tmp_qiSubbsn[i];
            }
        }
    }
// Old code stye of OpenMP for reduction on an array.
//#pragma omp parallel for
//    for (int i = 0; i < m_nCells; i++) {
//        float qiAllLayers = 0.f;
//        if (m_streamLink[i] > 0) {
//            qiAllLayers = 0.f;
//            for (int j = 0; j < (int) m_soilLayers[i]; j++) {
//                if (m_qiVol[i][j] > UTIL_ZERO) {
//                    qiAllLayers += m_qiVol[i][j] / m_dt;
//                } /// m^3/s
//            }
//            //cout << m_nSubbasin << "\tsubbasin:" << m_subbasin[i] << "\t" << qiAllLayers << endl;
//            m_qiSubbasin[int(m_subbasin[i])] += qiAllLayers;
//        }
//    }

    for (int i = 1; i <= m_nSubbasin; i++) {
        //cout<<", "<<i<<": "<<m_qiSubbasin[i];
        m_qiSubbasin[0] += m_qiSubbasin[i];
    }
    //cout<<endl;
    return 0;
}

void SSR_DA::SetValue(const char *key, float data) {
    string s(key);
    if (StringMatch(s, VAR_OMP_THREADNUM)) {
        SetOpenMPThread((int) data);
    } else if (StringMatch(s, VAR_T_SOIL)) {
        m_frozenT = data;
    } else if (StringMatch(s, VAR_KI)) {
        m_ki = data;
    } else if (StringMatch(s, Tag_CellWidth)) {
        m_CellWidth = data;
    } else if (StringMatch(s, Tag_TimeStep)) {
        m_dt = int(data);
    } else {
        throw ModelException(MID_SSR_DA, "SetValue", "Parameter " + s + " does not exist.");
    }
}

void SSR_DA::Set1DData(const char *key, int nRows, float *data) {
    string s(key);
    CheckInputSize(key, nRows);
    if (StringMatch(s, VAR_SLOPE)) {
        m_slope = data;
    } else if (StringMatch(s, VAR_CHWIDTH)) {
        m_chWidth = data;
    } else if (StringMatch(s, VAR_STREAM_LINK)) {
        m_streamLink = data;
    } else if (StringMatch(s, VAR_SOTE)) {
        m_soilT = data;
    } else if (StringMatch(s, VAR_SUBBSN)) {
        m_subbasin = data;
    } else if (StringMatch(s, VAR_SOILLAYERS)) {
        m_soilLayers = data;
    } else if (StringMatch(s, VAR_SOL_SW)) {
        m_soilStorageProfile = data;
    } else {
        throw ModelException(MID_SSR_DA, "SetValue", "Parameter " + s + " does not exist.");
    }
}

void SSR_DA::Set2DData(const char *key, int nrows, int ncols, float **data) {
    string sk(key);
    if (StringMatch(sk, VAR_SOILTHICK)) {
        CheckInputSize(key, nrows);
        m_nSoilLayers = ncols;
        m_soilThick = data;
    } else if (StringMatch(sk, VAR_CONDUCT)) {
        CheckInputSize(key, nrows);
        m_nSoilLayers = ncols;
        m_ks = data;
    } else if (StringMatch(sk, VAR_SOL_UL)) {
        CheckInputSize(key, nrows);
        m_nSoilLayers = ncols;
        m_satmm = data;
    } else if (StringMatch(sk, VAR_SOL_AWC)) {
        CheckInputSize(key, nrows);
        m_nSoilLayers = ncols;
        m_fcmm = data;
    } else if (StringMatch(sk, VAR_SOL_WPMM)) {
        CheckInputSize(key, nrows);
        m_nSoilLayers = ncols;
        m_wpmm = data;
    } else if (StringMatch(sk, VAR_POREIDX)) {
        CheckInputSize(key, nrows);
        m_nSoilLayers = ncols;
        m_poreIndex = data;
    } else if (StringMatch(sk, VAR_SOL_ST)) {
        CheckInputSize(key, nrows);
        m_nSoilLayers = ncols;
        m_soilStorage = data;
    } else if (StringMatch(sk, Tag_ROUTING_LAYERS)) {
        m_nRoutingLayers = nrows;
        m_routingLayers = data;
    } else if (StringMatch(sk, Tag_FLOWIN_INDEX_D8)) {
        CheckInputSize(key, nrows);
        m_flowInIndex = data;
    } else {
        throw ModelException(MID_SSR_DA, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void SSR_DA::SetSubbasins(clsSubbasins *subbasins) {
    if (subbasins != NULL) {
        if (m_nSubbasin < 0) {
            m_nSubbasin = subbasins->GetSubbasinNumber();
        }
    } else {
        throw ModelException(MID_SSR_DA, "SetSubbasins", "Subbasins data does not exist.");
    }
}

void SSR_DA::Get1DData(const char *key, int *n, float **data) {
    initialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SBIF)) {
        *data = m_qiSubbasin;
    } else {
        throw ModelException(MID_SSR_DA, "Get1DData", "Result " + sk + " does not exist.");
    }
    *n = m_nSubbasin + 1;
}

void SSR_DA::Get2DData(const char *key, int *nRows, int *nCols, float ***data) {
    initialOutputs();
    string sk(key);
    *nRows = m_nCells;
    *nCols = m_nSoilLayers;

    if (StringMatch(sk, VAR_SSRU)) {
        *data = m_qi;
    } else if (StringMatch(sk, VAR_SSRUVOL)) {
        *data = m_qiVol;
    } else {
        throw ModelException(MID_SSR_DA, "Get2DData", "Output " + sk + " does not exist.");
    }
}

bool SSR_DA::CheckInputData() {
    if (m_nCells <= 0) {
        throw ModelException(MID_SSR_DA, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    }
    if (m_ki <= 0) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "You have not set Ki.");
    }
    if (FloatEqual(m_frozenT, NODATA_VALUE)) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "You have not set frozen T.");
    }
    if (m_dt <= 0) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "You have not set time step.");
    }
    if (m_CellWidth <= 0) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "You have not set cell width.");
    }
    if (m_nSubbasin < 0) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "The number of subbasins can not be less than 0.");
    }
    if (m_subbasin == NULL) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "The parameter: subbasin can not be NULL.");
    }
    if (m_soilLayers == NULL) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "The soil layers number can not be NULL.");
    }
    if (m_soilThick == NULL) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "The soil thickness can not be NULL.");
    }
    if (m_slope == NULL) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "The slope can not be NULL.");
    }
    if (m_ks == NULL) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "The conductivity can not be NULL.");
    }
    if (m_satmm == NULL) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "The porosity can not be NULL.");
    }
    if (m_poreIndex == NULL) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "The pore index can not be NULL.");
    }
    if (m_fcmm == NULL) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "The field capacity can not be NULL.");
    }
    if (m_wpmm == NULL) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "The wilting point can not be NULL.");
    }
    if (m_soilStorage == NULL) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "The soil storage can not be NULL.");
    }
    if (m_soilStorageProfile == NULL) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "The soil storage on profile can not be NULL.");
    }
    if (m_soilT == NULL) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "The soil temperature can not be NULL.");
    }
    if (m_chWidth == NULL) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "The channel width can not be NULL.");
    }
    if (m_streamLink == NULL) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "The stream link can not be NULL.");
    }
    if (m_flowInIndex == NULL) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "The flow in index can not be NULL.");
    }
    if (m_routingLayers == NULL) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "The routing layers can not be NULL.");
    }
    if (m_nRoutingLayers <= 0) {
        throw ModelException(MID_SSR_DA, "CheckInputData", "The number of routing layers can not be less than 0.");
    }

    return true;
}

void SSR_DA::initialOutputs() {
    if (m_qiSubbasin == NULL) Initialize1DArray(m_nSubbasin + 1, m_qiSubbasin, 0.f);
    if (m_qi == NULL) {
        Initialize2DArray(m_nCells, m_nSoilLayers, m_qi, 0.f);
        Initialize2DArray(m_nCells, m_nSoilLayers, m_qiVol, 0.f);
    }
}

bool SSR_DA::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_SSR_DA, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    }
    if (this->m_nCells != n) {
        if (this->m_nCells <= 0) { this->m_nCells = n; }
        else {
            throw ModelException(MID_SSR_DA, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
        }
    }
    return true;
}
