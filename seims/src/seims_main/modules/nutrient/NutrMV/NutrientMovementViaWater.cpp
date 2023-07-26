#include "NutrientMovementViaWater.h"

#include "text.h"
#include "NutrientCommon.h"

NutrientMovementViaWater::NutrientMovementViaWater() :
    m_cellWth(-1.), m_cellArea(-1.), m_nCells(-1), m_nSoilLyrs(nullptr), m_maxSoilLyrs(-1),
    m_rchID(nullptr), m_cbnModel(-1),
    m_cvtWt(nullptr), m_qtile(-1.), m_phoskd(-1.),
    m_pperco(-1.), m_nperco(-1.), m_cod_n(-1.), m_cod_k(-1.),
    m_olWtrEroSed(nullptr), m_anionExclFr(nullptr),
    m_surfRf(nullptr), m_isep_opt(-1), m_drainLyr(nullptr),
    m_soilCrk(nullptr), m_distToRch(nullptr), m_soilSat(nullptr), m_subSurfRf(nullptr),
    m_soilPerco(nullptr), m_soilBD(nullptr),
    m_soilDepth(nullptr), m_flowOutIdx(nullptr), m_flowOutFrac(nullptr),
    m_rteLyrs(nullptr), m_nRteLyrs(-1),
    m_sedorgn(nullptr), m_meanTemp(nullptr), m_soilCbn(nullptr), m_soilThk(nullptr), m_latNO3(nullptr),
    m_percoN(nullptr), m_percoP(nullptr), m_surfRfNO3(nullptr),
    m_surfRfNH4(nullptr), m_surfRfSolP(nullptr),
    //output
    m_surfRfCod(nullptr), m_surfRfChlA(nullptr), m_latNO3ToCh(nullptr),
    m_surfRfNO3ToCh(nullptr), m_surfRfNH4ToCh(nullptr), m_surfRfSolPToCh(nullptr),
    m_percoNGw(nullptr), m_percoPGw(nullptr),
    m_surfRfCodToCh(nullptr), m_nSubbsns(-1), m_subbsnID(nullptr), m_subbasinsInfo(nullptr),
    m_wshdLchP(-1.), m_soilNO3(nullptr), m_soilSolP(nullptr), m_sedLossCbn(nullptr) {
}

NutrientMovementViaWater::~NutrientMovementViaWater() {
    Release1DArray(m_latNO3);
    Release1DArray(m_percoN);
    Release1DArray(m_percoP);
    Release1DArray(m_surfRfNO3);
    Release1DArray(m_surfRfNH4);
    Release1DArray(m_surfRfSolP);

    Release1DArray(m_latNO3ToCh);
    Release1DArray(m_surfRfNO3ToCh);
    Release1DArray(m_surfRfNH4ToCh);
    Release1DArray(m_surfRfSolPToCh);
    Release1DArray(m_surfRfCodToCh);
    Release1DArray(m_percoNGw);
    Release1DArray(m_percoPGw);

    Release1DArray(m_surfRfCod);
    Release1DArray(m_surfRfChlA);
    
    Release2DArray(m_subSurfRf);
    Release2DArray(m_soilPerco);
    Release1DArray(m_drainLyr);
}

void NutrientMovementViaWater::SumBySubbasin() {
    // reset to zero
    for (int i = 0; i <= m_nSubbsns; i++) {
        m_latNO3ToCh[i] = 0.;
        m_surfRfNO3ToCh[i] = 0.;
        m_surfRfNH4ToCh[i] = 0.;
        m_surfRfSolPToCh[i] = 0.;
        m_surfRfCodToCh[i] = 0.;
        m_percoNGw[i] = 0.;
        m_percoPGw[i] = 0.;
    }

    /// using openmp for reduction an array should be paid much more attention.
    /// here is a solution. https://stackoverflow.com/questions/20413995/reducing-on-array-in-openmp
    /// #pragma omp parallel for reduction(+:myArray[:6]) is supported with OpenMP 4.5.
    /// However, MSVC 2010-2015 are using OpenMP 2.0.
#pragma omp parallel
    {
        FLTPT* tmp_latNO3ToCh = new(nothrow) FLTPT[m_nSubbsns + 1];
        FLTPT* tmp_surfRfNO3ToCh = new(nothrow) FLTPT[m_nSubbsns + 1];
        FLTPT* tmp_surfRfNH4ToCh = new(nothrow) FLTPT[m_nSubbsns + 1];
        FLTPT* tmp_surfRfSolPToCh = new(nothrow) FLTPT[m_nSubbsns + 1];
        FLTPT* tmp_surfRfCodToCh = new(nothrow) FLTPT[m_nSubbsns + 1];
        FLTPT* tmp_percoNGw = new(nothrow) FLTPT[m_nSubbsns + 1];
        FLTPT* tmp_percoPGw = new(nothrow) FLTPT[m_nSubbsns + 1];
        for (int i = 0; i <= m_nSubbsns; i++) {
            tmp_latNO3ToCh[i] = 0.;
            tmp_surfRfNO3ToCh[i] = 0.;
            tmp_surfRfNH4ToCh[i] = 0.;
            tmp_surfRfSolPToCh[i] = 0.;
            tmp_surfRfCodToCh[i] = 0.;
            tmp_percoNGw[i] = 0.;
            tmp_percoPGw[i] = 0.;
        }
#pragma omp for
        for (int i = 0; i < m_nCells; i++) {
            int subid = CVT_INT(m_subbsnID[i]);
            if (m_rchID[i] > 0) tmp_latNO3ToCh[subid] += m_latNO3[i];
            tmp_surfRfNO3ToCh[subid] += m_surfRfNO3[i] * m_cellArea; // kg/ha * ha = kg
            tmp_surfRfNH4ToCh[subid] += m_surfRfNH4[i] * m_cellArea;
            tmp_surfRfSolPToCh[subid] += m_surfRfSolP[i] * m_cellArea;
            tmp_surfRfCodToCh[subid] += m_surfRfCod[i] * m_cellArea;
            FLTPT ratio2_gw = 1.;
            tmp_percoNGw[subid] += m_percoN[i] * m_cellArea * ratio2_gw;
            tmp_percoPGw[subid] += m_percoP[i] * m_cellArea * ratio2_gw;

        }
#pragma omp critical
        {
            for (int i = 1; i <= m_nSubbsns; i++) {
                m_latNO3ToCh[i] += tmp_latNO3ToCh[i];
                m_surfRfNO3ToCh[i] += tmp_surfRfNO3ToCh[i];
                m_surfRfNH4ToCh[i] += tmp_surfRfNH4ToCh[i];
                m_surfRfSolPToCh[i] += tmp_surfRfSolPToCh[i];
                m_surfRfCodToCh[i] += tmp_surfRfCodToCh[i];
                m_percoNGw[i] += tmp_percoNGw[i];
                m_percoPGw[i] += tmp_percoPGw[i];
            }
        }
        delete[] tmp_latNO3ToCh;
        delete[] tmp_surfRfNO3ToCh;
        delete[] tmp_surfRfNH4ToCh;
        delete[] tmp_surfRfSolPToCh;
        delete[] tmp_surfRfCodToCh;
        delete[] tmp_percoNGw;
        delete[] tmp_percoPGw;
        tmp_latNO3ToCh = nullptr;
        tmp_surfRfNO3ToCh = nullptr;
        tmp_surfRfNH4ToCh = nullptr;
        tmp_surfRfSolPToCh = nullptr;
        tmp_surfRfCodToCh = nullptr;
        tmp_percoNGw = nullptr;
        tmp_percoPGw = nullptr;
    } /* END of #pragma omp parallel */

    // sum all the subbasins and put the sum value in the first element of the array
    for (int i = 1; i <= m_nSubbsns; i++) {
        m_surfRfNO3ToCh[0] += m_surfRfNO3ToCh[i];
        m_surfRfNH4ToCh[0] += m_surfRfNH4ToCh[i];
        m_surfRfSolPToCh[0] += m_surfRfSolPToCh[i];
        m_surfRfCodToCh[0] += m_surfRfCodToCh[i];
        m_latNO3ToCh[0] += m_latNO3ToCh[i];
        m_percoNGw[0] += m_percoNGw[i];
        m_percoPGw[0] += m_percoPGw[i];
    }
}

bool NutrientMovementViaWater::CheckInputData() {
    CHECK_POSITIVE(M_NUTRMV[0], m_nSubbsns);
    CHECK_POSITIVE(M_NUTRMV[0], m_nCells);
    CHECK_POSITIVE(M_NUTRMV[0], m_cellWth);
    CHECK_POSITIVE(M_NUTRMV[0], m_maxSoilLyrs);
    CHECK_POINTER(M_NUTRMV[0], m_nSoilLyrs);
    CHECK_POINTER(M_NUTRMV[0], m_olWtrEroSed);
    CHECK_POINTER(M_NUTRMV[0], m_soilCbn);
    CHECK_POINTER(M_NUTRMV[0], m_anionExclFr);

    CHECK_NONNEGATIVE(M_NUTRMV[0], m_isep_opt);
    CHECK_POINTER(M_NUTRMV[0], m_distToRch);
    CHECK_POINTER(M_NUTRMV[0], m_surfRf);
    CHECK_POSITIVE(M_NUTRMV[0], m_nperco);
    CHECK_POSITIVE(M_NUTRMV[0], m_cod_n);
    CHECK_POSITIVE(M_NUTRMV[0], m_cod_k);
    CHECK_POINTER(M_NUTRMV[0], m_subSurfRf);
    CHECK_POINTER(M_NUTRMV[0], m_soilPerco);
    CHECK_POINTER(M_NUTRMV[0], m_soilSat);
    CHECK_POSITIVE(M_NUTRMV[0], m_phoskd);
    CHECK_POSITIVE(M_NUTRMV[0], m_pperco);
    CHECK_POINTER(M_NUTRMV[0], m_soilThk);
    CHECK_POINTER(M_NUTRMV[0], m_soilCrk);
    CHECK_POINTER(M_NUTRMV[0], m_soilBD);
    CHECK_POINTER(M_NUTRMV[0], m_soilDepth);
    CHECK_POINTER(M_NUTRMV[0], m_flowOutIdx);
    CHECK_POINTER(M_NUTRMV[0], m_soilThk);
    CHECK_POINTER(M_NUTRMV[0], m_subbasinsInfo);
    if (m_cbnModel == 2) CHECK_POINTER(M_NUTRMV[0], m_sedLossCbn);

    /** TEST CODE START **/
    /*
    for (int ilyr = 0; ilyr < m_nRteLyrs; ilyr++) {
        int ncells = CVT_INT(m_rteLyrs[ilyr][0]);
        cout << ilyr << ":" << ncells << "{";
        for (int icell = 1; icell <= ncells; icell++) {
            int id = CVT_INT(m_rteLyrs[ilyr][icell]);
            int nDownstream = CVT_INT(m_flowOutIdx[id][0]);
            cout << id << ": [";
            for (int downIndex = 1; downIndex <= nDownstream; downIndex++) {
                int flowOutID = CVT_INT(m_flowOutIdx[id][downIndex]);
                FLTPT flowOutFrac = 1.;
                if (nullptr != m_flowOutFrac) flowOutFrac = m_flowOutFrac[id][downIndex];
                cout << "(" << flowOutID << ": " << flowOutFrac << "), ";
            }
            cout << "], ";
        }
        cout << "}" << endl;
    }
    */
    /** TEST CODE END   **/
    return true;
}

void NutrientMovementViaWater::SetSubbasins(clsSubbasins* subbasins) {
    if (nullptr == m_subbasinsInfo) {
        m_subbasinsInfo = subbasins;
        // m_nSubbasins = m_subbasinsInfo->GetSubbasinNumber();  // Set in SetValue()
        m_subbasinIDs = m_subbasinsInfo->GetSubbasinIDs();
    }
}

void NutrientMovementViaWater::SetValue(const char* key, const FLTPT value) {
    string sk(key);
    if (StringMatch(sk, Tag_CellWidth[0])) m_cellWth = value;
    else if (StringMatch(sk, VAR_QTILE[0])) m_qtile = value;
    else if (StringMatch(sk, VAR_NPERCO[0])) m_nperco = value;
    else if (StringMatch(sk, VAR_PPERCO[0])) m_pperco = value;
    else if (StringMatch(sk, VAR_PHOSKD[0])) m_phoskd = value;
    else if (StringMatch(sk, VAR_COD_N[0])) m_cod_n = value;
    else if (StringMatch(sk, VAR_COD_K[0])) m_cod_k = value;
    else {
        throw ModelException(M_NUTRMV[0], "SetValue",
                             "Parameter " + sk + " does not exist.");
    }
}

void NutrientMovementViaWater::SetValue(const char* key, const int value) {
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSNID_NUM[0])) m_nSubbsns = value;
    else if (StringMatch(sk, VAR_ISEP_OPT[0])) m_isep_opt = value;
    else if (StringMatch(sk, VAR_CSWAT[0])) m_cbnModel = value;
    else {
        throw ModelException(M_NUTRMV[0], "SetValue",
                             "Integer Parameter " + sk + " does not exist.");
    }
}

void NutrientMovementViaWater::Set1DData(const char* key, const int n, FLTPT* data) {
    if (!CheckInputSize(M_NUTRMV[0], key, n, m_nCells)) return;
    string sk(key);
    if (StringMatch(sk, VAR_OLFLOW[0])) {
        m_surfRf = data;
    } else if (StringMatch(sk, VAR_ANION_EXCL[0])) {
        m_anionExclFr = data;
    } else if (StringMatch(sk, VAR_DISTSTREAM[0])) {
        m_distToRch = data;
    } else if (StringMatch(sk, VAR_SOL_CRK[0])) {
        m_soilCrk = data;
    } else if (StringMatch(sk, VAR_SEDYLD[0])) {
        m_olWtrEroSed = data;
    } else if (StringMatch(sk, VAR_SEDORGN[0])) {
        m_sedorgn = data;
    } else if (StringMatch(sk, VAR_TMEAN[0])) {
        m_meanTemp = data;
    } else if (StringMatch(sk, VAR_SEDLOSS_C[0])) {
        m_sedLossCbn = data;
    } else {
        throw ModelException(M_NUTRMV[0], "Set1DData",
                             "Parameter " + sk + " does not exist.");
    }
}


void NutrientMovementViaWater::Set1DData(const char* key, const int n, int* data) {
    if (!CheckInputSize(M_NUTRMV[0], key, n, m_nCells)) return;
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSN[0])) {
        m_subbsnID = data;
    } else if (StringMatch(sk, VAR_STREAM_LINK[0])) {
        m_rchID = data;
    }
    // else if (StringMatch(sk, VAR_LDRAIN[0])) { // not used ?
    //     m_drainLyr = data;
    // }
    else if (StringMatch(sk, VAR_SOILLAYERS[0])) {
        m_nSoilLyrs = data;
    } else {
        throw ModelException(M_NUTRMV[0], "Set1DData",
                             "Integer Parameter " + sk + " does not exist.");
    }
}

void NutrientMovementViaWater::Set2DData(const char* key, const int nrows, const int ncols, FLTPT** data) {
    string sk(key);
    if (StringMatch(sk, Tag_FLOWOUT_FRACTION[0])) {
        m_nCells = nrows;
        m_flowOutFrac = data;
        return;
    }
    if (!CheckInputSize2D(M_NUTRMV[0], key, nrows, ncols, m_nCells, m_maxSoilLyrs)) return;

    if (StringMatch(sk, VAR_SSRU[0])) m_subSurfRf = data;
    else if (StringMatch(sk, VAR_SOL_NO3[0])) m_soilNO3 = data;
    else if (StringMatch(sk, VAR_SOL_BD[0])) m_soilBD = data;
    else if (StringMatch(sk, VAR_SOL_SOLP[0])) m_soilSolP = data;
    else if (StringMatch(sk, VAR_SOILDEPTH[0])) m_soilDepth = data;
    else if (StringMatch(sk, VAR_PERCO[0])) m_soilPerco = data;
    else if (StringMatch(sk, VAR_SOL_CBN[0])) m_soilCbn = data;
    else if (StringMatch(sk, VAR_SOILTHICK[0])) m_soilThk = data;
    else if (StringMatch(sk, VAR_SOL_UL[0])) m_soilSat = data;
    else if (StringMatch(sk, VAR_CONV_WT[0])) m_cvtWt = data;
    else {
        throw ModelException(M_NUTRMV[0], "Set2DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void NutrientMovementViaWater::Set2DData(const char* key, const int nrows, const int ncols, int** data) {
    string sk(key);
    if (StringMatch(sk, Tag_ROUTING_LAYERS[0])) {
        m_nRteLyrs = nrows;
        m_rteLyrs = data;
        return;
    }
    if (StringMatch(sk, Tag_FLOWOUT_INDEX[0])) {
        m_nCells = nrows;
        m_flowOutIdx = data;
        return;
    }
    throw ModelException(M_NUTRMV[0], "Set2DData",
                         "Integer Parameter " + sk + " does not exist.");
}

void NutrientMovementViaWater::InitialOutputs() {
    CHECK_POSITIVE(M_NUTRMV[0], m_nSubbsns);
    CHECK_POSITIVE(M_NUTRMV[0], m_nCells);
    CHECK_POSITIVE(M_NUTRMV[0], m_maxSoilLyrs);
    if (m_cellArea < 0.) m_cellArea = m_cellWth * m_cellWth * 0.0001; /// unit: ha
    // allocate the output variables
    Initialize1DArray(m_nCells, m_latNO3, 0.);
    Initialize1DArray(m_nCells, m_percoN, 0.);
    Initialize1DArray(m_nCells, m_percoP, 0.);
    Initialize1DArray(m_nCells, m_surfRfNO3, 0.);
    Initialize1DArray(m_nCells, m_surfRfNH4, 0.);
    Initialize1DArray(m_nCells, m_surfRfSolP, 0.);

    Initialize1DArray(m_nSubbsns + 1, m_latNO3ToCh, 0.);
    Initialize1DArray(m_nSubbsns + 1, m_surfRfNO3ToCh, 0.);
    Initialize1DArray(m_nSubbsns + 1, m_surfRfNH4ToCh, 0.);
    Initialize1DArray(m_nSubbsns + 1, m_surfRfSolPToCh, 0.);
    Initialize1DArray(m_nSubbsns + 1, m_surfRfCodToCh, 0.);
    Initialize1DArray(m_nSubbsns + 1, m_percoNGw, 0.);
    Initialize1DArray(m_nSubbsns + 1, m_percoPGw, 0.);

    Initialize1DArray(m_nCells, m_surfRfCod, 0.);
    Initialize1DArray(m_nCells, m_surfRfChlA, 0.);
    if (m_wshdLchP < 0.) m_wshdLchP = 0.;

    // input variables
    Initialize2DArray(m_nCells, m_maxSoilLyrs, m_subSurfRf, 0.0001);
    Initialize2DArray(m_nCells, m_maxSoilLyrs, m_soilPerco, 0.0001);
    Initialize1DArray(m_nCells, m_drainLyr, -1.);
    if (m_qtile < 0.) m_qtile = 0.0001;
}

int NutrientMovementViaWater::Execute() {
    CheckInputData();
    InitialOutputs();
    // reset variables for the new timestep
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        m_latNO3[i] = 0.;
    }

    for (int ilyr = 0; ilyr < m_nRteLyrs; ilyr++) {
        // There are not any flow relationship within each routing layer.
        // So parallelization can be done here.
        int ncells = m_rteLyrs[ilyr][0];
#pragma omp parallel for
        for (int icell = 1; icell <= ncells; icell++) {
            int i = m_rteLyrs[ilyr][icell]; // cell ID
            if (m_rchID[i] > 0) continue;            // Skip the reach (stream) cells
            NitrateLoss(i);
            PhosphorusLoss(i);
            SubbasinWaterQuality(i); // compute chl-a, CBOD and dissolved oxygen loadings
        }
    }
    SumBySubbasin();
    return 0;
}

void NutrientMovementViaWater::NitrateLoss(const int i) {
    FLTPT percnlyr = 0.;
    for (int k = 0; k < CVT_INT(m_nSoilLyrs[i]); k++) {
        // add nitrate leached from layer above (kg/ha)
        m_soilNO3[i][k] += percnlyr;
        if (m_soilNO3[i][k] < 1.e-6) {
            m_soilNO3[i][k] = 0.;
        }
        // determine concentration of nitrate in mobile water
        FLTPT sro = k == 0 ? m_surfRf[i] : 0.; // surface runoff generated
        FLTPT mw = 0.;                         // amount of mobile water in the layer (vv in swat nlch.)
        FLTPT vno3 = 0.;                       // kg/ha
        FLTPT con = 0.;                        // 100 mg/L
        FLTPT ww = 0.;
        // Calculate the concentration of nitrate in the mobile water (con),
        // equation 4:2.1.2, 4:2.1.3 and 4:2.1.4 in SWAT Theory 2009, p269
        mw = m_soilPerco[i][k] + sro + m_subSurfRf[i][k] + 1.e-10;
        if (m_drainLyr[i] == k) mw += m_qtile;
        FLTPT satportion = (1. - m_anionExclFr[i]) * m_soilSat[i][k];
        ww = -mw / satportion;
        vno3 = m_soilNO3[i][k] * (1. - CalExp(ww)); // kg/ha
        if (mw > 1.e-10) con = Max(vno3 / mw, 0.);

        // calculate nitrate in surface runoff
        // concentration of nitrate in surface runoff (cosurf)
        FLTPT cosurf = 0.;
        if (m_isep_opt == 2)
            cosurf = 1. * con; // N percolation does not apply to failing septic place;
        else
            cosurf = m_nperco * con;
        if (k == 0) {
            m_surfRfNO3[i] = m_surfRf[i] * cosurf; // kg/ha
            m_surfRfNO3[i] = Min(m_surfRfNO3[i], m_soilNO3[i][k]);
            m_soilNO3[i][k] -= m_surfRfNO3[i];
        }
        // TODO: calculate nitrate in tile flow
        if (m_drainLyr[i] == k) {
        }
        // nitrate moved with subsuface flow (kg/ha)
        FLTPT ssfnlyr = k == 0 ? cosurf * m_subSurfRf[i][k] : con * m_subSurfRf[i][k]; // kg/ha
        // calculate nitrate in lateral flow
        // nitrate transported in lateral flow from layer (ssfnlyr)
        ssfnlyr = Min(ssfnlyr, m_soilNO3[i][k]);
        m_latNO3[i] += ssfnlyr;
        // move the lateral no3 flow to the downslope cell (routing considered)
        m_soilNO3[i][k] -= ssfnlyr;
        int down_count = CVT_INT(m_flowOutIdx[i][0]);
        for (int downi = 1; downi <= down_count; downi++) {
            int id_downstream = CVT_INT(m_flowOutIdx[i][downi]);
            if (id_downstream < 0) continue;
            if (nullptr == m_flowOutFrac) {
                m_soilNO3[id_downstream][k] += m_latNO3[i];
            } else {
                m_soilNO3[id_downstream][k] += m_latNO3[i] * m_flowOutFrac[i][downi];
            }
        }
        /// old code for D8 only: if (id_downstream >= 0) m_soilNO3[id_downstream][k] += m_latNO3[i];
        /// legacy code: m_soilNO3[idDownSlope][k] += ssfnlyr; /// changed by LJ, 16-10-13

        // calculate nitrate in percolate
        percnlyr = con * m_soilPerco[i][k];
        percnlyr = Min(percnlyr, m_soilNO3[i][k]);
        m_soilNO3[i][k] -= percnlyr;
    }
    // calculate nitrate leaching from soil profile
    m_percoN[i] = percnlyr; // percolation of the last soil layer, kg/ha

    // I think these should be removed, because the lost nitrate
    //   have been added to it's downslope cell. by LJ
    // FLTPT nloss = (2.18f * m_distToRch[i] - 8.63f) / 100.;
    // nloss = Min(1., Max(0., nloss));
    // m_latNO3[i] = (1. - nloss) * m_latno3[i];
}

void NutrientMovementViaWater::PhosphorusLoss(const int i) {
    // amount of P leached from soil layer, kg/ha (vap)
    FLTPT vap = 0.;
    FLTPT vap_tile = 0.;
    // compute soluble P lost in surface runoff
    // variable to hold intermediate calculation result
    FLTPT xx = m_soilBD[i][0] * m_soilDepth[i][0] * m_phoskd;
    // units ==> surqsolp = [kg/ha * mm] / [t/m^3 * mm * m^3/t] = kg/ha
    m_surfRfSolP[i] = m_soilSolP[i][0] * m_surfRf[i] / xx;
    m_surfRfSolP[i] = Min(m_surfRfSolP[i], m_soilSolP[i][0]);
    m_surfRfSolP[i] = Max(m_surfRfSolP[i], 0.);
    m_soilSolP[i][0] = m_soilSolP[i][0] - m_surfRfSolP[i];

    // compute soluble P leaching
    vap = m_soilSolP[i][0] * m_soilPerco[i][0] / (m_cvtWt[i][0] / 1000. * m_pperco);
    vap = Min(vap, 0.5 * m_soilSolP[i][0]);
    m_soilSolP[i][0] -= vap;

    // estimate soluble p in tiles due to crack flow
    if (m_drainLyr[i] > 0) {
        xx = Min(1., m_soilCrk[i] / 3.);
        vap_tile = xx * vap;
        vap = vap - vap_tile;
    }
    if (m_nSoilLyrs[i] >= 2) {
        m_soilSolP[i][1] += vap;
    }
    for (int k = 1; k < CVT_INT(m_nSoilLyrs[i]); k++) {
        vap = m_soilSolP[i][k] * m_soilPerco[i][k] / (m_cvtWt[i][k] * 0.001 * m_pperco);
        vap = Min(vap, 0.2 * m_soilSolP[i][k]);
        m_soilSolP[i][k] -= vap;
        if (k < m_nSoilLyrs[i] - 1) {
            // this is slightly different with swat code in solp.f
            m_soilSolP[i][k + 1] += vap; //leach to next layer
        } else {
            m_percoP[i] = vap;
        } //leach to groundwater
    }
}

void NutrientMovementViaWater::SubbasinWaterQuality(const int i) {
    /// Note, doxq (i.e., dissolved oxygen concentration in the surface runoff on current day)
    ///       is not included here, just because doxq will not be used anywhere.
    ///       Also, the algorithm will be executed in Qual2E model in channel process.   By LJ

    // total amount of water entering main channel on current day, mm
    FLTPT qdr = m_surfRf[i] + m_subSurfRf[i][0] + m_qtile;
    if (qdr > 1.e-4) {
        // kilo moles of phosphorus in nutrient loading to main channel (tp)
        FLTPT tp = 100. * (m_sedorgn[i] + m_surfRfNO3[i]) / qdr; //100*kg/ha/mm = ppm
        // regional adjustment on sub chla_a loading, the default value is 40
        FLTPT chla_subco = 40.;
        m_surfRfChlA[i] = chla_subco * tp;
        m_surfRfChlA[i] *= 0.001; // um/L to mg/L

        // calculate enrichment ratio
        if (m_olWtrEroSed[i] < 1e-4) m_olWtrEroSed[i] = 0.;
        FLTPT enratio = CalEnrichmentRatio(m_olWtrEroSed[i], m_surfRf[i], m_cellArea);

        // calculate organic carbon loading to main channel
        FLTPT org_c = 0.; /// kg
        if (m_cbnModel == 2) {
            org_c = m_sedLossCbn[i] * m_cellArea;
        } else {
            org_c = m_soilCbn[i][0] * 0.01 * enratio * (m_olWtrEroSed[i] * 0.001) * 1000.;
        }
        // calculate carbonaceous biological oxygen demand (CBOD) and COD(transform from CBOD)
        FLTPT cbodu = 2.7 * org_c / (qdr * m_cellWth * m_cellWth * 1.e-6); //  mg/L
        // convert cbod to cod
        // The translation relationship is combined Wang Cai-Qin et al. (2014) with
        // Guo and Long (1994); Xie et al. (2000); Jin et al. (2005).
        FLTPT cod = m_cod_n * (cbodu * (1. - CalExp(-5. * m_cod_k)));
        m_surfRfCod[i] = m_surfRf[i] * 0.001 * cod * 10.; // mg/L converted to kg/ha
    } else {
        m_surfRfChlA[i] = 0.;
        m_surfRfCod[i] = 0.;
    }
}

void NutrientMovementViaWater::GetValue(const char* key, FLTPT* value) {
    string sk(key);
    if (StringMatch(sk, VAR_WSHD_PLCH[0])) *value = m_wshdLchP;
    else {
        throw ModelException(M_NUTRMV[0], "GetValue",
                             "Parameter " + sk + " does not exist.");
    }
}

void NutrientMovementViaWater::Get1DData(const char* key, int* n, FLTPT** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_LATNO3[0])) {
        *data = m_latNO3;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_PERCO_N_GW[0])) {
        *data = m_percoNGw;
        *n = m_nSubbsns + 1;
    } else if (StringMatch(sk, VAR_PERCO_P_GW[0])) {
        *data = m_percoPGw;
        *n = m_nSubbsns + 1;
    } else if (StringMatch(sk, VAR_SUR_NO3[0])) {
        *data = m_surfRfNO3;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_SUR_NH4[0])) {
        *data = m_surfRfNH4;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_SUR_SOLP[0])) {
        *data = m_surfRfSolP;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_SUR_COD[0])) {
        *data = m_surfRfCod;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_CHL_A[0])) {
        *data = m_surfRfChlA;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_LATNO3_TOCH[0])) {
        *data = m_latNO3ToCh;
        *n = m_nSubbsns + 1;
    } else if (StringMatch(sk, VAR_SUR_NO3_TOCH[0])) {
        *data = m_surfRfNO3ToCh;
        *n = m_nSubbsns + 1;
    } else if (StringMatch(sk, VAR_SUR_NH4_TOCH[0])) {
        *data = m_surfRfNH4ToCh;
        *n = m_nSubbsns + 1;
    } else if (StringMatch(sk, VAR_SUR_SOLP_TOCH[0])) {
        *data = m_surfRfSolPToCh;
        *n = m_nSubbsns + 1;
    } else if (StringMatch(sk, VAR_SUR_COD_TOCH[0])) {
        *data = m_surfRfCodToCh;
        *n = m_nSubbsns + 1;
    } else {
        throw ModelException(M_NUTRMV[0], "Get1DData",
                             "Result " + sk + " does not exist.");
    }
}
