#include "seims.h"
#include "MUSLE_AS.h"

MUSLE_AS::MUSLE_AS(void) : m_nCells(-1), m_cellWidth(-1.f), m_nsub(-1), m_nSoilLayers(-1),
                           m_cellAreaKM(NODATA_VALUE), m_cellAreaKM1(NODATA_VALUE), m_cellAreaKM2(NODATA_VALUE),
                           m_usle_c(NULL), m_usle_p(NULL), m_usle_k(NULL), m_usle_ls(NULL),
                           m_flowacc(NULL), m_slope(NULL), m_streamLink(NULL), m_slopeForPq(NULL),
                           m_snowAccumulation(NULL), m_surfaceRunoff(NULL),
                           m_sedimentYield(NULL), m_sandYield(NULL), m_siltYield(NULL), m_clayYield(NULL),
                           m_smaggreYield(NULL), m_lgaggreYield(NULL) {
}

MUSLE_AS::~MUSLE_AS(void) {
    if (m_sedimentYield != NULL) Release1DArray(m_sedimentYield);
    if (m_sandYield != NULL) Release1DArray(m_sandYield);
    if (m_siltYield != NULL) Release1DArray(m_siltYield);
    if (m_clayYield != NULL) Release1DArray(m_clayYield);
    if (m_smaggreYield != NULL) Release1DArray(m_smaggreYield);
    if (m_lgaggreYield != NULL) Release1DArray(m_lgaggreYield);
    if (m_usle_ls != NULL) Release1DArray(m_usle_ls);
    if (m_slopeForPq != NULL) Release1DArray(m_slopeForPq);
}

bool MUSLE_AS::CheckInputData(void) {
    if (m_nCells <= 0) {
        throw ModelException(MID_MUSLE_AS, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    }
    if (m_cellWidth <= 0) {
        throw ModelException(MID_MUSLE_AS, "CheckInputData", "The cell width can not be less than zero.");
    }
    if (m_depRatio == NODATA_VALUE) {
        throw ModelException(MID_MUSLE_AS, "CheckInputData", "The deposition ratio can not be nodata.");
    }
    if (m_usle_c == NULL) throw ModelException(MID_MUSLE_AS, "CheckInputData", "The factor C can not be NULL.");
    if (m_usle_k == NULL) throw ModelException(MID_MUSLE_AS, "CheckInputData", "The factor K can not be NULL.");
    if (m_usle_p == NULL) throw ModelException(MID_MUSLE_AS, "CheckInputData", "The factor P can not be NULL.");
    if (m_flowacc == NULL) {
        throw ModelException(MID_MUSLE_AS, "CheckInputData", "The flow accumulation can not be NULL.");
    }
    if (m_slope == NULL) throw ModelException(MID_MUSLE_AS, "CheckInputData", "The slope can not be NULL.");
    if (m_snowAccumulation == NULL) {
        throw ModelException(MID_MUSLE_AS, "CheckInputData", "The snow accumulation can not be NULL.");
    }
    if (m_surfaceRunoff == NULL) {
        throw ModelException(MID_MUSLE_AS, "CheckInputData", "The surface runoff can not be NULL.");
    }
    if (m_streamLink == NULL) {
        throw ModelException(MID_MUSLE_AS, "CheckInputData", "The parameter: STREAM_LINK has not been set.");
    }
    if (m_detachSand == NULL) {
        throw ModelException(MID_MUSLE_AS, "CheckInputData", "The parameter: m_detachSand has not been set.");
    }
    if (m_detachSilt == NULL) {
        throw ModelException(MID_MUSLE_AS, "CheckInputData", "The parameter: m_detachSilt has not been set.");
    }
    if (m_detachClay == NULL) {
        throw ModelException(MID_MUSLE_AS, "CheckInputData", "The parameter: m_detachClay has not been set.");
    }
    if (m_detachSmAggre == NULL) {
        throw ModelException(MID_MUSLE_AS, "CheckInputData", "The parameter: m_detachSmAggre has not been set.");
    }
    if (m_detachLgAggre == NULL) {
        throw ModelException(MID_MUSLE_AS, "CheckInputData", "The parameter: m_detachLgAggre has not been set.");
    }
    return true;
}

void MUSLE_AS::initialOutputs() {
    if (m_nCells <= 0) {
        throw ModelException(MID_MUSLE_AS, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    }

    if (m_sedimentYield == NULL) Initialize1DArray(m_nCells, m_sedimentYield, 0.f);
    if (m_sandYield == NULL) Initialize1DArray(m_nCells, m_sandYield, 0.f);
    if (m_siltYield == NULL) Initialize1DArray(m_nCells, m_siltYield, 0.f);
    if (m_clayYield == NULL) Initialize1DArray(m_nCells, m_clayYield, 0.f);
    if (m_smaggreYield == NULL) Initialize1DArray(m_nCells, m_smaggreYield, 0.f);
    if (m_lgaggreYield == NULL) Initialize1DArray(m_nCells, m_lgaggreYield, 0.f);
    if (m_usle_ls == NULL) {
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

//float MUSLE_AS::getPeakRunoffRate(int cell) {
//    // peak flow
//    if (m_surfaceRunoff[cell] < 0.01f) {
//        return 0.f;
//    } else {
//        return m_cellAreaKM1 * m_slopeForPq[cell] * pow(m_surfaceRunoff[cell] / 25.4f, m_cellAreaKM2);
//    }
//}

int MUSLE_AS::Execute() {
    CheckInputData();
    initialOutputs();
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

        //if(i == 1000) cout << m_sedimentYield[i] << "," << m_surfaceRunoff[i] << endl;
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
    } else if (StringMatch(sk, VAR_OMP_THREADNUM)) {
        SetOpenMPThread((int) data);
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
    else if (StringMatch(s, VAR_SLOPE)) {
        m_slope = data;
        //else if (StringMatch(s, VAR_SUBBSN)) m_subbasin = data;
        //else if (StringMatch(s, VAR_SURU)) m_surfaceRunoff = data;
    } else if (StringMatch(s, VAR_OLFLOW)) { m_surfaceRunoff = data; }
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
    string s(key);
    CheckInputSize(key, nRows);
    m_nSoilLayers = nCols;
    if (StringMatch(s, VAR_USLE_K)) {
        this->m_usle_k = data;
    } else {
        throw ModelException(MID_MUSLE_AS, "Set2DData", "Parameter " + s +
            " does not exist in current module. Please contact the module developer.");
    }
}

void MUSLE_AS::SetSubbasins(clsSubbasins *subbasins) {
    if (m_nsub < 0) {
        m_nsub = subbasins->GetSubbasinNumber();
    }
}

void MUSLE_AS::GetValue(const char *key, float *value) {
    string s(key);
    throw ModelException(MID_MUSLE_AS, "GetValue", "Result " + s + " does not exist.");
}

void MUSLE_AS::Get1DData(const char *key, int *n, float **data) {
    initialOutputs();
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


