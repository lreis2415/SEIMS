#include "MUSLE_AS.h"

#include "text.h"

MUSLE_AS::MUSLE_AS() : m_nCells(-1), m_cellWidth(-1.f), m_nSoilLayers(-1),
                       m_cellAreaKM(NODATA_VALUE), m_cellAreaKM1(NODATA_VALUE), m_cellAreaKM2(NODATA_VALUE),
                       m_usle_c(nullptr), m_usle_p(nullptr), m_usle_k(nullptr), m_usle_ls(nullptr),
                       m_flowacc(nullptr), m_slope(nullptr), m_streamLink(nullptr), m_slopeForPq(nullptr),
                       m_snowAccumulation(nullptr), m_surfaceRunoff(nullptr),
                       m_sedimentYield(nullptr), m_sandYield(nullptr), m_siltYield(nullptr), m_clayYield(nullptr),
                       m_smaggreYield(nullptr), m_lgaggreYield(nullptr) {
}

MUSLE_AS::~MUSLE_AS() {
    if (m_sedimentYield != nullptr) Release1DArray(m_sedimentYield);
    if (m_sandYield != nullptr) Release1DArray(m_sandYield);
    if (m_siltYield != nullptr) Release1DArray(m_siltYield);
    if (m_clayYield != nullptr) Release1DArray(m_clayYield);
    if (m_smaggreYield != nullptr) Release1DArray(m_smaggreYield);
    if (m_lgaggreYield != nullptr) Release1DArray(m_lgaggreYield);
    if (m_usle_ls != nullptr) Release1DArray(m_usle_ls);
    if (m_slopeForPq != nullptr) Release1DArray(m_slopeForPq);
}

bool MUSLE_AS::CheckInputData() {
    CHECK_POSITIVE(MID_MUSLE_AS, m_nCells);
    CHECK_POSITIVE(MID_MUSLE_AS, m_cellWidth);
    CHECK_NONNEGATIVE(MID_MUSLE_AS, m_depRatio);
    CHECK_POINTER(MID_MUSLE_AS, m_usle_c);
    CHECK_POINTER(MID_MUSLE_AS, m_usle_k);
    CHECK_POINTER(MID_MUSLE_AS, m_usle_p);
    CHECK_POINTER(MID_MUSLE_AS, m_flowacc);
    CHECK_POINTER(MID_MUSLE_AS, m_slope);
    CHECK_POINTER(MID_MUSLE_AS, m_snowAccumulation);
    CHECK_POINTER(MID_MUSLE_AS, m_surfaceRunoff);
    CHECK_POINTER(MID_MUSLE_AS, m_streamLink);
    CHECK_POINTER(MID_MUSLE_AS, m_detachSand);
    CHECK_POINTER(MID_MUSLE_AS, m_detachSilt);
    CHECK_POINTER(MID_MUSLE_AS, m_detachClay);
    CHECK_POINTER(MID_MUSLE_AS, m_detachSmAggre);
    CHECK_POINTER(MID_MUSLE_AS, m_detachLgAggre);
    return true;
}

void MUSLE_AS::InitialOutputs() {
    CHECK_POSITIVE(MID_MUSLE_AS, m_nCells);
    if (nullptr == m_sedimentYield) Initialize1DArray(m_nCells, m_sedimentYield, 0.f);
    if (nullptr == m_sandYield) Initialize1DArray(m_nCells, m_sandYield, 0.f);
    if (nullptr == m_siltYield) Initialize1DArray(m_nCells, m_siltYield, 0.f);
    if (nullptr == m_clayYield) Initialize1DArray(m_nCells, m_clayYield, 0.f);
    if (nullptr == m_smaggreYield) Initialize1DArray(m_nCells, m_smaggreYield, 0.f);
    if (nullptr == m_lgaggreYield) Initialize1DArray(m_nCells, m_lgaggreYield, 0.f);
    if (nullptr == m_usle_ls) {
        float constant = pow(22.13f, 0.4f);
        m_usle_ls = new float[m_nCells];
        m_slopeForPq = new float[m_nCells];
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            if (m_usle_c[i] < 0.001f) {
                m_usle_c[i] = 0.001f;
            }
            if (m_usle_c[i] > 1.0f) {
                m_usle_c[i] = 1.0f;
            }
            float lambda_i1 = m_flowacc[i] * m_cellWidth;  // m
            float lambda_i = lambda_i1 + m_cellWidth;
            float L = pow(lambda_i, 1.4f) - pow(lambda_i1, 1.4f); // m^1.4
            L /= m_cellWidth * constant;  /// m^1.4 / m^0.4 = m

            float S = pow(sin(atan(m_slope[i])) / 0.0896f, 1.3f);

            m_usle_ls[i] = L * S;  // LS factor

            m_slopeForPq[i] = pow(m_slope[i] * 1000.f, 0.16f);
        }
    }
    if (FloatEqual(m_cellAreaKM, NODATA_VALUE)) {
        m_cellAreaKM = pow(m_cellWidth / 1000.f, 2.f);
        m_cellAreaKM1 = 3.79f * pow(m_cellAreaKM, 0.7f);
        m_cellAreaKM2 = 0.903f * pow(m_cellAreaKM, 0.017f);
    }
}

int MUSLE_AS::Execute() {
    CheckInputData();
    InitialOutputs();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (m_surfaceRunoff[i] < 0.0001f || m_streamLink[i] > 0) {
            m_sedimentYield[i] = 0.f;
            m_sandYield[i] = 0.f;
            m_siltYield[i] = 0.f;
            m_clayYield[i] = 0.f;
            m_smaggreYield[i] = 0.f;
            m_lgaggreYield[i] = 0.f;
            continue;
        }
        // peak flow
        float q = m_cellAreaKM1 * m_slopeForPq[i] * pow(m_surfaceRunoff[i] / 25.4f, m_cellAreaKM2);
        // sediment yield
        float Y = 11.8f * pow(m_surfaceRunoff[i] * m_cellAreaKM * 1000.0f * q,
                              0.56f) * m_usle_k[i][0] * m_usle_ls[i] * m_usle_c[i] * m_usle_p[i];
        // the snow pack effect
        if (m_snowAccumulation[i] > 0.0001f) {
            Y /= exp(3.f * m_snowAccumulation[i] / 25.4f);
        }
        m_sedimentYield[i] = Y * 1000.f; /// kg

        /// particle size distribution of sediment yield
        m_sandYield[i] = m_sedimentYield[i] * m_detachSand[i];
        m_siltYield[i] = m_sedimentYield[i] * m_detachSilt[i];
        m_clayYield[i] = m_sedimentYield[i] * m_detachClay[i];
        m_smaggreYield[i] = m_sedimentYield[i] * m_detachSmAggre[i];
        m_lgaggreYield[i] = m_sedimentYield[i] * m_detachLgAggre[i];
    }
    return 0;
}

bool MUSLE_AS::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_MUSLE_AS, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; }
        else {
            throw ModelException(MID_MUSLE_AS, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
        }
    }
    return true;
}

void MUSLE_AS::SetValue(const char *key, float data) {
    string sk(key);
    if (StringMatch(sk, Tag_CellWidth)) {
        m_cellWidth = data;
    } else if (StringMatch(sk, Tag_CellSize)) {
        m_nCells = (int) data;
    } else if (StringMatch(sk, VAR_DEPRATIO)) {
        m_depRatio = data;
    } else {
        throw ModelException(MID_MUSLE_AS, "SetValue", "Parameter " + sk + " does not exist in current module.");
    }
}

void MUSLE_AS::Set1DData(const char *key, int n, float *data) {
    CheckInputSize(key, n);
    string s(key);
    if (StringMatch(s, VAR_USLE_C)) { m_usle_c = data; }
    else if (StringMatch(s, VAR_USLE_P)) { m_usle_p = data; }
    else if (StringMatch(s, VAR_ACC)) { m_flowacc = data; }
    else if (StringMatch(s, VAR_SLOPE)) { m_slope = data; }
    else if (StringMatch(s, VAR_OLFLOW)) { m_surfaceRunoff = data; }
    else if (StringMatch(s, VAR_SNAC)) { m_snowAccumulation = data; }
    else if (StringMatch(s, VAR_STREAM_LINK)) { m_streamLink = data; }
    else if (StringMatch(s, VAR_DETACH_SAND)) { m_detachSand = data; }
    else if (StringMatch(s, VAR_DETACH_SILT)) { m_detachSilt = data; }
    else if (StringMatch(s, VAR_DETACH_CLAY)) { m_detachClay = data; }
    else if (StringMatch(s, VAR_DETACH_SAG)) { m_detachSmAggre = data; }
    else if (StringMatch(s, VAR_DETACH_LAG)) { m_detachLgAggre = data; }
    else {
        throw ModelException(MID_MUSLE_AS, "Set1DData", "Parameter " + s + " does not exist.");
    }
}

void MUSLE_AS::Set2DData(const char *key, int nRows, int nCols, float **data) {
    CheckInputSize(key, nRows);
    string s(key);
    m_nSoilLayers = nCols;
    if (StringMatch(s, VAR_USLE_K)) { m_usle_k = data; }
    else {
        throw ModelException(MID_MUSLE_AS, "Set2DData", "Parameter " + s +
            " does not exist in current module. Please contact the module developer.");
    }
}

void MUSLE_AS::Get1DData(const char *key, int *n, float **data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SOER)) { *data = m_sedimentYield; }
    else if (StringMatch(sk, VAR_USLE_LS)) { *data = m_usle_ls; }
    else if (StringMatch(sk, VAR_SANDYLD)) { *data = m_sandYield; }
    else if (StringMatch(sk, VAR_SILTYLD)) { *data = m_siltYield; }
    else if (StringMatch(sk, VAR_CLAYYLD)) { *data = m_clayYield; }
    else if (StringMatch(sk, VAR_SAGYLD)) { *data = m_smaggreYield; }
    else if (StringMatch(sk, VAR_LAGYLD)) { *data = m_lgaggreYield; }
    else {
        throw ModelException(MID_MUSLE_AS, "Get1DData", "Result " + sk + " does not exist.");
    }
    *n = m_nCells;
}
