#include "NutrientinGroundwater.h"

#include "text.h"

NutrientinGroundwater::NutrientinGroundwater() :
    m_inputSubbsnID(-1), m_cellWth(-1), m_nCells(-1), m_TimeStep(-1), m_gw0(NODATA_VALUE),
    m_gwNO3Conc(nullptr), m_gwNO3(nullptr),
    m_gwSolPConc(nullptr), m_gwSolP(nullptr), m_gw_q(nullptr), m_gwStor(nullptr),
    m_perco_no3_gw(nullptr), m_perco_solp_gw(nullptr), m_soilNO3(nullptr),
    m_soilSolP(nullptr), m_maxSoilLyrs(-1),
    m_nSoilLyrs(nullptr), m_gwNO3ToCh(nullptr), m_gwSolPToCh(nullptr), m_nSubbsns(-1),
    m_subbsnID(nullptr), m_subbasinsInfo(nullptr) {
}

NutrientinGroundwater::~NutrientinGroundwater() {
    if (nullptr != m_gwNO3ToCh) Release1DArray(m_gwNO3ToCh);
    if (nullptr != m_gwSolPToCh) Release1DArray(m_gwSolPToCh);
    // m_gwNO3 and m_gwSolP will be released in ~clsReaches(). By lj, 2017-12-26
}

void NutrientinGroundwater::SetSubbasins(clsSubbasins* subbasins) {
    if (nullptr == m_subbasinsInfo) {
        m_subbasinsInfo = subbasins;
        // m_nSubbasins = m_subbasinsInfo->GetSubbasinNumber(); // Set in SetValue()
        m_subbasinIDs = m_subbasinsInfo->GetSubbasinIDs();
    }
}

bool NutrientinGroundwater::CheckInputData() {
    CHECK_POSITIVE(M_NUTRGW[0], m_nCells);
    CHECK_POSITIVE(M_NUTRGW[0], m_TimeStep);
    CHECK_POSITIVE(M_NUTRGW[0], m_cellWth);
    CHECK_POSITIVE(M_NUTRGW[0], m_gw0);
    CHECK_POINTER(M_NUTRGW[0], m_gw_q);
    CHECK_POINTER(M_NUTRGW[0], m_gwStor);
    CHECK_POINTER(M_NUTRGW[0], m_perco_no3_gw);
    CHECK_POINTER(M_NUTRGW[0], m_perco_solp_gw);
    CHECK_POINTER(M_NUTRGW[0], m_nSoilLyrs);
    CHECK_POINTER(M_NUTRGW[0], m_soilNO3);
    return true;
}

void NutrientinGroundwater::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, Tag_TimeStep[0])) m_TimeStep = CVT_INT(value);
    else if (StringMatch(sk, Tag_CellWidth[0])) m_cellWth = value;
    else if (StringMatch(sk, VAR_GW0[0])) m_gw0 = value;
    else if (StringMatch(sk, VAR_SUBBSNID_NUM[0])) m_nSubbsns = CVT_INT(value);
    else if (StringMatch(sk, Tag_SubbasinId)) m_inputSubbsnID = CVT_INT(value);
    else {
        throw ModelException(M_NUTRGW[0], "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void NutrientinGroundwater::Set1DData(const char* key, const int n, float* data) {
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSN[0])) {
        CheckInputSize(M_NUTRGW[0], key, n, m_nCells);
        m_subbsnID = data;
    } else if (StringMatch(sk, VAR_SBQG[0])) m_gw_q = data;
    else if (StringMatch(sk, VAR_SBGS[0])) m_gwStor = data;
    else if (StringMatch(sk, VAR_PERCO_N_GW[0])) m_perco_no3_gw = data;
    else if (StringMatch(sk, VAR_PERCO_P_GW[0])) m_perco_solp_gw = data;
    else if (StringMatch(sk, VAR_SOILLAYERS[0])) {
        CheckInputSize(M_NUTRGW[0], key, n, m_nCells);
        m_nSoilLyrs = data;
    } else {
        throw ModelException(M_NUTRGW[0], "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void NutrientinGroundwater::Set2DData(const char* key, const int nrows, const int ncols, float** data) {
    CheckInputSize2D(M_NUTRGW[0], key, nrows, ncols, m_nCells, m_maxSoilLyrs);
    string sk(key);
    if (StringMatch(sk, VAR_SOL_NO3[0])) m_soilNO3 = data;
    else if (StringMatch(sk, VAR_SOL_SOLP[0])) m_soilSolP = data;
    else {
        throw ModelException(M_NUTRGW[0], "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void NutrientinGroundwater::SetReaches(clsReaches* reaches) {
    if (nullptr != reaches) {
        // m_nSubbasins = reaches->GetReachNumber();
        if (nullptr == m_gwNO3Conc) reaches->GetReachesSingleProperty(REACH_GWNO3, &m_gwNO3Conc);
        if (nullptr == m_gwSolPConc) reaches->GetReachesSingleProperty(REACH_GWSOLP, &m_gwSolPConc);
    } else {
        throw ModelException(M_NUTRGW[0], "SetReaches", "The reaches input can not to be NULL.");
    }
}

void NutrientinGroundwater::InitialOutputs() {
    CHECK_POSITIVE(M_NUTRGW[0], m_nCells);
    // allocate the output variables
    if (nullptr == m_gwNO3ToCh) {
        Initialize1DArray(m_nSubbsns + 1, m_gwNO3ToCh, 0.f);
        Initialize1DArray(m_nSubbsns + 1, m_gwSolPToCh, 0.f);
    }
    if (nullptr == m_gwNO3) {
        /// initial nutrient amount stored in groundwater
        m_gwNO3 = new float[m_nSubbsns + 1];
        m_gwSolP = new float[m_nSubbsns + 1];
        for (auto it = m_subbasinIDs.begin(); it != m_subbasinIDs.end(); ++it) {
            Subbasin* subbasin = m_subbasinsInfo->GetSubbasinByID(*it);
            float subArea = subbasin->GetArea();                           //m^2
            m_gwNO3[*it] = m_gw0 * m_gwNO3Conc[*it] * subArea * 0.000001f; /// mm * mg/L * m2 = 10^-6 kg
            m_gwSolP[*it] = m_gw0 * m_gwSolPConc[*it] * subArea * 0.000001f;
        }
    }
}

int NutrientinGroundwater::Execute() {
    CheckInputData();
    InitialOutputs();
    for (auto it = m_subbasinIDs.begin(); it != m_subbasinIDs.end(); ++it) {
        int id = *it;
        Subbasin* subbasin = m_subbasinsInfo->GetSubbasinByID(id);
        int nCells = subbasin->GetCellCount();
        float subArea = subbasin->GetArea(); // m^2
        float revap = subbasin->GetEg();
        /// 1. firstly, restore the groundwater storage during current day
        ///    since the m_gwStor has involved percolation water, just need add revap and runoff water
        float gwqVol = m_gw_q[id] * m_TimeStep;    // m^3, water volume flow out
        float reVapVol = revap * subArea * 0.001f; // m^3
        float tmpGwStorage = m_gwStor[id] * subArea * 0.001f + gwqVol + reVapVol;
        /// 2. secondly, update nutrient concentration
        m_gwNO3[id] += m_perco_no3_gw[id]; /// nutrient amount, kg
        m_gwSolP[id] += m_perco_solp_gw[id];
        m_gwNO3Conc[id] = m_gwNO3[id] / tmpGwStorage * 1000.f; // kg / m^3 * 1000. = mg/L
        m_gwSolPConc[id] = m_gwSolP[id] / tmpGwStorage * 1000.f;
        /// 3. thirdly, calculate nutrient in groundwater runoff
        //cout<<"subID: "<<id<<", gwQ: "<<m_gw_q[id] << ", ";
        m_gwNO3ToCh[id] = m_gwNO3Conc[id] * gwqVol * 0.001f; // g/m3 * m3 / 1000 = kg
        m_gwSolPToCh[id] = m_gwSolPConc[id] * gwqVol * 0.001f;
        //cout<<"subID: "<<id<<", gwno3Con: "<<m_gwno3Con[id] << ", no3gwToCh: "<<m_no3gwToCh[id] << ", ";
        /// 4. fourthly, calculate nutrient loss loss through revep and update no3 in the bottom soil layer
        float no3ToSoil = revap * 0.001f * m_gwNO3Conc[id] * 10.f; // kg/ha  (m*10*g/m3=kg/ha)
        float solpToSoil = revap * 0.001f * m_gwSolPConc[id] * 10.f;
        float no3ToSoil_kg = no3ToSoil * subArea * 0.0001f; /// kg/ha * m^2 / 10000.f = kg
        float solpToSoil_kg = solpToSoil * subArea * 0.0001f;
        int* cells = subbasin->GetCells();
        int index = 0;
        for (int i = 0; i < nCells; i++) {
            index = cells[i];
            m_soilNO3[index][CVT_INT(m_nSoilLyrs[index]) - 1] += no3ToSoil;
            m_soilSolP[index][CVT_INT(m_nSoilLyrs[index]) - 1] += solpToSoil;
        }
        /// finally, update nutrient amount
        m_gwNO3[id] -= m_gwNO3ToCh[id] + no3ToSoil_kg;
        m_gwSolP[id] -= m_gwSolPToCh[id] + solpToSoil_kg;
    }
    return 0;
}

void NutrientinGroundwater::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nSubbsns + 1;
    if (StringMatch(sk, VAR_NO3GW_TOCH[0])) {
        *data = m_gwNO3ToCh;
    } else if (StringMatch(sk, VAR_MINPGW_TOCH[0])) {
        *data = m_gwSolPToCh;
    } else if (StringMatch(sk, VAR_GWNO3_CONC[0])) {
        *data = m_gwNO3Conc;
    } else if (StringMatch(sk, VAR_GWSOLP_CONC[0])) {
        *data = m_gwSolPConc;
    } else if (StringMatch(sk, VAR_GWNO3[0])) {
        *data = m_gwNO3;
    } else if (StringMatch(sk, VAR_GWSOLP[0])) {
        *data = m_gwSolP;
    } else {
        throw ModelException(M_NUTRGW[0], "Get1DData", "Parameter " + sk + " does not exist.");
    }
}
