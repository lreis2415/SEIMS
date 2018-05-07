#include "NutrientTransportSediment.h"

#include "text.h"
#include "NutrientCommon.h"

NutrientTransportSediment::NutrientTransportSediment() :
//input
    m_nCells(-1), m_cellWidth(-1.f), m_cellArea(-1.f), m_nMaxSoiLayers(-1), m_nSoilLayers(nullptr),
    m_nSubbasins(-1), m_subbasinID(-1), m_subbasin(nullptr), m_subbasinsInfo(nullptr),
    m_sol_bd(nullptr), m_soilThick(nullptr), m_enratio(nullptr),
    m_sol_actp(nullptr), m_sol_stap(nullptr), m_sol_fop(nullptr), m_sol_orgp(nullptr),
    m_sol_orgn(nullptr), m_sol_aorgn(nullptr), m_sol_fon(nullptr),
    m_sedEroded(nullptr), m_surfaceRunoff(nullptr),
    /// for CENTURY C/N cycling model inputs
    m_CbnModel(0), m_sol_LSN(nullptr), m_sol_LMN(nullptr), m_sol_HPN(nullptr), m_sol_HSN(nullptr),
    m_sol_HPC(nullptr), m_sol_HSC(nullptr), m_sol_LMC(nullptr), m_sol_LSC(nullptr), m_sol_LS(nullptr),
    m_sol_LM(nullptr), m_sol_LSL(nullptr), m_sol_LSLC(nullptr), m_sol_LSLNC(nullptr), m_sol_BMC(nullptr),
    m_sol_WOC(nullptr), m_sol_perco(nullptr), m_sol_laterq(nullptr),
    /// for C-FARM one carbon model input
    m_sol_mp(nullptr),
    /// for CENTURY C/N cycling model outputs
    m_sol_latC(nullptr), m_sol_percoC(nullptr), m_laterC(nullptr), m_percoC(nullptr), m_sedCLoss(nullptr),
    //outputs
    m_sedorgn(nullptr), m_sedorgp(nullptr), m_sedminpa(nullptr), m_sedminps(nullptr),
    m_sedorgnToCh(nullptr), m_sedorgpToCh(nullptr), m_sedminpaToCh(nullptr), m_sedminpsToCh(nullptr) {
}

NutrientTransportSediment::~NutrientTransportSediment() {
    if (m_enratio != nullptr) Release1DArray(m_enratio);

    if (m_sedorgp != nullptr) Release1DArray(m_sedorgp);
    if (m_sedorgn != nullptr) Release1DArray(m_sedorgn);
    if (m_sedminpa != nullptr) Release1DArray(m_sedminpa);
    if (m_sedminps != nullptr) Release1DArray(m_sedminps);

    if (m_sedorgnToCh != nullptr) Release1DArray(m_sedorgnToCh);
    if (m_sedorgpToCh != nullptr) Release1DArray(m_sedorgpToCh);
    if (m_sedminpaToCh != nullptr) Release1DArray(m_sedminpaToCh);
    if (m_sedminpsToCh != nullptr) Release1DArray(m_sedminpsToCh);

    /// for CENTURY C/N cycling model outputs
    if (m_sol_latC != nullptr) Release2DArray(m_nCells, m_sol_percoC);
    if (m_sol_percoC != nullptr) Release2DArray(m_nCells, m_sol_percoC);
    if (m_laterC != nullptr) Release1DArray(m_laterC);
    if (m_percoC != nullptr) Release1DArray(m_percoC);
    if (m_sedCLoss != nullptr) Release1DArray(m_sedCLoss);
}

bool NutrientTransportSediment::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_NUTRSED, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.\n");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) {
            m_nCells = n;
        } else {
            throw ModelException(MID_NUTRSED, "CheckInputSize",
                                 "Input data for " + string(key) + " is invalid with size: " + ValueToString(n) +
                                     ". The origin size is " + ValueToString(m_nCells) + ".\n");
        }
    }
    return true;
}

bool NutrientTransportSediment::CheckInputData() {
    CHECK_POSITIVE(MID_NUTRSED, m_nCells);
    CHECK_POSITIVE(MID_NUTRSED, m_nSubbasins);
    CHECK_POSITIVE(MID_NUTRSED, m_cellWidth);
    CHECK_POSITIVE(MID_NUTRSED, m_nMaxSoiLayers);
    CHECK_POINTER(MID_NUTRSED, m_nSoilLayers);
    CHECK_POINTER(MID_NUTRSED, m_sedEroded);
    CHECK_POINTER(MID_NUTRSED, m_surfaceRunoff);
    CHECK_POINTER(MID_NUTRSED, m_sol_bd);
    CHECK_POINTER(MID_NUTRSED, m_sol_actp);
    CHECK_POINTER(MID_NUTRSED, m_sol_orgn);
    CHECK_POINTER(MID_NUTRSED, m_sol_orgp);
    CHECK_POINTER(MID_NUTRSED, m_sol_stap);
    CHECK_POINTER(MID_NUTRSED, m_sol_aorgn);
    CHECK_POINTER(MID_NUTRSED, m_sol_fon);
    CHECK_POINTER(MID_NUTRSED, m_sol_fop);
    CHECK_POINTER(MID_NUTRSED, m_subbasin);
    CHECK_POINTER(MID_NUTRSED, m_subbasinsInfo);
    return true;
}

bool NutrientTransportSediment::CheckInputData_CENTURY() {
    CHECK_POINTER(MID_NUTRSED, m_sol_LSN);
    CHECK_POINTER(MID_NUTRSED, m_sol_LMN);
    CHECK_POINTER(MID_NUTRSED, m_sol_HPN);
    CHECK_POINTER(MID_NUTRSED, m_sol_HSN);
    CHECK_POINTER(MID_NUTRSED, m_sol_HPC);
    CHECK_POINTER(MID_NUTRSED, m_sol_HSC);
    CHECK_POINTER(MID_NUTRSED, m_sol_LMC);
    CHECK_POINTER(MID_NUTRSED, m_sol_LSC);
    CHECK_POINTER(MID_NUTRSED, m_sol_LS);
    CHECK_POINTER(MID_NUTRSED, m_sol_LM);
    CHECK_POINTER(MID_NUTRSED, m_sol_LSL);
    CHECK_POINTER(MID_NUTRSED, m_sol_LSLC);
    CHECK_POINTER(MID_NUTRSED, m_sol_LSLNC);
    CHECK_POINTER(MID_NUTRSED, m_sol_BMC);
    CHECK_POINTER(MID_NUTRSED, m_sol_WOC);
    CHECK_POINTER(MID_NUTRSED, m_sol_perco);
    CHECK_POINTER(MID_NUTRSED, m_sol_laterq);
    return true;
}

bool NutrientTransportSediment::CheckInputData_CFARM() {
    CHECK_POINTER(MID_NUTRSED, m_sol_mp);
    return true;
}

void NutrientTransportSediment::SetValue(const char *key, float value) {
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSNID_NUM)) { m_nSubbasins = value; }
    else if (StringMatch(sk, Tag_SubbasinId)) { m_subbasinID = value; }
    else if (StringMatch(sk, Tag_CellWidth)) { m_cellWidth = value; }
    else if (StringMatch(sk, VAR_CSWAT)) { m_CbnModel = (int) value; }
    else {
        throw ModelException(MID_NUTRSED, "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void NutrientTransportSediment::Set1DData(const char *key, int n, float *data) {
    CheckInputSize(key, n);
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSN)) {
        m_subbasin = data;
    } else if (StringMatch(sk, VAR_SOILLAYERS)) {
        m_nSoilLayers = data;
    } else if (StringMatch(sk, VAR_SEDYLD)) {
        m_sedEroded = data;
    } else if (StringMatch(sk, VAR_OLFLOW)) {
        m_surfaceRunoff = data;
    } else {
        throw ModelException(MID_NUTRSED, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void NutrientTransportSediment::Set2DData(const char *key, int nRows, int nCols, float **data) {
    CheckInputSize(key, nRows);
    string sk(key);
    m_nMaxSoiLayers = nCols;
    if (StringMatch(sk, VAR_SOILTHICK)) { m_soilThick = data; }
    else if (StringMatch(sk, VAR_SOL_BD)) { m_sol_bd = data; }
    else if (StringMatch(sk, VAR_SOL_AORGN)) { m_sol_aorgn = data; }
    else if (StringMatch(sk, VAR_SOL_SORGN)) { m_sol_orgn = data; }
    else if (StringMatch(sk, VAR_SOL_HORGP)) { m_sol_orgp = data; }
    else if (StringMatch(sk, VAR_SOL_FORGP)) { m_sol_fop = data; }
    else if (StringMatch(sk, VAR_SOL_FORGN)) { m_sol_fon = data; }
    else if (StringMatch(sk, VAR_SOL_ACTP)) { m_sol_actp = data; }
    else if (StringMatch(sk, VAR_SOL_STAP)) { m_sol_stap = data; }
        /// for CENTURY C/Y cycling model, optional inputs
    else if (StringMatch(sk, VAR_ROCK)) { m_sol_rock = data; }
    else if (StringMatch(sk, VAR_SOL_UL)) { m_sol_wsatur = data; }
    else if (StringMatch(sk, VAR_SOL_LSN)) { m_sol_LSN = data; }
    else if (StringMatch(sk, VAR_SOL_LMN)) { m_sol_LMN = data; }
    else if (StringMatch(sk, VAR_SOL_HPN)) { m_sol_HPN = data; }
    else if (StringMatch(sk, VAR_SOL_HSN)) { m_sol_HSN = data; }
    else if (StringMatch(sk, VAR_SOL_HPC)) { m_sol_HPC = data; }
    else if (StringMatch(sk, VAR_SOL_HSC)) { m_sol_HSC = data; }
    else if (StringMatch(sk, VAR_SOL_LMC)) { m_sol_LMC = data; }
    else if (StringMatch(sk, VAR_SOL_LSC)) { m_sol_LSC = data; }
    else if (StringMatch(sk, VAR_SOL_LS)) { m_sol_LS = data; }
    else if (StringMatch(sk, VAR_SOL_LM)) { m_sol_LM = data; }
    else if (StringMatch(sk, VAR_SOL_LSL)) { m_sol_LSL = data; }
    else if (StringMatch(sk, VAR_SOL_LSLC)) { m_sol_LSLC = data; }
    else if (StringMatch(sk, VAR_SOL_LSLNC)) { m_sol_LSLNC = data; }
    else if (StringMatch(sk, VAR_SOL_BMC)) { m_sol_BMC = data; }
    else if (StringMatch(sk, VAR_SOL_WOC)) { m_sol_WOC = data; }
    else if (StringMatch(sk, VAR_PERCO)) { m_sol_perco = data; }
    else if (StringMatch(sk, VAR_SSRU)) {
        m_sol_laterq = data;
        /// for C-FARM one carbon model
    } else if (StringMatch(sk, VAR_SOL_MP)) { m_sol_mp = data; }
    else {
        throw ModelException(MID_NUTRSED, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void NutrientTransportSediment:: InitialOutputs() {
    CHECK_POSITIVE(MID_NUTRSED, m_nCells);
    // initial enrichment ratio
    if (m_enratio == nullptr) {
        Initialize1DArray(m_nCells, m_enratio, 0.f);
    }
    if (m_cellArea < 0) {
        m_cellArea = m_cellWidth * m_cellWidth * 0.0001f; //Unit is ha
    }
    // allocate the output variables
    if (m_sedorgn == nullptr) {
        Initialize1DArray(m_nCells, m_sedorgn, 0.f);
        Initialize1DArray(m_nCells, m_sedorgp, 0.f);
        Initialize1DArray(m_nCells, m_sedminpa, 0.f);
        Initialize1DArray(m_nCells, m_sedminps, 0.f);

        Initialize1DArray(m_nSubbasins + 1, m_sedorgnToCh, 0.f);
        Initialize1DArray(m_nSubbasins + 1, m_sedorgpToCh, 0.f);
        Initialize1DArray(m_nSubbasins + 1, m_sedminpaToCh, 0.f);
        Initialize1DArray(m_nSubbasins + 1, m_sedminpsToCh, 0.f);
    }
    /// for CENTURY C/N cycling model outputs
    if (m_CbnModel == 2 && m_sol_latC == nullptr) {
        Initialize2DArray(m_nCells, m_nMaxSoiLayers, m_sol_latC, 0.f);
        Initialize2DArray(m_nCells, m_nMaxSoiLayers, m_sol_percoC, 0.f);
        Initialize1DArray(m_nCells, m_laterC, 0.f);
        Initialize1DArray(m_nCells, m_percoC, 0.f);
        Initialize1DArray(m_nCells, m_sedCLoss, 0.f);
    }
}

void NutrientTransportSediment::SetSubbasins(clsSubbasins *subbasins) {
    if (m_subbasinsInfo == nullptr) {
        m_subbasinsInfo = subbasins;
        // m_nSubbasins = m_subbasinsInfo->GetSubbasinNumber(); // Set in SetValue()
        m_subbasinIDs = m_subbasinsInfo->GetSubbasinIDs();
    }
}

int NutrientTransportSediment::Execute() {
    CheckInputData();
    if (m_CbnModel == 1) {
        if (!CheckInputData_CFARM()) { return false; }
    }
    if (m_CbnModel == 2) {
        if (!CheckInputData_CENTURY()) { return false; }
    }
     InitialOutputs();
    // initial nutrient to channel for each day
    for (int i = 0; i < m_nSubbasins + 1; i++) {
        m_sedorgnToCh[i] = 0.f;
        m_sedorgpToCh[i] = 0.f;
        m_sedminpaToCh[i] = 0.f;
        m_sedminpsToCh[i] = 0.f;
    }

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (m_sedEroded[i] < 1.e-4f) m_sedEroded[i] = 0.f;
        // CREAMS method for calculating enrichment ratio
        m_enratio[i] = CalEnrichmentRatio(m_sedEroded[i], m_surfaceRunoff[i], m_cellArea);
        //if(i == 1000) cout << ""<< m_sedEroded[i]<<","<< m_surfaceRunoff[i] << "," << m_enratio[i]<<endl;

        //Calculates the amount of organic nitrogen removed in surface runoff
        if (m_CbnModel == 0) {
            OrgNRemovedInRunoff_StaticMethod(i);
        } else if (m_CbnModel == 1) {
            OrgNRemovedInRunoff_CFARMOneCarbonModel(i);
        } else if (m_CbnModel == 2) {
            OrgNRemovedInRunoff_CENTURY(i);
        }
        //Calculates the amount of organic and mineral phosphorus attached to sediment in surface runoff. psed.f of SWAT
        OrgPAttachedtoSed(i);
    }
    // sum by subbasin
    // See https://github.com/lreis2415/SEIMS/issues/36 for more descriptions. By lj
#pragma omp parallel
    {
        float *tmp_orgn2ch = new float[m_nSubbasins + 1];
        float *tmp_orgp2ch = new float[m_nSubbasins + 1];
        float *tmp_minpa2ch = new float[m_nSubbasins + 1];
        float *tmp_minps2ch = new float[m_nSubbasins + 1];
        for (int i = 0; i <= m_nSubbasins; i++) {
            tmp_orgn2ch[i] = 0.f;
            tmp_orgp2ch[i] = 0.f;
            tmp_minpa2ch[i] = 0.f;
            tmp_minps2ch[i] = 0.f;
        }
#pragma omp for
        for (int i = 0; i < m_nCells; i++) {
            tmp_orgn2ch[(int) m_subbasin[i]] += m_sedorgn[i];
            tmp_orgp2ch[(int) m_subbasin[i]] += m_sedorgp[i];
            tmp_minpa2ch[(int) m_subbasin[i]] += m_sedminpa[i];
            tmp_minps2ch[(int) m_subbasin[i]] += m_sedminps[i];
        }
#pragma omp critical
        {
            for (int i = 1; i <= m_nSubbasins; i++) {
                m_sedorgnToCh[i] += tmp_orgn2ch[i];
                m_sedorgpToCh[i] += tmp_orgp2ch[i];
                m_sedminpaToCh[i] += tmp_minpa2ch[i];
                m_sedminpsToCh[i] += tmp_minps2ch[i];
            }
        }
        delete[] tmp_orgn2ch;
        delete[] tmp_orgp2ch;
        delete[] tmp_minpa2ch;
        delete[] tmp_minps2ch;
    }  /* END of #pragma omp parallel */
    // sum all the subbasins and put the sum value in the zero-index of the array
    for (int i = 1; i < m_nSubbasins + 1; i++) {
        m_sedorgnToCh[i] *= m_cellArea;
        m_sedorgpToCh[i] *= m_cellArea;
        m_sedminpaToCh[i] *= m_cellArea;
        m_sedminpsToCh[i] *= m_cellArea;
        m_sedorgnToCh[0] += m_sedorgnToCh[i];
        m_sedorgpToCh[0] += m_sedorgpToCh[i];
        m_sedminpaToCh[0] += m_sedminpaToCh[i];
        m_sedminpsToCh[0] += m_sedminpsToCh[i];
    }
    return 0;
}

void NutrientTransportSediment::OrgNRemovedInRunoff_StaticMethod(int i) {
    //amount of organic N in first soil layer (orgninfl)
    float orgninfl = 0.f;
    //conversion factor (wt)
    float wt = 0.f;
    orgninfl = m_sol_orgn[i][0] + m_sol_aorgn[i][0] + m_sol_fon[i][0];
    wt = m_sol_bd[i][0] * m_soilThick[i][0] / 100.f;
    //concentration of organic N in soil (concn)
    float concn = 0.f;
    concn = orgninfl * m_enratio[i] / wt;
    //Calculate the amount of organic nitrogen transported with sediment to the stream, equation 4:2.2.1 in SWAT Theory 2009, p271
    m_sedorgn[i] = 0.001f * concn * m_sedEroded[i] / 1000.f / m_cellArea;  /// kg/ha
    //update soil nitrogen pools
    if (orgninfl > 1.e-6f) {
        m_sol_aorgn[i][0] = m_sol_aorgn[i][0] - m_sedorgn[i] * (m_sol_aorgn[i][0] / orgninfl);
        m_sol_orgn[i][0] = m_sol_orgn[i][0] - m_sedorgn[i] * (m_sol_orgn[i][0] / orgninfl);
        m_sol_fon[i][0] = m_sol_fon[i][0] - m_sedorgn[i] * (m_sol_fon[i][0] / orgninfl);
        if (m_sol_aorgn[i][0] < 0.f) {
            m_sedorgn[i] = m_sedorgn[i] + m_sol_aorgn[i][0];
            m_sol_aorgn[i][0] = 0.f;
        }
        if (m_sol_orgn[i][0] < 0.f) {
            m_sedorgn[i] = m_sedorgn[i] + m_sol_orgn[i][0];
            m_sol_orgn[i][0] = 0.f;
        }
        if (m_sol_fon[i][0] < 0.f) {
            m_sedorgn[i] = m_sedorgn[i] + m_sol_fon[i][0];
            m_sol_fon[i][0] = 0.f;
        }
    }
}

void NutrientTransportSediment::OrgNRemovedInRunoff_CFARMOneCarbonModel(int i) {
    /// TODO
}

void NutrientTransportSediment::OrgNRemovedInRunoff_CENTURY(int i) {
    float totOrgN_lyr0 = 0.f; /// kg N/ha, amount of organic N in first soil layer, i.e., xx in SWAT src.
    float wt1 = 0.f; /// conversion factor, mg/kg => kg/ha
    float er = 0.f; /// enrichment ratio
    float conc = 0.f; /// concentration of organic N in soil
    float sol_mass = 0.f; /// soil mass, kg
    float QBC = 0.f; /// C loss with runoff or lateral flow
    float VBC = 0.f; /// C loss with vertical flow
    float YBC = 0.f; /// BMC loss with sediment
    float YOC = 0.f; /// Organic C loss with sediment
    float YW = 0.f; /// Wind erosion, kg
    float TOT = 0.f; /// total organic carbon in layer 1
    float YEW = 0.f; /// fraction of soil erosion of total soil mass
    float X1 = 0.f, PRMT_21 = 0.f;
    float PRMT_44 = 0.f; /// ratio of soluble C concentration in runoff to percolate (0.1 - 1.0)
    float XX = 0.f, DK = 0.f, V = 0.f, X3 = 0.f;
    float CO = 0.f; /// the vertical concentration
    float CS = 0.f; /// the horizontal concentration
    float perc_clyr = 0.f, latc_clyr = 0.f;

    totOrgN_lyr0 = m_sol_LSN[i][0] + m_sol_LMN[i][0] + m_sol_HPN[i][0] + m_sol_HSN[i][0];
    wt1 = m_sol_bd[i][0] * m_soilThick[i][0] / 100.f;
    er = m_enratio[i];
    conc = totOrgN_lyr0 * er / wt1;
    m_sedorgn[i] = 0.001f * conc * m_sedEroded[i] / 1000.f / m_cellArea;
    /// update soil nitrogen pools
    if (totOrgN_lyr0 > UTIL_ZERO) {
        float xx1 = (1.f - m_sedorgn[i] / totOrgN_lyr0);
        m_sol_LSN[i][0] *= xx1;
        m_sol_LMN[i][0] *= xx1;
        m_sol_HPN[i][0] *= xx1;
        m_sol_HSN[i][0] *= xx1;
    }
    /// Calculate runoff and leached C&N from micro-biomass
    sol_mass = m_soilThick[i][0] / 1000.f * 10000.f * m_sol_bd[i][0] * 1000.f *
        (1.f - m_sol_rock[i][0] / 100.f); /// kg/ha
    /// total organic carbon in layer 1
    TOT = m_sol_HPC[i][0] + m_sol_HSC[i][0] + m_sol_LMC[i][0] + m_sol_LSC[i][0];
    /// fraction of soil erosion of total soil mass
    YEW = min((m_sedEroded[i] / m_cellArea + YW / m_cellArea) / sol_mass, 0.9f);

    X1 = 1.f - YEW;
    YOC = YEW * TOT;
    m_sol_HSC[i][0] *= X1;
    m_sol_HPC[i][0] *= X1;
    m_sol_LS[i][0] *= X1;
    m_sol_LM[i][0] *= X1;
    m_sol_LSL[i][0] *= X1;
    m_sol_LSC[i][0] *= X1;
    m_sol_LMC[i][0] *= X1;
    m_sol_LSLC[i][0] *= X1;
    m_sol_LSLNC[i][0] = m_sol_LSC[i][0] - m_sol_LSLC[i][0];

    if (m_sol_BMC[i][0] > 0.01f) {
        ///KOC FOR CARBON LOSS IN WATER AND SEDIMENT(500._1500.) KD = KOC * C
        PRMT_21 = 1000.f;
        m_sol_WOC[i][0] = m_sol_LSC[i][0] + m_sol_LMC[i][0] + m_sol_HPC[i][0] + m_sol_HSC[i][0] + m_sol_BMC[i][0];
        DK = 0.0001f * PRMT_21 * m_sol_WOC[i][0];
        X1 = m_sol_wsatur[i][0];
        if (X1 <= 0.f) X1 = 0.01f;
        XX = X1 + DK;
        V = m_surfaceRunoff[i] + m_sol_perco[i][0] + m_sol_laterq[i][0];
        if (V > 1.e-10f) {
            X3 = m_sol_BMC[i][0] * (1.f - exp(-V / XX)); /// loss of biomass C
            PRMT_44 = 0.5;
            CO = X3 / (m_sol_perco[i][0] + PRMT_44 * (m_surfaceRunoff[i] + m_sol_laterq[i][0]));
            CS = PRMT_44 * CO;
            VBC = CO * m_sol_perco[i][0];
            m_sol_BMC[i][0] -= X3;
            QBC = CS * (m_surfaceRunoff[i] + m_sol_laterq[i][0]);
            /// Compute WBMC loss with sediment
            if (YEW > 0.f) {
                CS = DK * m_sol_BMC[i][0] / XX;
                YBC = YEW * CS;
            }
        }
    }
    m_sol_BMC[i][0] -= YBC;
    /// surfqc_d(j) = QBC*(surfq(j)/(surfq(j)+flat(1,j)+1.e-6))  is for print purpose, thus not implemented.
    m_sol_latC[i][0] = QBC * (m_sol_laterq[i][0] / (m_surfaceRunoff[i] + m_sol_laterq[i][0] + UTIL_ZERO));
    m_sol_percoC[i][0] = VBC;
    m_sedCLoss[i] = YOC + YBC;

    latc_clyr += m_sol_latC[i][0];
    for (int k = 1; k < (int) m_nSoilLayers[i]; k++) {
        m_sol_WOC[i][k] = m_sol_LSC[i][k] + m_sol_LMC[i][k] + m_sol_HPC[i][k] + m_sol_HSC[i][k];
        float Y1 = m_sol_BMC[i][k] + VBC;
        VBC = 0.f;
        if (Y1 > 0.01f) {
            V = m_sol_perco[i][k] + m_sol_laterq[i][k];
            if (V > 0.f) {
                VBC = Y1 * (1.f - exp(-V / (m_sol_wsatur[i][k] + 0.0001f * PRMT_21 * m_sol_WOC[i][k])));
            }
        }
        m_sol_latC[i][k] = VBC * (m_sol_laterq[i][k] / (m_sol_laterq[i][k] + m_sol_perco[i][k] + UTIL_ZERO));
        m_sol_percoC[i][k] = VBC - m_sol_latC[i][k];
        m_sol_BMC[i][k] = Y1 - VBC;

        /// calculate nitrate in percolate and lateral flow
        perc_clyr += m_sol_percoC[i][k];
        latc_clyr += m_sol_latC[i][k];
    }
    m_laterC[i] = latc_clyr;
    m_percoC[i] = perc_clyr;
}

void NutrientTransportSediment::OrgPAttachedtoSed(int i) {
    //amount of phosphorus attached to sediment in soil (sol_attp)
    float sol_attp = 0.f;
    //fraction of active mineral/organic/stable mineral phosphorus in soil (sol_attp_o, sol_attp_a, sol_attp_s)
    float sol_attp_o = 0.f;
    float sol_attp_a = 0.f;
    float sol_attp_s = 0.f;
    //Calculate sediment
    sol_attp = m_sol_orgp[i][0] + m_sol_fop[i][0] + m_sol_actp[i][0] + m_sol_stap[i][0];
    if (m_sol_mp != nullptr) {
        sol_attp += m_sol_mp[i][0];
    }
    if (sol_attp > 1.e-3f) {
        sol_attp_o = (m_sol_orgp[i][0] + m_sol_fop[i][0]) / sol_attp;
        if (m_sol_mp != nullptr) {
            sol_attp_o += m_sol_mp[i][0] / sol_attp;
        }
        sol_attp_a = m_sol_actp[i][0] / sol_attp;
        sol_attp_s = m_sol_stap[i][0] / sol_attp;
    }
    //conversion factor (mg/kg => kg/ha) (wt)
    float wt = m_sol_bd[i][0] * m_soilThick[i][0] / 100.f;
    //concentration of organic P in soil (concp)
    float concp = 0.f;
    concp = sol_attp * m_enratio[i] / wt;  /// mg/kg
    //total amount of P removed in sediment erosion (sedp)
    float sedp = 1.e-6f * concp * m_sedEroded[i] / m_cellArea; /// kg/ha
    m_sedorgp[i] = sedp * sol_attp_o;
    m_sedminpa[i] = sedp * sol_attp_a;
    m_sedminps[i] = sedp * sol_attp_s;

    //if(i==100)cout << "sedp: " << sedp<< ",sol_attp_o: "  << sol_attp_o << endl;
    //modify phosphorus pools

    //total amount of P in mineral sediment pools prior to sediment removal (psedd)		// Not used
    //float psedd = 0.f;
    //psedd = m_sol_actp[i][0] + m_sol_stap[i][0];

    //total amount of P in organic pools prior to sediment removal (porgg)
    float porgg = 0.f;
    porgg = m_sol_orgp[i][0] + m_sol_fop[i][0];
    if (porgg > 1.e-3f) {
        m_sol_orgp[i][0] = m_sol_orgp[i][0] - m_sedorgp[i] * (m_sol_orgp[i][0] / porgg);
        m_sol_fop[i][0] = m_sol_fop[i][0] - m_sedorgp[i] * (m_sol_fop[i][0] / porgg);
    }
    m_sol_actp[i][0] = m_sol_actp[i][0] - m_sedminpa[i];
    m_sol_stap[i][0] = m_sol_stap[i][0] - m_sedminps[i];
    if (m_sol_orgp[i][0] < 0.f) {
        m_sedorgp[i] = m_sedorgp[i] + m_sol_orgp[i][0];
        m_sol_orgp[i][0] = 0.f;
    }
    if (m_sol_fop[i][0] < 0.f) {
        m_sedorgp[i] = m_sedorgp[i] + m_sol_fop[i][0];
        m_sol_fop[i][0] = 0.f;
    }
    if (m_sol_actp[i][0] < 0.f) {
        m_sedminpa[i] = m_sedminpa[i] + m_sol_actp[i][0];
        m_sol_actp[i][0] = 0.f;
    }
    if (m_sol_stap[i][0] < 0.f) {
        m_sedminps[i] = m_sedminps[i] + m_sol_stap[i][0];
        m_sol_stap[i][0] = 0.f;
    }
}

void NutrientTransportSediment::Get1DData(const char *key, int *n, float **data) {
     InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SEDORGN)) {
        *data = m_sedorgn;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_SEDORGP)) {
        *data = m_sedorgp;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_SEDMINPA)) {
        *data = m_sedminpa;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_SEDMINPS)) {
        *data = m_sedminps;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_SEDORGN_TOCH)) {
        *data = m_sedorgnToCh;
        *n = m_nSubbasins + 1;
    } else if (StringMatch(sk, VAR_SEDORGP_TOCH)) {
        *data = m_sedorgpToCh;
        *n = m_nSubbasins + 1;
    } else if (StringMatch(sk, VAR_SEDMINPA_TOCH)) {
        *data = m_sedminpaToCh;
        *n = m_nSubbasins + 1;
    } else if (StringMatch(sk, VAR_SEDMINPS_TOCH)) {
        *data = m_sedminpsToCh;
        *n = m_nSubbasins + 1;
    }
        /// outputs of CENTURY C/N cycling model
    else if (StringMatch(sk, VAR_LATERAL_C)) {
        *data = m_laterC;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_PERCO_C)) {
        *data = m_percoC;
        *n = m_nCells;
    } else if (StringMatch(sk, VAR_SEDLOSS_C)) {
        *data = m_sedCLoss;
        *n = m_nCells;
    } else {
        throw ModelException(MID_NUTRSED, "Get1DData", "Parameter " + sk + " does not exist");
    }
}

void NutrientTransportSediment::Get2DData(const char *key, int *nRows, int *nCols, float ***data) {
     InitialOutputs();
    string sk(key);
    *nRows = m_nCells;
    *nCols = m_nMaxSoiLayers;
    if (StringMatch(sk, VAR_SOL_AORGN)) { *data = m_sol_aorgn; }
    else if (StringMatch(sk, VAR_SOL_FORGN)) { *data = m_sol_fon; }
    else if (StringMatch(sk, VAR_SOL_SORGN)) { *data = m_sol_orgn; }
    else if (StringMatch(sk, VAR_SOL_HORGP)) { *data = m_sol_orgp; }
    else if (StringMatch(sk, VAR_SOL_FORGP)) { *data = m_sol_fop; }
    else if (StringMatch(sk, VAR_SOL_STAP)) { *data = m_sol_stap; }
    else if (StringMatch(sk, VAR_SOL_ACTP)) { *data = m_sol_actp; }
        /// outputs of CENTURY C/N cycling model
    else if (StringMatch(sk, VAR_SOL_LATERAL_C)) { *data = m_sol_latC; }
    else if (StringMatch(sk, VAR_SOL_PERCO_C)) { *data = m_sol_percoC; }
    else {
        throw ModelException(MID_NUTRSED, "Get2DData", "Output " + sk + " does not exist.");
    }
}
