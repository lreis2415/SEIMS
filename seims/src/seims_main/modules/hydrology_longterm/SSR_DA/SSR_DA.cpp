#include "SSR_DA.h"

#include "text.h"

SSR_DA::SSR_DA() :
    m_inputSubbsnID(-1), m_nCells(-1), m_CellWth(-1.f), m_maxSoilLyrs(-1), m_nSoilLyrs(nullptr),
    m_soilThk(nullptr),
    m_dt(-1), m_ki(NODATA_VALUE),
    m_soilFrozenTemp(NODATA_VALUE), m_slope(nullptr), m_ks(nullptr), m_soilSat(nullptr),
    m_poreIdx(nullptr),
    m_soilFC(nullptr), m_soilWP(nullptr),
    m_soilWtrSto(nullptr), m_soilWtrStoPrfl(nullptr), m_soilTemp(nullptr), m_chWidth(nullptr),
    m_rchID(nullptr), m_flowInIdx(nullptr), m_flowInFrac(nullptr), m_rteLyrs(nullptr),
    m_nRteLyrs(-1), m_nSubbsns(-1), m_subbsnID(nullptr),
    /// outputs
    m_subSurfRf(nullptr), m_subSurfRfVol(nullptr), m_ifluQ2Rch(nullptr) {
}

SSR_DA::~SSR_DA() {
    if (m_subSurfRf != nullptr) Release2DArray(m_nCells, m_subSurfRf);
    if (m_subSurfRfVol != nullptr) Release2DArray(m_nCells, m_subSurfRfVol);
    if (m_ifluQ2Rch != nullptr) Release1DArray(m_ifluQ2Rch);
}

float SSR_DA::GetFlowInFraction(const int id, const int up_idx) {
    if (nullptr == m_flowInFrac) return 1.f;
    return m_flowInFrac[id][up_idx];
}

bool SSR_DA::FlowInSoil(const int id) {
    float s0 = Max(m_slope[id], 0.01f);
    float flowWidth = m_CellWth;
    // there is no land in this cell
    if (m_rchID[id] > 0) {
        flowWidth -= m_chWidth[id];
    }
    // initialization for current cell of current timestep
    for (int j = 0; j < CVT_INT(m_nSoilLyrs[id]); j++) {
        m_subSurfRf[id][j] = 0.f;
        m_subSurfRfVol[id][j] = 0.f;
    }
    /* Previous code */
    // return with initial values if flowWidth is less than 0
    // if (flowWidth <= 0) return true;

    /* Update: In my view, if the flowWidth is less than 0, the subsurface flow
    * from the upstream cells should be added to stream cell directly, which will be summarized
    * for channel flow routing. By lj, 2018-4-12 */

    // number of flow-in cells
    int nUpstream = CVT_INT(m_flowInIdx[id][0]);
    m_soilWtrStoPrfl[id] = 0.f; // update soil storage on profile
    for (int j = 0; j < CVT_INT(m_nSoilLyrs[id]); j++) {
        float smOld = m_soilWtrSto[id][j]; // Just for potential error message print
        // Sum subsurface flow in from upstream cells
        float qUp = 0.f;    // mm
        float qUpVol = 0.f; // m^3
        for (int upIndex = 1; upIndex <= nUpstream; upIndex++) {
            int flowInID = CVT_INT(m_flowInIdx[id][upIndex]);
            // IMPORTANT!!! If the upstream cell is from another subbasin, CONTINUE to next upstream cell. By lj.
            if (CVT_INT(m_subbsnID[flowInID]) != CVT_INT(m_subbsnID[id])) { continue; }
            // If no in cells flowin (i.e., nUpstream = 0), the for-loop will be ignored.
            if (m_subSurfRf[flowInID][j] < 0.f) { continue; }
            qUp += m_subSurfRf[flowInID][j] * GetFlowInFraction(id, upIndex);
            qUpVol += m_subSurfRfVol[flowInID][j] * GetFlowInFraction(id, upIndex);
        }
        if (qUp <= 0.f || qUpVol <= 0.f) {
            qUp = 0.f;
            qUpVol = 0.f;
        }
        // if the flowWidth is less than 0, the subsurface flow from the upstream cells
        // should be added to stream cell directly, which will be summarized
        // for channel flow routing. By lj, 2018-4-12
        if (flowWidth <= 0.f) {
            m_subSurfRf[id][j] = qUp;
            m_subSurfRfVol[id][j] = qUpVol;
            continue;
        }
        if (m_soilWtrSto[id][j] != m_soilWtrSto[id][j] || m_soilWtrSto[id][j] < 0.f) {
            cout << "cell id: " << id << ", layer: " << j << ", moisture is less than zero: "
                    << m_soilWtrSto[id][j] << ", previous: " << smOld << ", qUp: " << qUp << ", depth:"
                    << m_soilThk[id][j] << endl;
            return false;
        }

        // Add upstream's flowin to the water storage of current soil layer
        m_soilWtrSto[id][j] += qUp; // mm

        // if soil moisture is below the field capacity, no interflow will be generated
        if (m_soilWtrSto[id][j] <= m_soilFC[id][j]) continue;
        // Otherwise, calculate interflow:
        // for the upper two layers, soil may be frozen
        // also check if there are upstream inflow
        if (j == 0 && m_soilTemp[id] <= m_soilFrozenTemp && qUp <= 0.f) {
            continue;
        }

        float k = 0.f, maxSoilWater = 0.f, soilWater = 0.f, fcSoilWater = 0.f;
        soilWater = m_soilWtrSto[id][j];
        maxSoilWater = m_soilSat[id][j];
        fcSoilWater = m_soilFC[id][j];
        //the moisture content can exceed the porosity in the way the algorithm is implemented
        if (m_soilWtrSto[id][j] > maxSoilWater) {
            k = m_ks[id][j];
        } else {
            /// Using Clapp and Hornberger (1978) equation to calculate unsaturated hydraulic conductivity.
            float dcIndex = 2.f * m_poreIdx[id][j] + 3.f; // pore disconnectedness index
            k = m_ks[id][j] * pow(m_soilWtrSto[id][j] / maxSoilWater, dcIndex);
            if (k <= UTIL_ZERO) k = 0.f;
            //cout << id << "\t" << j << "\t" << k << endl;
        }
        // 1. / 3600. = 0.0002777777777777778
        m_subSurfRf[id][j] = m_ki * s0 * k * m_dt * 0.0002777777777777778f * m_soilThk[id][j] * 0.001f / flowWidth;
        // the unit is mm

        if (soilWater - m_subSurfRf[id][j] > maxSoilWater) {
            m_subSurfRf[id][j] = soilWater - maxSoilWater;
        } else if (soilWater - m_subSurfRf[id][j] < fcSoilWater) {
            m_subSurfRf[id][j] = soilWater - fcSoilWater;
        }
        m_subSurfRf[id][j] = Max(0.f, m_subSurfRf[id][j]);

        m_subSurfRfVol[id][j] = m_subSurfRf[id][j] * 0.001f * m_CellWth * flowWidth; // m^3
        m_subSurfRfVol[id][j] = Max(UTIL_ZERO, m_subSurfRfVol[id][j]);
        //Adjust the moisture content in the current layer, and the layer immediately below it
        m_soilWtrSto[id][j] -= m_subSurfRf[id][j];
        m_soilWtrSto[id][j] = Max(UTIL_ZERO, m_soilWtrSto[id][j]);
        m_soilWtrStoPrfl[id] += m_soilWtrSto[id][j];
        if (m_soilWtrSto[id][j] != m_soilWtrSto[id][j] || m_soilWtrSto[id][j] < 0.f) {
            cout << "cell id: " << id << ", layer: " << j << ", moisture is less than zero: "
                    << m_soilWtrSto[id][j] << ", subsurface runoff: " << m_subSurfRf[id][j] << ", depth:"
                    << m_soilThk[id][j] << endl;
            return false;
        }
    }
    return true;
}

int SSR_DA::Execute() {
    CheckInputData();
    InitialOutputs();

    for (int ilyr = 0; ilyr < m_nRteLyrs; ilyr++) {
        // There are not any flow relationship within each routing layer.
        // So parallelization can be done here.
        int ncells = CVT_INT(m_rteLyrs[ilyr][0]);
        // DO NOT THROW EXCEPTION IN OMP FOR LOOP, i.e., FlowInSoil(id) function.
        int errCount = 0;
#pragma omp parallel for reduction(+: errCount)
        for (int icell = 1; icell <= ncells; icell++) {
            int id = CVT_INT(m_rteLyrs[ilyr][icell]);
            if (!FlowInSoil(id)) errCount++;
        }
        if (errCount > 0) {
            throw ModelException(M_SSR_DA[0], "Execute:FlowInSoil",
                                 "Please check the error message for more information");
        }
    }

    //cout << "cell id: " << 18252 << ", layer: " << 0 << ", moisture: "
    //    << m_soilWtrSto[18252][0] << ", subsurface runoff: " << m_subSurfRf[18252][0] << ", depth:"
    //    << m_soilThk[18252][0] << endl;

    for (int i = 0; i <= m_nSubbsns; i++) {
        m_ifluQ2Rch[i] = 0.f;
    }
    /// using openmp for reduction an array should be paid much more attention.
    /// here is a solution. https://stackoverflow.com/questions/20413995/reducing-on-array-in-openmp
    /// #pragma omp parallel for reduction(+:myArray[:6]) is supported with OpenMP 4.5.
    /// However, MSVC 2010-2015 are using OpenMP 2.0.
    /// Added by lj, 2017-8-23
#pragma omp parallel
    {
        float* tmp_qiSubbsn = new(nothrow) float[m_nSubbsns + 1];
        for (int i = 0; i <= m_nSubbsns; i++) {
            tmp_qiSubbsn[i] = 0.f;
        }
#pragma omp for
        for (int i = 0; i < m_nCells; i++) {
            if (m_rchID[i] <= 0.f) continue;
            float qiAllLayers = 0.f;
            for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) {
                if (m_subSurfRfVol[i][j] > UTIL_ZERO) {
                    qiAllLayers += m_subSurfRfVol[i][j] / m_dt; /// m^3/s
                }
            }
            tmp_qiSubbsn[CVT_INT(m_rchID[i])] += qiAllLayers;
        }
#pragma omp critical
        {
            for (int i = 1; i <= m_nSubbsns; i++) {
                m_ifluQ2Rch[i] += tmp_qiSubbsn[i];
            }
        }
        delete[] tmp_qiSubbsn;
        tmp_qiSubbsn = nullptr;
    } /* END of #pragma omp parallel */

    for (int i = 1; i <= m_nSubbsns; i++) {
        m_ifluQ2Rch[0] += m_ifluQ2Rch[i];
    }
    return 0;
}

void SSR_DA::SetValue(const char* key, const float value) {
    string s(key);
    if (StringMatch(s, VAR_T_SOIL[0])) {
        m_soilFrozenTemp = value;
    } else if (StringMatch(s, VAR_KI[0])) {
        m_ki = value;
    } else if (StringMatch(s, VAR_SUBBSNID_NUM[0])) {
        m_nSubbsns = CVT_INT(value);
    } else if (StringMatch(s, Tag_SubbasinId)) {
        m_inputSubbsnID = CVT_INT(value);
    } else if (StringMatch(s, Tag_CellWidth[0])) {
        m_CellWth = value;
    } else if (StringMatch(s, Tag_TimeStep[0])) {
        m_dt = CVT_INT(value);
    } else {
        throw ModelException(M_SSR_DA[0], "SetValue", 
                             "Parameter " + s + " does not exist.");
    }
}

void SSR_DA::Set1DData(const char* key, const int nrows, float* data) {
    string s(key);
    CheckInputSize(M_SSR_DA[0], key, nrows, m_nCells);
    if (StringMatch(s, VAR_SLOPE[0])) {
        m_slope = data;
    } else if (StringMatch(s, VAR_CHWIDTH[0])) {
        m_chWidth = data;
    } else if (StringMatch(s, VAR_STREAM_LINK[0])) {
        m_rchID = data;
    } else if (StringMatch(s, VAR_SOTE[0])) {
        m_soilTemp = data;
    } else if (StringMatch(s, VAR_SUBBSN[0])) {
        m_subbsnID = data;
    } else if (StringMatch(s, VAR_SOILLAYERS[0])) {
        m_nSoilLyrs = data;
    } else if (StringMatch(s, VAR_SOL_SW[0])) {
        m_soilWtrStoPrfl = data;
    } else {
        throw ModelException(M_SSR_DA[0], "Set1DData", "Parameter " + s + " does not exist.");
    }
}

void SSR_DA::Set2DData(const char* key, const int nrows, const int ncols, float** data) {
    string sk(key);
    if (StringMatch(sk, VAR_SOILTHICK[0])) {
        CheckInputSize2D(M_SSR_DA[0], key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_soilThk = data;
    } else if (StringMatch(sk, VAR_CONDUCT[0])) {
        CheckInputSize2D(M_SSR_DA[0], key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_ks = data;
    } else if (StringMatch(sk, VAR_SOL_UL[0])) {
        CheckInputSize2D(M_SSR_DA[0], key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_soilSat = data;
    } else if (StringMatch(sk, VAR_SOL_AWC[0])) {
        CheckInputSize2D(M_SSR_DA[0], key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_soilFC = data;
    } else if (StringMatch(sk, VAR_SOL_WPMM[0])) {
        CheckInputSize2D(M_SSR_DA[0], key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_soilWP = data;
    } else if (StringMatch(sk, VAR_POREIDX[0])) {
        CheckInputSize2D(M_SSR_DA[0], key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_poreIdx = data;
    } else if (StringMatch(sk, VAR_SOL_ST[0])) {
        CheckInputSize2D(M_SSR_DA[0], key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_soilWtrSto = data;
    } else if (StringMatch(sk, Tag_ROUTING_LAYERS[0])) {
        CheckInputSize(M_SSR_DA[0], key, nrows, m_nRteLyrs);
        m_rteLyrs = data;
    } else if (StringMatch(sk, Tag_FLOWIN_INDEX[0])) {
        CheckInputSize(M_SSR_DA[0], key, nrows, m_nCells);
        m_flowInIdx = data;
    } else if (StringMatch(sk, Tag_FLOWIN_FRACTION[0])) {
        CheckInputSize(M_SSR_DA[0], key, nrows, m_nCells);
        m_flowInFrac = data;
    } else {
        throw ModelException(M_SSR_DA[0], "Set2DData", 
                             "Parameter " + sk + " does not exist.");
    }
}

void SSR_DA::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SBIF[0])) *data = m_ifluQ2Rch;
    else {
        throw ModelException(M_SSR_DA[0], "Get1DData", 
                             "Result " + sk + " does not exist.");
    }
    *n = m_nSubbsns + 1;
}

void SSR_DA::Get2DData(const char* key, int* nrows, int* ncols, float*** data) {
    InitialOutputs();
    string sk(key);
    *nrows = m_nCells;
    *ncols = m_maxSoilLyrs;

    if (StringMatch(sk, VAR_SSRU[0])) {
        *data = m_subSurfRf;
    } else if (StringMatch(sk, VAR_SSRUVOL[0])) {
        *data = m_subSurfRfVol;
    } else {
        throw ModelException(M_SSR_DA[0], "Get2DData", 
                             "Output " + sk + " does not exist.");
    }
}

bool SSR_DA::CheckInputData() {
    CHECK_NONNEGATIVE(M_SSR_DA[0], m_inputSubbsnID);
    CHECK_POSITIVE(M_SSR_DA[0], m_nCells);
    CHECK_POSITIVE(M_SSR_DA[0], m_ki);
    CHECK_NODATA(M_SSR_DA[0], m_soilFrozenTemp);
    CHECK_POSITIVE(M_SSR_DA[0], m_dt);
    CHECK_POSITIVE(M_SSR_DA[0], m_CellWth);
    CHECK_POSITIVE(M_SSR_DA[0], m_nSubbsns);
    CHECK_POSITIVE(M_SSR_DA[0], m_nRteLyrs);
    CHECK_POINTER(M_SSR_DA[0], m_subbsnID);
    CHECK_POINTER(M_SSR_DA[0], m_nSoilLyrs);
    CHECK_POINTER(M_SSR_DA[0], m_soilThk);
    CHECK_POINTER(M_SSR_DA[0], m_slope);
    CHECK_POINTER(M_SSR_DA[0], m_poreIdx);
    CHECK_POINTER(M_SSR_DA[0], m_ks);
    CHECK_POINTER(M_SSR_DA[0], m_soilSat);
    CHECK_POINTER(M_SSR_DA[0], m_soilFC);
    CHECK_POINTER(M_SSR_DA[0], m_soilWP);
    CHECK_POINTER(M_SSR_DA[0], m_soilWtrSto);
    CHECK_POINTER(M_SSR_DA[0], m_soilWtrStoPrfl);
    CHECK_POINTER(M_SSR_DA[0], m_soilTemp);
    CHECK_POINTER(M_SSR_DA[0], m_chWidth);
    CHECK_POINTER(M_SSR_DA[0], m_rchID);
    CHECK_POINTER(M_SSR_DA[0], m_flowInIdx);
    // m_flowInFrac should not be checked since it is optional for single flow direction alg.
    CHECK_POINTER(M_SSR_DA[0], m_rteLyrs);
    /** TEST CODE START **/
    /*
    for (int ilyr = 0; ilyr < m_nRteLyrs; ilyr++) {
        int ncells = CVT_INT(m_rteLyrs[ilyr][0]);
        cout << ilyr << ":" << ncells << "{";
        for (int icell = 1; icell <= ncells; icell++) {
            int id = CVT_INT(m_rteLyrs[ilyr][icell]);
            int nUpstream = CVT_INT(m_flowInIdx[id][0]);
            cout << id << ": [";
            for (int upIndex = 1; upIndex <= nUpstream; upIndex++) {
                int flowInID = CVT_INT(m_flowInIdx[id][upIndex]);
                float flowInFrac = GetFlowInFraction(id, upIndex);
                cout << "(" << flowInID << ": " << flowInFrac << "), ";
            }
            cout << "], ";
        }
        cout << "}" << endl;
    }
    */
    /** TEST CODE END   **/
    return true;
}

void SSR_DA::InitialOutputs() {
    CHECK_POSITIVE(M_SSR_DA[0], m_nCells);
    CHECK_POSITIVE(M_SSR_DA[0], m_nSubbsns);
    if (nullptr == m_ifluQ2Rch) Initialize1DArray(m_nSubbsns + 1, m_ifluQ2Rch, 0.f);
    if (nullptr == m_subSurfRf) Initialize2DArray(m_nCells, m_maxSoilLyrs, m_subSurfRf, 0.f);
    if (nullptr == m_subSurfRfVol) Initialize2DArray(m_nCells, m_maxSoilLyrs, m_subSurfRfVol, 0.f);
}
