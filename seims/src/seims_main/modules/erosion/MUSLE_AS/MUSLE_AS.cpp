#include "MUSLE_AS.h"

#include "text.h"

MUSLE_AS::MUSLE_AS() : m_nCells(-1), m_cellWth(-1.f), m_nSoilLayers(-1),
                       m_depRatio(NODATA_VALUE), m_detSand(nullptr), m_detSilt(nullptr),
                       m_detClay(nullptr), m_detSmAgg(nullptr), m_detLgAgg(nullptr), m_usleP(nullptr),
                       m_usleK(nullptr), m_usleC(nullptr),
                       m_slope(nullptr), m_flowAccm(nullptr), m_rchID(nullptr), m_usleLS(nullptr),
                       m_cellAreaKM(NODATA_VALUE), m_cellAreaKM1(NODATA_VALUE), m_cellAreaKM2(NODATA_VALUE),
                       m_slopeForPq(nullptr),
                       m_snowAccum(nullptr), m_surfRf(nullptr),
                       m_eroSed(nullptr), m_eroSand(nullptr), m_eroSilt(nullptr), m_eroClay(nullptr),
                       m_eroSmAgg(nullptr), m_eroLgAgg(nullptr) {
}

MUSLE_AS::~MUSLE_AS() {
    if (m_eroSed != nullptr) Release1DArray(m_eroSed);
    if (m_eroSand != nullptr) Release1DArray(m_eroSand);
    if (m_eroSilt != nullptr) Release1DArray(m_eroSilt);
    if (m_eroClay != nullptr) Release1DArray(m_eroClay);
    if (m_eroSmAgg != nullptr) Release1DArray(m_eroSmAgg);
    if (m_eroLgAgg != nullptr) Release1DArray(m_eroLgAgg);
    if (m_usleLS != nullptr) Release1DArray(m_usleLS);
    if (m_slopeForPq != nullptr) Release1DArray(m_slopeForPq);
}

bool MUSLE_AS::CheckInputData() {
    CHECK_POSITIVE(MID_MUSLE_AS, m_nCells);
    CHECK_POSITIVE(MID_MUSLE_AS, m_cellWth);
    CHECK_NONNEGATIVE(MID_MUSLE_AS, m_depRatio);
    CHECK_POINTER(MID_MUSLE_AS, m_usleC);
    CHECK_POINTER(MID_MUSLE_AS, m_usleK);
    CHECK_POINTER(MID_MUSLE_AS, m_usleP);
    CHECK_POINTER(MID_MUSLE_AS, m_flowAccm);
    CHECK_POINTER(MID_MUSLE_AS, m_slope);
    CHECK_POINTER(MID_MUSLE_AS, m_snowAccum);
    CHECK_POINTER(MID_MUSLE_AS, m_surfRf);
    CHECK_POINTER(MID_MUSLE_AS, m_rchID);
    CHECK_POINTER(MID_MUSLE_AS, m_detSand);
    CHECK_POINTER(MID_MUSLE_AS, m_detSilt);
    CHECK_POINTER(MID_MUSLE_AS, m_detClay);
    CHECK_POINTER(MID_MUSLE_AS, m_detSmAgg);
    CHECK_POINTER(MID_MUSLE_AS, m_detLgAgg);
    return true;
}

void MUSLE_AS::InitialOutputs() {
    CHECK_POSITIVE(MID_MUSLE_AS, m_nCells);
    if (nullptr == m_eroSed) Initialize1DArray(m_nCells, m_eroSed, 0.f);
    if (nullptr == m_eroSand) Initialize1DArray(m_nCells, m_eroSand, 0.f);
    if (nullptr == m_eroSilt) Initialize1DArray(m_nCells, m_eroSilt, 0.f);
    if (nullptr == m_eroClay) Initialize1DArray(m_nCells, m_eroClay, 0.f);
    if (nullptr == m_eroSmAgg) Initialize1DArray(m_nCells, m_eroSmAgg, 0.f);
    if (nullptr == m_eroLgAgg) Initialize1DArray(m_nCells, m_eroLgAgg, 0.f);
    if (nullptr == m_usleLS) {
        float constant = 3.451378310759016f; // pow(22.13f, 0.4f);
        m_usleLS = new(nothrow) float[m_nCells];
        m_slopeForPq = new(nothrow) float[m_nCells];
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            if (m_usleC[i] < 0.001f) {
                m_usleC[i] = 0.001f;
            }
            if (m_usleC[i] > 1.0f) {
                m_usleC[i] = 1.0f;
            }
            float lambda_i1 = m_flowAccm[i] * m_cellWth; // m
            float lambda_i = lambda_i1 + m_cellWth;
            float L = pow(lambda_i, 1.4f) - pow(lambda_i1, 1.4f);             // m^1.4
            L /= m_cellWth * constant;                                        /// m^1.4 / m^0.4 = m
            float S = pow(sin(atan(m_slope[i])) * 11.160714285714286f, 1.3f); // 1. / 0.0896 =  11.160714285714286

            m_usleLS[i] = L * S; // LS factor
            m_slopeForPq[i] = pow(m_slope[i] * 1000.f, 0.16f);
        }
    }
    if (FloatEqual(m_cellAreaKM, NODATA_VALUE)) {
        m_cellAreaKM = m_cellWth * m_cellWth * 0.000001f; // pow(m_cellWidth / 1000.f, 2.f);
        m_cellAreaKM1 = 3.79f * pow(m_cellAreaKM, 0.7f);
        m_cellAreaKM2 = 0.903f * pow(m_cellAreaKM, 0.017f);
    }
}

int MUSLE_AS::Execute() {
    CheckInputData();
    InitialOutputs();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (m_surfRf[i] < 0.0001f || m_rchID[i] > 0) {
            m_eroSed[i] = 0.f;
            m_eroSand[i] = 0.f;
            m_eroSilt[i] = 0.f;
            m_eroClay[i] = 0.f;
            m_eroSmAgg[i] = 0.f;
            m_eroLgAgg[i] = 0.f;
            continue;
        }
        // peak flow, 1. / 25.4 = 0.03937007874015748
        float q = m_cellAreaKM1 * m_slopeForPq[i] * pow(m_surfRf[i] * 0.03937007874015748f, m_cellAreaKM2);
        // sediment yield
        float Y = 11.8f * pow(m_surfRf[i] * m_cellAreaKM * 1000.0f * q,
                              0.56f) * m_usleK[i][0] * m_usleLS[i] * m_usleC[i] * m_usleP[i];
        // the snow pack effect
        if (m_snowAccum[i] > 0.0001f) {
            Y /= exp(3.f * m_snowAccum[i] * 0.03937007874015748f);
        }
        m_eroSed[i] = Y * 1000.f; /// kg

        /// particle size distribution of sediment yield
        m_eroSand[i] = m_eroSed[i] * m_detSand[i];
        m_eroSilt[i] = m_eroSed[i] * m_detSilt[i];
        m_eroClay[i] = m_eroSed[i] * m_detClay[i];
        m_eroSmAgg[i] = m_eroSed[i] * m_detSmAgg[i];
        m_eroLgAgg[i] = m_eroSed[i] * m_detLgAgg[i];
    }
    return 0;
}

bool MUSLE_AS::CheckInputSize(const char* key, const int n) {
    if (n <= 0) {
        throw ModelException(MID_MUSLE_AS, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) {
            m_nCells = n;
        } else {
            throw ModelException(MID_MUSLE_AS, "CheckInputSize", "Input data for " + string(key) +
                                 " is invalid. All the input data should have same size.");
        }
    }
    return true;
}

void MUSLE_AS::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, Tag_CellWidth)) {
        m_cellWth = value;
    } else if (StringMatch(sk, VAR_DEPRATIO)) {
        m_depRatio = value;
    } else {
        throw ModelException(MID_MUSLE_AS, "SetValue", "Parameter " + sk + " does not exist in current module.");
    }
}

void MUSLE_AS::Set1DData(const char* key, const int n, float* data) {
    CheckInputSize(key, n);
    string s(key);
    if (StringMatch(s, VAR_USLE_C)) m_usleC = data;
    else if (StringMatch(s, VAR_USLE_P)) m_usleP = data;
    else if (StringMatch(s, VAR_ACC)) m_flowAccm = data;
    else if (StringMatch(s, VAR_SLOPE)) m_slope = data;
    else if (StringMatch(s, VAR_OLFLOW)) m_surfRf = data;
    else if (StringMatch(s, VAR_SNAC)) m_snowAccum = data;
    else if (StringMatch(s, VAR_STREAM_LINK)) m_rchID = data;
    else if (StringMatch(s, VAR_DETACH_SAND)) m_detSand = data;
    else if (StringMatch(s, VAR_DETACH_SILT)) m_detSilt = data;
    else if (StringMatch(s, VAR_DETACH_CLAY)) m_detClay = data;
    else if (StringMatch(s, VAR_DETACH_SAG)) m_detSmAgg = data;
    else if (StringMatch(s, VAR_DETACH_LAG)) m_detLgAgg = data;
    else {
        throw ModelException(MID_MUSLE_AS, "Set1DData", "Parameter " + s + " does not exist.");
    }
}

void MUSLE_AS::Set2DData(const char* key, int nRows, int nCols, float** data) {
    CheckInputSize(key, nRows);
    string s(key);
    m_nSoilLayers = nCols;
    if (StringMatch(s, VAR_USLE_K)) m_usleK = data;
    else {
        throw ModelException(MID_MUSLE_AS, "Set2DData", "Parameter " + s +
                             " does not exist in current module. Please contact the module developer.");
    }
}

void MUSLE_AS::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SOER)) *data = m_eroSed;
    else if (StringMatch(sk, VAR_USLE_LS)) *data = m_usleLS;
    else if (StringMatch(sk, VAR_SANDYLD)) *data = m_eroSand;
    else if (StringMatch(sk, VAR_SILTYLD)) *data = m_eroSilt;
    else if (StringMatch(sk, VAR_CLAYYLD)) *data = m_eroClay;
    else if (StringMatch(sk, VAR_SAGYLD)) *data = m_eroSmAgg;
    else if (StringMatch(sk, VAR_LAGYLD)) *data = m_eroLgAgg;
    else {
        throw ModelException(MID_MUSLE_AS, "Get1DData", "Result " + sk + " does not exist.");
    }
    *n = m_nCells;
}
