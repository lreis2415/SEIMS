#include "SSR_DA.h"

#include "text.h"

SSR_DA::SSR_DA() : m_nSoilLayers(-1), m_dt(-1), m_nCells(-1), m_CellWidth(-1.f), m_nSubbasin(-1), m_subbasinID(-1),
                   m_frozenT(NODATA_VALUE), m_ki(NODATA_VALUE),
                   m_soilLayers(nullptr), m_soilThick(nullptr), m_ks(nullptr), m_satmm(nullptr), m_poreIndex(nullptr),
                   m_fcmm(nullptr), m_wpmm(nullptr),
                   m_slope(nullptr), m_chWidth(nullptr), m_streamLink(NULL), m_subbasin(nullptr),
                   m_flowInIndex(nullptr), m_flowInPercentage(nullptr), m_routingLayers(nullptr), m_nRoutingLayers(-1),
    /// input from other modules
                   m_soilStorage(nullptr), m_soilStorageProfile(nullptr), m_soilT(nullptr),
    /// outputs
                   m_qi(nullptr), m_qiVol(nullptr), m_qiSubbasin(nullptr) {
}

SSR_DA::~SSR_DA() {
    if (m_qi != nullptr) Release2DArray(m_nCells, m_qi);
    if (m_qiVol != nullptr) Release2DArray(m_nCells, m_qiVol);
    if (m_qiSubbasin != nullptr) Release1DArray(m_qiSubbasin);
}

bool SSR_DA::FlowInSoil(int id) {
    float s0 = max(m_slope[id], 0.01f);
    float flowWidth = m_CellWidth;
    // there is no land in this cell
    if (m_streamLink[id] > 0) {
        flowWidth -= m_chWidth[id];
    }
    // initialize for current cell of current timestep
    for (int j = 0; j < int(m_soilLayers[id]); j++) {
        m_qi[id][j] = 0.f;
        m_qiVol[id][j] = 0.f;
    }
    /* Previous code. Update: In my view, if the flowWidth is less than 0, the subsurface flow
     * from the upstream cells should be added to stream cell directly, which will be summarized
     * for channel flow routing. By lj, 2018-4-12
    // return with initial values if flowWidth is less than 0
    if (flowWidth <= 0) return true;
    */
    // number of flow-in cells
    int nUpstream = int(m_flowInIndex[id][0]);
    m_soilStorageProfile[id] = 0.f; // update soil storage on profile
    for (int j = 0; j < int(m_soilLayers[id]); j++) {
        float smOld = m_soilStorage[id][j];
        //sum the upstream subsurface flow
        float qUp = 0.f; // mm
        float qUpVol = 0.f; // m^3
        // If no in cells flowin (i.e., nUpstream = 0), the for-loop will be ignored.
        for (int upIndex = 1; upIndex <= nUpstream; upIndex++) {
            int flowInID = int(m_flowInIndex[id][upIndex]);
            // IMPORTANT!!! If the upstream cell is from another subbasin, CONTINUE to next upstream cell. By lj.
            if (int(m_subbasin[flowInID]) != int(m_subbasin[id])) { continue; }
            if (m_qi[flowInID][j] > 0.f) {
                qUp += m_qi[flowInID][j]; // * m_flowInPercentage[id][upIndex]; // TODO: Consider MFD algorithms
                qUpVol += m_qiVol[flowInID][j];
            }
        }
        // add upstream water to the current cell
        if (qUp < 0.f) qUp = 0.f;
        if (qUpVol < 0.f) qUpVol = 0.f;
        // if the flowWidth is less than 0, the subsurface flow from the upstream cells
        // should be added to stream cell directly, which will be summarized
        // for channel flow routing. By lj, 2018-4-12
        if (flowWidth <= 0.f) {
            m_qi[id][j] = qUp;
            m_qiVol[id][j] = qUpVol;
            continue;
        }

        m_soilStorage[id][j] += qUp; // mm
        //TEST
        if (m_soilStorage[id][j] != m_soilStorage[id][j] || m_soilStorage[id][j] < 0.f) {
            cout << "cell id: " << id << ", layer: " << j << ", moisture is less than zero: "
                 << m_soilStorage[id][j] << ", previous: " << smOld << ", qUp: " << qUp << ", depth:"
                 << m_soilThick[id][j] << endl;
            return false;
        }

        // if soil moisture is below the field capacity, no interflow will be generated
        if (m_soilStorage[id][j] <= m_fcmm[id][j]) continue;
        // Otherwise, calculate interflow:
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
            cout << "cell id: " << id << ", layer: " << j << ", moisture is less than zero: "
                << m_soilStorage[id][j] << ", subsurface runoff: " << m_qi[id][j] << ", depth:"
                << m_soilThick[id][j] << endl;
            return false;
        }
    }
    return true;
}

int SSR_DA::Execute() {
    CheckInputData();
     InitialOutputs();

    for (int iLayer = 0; iLayer < m_nRoutingLayers; iLayer++) {
        // There are not any flow relationship within each routing layer.
        // So parallelization can be done here.
        int nCells = int(m_routingLayers[iLayer][0]);
        // DO NOT THROW EXCEPTION IN OMP FOR LOOP, i.e., FlowInSoil(id) function.
        int errCount = 0;
#pragma omp parallel for reduction(+: errCount)
        for (int iCell = 1; iCell <= nCells; iCell++) {
            int id = int(m_routingLayers[iLayer][iCell]);
            if (!FlowInSoil(id)) errCount++;
        }
        if (errCount > 0) {
            throw ModelException(MID_SSR_DA, "Execute:FlowInSoil",
                                 "Please check the error message for more information");
        }
    }
    for (int i = 0; i <= m_nSubbasin; i++) {
        m_qiSubbasin[i] = 0.f;
    }
    /// using openmp for reduction an array should be paid much more attention.
    /// here is a solution. https://stackoverflow.com/questions/20413995/reducing-on-array-in-openmp
    /// #pragma omp parallel for reduction(+:myArray[:6]) is supported with OpenMP 4.5.
    /// However, MSVC 2010-2015 are using OpenMP 2.0.
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
                for (int j = 0; j < int(m_soilLayers[i]); j++) {
                    if (m_qiVol[i][j] > UTIL_ZERO) {
                        qiAllLayers += m_qiVol[i][j] / m_dt;
                    } /// m^3/s
                }
                tmp_qiSubbsn[int(m_subbasin[i])] += qiAllLayers;
            }
        }
#pragma omp critical
        {
            for (int i = 1; i <= m_nSubbasin; i++) {
                m_qiSubbasin[i] += tmp_qiSubbsn[i];
            }
        }
        delete[] tmp_qiSubbsn;
    } /* END of #pragma omp parallel */

    for (int i = 1; i <= m_nSubbasin; i++) {
        m_qiSubbasin[0] += m_qiSubbasin[i];
    }
    return 0;
}

void SSR_DA::SetValue(const char *key, float data) {
    string s(key);
    if (StringMatch(s, VAR_T_SOIL)) {
        m_frozenT = data;
    } else if (StringMatch(s, VAR_KI)) {
        m_ki = data;
    } else if (StringMatch(s, VAR_SUBBSNID_NUM)) {
        m_nSubbasin = data;
    } else if (StringMatch(s, Tag_SubbasinId)) {
        m_subbasinID = data;
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

void SSR_DA::GetValue(const char *key, float *value) {
     InitialOutputs();
    string sk(key);
    /// For MPI version to transfer data across subbasins
    if (StringMatch(sk, VAR_SBIF) && m_subbasinID > 0) { *value = m_qiSubbasin[m_subbasinID]; }
    else {
        throw ModelException(MID_SSR_DA, "GetValue", "Result " + sk + " does not exist.");
    }
}

void SSR_DA::Get1DData(const char *key, int *n, float **data) {
     InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SBIF)) { *data = m_qiSubbasin; }
    else {
        throw ModelException(MID_SSR_DA, "Get1DData", "Result " + sk + " does not exist.");
    }
    *n = m_nSubbasin + 1;
}

void SSR_DA::Get2DData(const char *key, int *nRows, int *nCols, float ***data) {
     InitialOutputs();
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
    CHECK_NONNEGATIVE(MID_SSR_DA, m_subbasinID);
    CHECK_POSITIVE(MID_SSR_DA, m_nCells);
    CHECK_POSITIVE(MID_SSR_DA, m_ki);
    CHECK_DATA(MID_SSR_DA, (FloatEqual(m_frozenT, NODATA_VALUE)), "You have not set frozen T");
    CHECK_POSITIVE(MID_SSR_DA, m_dt);
    CHECK_POSITIVE(MID_SSR_DA, m_CellWidth);
    CHECK_POSITIVE(MID_SSR_DA, m_nSubbasin);
    CHECK_POSITIVE(MID_SSR_DA, m_nRoutingLayers);
    CHECK_POINTER(MID_SSR_DA, m_subbasin);
    CHECK_POINTER(MID_SSR_DA, m_soilLayers);
    CHECK_POINTER(MID_SSR_DA, m_soilThick);
    CHECK_POINTER(MID_SSR_DA, m_slope);
    CHECK_POINTER(MID_SSR_DA, m_poreIndex);
    CHECK_POINTER(MID_SSR_DA, m_ks);
    CHECK_POINTER(MID_SSR_DA, m_satmm);
    CHECK_POINTER(MID_SSR_DA, m_fcmm);
    CHECK_POINTER(MID_SSR_DA, m_wpmm);
    CHECK_POINTER(MID_SSR_DA, m_soilStorage);
    CHECK_POINTER(MID_SSR_DA, m_soilStorageProfile);
    CHECK_POINTER(MID_SSR_DA, m_soilT);
    CHECK_POINTER(MID_SSR_DA, m_chWidth);
    CHECK_POINTER(MID_SSR_DA, m_streamLink);
    CHECK_POINTER(MID_SSR_DA, m_flowInIndex);
    CHECK_POINTER(MID_SSR_DA, m_routingLayers);

    return true;
}

void SSR_DA:: InitialOutputs() {
    CHECK_POSITIVE(MID_SSR_DA, m_nCells);
    CHECK_POSITIVE(MID_SSR_DA, m_nSubbasin);
    if (nullptr == m_qiSubbasin) Initialize1DArray(m_nSubbasin + 1, m_qiSubbasin, 0.f);
    if (nullptr == m_qi) Initialize2DArray(m_nCells, m_nSoilLayers, m_qi, 0.f);
    if (nullptr == m_qiVol) Initialize2DArray(m_nCells, m_nSoilLayers, m_qiVol, 0.f);
}

bool SSR_DA::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_SSR_DA, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; }
        else {
            throw ModelException(MID_SSR_DA, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
        }
    }
    return true;
}
