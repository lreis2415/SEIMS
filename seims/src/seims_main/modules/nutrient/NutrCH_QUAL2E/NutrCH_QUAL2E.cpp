#include "seims.h"
#include "NutrCH_QUAL2E.h"

using namespace std;

NutrCH_QUAL2E::NutrCH_QUAL2E() :
//input
    m_nCells(-1), m_dt(-1), m_layeringMethod(UP_DOWN), m_qUpReach(0.f), m_rnum1(0.f), igropt(-1),
    m_ai0(-1.f), m_ai1(-1.f), m_ai2(-1.f), m_ai3(-1.f), m_ai4(-1.f), m_ai5(-1.f),
    m_ai6(-1.f), m_lambda0(-1.f), m_lambda1(-1.f), m_lambda2(-1.f), m_cod_n(-1), m_cod_k(-1),
    m_k_l(-1.f), m_k_n(-1.f), m_k_p(-1.f), m_p_n(-1.f), tfact(-1.f), m_mumax(-1.f), m_rhoq(-1.f),
    m_streamLink(nullptr), m_soilTemp(nullptr), m_daylen(nullptr), m_sra(nullptr),
    m_bankStorage(nullptr), m_qOutCh(nullptr), m_chStorage(nullptr), m_preChStorage(nullptr),
    m_chWTdepth(nullptr), m_preChWTDepth(nullptr), m_chTemp(nullptr),
    m_bc1(nullptr), m_bc2(nullptr), m_bc3(nullptr), m_bc4(nullptr),
    m_rs1(nullptr), m_rs2(nullptr), m_rs3(nullptr), m_rs4(nullptr), m_rs5(nullptr),
    m_rk1(nullptr), m_rk2(nullptr), m_rk3(nullptr), m_rk4(nullptr),
    m_latNO3ToCh(nullptr), m_surNO3ToCh(nullptr), m_surNH4ToCh(nullptr), m_surSolPToCh(nullptr), m_gwNO3ToCh(nullptr),
    m_gwSolPToCh(nullptr), m_sedOrgNToCh(nullptr), m_sedOrgPToCh(nullptr), m_sedMinPAToCh(nullptr),
    m_sedMinPSToCh(nullptr), m_no2ToCh(nullptr), m_surCodToCh(nullptr),
    m_chSr(nullptr), m_chDaylen(nullptr), m_chCellCount(nullptr),
    // reaches related
    m_reachDownStream(nullptr), m_chOrgNCo(NODATA_VALUE), m_chOrgPCo(NODATA_VALUE),
    // point source loadings to channel
    m_ptNO3ToCh(nullptr), m_ptNH4ToCh(nullptr), m_ptOrgNToCh(nullptr), m_ptTNToCh(nullptr),
    m_ptSolPToCh(nullptr), m_ptOrgPToCh(nullptr), m_ptTPToCh(nullptr), m_ptCODToCh(nullptr),
    // channel erosion
    m_chDeg(nullptr),
    // nutrient storage in channel
    m_chAlgae(nullptr), m_chOrgN(nullptr), m_chNH4(nullptr), m_chNO2(nullptr), m_chNO3(nullptr), m_chTN(nullptr),
    m_chOrgP(nullptr), m_chSolP(nullptr), m_chTP(nullptr), m_chCOD(nullptr), m_chDOx(nullptr), m_chChlora(nullptr),
    m_chSatDOx(NODATA_VALUE),
    // nutrient amount outputs of channels
    m_chOutChlora(nullptr), m_chOutAlgae(nullptr), m_chOutOrgN(nullptr), m_chOutOrgP(nullptr), m_chOutNH4(nullptr),
    m_chOutNO2(nullptr), m_chOutNO3(nullptr), m_chOutSolP(nullptr), m_chOutDOx(nullptr), m_chOutCOD(nullptr),
    m_chOutTN(nullptr), m_chOutTP(nullptr),
    // nutrient concentration outputs of channels
    m_chOutAlgaeConc(nullptr), m_chOutChloraConc(nullptr), m_chOutOrgNConc(nullptr), m_chOutOrgPConc(nullptr),
    m_chOutNH4Conc(nullptr), m_chOutNO2Conc(nullptr), m_chOutNO3Conc(nullptr),
    m_chOutSolPConc(nullptr), m_chOutCODConc(nullptr),
    m_chOutDOxConc(nullptr), m_chOutTNConc(nullptr), m_chOutTPConc(nullptr) {
}

NutrCH_QUAL2E::~NutrCH_QUAL2E() {
    /// reach parameters, will be released in ~clsReaches(). By lj, 2017-12-26

    if (nullptr != m_ptNO3ToCh) Release1DArray(m_ptNO3ToCh);
    if (nullptr != m_ptNH4ToCh) Release1DArray(m_ptNH4ToCh);
    if (nullptr != m_ptOrgNToCh) Release1DArray(m_ptOrgNToCh);
    if (nullptr != m_ptTNToCh) Release1DArray(m_ptTNToCh);
    if (nullptr != m_ptSolPToCh) Release1DArray(m_ptSolPToCh);
    if (nullptr != m_ptOrgPToCh) Release1DArray(m_ptOrgPToCh);
    if (nullptr != m_ptTPToCh) Release1DArray(m_ptTPToCh);
    if (nullptr != m_ptCODToCh) Release1DArray(m_ptCODToCh);
    /// storage in channel
    if (nullptr != m_chTN) Release1DArray(m_chTN);
    if (nullptr != m_chTP) Release1DArray(m_chTP);
    if (nullptr != m_chChlora) Release1DArray(m_chChlora);
    /// amount out of channel
    if (nullptr != m_chOutChlora) Release1DArray(m_chOutChlora);
    if (nullptr != m_chOutAlgae) Release1DArray(m_chOutAlgae);
    if (nullptr != m_chOutOrgN) Release1DArray(m_chOutOrgN);
    if (nullptr != m_chOutOrgP) Release1DArray(m_chOutOrgP);
    if (nullptr != m_chOutNH4) Release1DArray(m_chOutNH4);
    if (nullptr != m_chOutNO2) Release1DArray(m_chOutNO2);
    if (nullptr != m_chOutNO3) Release1DArray(m_chOutNO3);
    if (nullptr != m_chOutSolP) Release1DArray(m_chOutSolP);
    if (nullptr != m_chOutDOx) Release1DArray(m_chOutDOx);
    if (nullptr != m_chOutCOD) Release1DArray(m_chOutCOD);
    if (nullptr != m_chOutTN) Release1DArray(m_chOutTN);
    if (nullptr != m_chOutTP) Release1DArray(m_chOutTP);
    /// concentration out of channel
    if (nullptr != m_chOutChloraConc) Release1DArray(m_chOutChloraConc);
    if (nullptr != m_chOutAlgaeConc) Release1DArray(m_chOutAlgaeConc);
    if (nullptr != m_chOutOrgNConc) Release1DArray(m_chOutOrgNConc);
    if (nullptr != m_chOutOrgPConc) Release1DArray(m_chOutOrgPConc);
    if (nullptr != m_chOutNH4Conc) Release1DArray(m_chOutNH4Conc);
    if (nullptr != m_chOutNO2Conc) Release1DArray(m_chOutNO2Conc);
    if (nullptr != m_chOutNO3Conc) Release1DArray(m_chOutNO3Conc);
    if (nullptr != m_chOutSolPConc) Release1DArray(m_chOutSolPConc);
    if (nullptr != m_chOutDOxConc) Release1DArray(m_chOutDOxConc);
    if (nullptr != m_chOutCODConc) Release1DArray(m_chOutCODConc);
    if (nullptr != m_chOutTNConc) Release1DArray(m_chOutTNConc);
    if (nullptr != m_chOutTPConc) Release1DArray(m_chOutTPConc);
}

void NutrCH_QUAL2E::ParametersSubbasinForChannel() {
    if (nullptr == m_chCellCount) {
        Initialize1DArray(m_nReaches + 1, m_chCellCount, 0);
    }
    if (nullptr == m_chDaylen) {
        Initialize1DArray(m_nReaches + 1, m_chDaylen, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chSr, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chTemp, 0.f);
    } else {
        return;
    }
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (m_streamLink[i] <= 0.f) {
            continue;
        }
        int subi = (int) m_streamLink[i];
        if (m_nReaches == 1) {
            subi = 1;
        } else if (subi >= m_nReaches + 1) {
            throw ModelException(MID_NUTRCH_QUAL2E, "Execute", "The subbasin " + ValueToString(subi) + " is invalid.");
        }

        m_chDaylen[subi] += m_daylen[i];
        m_chSr[subi] += m_sra[i];
        m_chTemp[subi] += m_soilTemp[i];
        m_chCellCount[subi] += 1;
    }

    for (int i = 1; i <= m_nReaches; i++) {
        m_chDaylen[i] /= m_chCellCount[i];
        m_chSr[i] /= m_chCellCount[i];
        m_chTemp[i] /= m_chCellCount[i];
    }
}

bool NutrCH_QUAL2E::CheckInputCellSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_NUTRCH_QUAL2E, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) {
            m_nCells = n;
        } else {
            throw ModelException(MID_NUTRCH_QUAL2E, "CheckInputCellSize", "Input data for " + string(key) +
                " is invalid with size: " + ValueToString(n) +
                ". The origin size is " +
                ValueToString(m_nCells) + ".\n");
        }
    }
    return true;
}

bool NutrCH_QUAL2E::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_NUTRCH_QUAL2E, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_nReaches != n - 1) {
        if (m_nReaches <= 0) {
            m_nReaches = n - 1;
        } else {
            throw ModelException(MID_NUTRCH_QUAL2E, "CheckInputSize", "Input data for " + string(key) +
                " is invalid with size: " + ValueToString(n) +
                ". The origin size is " +
                ValueToString(m_nReaches) + ".\n");
        }
    }
    return true;
}

bool NutrCH_QUAL2E::CheckInputData() {
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_dt, "m_dt");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_nReaches, "m_nReaches");
    CHECK_DATA(MID_NUTRCH_QUAL2E, m_qUpReach < 0.f, m_qUpReach, "should greater or equal than 0.");
    //CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_qUpReach, "m_qUpReach");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_rnum1, "m_rnum1");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, igropt, "igropt");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_cod_n, "m_cod_n");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_cod_k, "m_cod_k");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_ai0, "m_ai0");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_ai1, "m_ai1");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_ai2, "m_ai2");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_ai3, "m_ai3");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_ai4, "m_ai4");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_ai5, "m_ai5");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_ai6, "m_ai6");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_lambda0, "m_lambda0");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_lambda1, "m_lambda1");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_lambda2, "m_lambda2");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_k_l, "m_k_l");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_k_n, "m_k_n");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_k_p, "m_k_p");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_p_n, "m_p_n");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, tfact, "tfact");
    CHECK_POSITIVE(MID_NUTRCH_QUAL2E, m_mumax, "m_mumax");
    //CHECK_POSITIVE(MID_NutCHRout, m_rhoqv, "m_rhoqv");

    CHECK_POINTER(MID_NUTRCH_QUAL2E, m_daylen, "m_daylen");
    CHECK_POINTER(MID_NUTRCH_QUAL2E, m_sra, "m_sra");
    CHECK_POINTER(MID_NUTRCH_QUAL2E, m_qOutCh, "m_qOutCh");
    CHECK_POINTER(MID_NUTRCH_QUAL2E, m_chStorage, "m_chStorage");
    CHECK_POINTER(MID_NUTRCH_QUAL2E, m_preChStorage, "m_preChStorage");
    CHECK_POINTER(MID_NUTRCH_QUAL2E, m_chWTdepth, "m_chWTdepth");
    CHECK_POINTER(MID_NUTRCH_QUAL2E, m_latNO3ToCh, "m_latNO3ToCh");
    CHECK_POINTER(MID_NUTRCH_QUAL2E, m_surNO3ToCh, "m_surNO3ToCh");
    CHECK_POINTER(MID_NUTRCH_QUAL2E, m_surSolPToCh, "m_surSolPToCh");
    CHECK_POINTER(MID_NUTRCH_QUAL2E, m_surCodToCh, "m_codToCh");
    CHECK_POINTER(MID_NUTRCH_QUAL2E, m_gwNO3ToCh, "m_gwNO3ToCh");
    CHECK_POINTER(MID_NUTRCH_QUAL2E, m_gwSolPToCh, "m_gwSolPToCh");
    CHECK_POINTER(MID_NUTRCH_QUAL2E, m_sedOrgNToCh, "m_sedOrgNToCh");
    CHECK_POINTER(MID_NUTRCH_QUAL2E, m_sedOrgPToCh, "m_sedOrgPToCh");
    CHECK_POINTER(MID_NUTRCH_QUAL2E, m_sedMinPAToCh, "m_sedMinPAToCh");
    CHECK_POINTER(MID_NUTRCH_QUAL2E, m_sedMinPSToCh, "m_sedMinPSToCh");
    CHECK_POINTER(MID_NUTRCH_QUAL2E, m_streamLink, "m_streamLink");
    CHECK_POINTER(MID_NUTRCH_QUAL2E, m_soilTemp, "m_soilTemp");
    return true;
}

void NutrCH_QUAL2E::SetValue(const char *key, float value) {
    string sk(key);
    if (StringMatch(sk, VAR_OMP_THREADNUM)) { SetOpenMPThread((int) value); }
    else if (StringMatch(sk, Tag_ChannelTimeStep)) { m_dt = (int) value; }
    else if (StringMatch(sk, Tag_LayeringMethod)) { m_layeringMethod = (LayeringMethod) int(value); }
    else if (StringMatch(sk, VAR_QUPREACH)) { m_qUpReach = value; }
    else if (StringMatch(sk, VAR_RNUM1)) { m_rnum1 = value; }
    else if (StringMatch(sk, VAR_IGROPT)) { igropt = (int) value; }
    else if (StringMatch(sk, VAR_COD_N)) { m_cod_n = value; }
    else if (StringMatch(sk, VAR_COD_K)) { m_cod_k = value; }
    else if (StringMatch(sk, VAR_AI0)) { m_ai0 = value; }
    else if (StringMatch(sk, VAR_AI1)) { m_ai1 = value; }
    else if (StringMatch(sk, VAR_AI2)) { m_ai2 = value; }
    else if (StringMatch(sk, VAR_AI3)) { m_ai3 = value; }
    else if (StringMatch(sk, VAR_AI4)) { m_ai4 = value; }
    else if (StringMatch(sk, VAR_AI5)) { m_ai5 = value; }
    else if (StringMatch(sk, VAR_AI6)) { m_ai6 = value; }
    else if (StringMatch(sk, VAR_LAMBDA0)) { m_lambda0 = value; }
    else if (StringMatch(sk, VAR_LAMBDA1)) { m_lambda1 = value; }
    else if (StringMatch(sk, VAR_LAMBDA2)) { m_lambda2 = value; }
    else if (StringMatch(sk, VAR_K_L)) {
        m_k_l = value;
        //convert units on k_l:read in as kJ/(m2*min), use as MJ/(m2*hr)
        m_k_l = m_k_l * 1.e-3f * 60.f;
    } else if (StringMatch(sk, VAR_K_N)) { m_k_n = value; }
    else if (StringMatch(sk, VAR_K_P)) { m_k_p = value; }
    else if (StringMatch(sk, VAR_P_N)) { m_p_n = value; }
    else if (StringMatch(sk, VAR_TFACT)) { tfact = value; }
    else if (StringMatch(sk, VAR_MUMAX)) { m_mumax = value; }
    else if (StringMatch(sk, VAR_RHOQ)) { m_rhoq = value; }
    else if (StringMatch(sk, VAR_CH_ONCO)) { m_chOrgNCo = value; }
    else if (StringMatch(sk, VAR_CH_OPCO)) { m_chOrgPCo = value; }
    else {
        throw ModelException(MID_NUTRCH_QUAL2E, "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void NutrCH_QUAL2E::Set1DData(const char *key, int n, float *data) {
    string sk(key);
    if (StringMatch(sk, VAR_DAYLEN)) {
        if (!CheckInputCellSize(key, n)) {
            return;
        }
        m_daylen = data;
        return;
    } else if (StringMatch(sk, DataType_SolarRadiation)) {
        if (!CheckInputCellSize(key, n)) {
            return;
        }
        m_sra = data;
        return;
    } else if (StringMatch(sk, VAR_STREAM_LINK)) {
        if (!CheckInputCellSize(key, n)) {
            return;
        }
        m_streamLink = data;
        return;
    } else if (StringMatch(sk, VAR_SOTE)) {
        if (!CheckInputCellSize(key, n)) {
            return;
        }
        m_soilTemp = data;
        return;
    }

    CheckInputSize(key, n);
    if (StringMatch(sk, VAR_BKST)) { m_bankStorage = data; }
    else if (StringMatch(sk, VAR_QRECH)) { m_qOutCh = data; }
    else if (StringMatch(sk, VAR_CHST)) {
        m_chStorage = data;
    } else if (StringMatch(sk, VAR_PRECHST)) {
        m_preChStorage = data;
        for (int i = 0; i <= m_nReaches; i++) {
            // input from SetReaches(), unit is mg/L, need to be converted to kg
            float cvt_conc2amount = m_preChStorage[i] / 1000.f;
            m_chAlgae[i] *= cvt_conc2amount;
            m_chOrgN[i] *= cvt_conc2amount;
            m_chOrgP[i] *= cvt_conc2amount;
            m_chNH4[i] *= cvt_conc2amount;
            m_chNO2[i] *= cvt_conc2amount;
            m_chNO3[i] *= cvt_conc2amount;
            m_chSolP[i] *= cvt_conc2amount;
            m_chDOx[i] *= cvt_conc2amount;
            m_chCOD[i] *= cvt_conc2amount;
        }
    } else if (StringMatch(sk, VAR_CHWTDEPTH)) { m_chWTdepth = data; }
    else if (StringMatch(sk, VAR_PRECHWTDEPTH)) { m_preChWTDepth = data; }
    else if (StringMatch(sk, VAR_WATTEMP)) { m_chTemp = data; }

    else if (StringMatch(sk, VAR_LATNO3_TOCH)) { m_latNO3ToCh = data; }
    else if (StringMatch(sk, VAR_SUR_NO3_TOCH)) { m_surNO3ToCh = data; }
    else if (StringMatch(sk, VAR_SUR_NH4_TOCH)) { m_surNH4ToCh = data; }
    else if (StringMatch(sk, VAR_SUR_SOLP_TOCH)) { m_surSolPToCh = data; }
    else if (StringMatch(sk, VAR_SUR_COD_TOCH)) { m_surCodToCh = data; }
    else if (StringMatch(sk, VAR_NO3GW_TOCH)) { m_gwNO3ToCh = data; }
    else if (StringMatch(sk, VAR_MINPGW_TOCH)) { m_gwSolPToCh = data; }
    else if (StringMatch(sk, VAR_SEDORGN_TOCH)) { m_sedOrgNToCh = data; }
    else if (StringMatch(sk, VAR_SEDORGP_TOCH)) { m_sedOrgPToCh = data; }
    else if (StringMatch(sk, VAR_SEDMINPA_TOCH)) { m_sedMinPAToCh = data; }
    else if (StringMatch(sk, VAR_SEDMINPS_TOCH)) { m_sedMinPSToCh = data; }
    else if (StringMatch(sk, VAR_NO2_TOCH)) { m_no2ToCh = data; }
    else if (StringMatch(sk, VAR_RCH_DEG)) { m_chDeg = data; }
    else {
        throw ModelException(MID_NUTRCH_QUAL2E, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void NutrCH_QUAL2E::SetReaches(clsReaches *reaches) {
    if (nullptr == reaches) {
        throw ModelException(MID_NUTRCH_QUAL2E, "SetReaches", "The reaches input can not to be NULL.");
    }
    m_nReaches = reaches->GetReachNumber();

    if (nullptr == m_reachDownStream) reaches->GetReachesSingleProperty(REACH_DOWNSTREAM, &m_reachDownStream);
    if (nullptr == m_bc1) reaches->GetReachesSingleProperty(REACH_BC1, &m_bc1);
    if (nullptr == m_bc2) reaches->GetReachesSingleProperty(REACH_BC2, &m_bc2);
    if (nullptr == m_bc3) reaches->GetReachesSingleProperty(REACH_BC3, &m_bc3);
    if (nullptr == m_bc4) reaches->GetReachesSingleProperty(REACH_BC4, &m_bc4);
    if (nullptr == m_rk1) reaches->GetReachesSingleProperty(REACH_RK1, &m_rk1);
    if (nullptr == m_rk2) reaches->GetReachesSingleProperty(REACH_RK2, &m_rk2);
    if (nullptr == m_rk3) reaches->GetReachesSingleProperty(REACH_RK3, &m_rk3);
    if (nullptr == m_rk4) reaches->GetReachesSingleProperty(REACH_RK4, &m_rk4);
    if (nullptr == m_rs1) reaches->GetReachesSingleProperty(REACH_RS1, &m_rs1);
    if (nullptr == m_rs2) reaches->GetReachesSingleProperty(REACH_RS2, &m_rs2);
    if (nullptr == m_rs3) reaches->GetReachesSingleProperty(REACH_RS3, &m_rs3);
    if (nullptr == m_rs4) reaches->GetReachesSingleProperty(REACH_RS4, &m_rs4);
    if (nullptr == m_rs5) reaches->GetReachesSingleProperty(REACH_RS5, &m_rs5);
    /// these parameters' unit is mg/L now, and will be converted to kg in Set1DData.
    if (nullptr == m_chAlgae) reaches->GetReachesSingleProperty(REACH_ALGAE, &m_chAlgae);
    if (nullptr == m_chOrgN) reaches->GetReachesSingleProperty(REACH_ORGN, &m_chOrgN);
    if (nullptr == m_chOrgP) reaches->GetReachesSingleProperty(REACH_ORGP, &m_chOrgP);
    if (nullptr == m_chNH4) reaches->GetReachesSingleProperty(REACH_NH4, &m_chNH4);
    if (nullptr == m_chNO2) reaches->GetReachesSingleProperty(REACH_NO2, &m_chNO2);
    if (nullptr == m_chNO3) reaches->GetReachesSingleProperty(REACH_NO3, &m_chNO3);
    if (nullptr == m_chSolP) reaches->GetReachesSingleProperty(REACH_SOLP, &m_chSolP);
    if (nullptr == m_chDOx) reaches->GetReachesSingleProperty(REACH_DISOX, &m_chDOx);
    if (nullptr == m_chCOD) reaches->GetReachesSingleProperty(REACH_BOD, &m_chCOD);

    if (nullptr == m_chChlora) Initialize1DArray(m_nReaches + 1, m_chChlora, 0.f);
    if (nullptr == m_chTP) Initialize1DArray(m_nReaches + 1, m_chTP, 0.f);
    if (nullptr == m_chTN) Initialize1DArray(m_nReaches + 1, m_chTN, 0.f);

    m_reachUpStream = reaches->GetUpStreamIDs();
    m_reachLayers = reaches->GetReachLayers(m_layeringMethod);
}

void NutrCH_QUAL2E::SetScenario(Scenario *sce) {
    if (nullptr != sce) {
        map<int, BMPFactory *> tmpBMPFactories = sce->GetBMPFactories();
        for (auto it = tmpBMPFactories.begin(); it != tmpBMPFactories.end(); it++) {
            /// Key is uniqueBMPID, which is calculated by BMP_ID * 100000 + subScenario;
            if (it->first / 100000 == BMP_TYPE_POINTSOURCE) {
                m_ptSrcFactory[it->first] = (BMPPointSrcFactory *) it->second;
            }
        }
    } else {
        throw ModelException(MID_NUTRCH_QUAL2E, "SetScenario", "The scenario can not to be NULL.");
    }
}

void NutrCH_QUAL2E::initialOutputs() {
    if (m_nReaches <= 0) {
        throw ModelException(MID_NUTRCH_QUAL2E, "initialOutputs", "Reaches data should be set.");
    }

    if (nullptr == m_chOutAlgae) {
        m_chSatDOx = 0.f;
        Initialize1DArray(m_nReaches + 1, m_chOutChlora, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutAlgae, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutOrgN, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutOrgP, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutNH4, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutNO2, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutNO3, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutSolP, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutDOx, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutCOD, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutTN, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutTP, 0.f);

        Initialize1DArray(m_nReaches + 1, m_chOutChloraConc, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutAlgaeConc, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutOrgNConc, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutOrgPConc, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutNH4Conc, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutNO2Conc, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutNO3Conc, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutSolPConc, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutDOxConc, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutCODConc, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutTNConc, 0.f);
        Initialize1DArray(m_nReaches + 1, m_chOutTPConc, 0.f);
    }
}

void NutrCH_QUAL2E::PointSourceLoading() {
    /// initialization and reset to 0.f
    if (nullptr == m_ptNO3ToCh) {
        Initialize1DArray(m_nReaches + 1, m_ptNO3ToCh, 0.f);
        Initialize1DArray(m_nReaches + 1, m_ptNH4ToCh, 0.f);
        Initialize1DArray(m_nReaches + 1, m_ptOrgNToCh, 0.f);
        Initialize1DArray(m_nReaches + 1, m_ptTNToCh, 0.f);
        Initialize1DArray(m_nReaches + 1, m_ptSolPToCh, 0.f);
        Initialize1DArray(m_nReaches + 1, m_ptOrgPToCh, 0.f);
        Initialize1DArray(m_nReaches + 1, m_ptTPToCh, 0.f);
        Initialize1DArray(m_nReaches + 1, m_ptCODToCh, 0.f);
    } else {
        /// reset to zero for the current timestep
#pragma omp parallel for
        for (int i = 0; i <= m_nReaches; i++) {
            m_ptNO3ToCh[i] = 0.f;
            m_ptNH4ToCh[i] = 0.f;
            m_ptOrgNToCh[i] = 0.f;
            m_ptTNToCh[i] = 0.f;
            m_ptSolPToCh[i] = 0.f;
            m_ptOrgPToCh[i] = 0.f;
            m_ptTPToCh[i] = 0.f;
            m_ptCODToCh[i] = 0.f;
        }
    }
    /// load point source nutrient (kg) on current day from Scenario
    for (auto it = m_ptSrcFactory.begin(); it != m_ptSrcFactory.end(); it++) {
        //cout<<"unique Point Source Factory ID: "<<it->first<<endl;
        vector<int> m_ptSrcMgtSeqs = it->second->GetPointSrcMgtSeqs();
        map < int, PointSourceMgtParams * > m_pointSrcMgtMap = it->second->GetPointSrcMgtMap();
        vector<int> m_ptSrcIDs = it->second->GetPointSrcIDs();
        map < int, PointSourceLocations * > m_pointSrcLocsMap = it->second->GetPointSrcLocsMap();
        // 1. looking for management operations from m_pointSrcMgtMap
        for (auto seqIter = m_ptSrcMgtSeqs.begin(); seqIter != m_ptSrcMgtSeqs.end(); seqIter++) {
            PointSourceMgtParams *curPtMgt = m_pointSrcMgtMap.at(*seqIter);
            // 1.1 If current day is beyond the date range, then continue to next management
            if (curPtMgt->GetStartDate() != 0 && curPtMgt->GetEndDate() != 0) {
                if (m_date < curPtMgt->GetStartDate() || m_date > curPtMgt->GetEndDate()) {
                    continue;
                }
            }
            // 1.2 Otherwise, get the nutrient concentration, mg/L
            float per_wtr = curPtMgt->GetWaterVolume();
            float per_no3 = curPtMgt->GetNO3();
            float per_nh4 = curPtMgt->GetNH4();
            float per_orgn = curPtMgt->GetOrgN();
            float per_solp = curPtMgt->GetSolP();
            float per_orgP = curPtMgt->GetOrgP();
            float per_cod = curPtMgt->GetCOD();
            // 1.3 Sum up all point sources
            for (auto locIter = m_ptSrcIDs.begin(); locIter != m_ptSrcIDs.end(); locIter++) {
                if (m_pointSrcLocsMap.find(*locIter) != m_pointSrcLocsMap.end()) {
                    PointSourceLocations *curPtLoc = m_pointSrcLocsMap.at(*locIter);
                    int curSubID = curPtLoc->GetSubbasinID();
                    float cvt = per_wtr * curPtLoc->GetSize() / 1000.f * m_dt / 86400.f;/// mg/L ==> kg / timestep
                    m_ptNO3ToCh[curSubID] += per_no3 * cvt;
                    m_ptNH4ToCh[curSubID] += per_nh4 * cvt;
                    m_ptOrgNToCh[curSubID] += per_orgn * cvt;
                    m_ptOrgPToCh[curSubID] += per_orgP * cvt;
                    m_ptSolPToCh[curSubID] += per_solp * cvt;
                    m_ptCODToCh[curSubID] += per_cod * cvt;
                    m_ptTNToCh[curSubID] += (per_no3 + per_nh4 + per_orgn) * cvt;
                    m_ptTPToCh[curSubID] += (per_solp + per_orgP) * cvt;
                }
            }
        }
    }
    // sum up point sources loadings of all subbasins
    for (int i = 1; i <= m_nReaches; i++) {
        m_ptTNToCh[0] += m_ptTNToCh[i];
        m_ptTPToCh[0] += m_ptTPToCh[i];
        m_ptCODToCh[0] += m_ptCODToCh[i];
    }
}

int NutrCH_QUAL2E::Execute() {
    CheckInputData();
    initialOutputs();
    // load point source loadings from Scenarios
    PointSourceLoading();
    // Calculate average day length, solar radiation, and temperature for each channel
    ParametersSubbasinForChannel();

    for (auto it = m_reachLayers.begin(); it != m_reachLayers.end(); it++) {
        // There are not any flow relationship within each routing layer.
        // So parallelization can be done here.
        int reachNum = it->second.size();
        // the size of m_reachLayers (map) is equal to the maximum stream order
#pragma omp parallel for
        for (int i = 0; i < reachNum; i++) {
            // index in the array
            int reachIndex = it->second[i];
            NutrientTransform(reachIndex);
            AddInputNutrient(reachIndex);
            RouteOut(reachIndex);
        }
    }
    //cout<<"NUTR_QUAL2E, surNO3ToCh: "<<m_surNO3ToCh[12]<<", gwno3ToCh: "<<m_gwNO3ToCh[12]<<", ptNo3ToCh: "<<m_ptNO3ToCh[12]
    //<<", chOutNO3: "<<m_chOutNO3[12]<<", chOutNO3Conc: "<<m_chOutTNConc[12]<<", TN: "<<m_chOutTN[12]<<", TNConc: "<<m_chOutTNConc[12]<<endl;
    //cout<<"chStr_NO3: "<<m_chNO3[12]<<", chStr_NH4: "<<m_chNH4[12]<<", chStr_TN: "<<m_chTN[12]<<endl;
    return 0;
}

void NutrCH_QUAL2E::AddInputNutrient(int i) {
    //cout<<"subID: "<<i<<", initial nh4: "<<m_chNH4[i]<<", ";
    //cout<<"subID: "<<i<<", initial cod: "<<m_chCOD[i]<<", ";
    /// nutrient amount from upstream routing will be accumulated to current storage
    for (size_t j = 0; j < m_reachUpStream[i].size(); ++j) {
        int upReachId = m_reachUpStream[i][j];
        //cout<<"upSubID: "<<upReachId<<", "<<m_chOutNH4[upReachId]<<", ";
        m_chOrgN[i] += m_chOutOrgN[upReachId];
        m_chNO3[i] += m_chOutNO3[upReachId];
        m_chNO2[i] += m_chOutNO2[upReachId];
        m_chNH4[i] += m_chOutNH4[upReachId];
        m_chOrgP[i] += m_chOutOrgP[upReachId];
        m_chSolP[i] += m_chOutSolP[upReachId];
        m_chCOD[i] += m_chOutCOD[upReachId];
        m_chDOx[i] += m_chOutDOx[upReachId];
        m_chChlora[i] += m_chOutChlora[upReachId];
        m_chAlgae[i] += m_chOutAlgae[upReachId];
    }
    //cout<<", added upstream, nh4: "<<m_chNH4[i]<<endl;
    //cout<<", added upstream, cod: "<<m_chCOD[i]<<", ";
    /// absorbed organic N, P from overland sediment routing
    m_chOrgN[i] += m_sedOrgNToCh[i];
    m_chOrgP[i] += m_sedOrgPToCh[i];
    /// organic N, P contribution from channel erosion
    if (nullptr != m_chDeg && m_chOrgPCo != NODATA_VALUE && m_chOrgNCo != NODATA_VALUE) {
        m_chOrgN[i] += m_chDeg[i] * m_chOrgNCo / 1000.f;
        m_chOrgP[i] += m_chDeg[i] * m_chOrgPCo / 1000.f;
    }
    /// dissolved N, P from overland surface flow routing and groundwater
    m_chNO3[i] += m_surNO3ToCh[i] + m_latNO3ToCh[i] + m_gwNO3ToCh[i];
    if (nullptr != m_surNH4ToCh && m_surNH4ToCh[i] > 0.f) m_chNH4[i] += m_surNH4ToCh[i];
    m_chSolP[i] += m_surSolPToCh[i] + m_gwSolPToCh[i];

    // if(nullptr != m_nh4ToCh && m_nh4ToCh[i] > 0.f) m_chNH4[i] += m_nh4ToCh[i];
    if (nullptr != m_no2ToCh && m_no2ToCh[i] > 0.f) m_chNO2[i] += m_no2ToCh[i];
    if (nullptr != m_surCodToCh && m_surCodToCh[i] > 0.f) {
        m_chCOD[i] += m_surCodToCh[i];
        //cout<<", added surface, cod: "<<m_chCOD[i]<<", ";
    }
    /// add point source loadings to channel
    if (nullptr != m_ptNO3ToCh && m_ptNO3ToCh[i] > 0.f) m_chNO3[i] += m_ptNO3ToCh[i];
    if (nullptr != m_ptNH4ToCh && m_ptNH4ToCh[i] > 0.f) m_chNH4[i] += m_ptNH4ToCh[i];
    if (nullptr != m_ptOrgNToCh && m_ptOrgNToCh[i] > 0.f) m_chOrgN[i] += m_ptOrgNToCh[i];
    if (nullptr != m_ptSolPToCh && m_ptSolPToCh[i] > 0.f) m_chSolP[i] += m_ptSolPToCh[i];
    if (nullptr != m_ptOrgPToCh && m_ptOrgPToCh[i] > 0.f) m_chOrgP[i] += m_ptOrgPToCh[i];
    if (nullptr != m_ptCODToCh && m_ptCODToCh[i] > 0.f) {
        m_chCOD[i] += m_ptCODToCh[i];
        //cout<<", added point source, cod: "<<m_chCOD[i]<<endl;
    }
}

void NutrCH_QUAL2E::RouteOut(int i) {
    // reinitialize out variables to 0
    m_chOutAlgae[i] = 0.f;
    m_chOutAlgaeConc[i] = 0.f;
    m_chOutChlora[i] = 0.f;
    m_chOutChloraConc[i] = 0.f;
    m_chOutOrgN[i] = 0.f;
    m_chOutOrgNConc[i] = 0.f;
    m_chOutNH4[i] = 0.f;
    m_chOutNH4Conc[i] = 0.f;
    m_chOutNO2[i] = 0.f;
    m_chOutNO2Conc[i] = 0.f;
    m_chOutNO3[i] = 0.f;
    m_chOutNO3Conc[i] = 0.f;
    m_chOutOrgP[i] = 0.f;
    m_chOutOrgPConc[i] = 0.f;
    m_chOutSolP[i] = 0.f;
    m_chOutSolPConc[i] = 0.f;
    m_chOutCOD[i] = 0.f;
    m_chOutCODConc[i] = 0.f;
    m_chOutDOx[i] = 0.f;
    m_chOutDOxConc[i] = 0.f;
    m_chOutTN[i] = 0.f;
    m_chOutTNConc[i] = 0.f;
    m_chOutTP[i] = 0.f;
    m_chOutTPConc[i] = 0.f;
    //get out flow water fraction
    float wtrOut = m_qOutCh[i] * m_dt; // m**3
    // float wtrTotal = wtrOut + m_chStorage[i];
    float wtrTotal = m_preChStorage[i];
    if (wtrTotal <= 0.f)//wtrOut <= 0.f ||  || m_chWTdepth[i] <= 0.f)
    {
        // return with initialized values directly
        return;
    }
    float outFraction = wtrOut / wtrTotal;
    //if(i == 12) cout << "wtrOut: " << wtrOut << ", m_chStorage: " << m_chStorage[i] << ", outFrac: "<<outFraction<<endl;
    if (outFraction >= 1.f) outFraction = 1.f;
    if (outFraction <= UTIL_ZERO) outFraction = UTIL_ZERO;
    m_chOutOrgN[i] = m_chOrgN[i] * outFraction;
    m_chOutNO3[i] = m_chNO3[i] * outFraction;
    m_chOutNO2[i] = m_chNO2[i] * outFraction;
    m_chOutNH4[i] = m_chNH4[i] * outFraction;
    m_chOutOrgP[i] = m_chOrgP[i] * outFraction;
    m_chOutSolP[i] = m_chSolP[i] * outFraction;
    m_chOutCOD[i] = m_chCOD[i] * outFraction;
    m_chOutDOx[i] = m_chDOx[i] * outFraction;
    m_chOutAlgae[i] = m_chAlgae[i] * outFraction;
    m_chOutChlora[i] = m_chChlora[i] * outFraction;
    m_chOutTN[i] = m_chOutOrgN[i] + m_chOutNH4[i] + m_chOutNO2[i] + m_chOutNO3[i];
    m_chOutTP[i] = m_chOutOrgP[i] + m_chOutSolP[i];
    //if(i == 12) cout << "m_chOutOrgP: " << m_chOutOrgP[i] << ", m_chOrgP: " << m_chOrgP[i] << ", outFrac: "<<outFraction<<endl;
    // kg ==> mg/L
    float cvt = 1000.f / wtrOut;
    m_chOutOrgNConc[i] = m_chOutOrgN[i] * cvt;
    m_chOutNO3Conc[i] = m_chOutNO3[i] * cvt;
    m_chOutNO2Conc[i] = m_chOutNO2[i] * cvt;
    m_chOutNH4Conc[i] = m_chOutNH4[i] * cvt;
    m_chOutOrgPConc[i] = m_chOutOrgP[i] * cvt;
    m_chOutSolPConc[i] = m_chOutSolP[i] * cvt;
    m_chOutCODConc[i] = m_chOutCOD[i] * cvt;
    m_chOutDOxConc[i] = m_chOutDOx[i] * cvt;
    m_chOutAlgaeConc[i] = m_chOutAlgae[i] * cvt;
    m_chOutChloraConc[i] = m_chOutChlora[i] * cvt;
    /// total N and total P
    m_chOutTNConc[i] = m_chOutTN[i] * cvt;
    m_chOutTPConc[i] = m_chOutTP[i] * cvt;

    m_chNO3[i] -= m_chOutNO3[i];
    m_chNO2[i] -= m_chOutNO2[i];
    m_chNH4[i] -= m_chOutNH4[i];
    m_chOrgN[i] -= m_chOutOrgN[i];
    m_chOrgP[i] -= m_chOutOrgP[i];
    m_chSolP[i] -= m_chOutSolP[i];
    m_chCOD[i] -= m_chOutCOD[i];
    m_chDOx[i] -= m_chOutDOx[i];
    m_chAlgae[i] -= m_chOutAlgae[i];
    m_chChlora[i] -= m_chOutChlora[i];
    // recalculate TN TP stored in reach
    m_chTN[i] = m_chOrgN[i] + m_chNH4[i] + m_chNO2[i] + m_chNO3[i];
    m_chTP[i] = m_chOrgP[i] + m_chSolP[i];

    /// in case of zero
    if (m_chNO3[i] < UTIL_ZERO) m_chNO3[i] = UTIL_ZERO;
    if (m_chNO2[i] < UTIL_ZERO) m_chNO2[i] = UTIL_ZERO;
    if (m_chNH4[i] < UTIL_ZERO) m_chNH4[i] = UTIL_ZERO;
    if (m_chOrgN[i] < UTIL_ZERO) m_chOrgN[i] = UTIL_ZERO;
    if (m_chOrgP[i] < UTIL_ZERO) m_chOrgP[i] = UTIL_ZERO;
    if (m_chSolP[i] < UTIL_ZERO) m_chSolP[i] = UTIL_ZERO;
    if (m_chCOD[i] < UTIL_ZERO) m_chCOD[i] = UTIL_ZERO;
    if (m_chDOx[i] < UTIL_ZERO) m_chDOx[i] = UTIL_ZERO;
    if (m_chAlgae[i] < UTIL_ZERO) m_chAlgae[i] = UTIL_ZERO;
    if (m_chChlora[i] < UTIL_ZERO) m_chChlora[i] = UTIL_ZERO;
}

void NutrCH_QUAL2E::NutrientTransform(int i) {
    float thbc1 = 1.083f;    ///temperature adjustment factor for local biological oxidation of NH3 to NO2
    float thbc2 = 1.047f;    ///temperature adjustment factor for local biological oxidation of NO2 to NO3
    float thbc3 = 1.04f;    ///temperature adjustment factor for local hydrolysis of organic N to ammonia N
    float thbc4 = 1.047f;    ///temperature adjustment factor for local decay of organic P to dissolved P

    float thgra = 1.047f;    ///temperature adjustment factor for local algal growth rate
    float thrho = 1.047f;    ///temperature adjustment factor for local algal respiration rate

    float thm_rk1 = 1.047f;    ///temperature adjustment factor for local CBOD deoxygenation
    float thm_rk2 = 1.024f;    ///temperature adjustment factor for local oxygen reaeration rate
    float thm_rk3 = 1.024f;    ///temperature adjustment factor for loss of CBOD due to settling
    float thm_rk4 = 1.060f;    ///temperature adjustment factor for local sediment oxygen demand

    float thrs1 = 1.024f;    ///temperature adjustment factor for local algal settling rate
    float thrs2 = 1.074f;    ///temperature adjustment factor for local benthos source rate for dissolved phosphorus
    float thrs3 = 1.074f;    ///temperature adjustment factor for local benthos source rate for ammonia nitrogen
    float thrs4 = 1.024f;    ///temperature adjustment factor for local organic N settling rate
    float thrs5 = 1.024f;    ///temperature adjustment factor for local organic P settling rate
    // Currently, by junzhi
    // assume the water volume used to contain nutrients at current time step equals to :
    //     flowout plus the storage at the end of day (did not consider the nutrients
    //     from stream to groundwater through seepage and bank storage)
    // float wtrOut = m_qOutCh[i] * m_dt;
    // float wtrTotal = wtrOut + m_chStorage[i]; /// m3
    float wtrTotal = m_preChStorage[i]; // by LJ
    float tmpChWtDepth = m_preChWTDepth[i]; /// m
    if (tmpChWtDepth <= 0.01f) {
        tmpChWtDepth = 0.01f;
    }
    if (wtrTotal <= 0.f)/// which means no flow out of current channel    || wtrOut <= 0.f
    {
        m_chAlgae[i] = 0.f;
        m_chChlora[i] = 0.f;
        m_chOrgN[i] = 0.f;
        m_chNH4[i] = 0.f;
        m_chNO2[i] = 0.f;
        m_chNO3[i] = 0.f;
        m_chTN[i] = 0.f;
        m_chOrgP[i] = 0.f;
        m_chSolP[i] = 0.f;
        m_chTP[i] = 0.f;
        m_chDOx[i] = 0.f;
        m_chCOD[i] = 0.f;
        m_chSatDOx = 0.f;
        return;// return and route out with 0.f
    }
    // initial algal biomass concentration in reach (algcon mg/L, i.e. g/m3)   kg ==> mg/L
    float cvt_amout2conc = 1000.f / wtrTotal;
    float algcon = cvt_amout2conc * m_chAlgae[i];
    // initial organic N concentration in reach (orgncon mg/L)
    float orgncon = cvt_amout2conc * m_chOrgN[i];
    // initial ammonia concentration in reach (nh4con mg/L)
    float nh4con = cvt_amout2conc * m_chNH4[i];
    // initial nitrite concentration in reach (no2con mg/L)
    float no2con = cvt_amout2conc * m_chNO2[i];
    // initial nitrate concentration in reach (no3con mg/L)
    float no3con = cvt_amout2conc * m_chNO3[i];
    // initial organic P concentration in reach (orgpcon  mg/L)
    float orgpcon = cvt_amout2conc * m_chOrgP[i];
    // initial soluble P concentration in reach (solpcon mg/L)
    float solpcon = cvt_amout2conc * m_chSolP[i];
    // initial carbonaceous biological oxygen demand (cbodcon mg/L)
    float cbodcon = cvt_amout2conc * m_chCOD[i];
    // initial dissolved oxygen concentration in reach (o2con mg/L)
    float o2con = cvt_amout2conc * m_chDOx[i];

    // temperature of water in reach (wtmp deg C)
    float wtmp = max(m_chTemp[i], 0.1f);
    // calculate effective concentration of available nitrogen (cinn), QUAL2E equation III-15
    float cinn = nh4con + no3con;

    // calculate saturation concentration for dissolved oxygen, QUAL2E section 3.6.1 equation III-29
    // variable to hold intermediate calculation result
    float ww = -139.34410f + (1.575701e+05f / (wtmp + 273.15f));
    float xx = 6.642308e+07f / pow((wtmp + 273.15f), 2.f);
    float yy = 1.243800e+10f / pow((wtmp + 273.15f), 3.f);
    float zz = 8.621949e+11f / pow((wtmp + 273.15f), 4.f);
    m_chSatDOx = 0.f;
    m_chSatDOx = exp(ww - xx + yy - zz);
    if (m_chSatDOx < 1.e-6f) {
        m_chSatDOx = 0.f;
    }

    // O2 impact calculations
    // calculate nitrification rate correction factor for low oxygen QUAL2E equation III-21(cordo)
    float cordo = 0.f;
    float o2con2 = o2con;
    if (o2con2 <= 0.1f) {
        o2con2 = 0.1f;
    }
    if (o2con2 > 30.f) {
        o2con2 = 30.f;
    }
    cordo = 1.f - exp(-0.6f * o2con2);
    if (o2con <= 0.001f) {
        o2con = 0.001f;
    }
    if (o2con > 30.f) {
        o2con = 30.f;
    }
    cordo = 1.f - exp(-0.6f * o2con);


    // modify ammonia and nitrite oxidation rates to account for low oxygen
    // rate constant for biological oxidation of NH3 to NO2 modified to reflect impact of low oxygen concentration (bc1mod)
    float bc1mod = 0.f;
    // rate constant for biological oxidation of NO2 to NO3 modified to reflect impact of low oxygen concentration (bc1mod)
    float bc2mod = 0.f;
    bc1mod = m_bc1[i] * cordo;
    bc2mod = m_bc2[i] * cordo;

    //	tday is the calculation time step = 1 day
    float tday = 1.f;

    // algal growth
    // calculate light extinction coefficient (lambda)
    float lambda = 0.f;
    if (m_ai0 * algcon > 1.e-6f) {
        lambda = m_lambda0 + (m_lambda1 * m_ai0 * algcon) + m_lambda2 * pow((m_ai0 * algcon), 0.66667f);
    } else {
        lambda = m_lambda0;
    }
    if (lambda > m_lambda0) lambda = m_lambda0;
    // calculate algal growth limitation factors for nitrogen and phosphorus, QUAL2E equations III-13 & III-14
    float fnn = 0.f;
    float fpp = 0.f;
    fnn = cinn / (cinn + m_k_n);
    fpp = solpcon / (solpcon + m_k_p);

    // calculate daylight average, photo synthetically active (algi)
    float algi = 0.f;
    if (m_chDaylen[i] > 0.f) {
        algi = m_chSr[i] * tfact / m_chDaylen[i];
    } else {
        algi = 0.00001f;
    }

    // calculate growth attenuation factor for light, based on daylight average light intensity
    float fl_1 = 0.f;
    float fll = 0.f;
    fl_1 = (1.f / (lambda * tmpChWtDepth)) *
        log((m_k_l + algi) / (m_k_l + algi * exp(-lambda * tmpChWtDepth)));

    fll = 0.92f * (m_chDaylen[i] / 24.f) * fl_1;

    // temporary variables
    float gra = 0.f;
    //float dchla = 0.f;
    float dbod = 0.f;
    float ddisox = 0.f;
    float dorgn = 0.f;
    float dnh4 = 0.f;
    float dno2 = 0.f;
    float dno3 = 0.f;
    float dorgp = 0.f;
    float dsolp = 0.f;
    switch (igropt) {
        case 1:
            // multiplicative
            gra = m_mumax * fll * fnn * fpp;
        case 2:
            // limiting nutrient
            gra = m_mumax * fll * min(fnn, fpp);
        case 3:
            // harmonic mean
            if (fnn > 1.e-6f && fpp > 1.e-6f) {
                gra = m_mumax * fll * 2.f / ((1.f / fnn) + (1.f / fpp));
            } else {
                gra = 0.f;
            }
        default:break;
    }

    // calculate algal biomass concentration at end of day (phytoplanktonic algae), QUAL2E equation III-2
    float dalgae = 0.f;
    float setl = min(1.f, corTempc(m_rs1[i], thrs1, wtmp) / tmpChWtDepth);
    dalgae = algcon +
        (corTempc(gra, thgra, wtmp) * algcon - corTempc(m_rhoq, thrho, wtmp) * algcon - setl * algcon) * tday;
    if (dalgae < 1.e-6f) {
        dalgae = 1.e-6f;
    }
    float dcoef = 3.f;
    /// set algae limit (watqual.f)
    if (dalgae > 5000.f) dalgae = 5000.f;
    if (dalgae > dcoef * algcon) dalgae = dcoef * algcon;
    // calculate chlorophyll-a concentration at end of day, QUAL2E equation III-1
    //dchla = dalgae * m_ai0 / 1000.f;

    // oxygen calculations
    // calculate carbonaceous biological oxygen demand at end of day (dbod)
    float yyy = 0.f;
    float zzz = 0.f;
    //1. COD convert to CBOD
    //if(i == 12) cout << "pre_cod, mg/L: " << cbodcon << ", ";
    cbodcon /= (m_cod_n * (1.f - exp(-5.f * m_cod_k)));
    //if(i == 12) cout << "pre_cbod, mg/L: " << cbodcon << ", ";
    yyy = corTempc(m_rk1[i], thm_rk1, wtmp) * cbodcon;
    zzz = corTempc(m_rk3[i], thm_rk3, wtmp) * cbodcon;
    dbod = 0.f;
    dbod = cbodcon - (yyy + zzz) * tday;
    /********* watqual.f code ***********/
    //float coef = 0.f;
    ///// deoxygenation rate
    //coef = exp(-1.f * corTempc(m_rk1[i], thm_rk1, wtmp)*tday);
    //float tmp = coef * cbodcon;
    //// cbod rate loss due to setting
    //coef = exp(-1.f * corTempc(m_rk3[i], thm_rk1, wtmp)*tday);
    //tmp *= coef;
    //dbod = tmp;
    ////if(i == 12) cout << "trans_cbod, mg/L: " << dbod << ", ";
    //if (dbod < 1.e-6f) dbod = 1.e-6f;
    //if (dbod > dcoef * cbodcon) dbod = dcoef * cbodcon;
    /********* watqual2.f code ***********/
    if (dbod < 1.e-6f) dbod = 1.e-6f;
    //if(i == 12) cout << "trans_cbod2, mg/L: " << dbod << ", ";
    //2. CBOD convert to COD, now dbod is COD
    dbod *= m_cod_n * (1.f - exp(-5.f * m_cod_k));
    //if(i == 12) cout << "cod: " << dbod << endl;

    // calculate dissolved oxygen concentration if reach at end of day (ddisox)
    float uu = 0.f;     // variable to hold intermediate calculation result
    float vv = 0.f;      // variable to hold intermediate calculation result
    ww = 0.f;    // variable to hold intermediate calculation result
    xx = 0.f;     // variable to hold intermediate calculation result
    yy = 0.f;     // variable to hold intermediate calculation result
    zz = 0.f;     // variable to hold intermediate calculation result

    //m_rk2[i] =1.f;	// Why define this value?

    //float hh = corTempc(m_rk2[i], thm_rk2, wtmp);
    uu = corTempc(m_rk2[i], thm_rk2, wtmp) * (m_chSatDOx - o2con);
    //vv = (m_ai3 * corTempc(gra, thgra, wtmp) - m_ai4 * corTempc(m_rhoq, thrho, wtmp)) * algcon;
    if (algcon > 0.001f) {
        vv = (m_ai3 * corTempc(gra, thgra, wtmp) - m_ai4 * corTempc(m_rhoq, thrho, wtmp)) * algcon;
    } else {
        algcon = 0.001f;
    }

    ww = corTempc(m_rk1[i], thm_rk1, wtmp) * cbodcon;
    if (tmpChWtDepth > 0.001f) {
        xx = corTempc(m_rk4[i], thm_rk4, wtmp) / (tmpChWtDepth * 1000.f);
    }
    if (nh4con > 0.001f) {
        yy = m_ai5 * corTempc(bc1mod, thbc1, wtmp) * nh4con;
    } else {
        nh4con = 0.001f;
    }
    if (no2con > 0.001f) {
        zz = m_ai6 * corTempc(bc2mod, thbc2, wtmp) * no2con;
    } else {
        no2con = 0.001f;
    }
    ddisox = 0.f;
    ddisox = o2con + (uu + vv - ww - xx - yy - zz) * tday;
    //o2proc = o2con - ddisox;   // not found variable "o2proc"
    if (ddisox < 0.1f || ddisox != ddisox) {
        ddisox = 0.1f;
    }
    // algea O2 production minus respiration
    //float doxrch = m_chSatDOx;
    // cbod deoxygenation
    //coef = exp(-0.1f * ww);
    //doxrch *= coef;
    // benthic sediment oxidation
    //coef = 1.f - corTempc(m_rk4[i], thm_rk4, wtmp) / 100.f;
    //doxrch *= coef;
    // ammonia oxydation
    //coef = exp(-0.05f * yy);
    //doxrch *= coef;
    // nitrite oxydation
    //coef = exp(-0.05f * zz);
    //doxrch *= coef;
    // reaeration
    //uu = corTempc(m_rk2[i], thm_rk2, wtmp) / 100.f * (m_chSatDOx - doxrch);
    //ddisox = doxrch + uu;
    //if (ddisox < 1.e-6f) ddisox = 0.f;
    //if (ddisox > m_chSatDOx) ddisox = m_chSatDOx;
    //if (ddisox > dcoef * o2con) ddisox = dcoef * o2con;
    //////end oxygen calculations//////
    // nitrogen calculations
    // calculate organic N concentration at end of day (dorgn)
    xx = 0.f;
    yy = 0.f;
    zz = 0.f;
    xx = m_ai1 * corTempc(m_rhoq, thrho, wtmp) * algcon;
    yy = corTempc(m_bc3[i], thbc3, wtmp) * orgncon;
    zz = corTempc(m_rs4[i], thrs4, wtmp) * orgncon;
    dorgn = 0.f;
    dorgn = orgncon + (xx - yy - zz) * tday;
    if (dorgn < 1.e-6f) dorgn = 0.f;
    if (dorgn > dcoef * orgncon) dorgn = dcoef * orgncon;
    // calculate fraction of algal nitrogen uptake from ammonia pool
    float f1 = 0.f;
    f1 = m_p_n * nh4con / (m_p_n * nh4con + (1.f - m_p_n) * no3con + 1.e-6f);

    //cout<<"subID: "<<i<<", initial nh4 conc: "<<nh4con<<", "<<"initial orgn: "<<orgncon<<", ";
    // calculate ammonia nitrogen concentration at end of day (dnh4)
    ww = 0.f;
    xx = 0.f;
    yy = 0.f;
    zz = 0.f;
    ww = corTempc(m_bc3[i], thbc3, wtmp) * orgncon;
    xx = corTempc(bc1mod, thbc1, wtmp) * nh4con;
    yy = corTempc(m_rs3[i], thrs3, wtmp) / (tmpChWtDepth * 1000.f);
    zz = f1 * m_ai1 * algcon * corTempc(gra, thgra, wtmp);
    dnh4 = 0.f;
    dnh4 = nh4con + (ww - xx + yy - zz) * tday;
    if (dnh4 < 1.e-6f) dnh4 = 0.f;
    if (dnh4 > dcoef * nh4con && nh4con > 0.f) {
        dnh4 = dcoef * nh4con;
    }
    //if(i == 12) cout<<"orgncon: "<<orgncon<<", ��: "<<corTempc(m_bc3[i], thbc3, wtmp) <<", xx: "<<xx<<", yy: "<<yy<<", zz: "<<zz<<",\n nh4 out: "<<dnh4<<endl;
    // calculate concentration of nitrite at end of day (dno2)
    yy = 0.f;
    zz = 0.f;
    yy = corTempc(bc1mod, thbc1, wtmp) * nh4con;
    zz = corTempc(bc2mod, thbc2, wtmp) * no2con;
    dno2 = 0.f;
    dno2 = no2con + (yy - zz) * tday;
    if (dno2 < 1.e-6f) dno2 = 0.f;
    if (dno2 > dcoef * no2con && no2con > 0.f) {
        dno2 = dcoef * no2con;
    }
    // calculate nitrate concentration at end of day (dno3)
    yy = 0.f;
    zz = 0.f;
    yy = corTempc(bc2mod, thbc2, wtmp) * no2con;
    zz = (1.f - f1) * m_ai1 * algcon * corTempc(gra, thgra, wtmp);
    dno3 = 0.f;
    dno3 = no3con + (yy - zz) * tday;
    if (dno3 < 1.e-6) dno3 = 0.f;
    /////end nitrogen calculations//////
    // phosphorus calculations
    // calculate organic phosphorus concentration at end of day
    xx = 0.f;
    yy = 0.f;
    zz = 0.f;
    xx = m_ai2 * corTempc(m_rhoq, thrho, wtmp) * algcon;
    yy = corTempc(m_bc4[i], thbc4, wtmp) * orgpcon;
    zz = corTempc(m_rs5[i], thrs5, wtmp) * orgpcon;
    dorgp = 0.f;
    dorgp = orgpcon + (xx - yy - zz) * tday;
    if (dorgp < 1.e-6f) dorgp = 0.f;
    if (dorgp > dcoef * orgpcon) {
        dorgp = dcoef * orgpcon;
    }

    // calculate dissolved phosphorus concentration at end of day (dsolp)
    xx = 0.f;
    yy = 0.f;
    zz = 0.f;
    xx = corTempc(m_bc4[i], thbc4, wtmp) * orgpcon;
    yy = corTempc(m_rs2[i], thrs2, wtmp) / (tmpChWtDepth * 1000.f);
    zz = m_ai2 * corTempc(gra, thgra, wtmp) * algcon;
    dsolp = 0.f;
    dsolp = solpcon + (xx + yy - zz) * tday;
    if (dsolp < 1.e-6) dsolp = 0.f;
    if (dsolp > dcoef * solpcon) {
        dsolp = dcoef * solpcon;
    }
    /////////end phosphorus calculations/////////
    // storage amount (kg) at end of day
    m_chAlgae[i] = dalgae * wtrTotal / 1000.f;
    m_chChlora[i] = m_chAlgae[i] * m_ai0;
    m_chOrgN[i] = dorgn * wtrTotal / 1000.f;
    m_chNH4[i] = dnh4 * wtrTotal / 1000.f;
    m_chNO2[i] = dno2 * wtrTotal / 1000.f;
    m_chNO3[i] = dno3 * wtrTotal / 1000.f;
    m_chOrgP[i] = dorgp * wtrTotal / 1000.f;
    m_chSolP[i] = dsolp * wtrTotal / 1000.f;
    m_chCOD[i] = dbod * wtrTotal / 1000.f;
    //if(i == 12) cout << "chCOD: " << m_chCOD[i] << endl;
    m_chDOx[i] = ddisox * wtrTotal / 1000.f;
}

float NutrCH_QUAL2E::corTempc(float r20, float thk, float tmp) {
    float theta = 0.f;
    theta = r20 * pow(thk, (tmp - 20.f));
    return theta;
}

void NutrCH_QUAL2E::GetValue(const char *key, float *value) {
    string sk(key);
    if (StringMatch(sk, VAR_SOXY)) {
        *value = m_chSatDOx;
    }
}

void NutrCH_QUAL2E::Get1DData(const char *key, int *n, float **data) {
    initialOutputs();
    string sk(key);
    *n = m_nReaches + 1;
    if (StringMatch(sk, VAR_CH_ALGAE)) { *data = m_chOutAlgae; }
    else if (StringMatch(sk, VAR_CH_ALGAEConc)) { *data = m_chOutAlgaeConc; }
    else if (StringMatch(sk, VAR_CH_NO2)) { *data = m_chOutNO2; }
    else if (StringMatch(sk, VAR_CH_NO2Conc)) { *data = m_chOutNO2Conc; }
    else if (StringMatch(sk, VAR_CH_COD)) { *data = m_chOutCOD; }
    else if (StringMatch(sk, VAR_CH_CODConc)) { *data = m_chOutCODConc; }
    else if (StringMatch(sk, VAR_CH_CHLORA)) { *data = m_chOutChlora; }
    else if (StringMatch(sk, VAR_CH_CHLORAConc)) { *data = m_chOutChloraConc; }
    else if (StringMatch(sk, VAR_CH_NO3)) { *data = m_chOutNO3; }
    else if (StringMatch(sk, VAR_CH_NO3Conc)) { *data = m_chOutNO3Conc; }
    else if (StringMatch(sk, VAR_CH_SOLP)) { *data = m_chOutSolP; }
    else if (StringMatch(sk, VAR_CH_SOLPConc)) { *data = m_chOutSolPConc; }
    else if (StringMatch(sk, VAR_CH_ORGN)) { *data = m_chOutOrgN; }
    else if (StringMatch(sk, VAR_CH_ORGNConc)) { *data = m_chOutOrgNConc; }
    else if (StringMatch(sk, VAR_CH_ORGP)) { *data = m_chOutOrgP; }
    else if (StringMatch(sk, VAR_CH_ORGPConc)) { *data = m_chOutOrgPConc; }
    else if (StringMatch(sk, VAR_CH_NH4)) { *data = m_chOutNH4; }
    else if (StringMatch(sk, VAR_CH_NH4Conc)) { *data = m_chOutNH4Conc; }
    else if (StringMatch(sk, VAR_CH_DOX)) { *data = m_chOutDOx; }
    else if (StringMatch(sk, VAR_CH_DOXConc)) { *data = m_chOutDOxConc; }
    else if (StringMatch(sk, VAR_CH_TN)) { *data = m_chOutTN; }
    else if (StringMatch(sk, VAR_CH_TNConc)) { *data = m_chOutTNConc; }
    else if (StringMatch(sk, VAR_CH_TP)) { *data = m_chOutTP; }
    else if (StringMatch(sk, VAR_CH_TPConc)) { *data = m_chOutTPConc; }
    else if (StringMatch(sk, VAR_PTTN2CH)) { *data = m_ptTNToCh; }
    else if (StringMatch(sk, VAR_PTTP2CH)) { *data = m_ptTPToCh; }
    else if (StringMatch(sk, VAR_PTCOD2CH)) {
        *data = m_ptCODToCh;
        /// output nutrient storage in channel
    } else if (StringMatch(sk, VAR_CHSTR_NO3)) { *data = m_chNO3; }
    else if (StringMatch(sk, VAR_CHSTR_NH4)) { *data = m_chNH4; }
    else if (StringMatch(sk, VAR_CHSTR_TN)) { *data = m_chTN; }
    else if (StringMatch(sk, VAR_CHSTR_TP)) { *data = m_chTP; }
    else {
        throw ModelException(MID_NUTRCH_QUAL2E, "Get1DData", "Parameter " + sk + " does not exist.");
    }
}
