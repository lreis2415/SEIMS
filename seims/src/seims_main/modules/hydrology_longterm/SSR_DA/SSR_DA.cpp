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
    m_rchID(nullptr), m_flowInIdxD8(nullptr), m_rteLyrs(nullptr),
    m_nRteLyrs(-1), m_nSubbsns(-1), m_subbsnID(nullptr),
    /// outputs
    m_subSurfRf(nullptr), m_subSurfRfVol(nullptr), m_ifluQ2Rch(nullptr) {
}

SSR_DA::~SSR_DA() {
    if (m_subSurfRf != nullptr) Release2DArray(m_nCells, m_subSurfRf);
    if (m_subSurfRfVol != nullptr) Release2DArray(m_nCells, m_subSurfRfVol);
    if (m_ifluQ2Rch != nullptr) Release1DArray(m_ifluQ2Rch);
}

bool SSR_DA::FlowInSoil(const int id) {
    float s0 = Max(m_slope[id], 0.01f);
    float flowWidth = m_CellWth;
    // there is no land in this cell
    if (m_rchID[id] > 0) {
        flowWidth -= m_chWidth[id];
    }
    // initialize for current cell of current timestep
    for (int j = 0; j < CVT_INT(m_nSoilLyrs[id]); j++) {
        m_subSurfRf[id][j] = 0.f;
        m_subSurfRfVol[id][j] = 0.f;
    }
    /* Previous code. Update: In my view, if the flowWidth is less than 0, the subsurface flow
     * from the upstream cells should be added to stream cell directly, which will be summarized
     * for channel flow routing. By lj, 2018-4-12
    // return with initial values if flowWidth is less than 0
    if (flowWidth <= 0) return true;
    */
    // number of flow-in cells
    int nUpstream = CVT_INT(m_flowInIdxD8[id][0]);
    m_soilWtrStoPrfl[id] = 0.f; // update soil storage on profile
    for (int j = 0; j < CVT_INT(m_nSoilLyrs[id]); j++) {
        float smOld = m_soilWtrSto[id][j];
        //sum the upstream subsurface flow
        float qUp = 0.f;    // mm
        float qUpVol = 0.f; // m^3
        // If no in cells flowin (i.e., nUpstream = 0), the for-loop will be ignored.
        for (int upIndex = 1; upIndex <= nUpstream; upIndex++) {
            int flowInID = CVT_INT(m_flowInIdxD8[id][upIndex]);
            // IMPORTANT!!! If the upstream cell is from another subbasin, CONTINUE to next upstream cell. By lj.
            if (CVT_INT(m_subbsnID[flowInID]) != CVT_INT(m_subbsnID[id])) { continue; }
            if (m_subSurfRf[flowInID][j] > 0.f) {
                qUp += m_subSurfRf[flowInID][j]; // * m_flowInPercentage[id][upIndex]; // TODO: Consider MFD algorithms
                qUpVol += m_subSurfRfVol[flowInID][j];
            }
        }
        // add upstream water to the current cell
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

        m_subSurfRfVol[id][j] = m_subSurfRf[id][j] * 0.001f * m_CellWth * flowWidth; //m3
        m_subSurfRfVol[id][j] = Max(UTIL_ZERO, m_subSurfRfVol[id][j]);
        //Adjust the moisture content in the current layer, and the layer immediately below it
        m_soilWtrSto[id][j] -= m_subSurfRf[id][j];
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
            throw ModelException(MID_SSR_DA, "Execute:FlowInSoil",
                                 "Please check the error message for more information");
        }
    }
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
    if (StringMatch(s, VAR_T_SOIL)) {
        m_soilFrozenTemp = value;
    } else if (StringMatch(s, VAR_KI)) {
        m_ki = value;
    } else if (StringMatch(s, VAR_SUBBSNID_NUM)) {
        m_nSubbsns = CVT_INT(value);
    } else if (StringMatch(s, Tag_SubbasinId)) {
        m_inputSubbsnID = CVT_INT(value);
    } else if (StringMatch(s, Tag_CellWidth)) {
        m_CellWth = value;
    } else if (StringMatch(s, Tag_TimeStep)) {
        m_dt = CVT_INT(value);
    } else {
        throw ModelException(MID_SSR_DA, "SetValue", "Parameter " + s + " does not exist.");
    }
}

void SSR_DA::Set1DData(const char* key, const int nrows, float* data) {
    string s(key);
    CheckInputSize(MID_SSR_DA, key, nrows, m_nCells);
    if (StringMatch(s, VAR_SLOPE)) {
        m_slope = data;
    } else if (StringMatch(s, VAR_CHWIDTH)) {
        m_chWidth = data;
    } else if (StringMatch(s, VAR_STREAM_LINK)) {
        m_rchID = data;
    } else if (StringMatch(s, VAR_SOTE)) {
        m_soilTemp = data;
    } else if (StringMatch(s, VAR_SUBBSN)) {
        m_subbsnID = data;
    } else if (StringMatch(s, VAR_SOILLAYERS)) {
        m_nSoilLyrs = data;
    } else if (StringMatch(s, VAR_SOL_SW)) {
        m_soilWtrStoPrfl = data;
    } else {
        throw ModelException(MID_SSR_DA, "Set1DData", "Parameter " + s + " does not exist.");
    }
}

void SSR_DA::Set2DData(const char* key, const int nrows, const int ncols, float** data) {
    string sk(key);
    if (StringMatch(sk, VAR_SOILTHICK)) {
        CheckInputSize2D(MID_SSR_DA, key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_soilThk = data;
    } else if (StringMatch(sk, VAR_CONDUCT)) {
        CheckInputSize2D(MID_SSR_DA, key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_ks = data;
    } else if (StringMatch(sk, VAR_SOL_UL)) {
        CheckInputSize2D(MID_SSR_DA, key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_soilSat = data;
    } else if (StringMatch(sk, VAR_SOL_AWC)) {
        CheckInputSize2D(MID_SSR_DA, key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_soilFC = data;
    } else if (StringMatch(sk, VAR_SOL_WPMM)) {
        CheckInputSize2D(MID_SSR_DA, key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_soilWP = data;
    } else if (StringMatch(sk, VAR_POREIDX)) {
        CheckInputSize2D(MID_SSR_DA, key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_poreIdx = data;
    } else if (StringMatch(sk, VAR_SOL_ST)) {
        CheckInputSize2D(MID_SSR_DA, key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_soilWtrSto = data;
    } else if (StringMatch(sk, Tag_ROUTING_LAYERS)) {
        CheckInputSize(MID_SSR_DA, key, nrows, m_nRteLyrs);
        m_rteLyrs = data;
    } else if (StringMatch(sk, Tag_FLOWIN_INDEX_D8)) {
        CheckInputSize(MID_SSR_DA, key, nrows, m_nCells);
        m_flowInIdxD8 = data;
    } else {
        throw ModelException(MID_SSR_DA, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void SSR_DA::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SBIF)) *data = m_ifluQ2Rch;
    else {
        throw ModelException(MID_SSR_DA, "Get1DData", "Result " + sk + " does not exist.");
    }
    *n = m_nSubbsns + 1;
}

void SSR_DA::Get2DData(const char* key, int* nrows, int* ncols, float*** data) {
    InitialOutputs();
    string sk(key);
    *nrows = m_nCells;
    *ncols = m_maxSoilLyrs;

    if (StringMatch(sk, VAR_SSRU)) {
        *data = m_subSurfRf;
    } else if (StringMatch(sk, VAR_SSRUVOL)) {
        *data = m_subSurfRfVol;
    } else {
        throw ModelException(MID_SSR_DA, "Get2DData", "Output " + sk + " does not exist.");
    }
}

bool SSR_DA::CheckInputData() {
    CHECK_NONNEGATIVE(MID_SSR_DA, m_inputSubbsnID);
    CHECK_POSITIVE(MID_SSR_DA, m_nCells);
    CHECK_POSITIVE(MID_SSR_DA, m_ki);
    CHECK_NODATA(MID_SSR_DA, m_soilFrozenTemp);
    CHECK_POSITIVE(MID_SSR_DA, m_dt);
    CHECK_POSITIVE(MID_SSR_DA, m_CellWth);
    CHECK_POSITIVE(MID_SSR_DA, m_nSubbsns);
    CHECK_POSITIVE(MID_SSR_DA, m_nRteLyrs);
    CHECK_POINTER(MID_SSR_DA, m_subbsnID);
    CHECK_POINTER(MID_SSR_DA, m_nSoilLyrs);
    CHECK_POINTER(MID_SSR_DA, m_soilThk);
    CHECK_POINTER(MID_SSR_DA, m_slope);
    CHECK_POINTER(MID_SSR_DA, m_poreIdx);
    CHECK_POINTER(MID_SSR_DA, m_ks);
    CHECK_POINTER(MID_SSR_DA, m_soilSat);
    CHECK_POINTER(MID_SSR_DA, m_soilFC);
    CHECK_POINTER(MID_SSR_DA, m_soilWP);
    CHECK_POINTER(MID_SSR_DA, m_soilWtrSto);
    CHECK_POINTER(MID_SSR_DA, m_soilWtrStoPrfl);
    CHECK_POINTER(MID_SSR_DA, m_soilTemp);
    CHECK_POINTER(MID_SSR_DA, m_chWidth);
    CHECK_POINTER(MID_SSR_DA, m_rchID);
    CHECK_POINTER(MID_SSR_DA, m_flowInIdxD8);
    CHECK_POINTER(MID_SSR_DA, m_rteLyrs);
    return true;
}

void SSR_DA::InitialOutputs() {
    CHECK_POSITIVE(MID_SSR_DA, m_nCells);
    CHECK_POSITIVE(MID_SSR_DA, m_nSubbsns);
    if (nullptr == m_ifluQ2Rch) Initialize1DArray(m_nSubbsns + 1, m_ifluQ2Rch, 0.f);
    if (nullptr == m_subSurfRf) Initialize2DArray(m_nCells, m_maxSoilLyrs, m_subSurfRf, 0.f);
    if (nullptr == m_subSurfRfVol) Initialize2DArray(m_nCells, m_maxSoilLyrs, m_subSurfRfVol, 0.f);
}
