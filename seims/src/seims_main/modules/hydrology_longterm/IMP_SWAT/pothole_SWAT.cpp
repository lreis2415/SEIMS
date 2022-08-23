#include "pothole_SWAT.h"

#include "text.h"

IMP_SWAT::IMP_SWAT() :
    m_cnv(NODATA_VALUE), m_nCells(-1), m_cellWidth(NODATA_VALUE), m_cellArea(NODATA_VALUE), m_timestep(-1),
    m_nSoilLyrs(nullptr), m_maxSoilLyrs(-1), m_subbasin(nullptr), m_nSubbasins(-1),
    m_rteLyrs(nullptr), m_nRteLyrs(-1),
    m_evLAI(NODATA_VALUE), m_slope(nullptr), m_ks(nullptr), m_sol_sat(nullptr), m_sol_sumfc(nullptr),
    m_soilThick(nullptr),
    m_sol_por(nullptr), m_potTilemm(0.), m_potNo3Decay(NODATA_VALUE),
    m_potSolPDecay(NODATA_VALUE),
    m_kVolat(NODATA_VALUE),
    m_kNitri(NODATA_VALUE), m_pot_k(NODATA_VALUE), m_impoundTrig(nullptr), m_potArea(nullptr),
    m_LAIDay(nullptr), m_pet(nullptr),
    m_depEvapor(nullptr), m_depStorage(nullptr), m_surfaceRunoff(nullptr), m_sedYield(nullptr), m_sandYield(nullptr),
    m_siltYield(nullptr),
    m_clayYield(nullptr), m_smaggreYield(nullptr), m_lgaggreYield(nullptr), m_soilStorage(nullptr),
    m_soilStorageProfile(nullptr),
    m_surqNo3(nullptr), m_surqNH4(nullptr), m_surqSolP(nullptr), m_surqCOD(nullptr),
    m_sedOrgN(nullptr), m_sedOrgP(nullptr), m_sedActiveMinP(nullptr), m_sedStableMinP(nullptr), m_potNo3(nullptr),
    m_potNH4(nullptr),
    m_potOrgN(nullptr), m_potSolP(nullptr), m_potOrgP(nullptr), m_potActMinP(nullptr), m_potStaMinP(nullptr),
    m_potSed(nullptr), m_potSand(nullptr),
    m_potSilt(nullptr), m_potClay(nullptr), m_potSag(nullptr), m_potLag(nullptr), m_potVol(nullptr),
    m_potVolMax(nullptr),
    m_potVolMin(nullptr), m_potSeep(nullptr), m_potEvap(nullptr),
    /// overland to channel
    m_surfqToCh(nullptr), m_sedToCh(nullptr), m_surNO3ToCh(nullptr), m_surNH4ToCh(nullptr),
    m_surSolPToCh(nullptr), m_surCodToCh(nullptr),
    m_sedOrgNToCh(nullptr), m_sedOrgPToCh(nullptr), m_sedMinPAToCh(nullptr), m_sedMinPSToCh(nullptr) {
    //m_potSedIn(nullptr), m_potSandIn(nullptr), m_potSiltIn(nullptr), m_potClayIn(nullptr), m_potSagIn(nullptr), m_potLagIn(nullptr),
}

IMP_SWAT::~IMP_SWAT() {
    if (m_potArea != nullptr) Release1DArray(m_potArea);
    if (m_potNo3 != nullptr) Release1DArray(m_potNo3);
    if (m_potNH4 != nullptr) Release1DArray(m_potNH4);
    if (m_potOrgN != nullptr) Release1DArray(m_potOrgN);
    if (m_potSolP != nullptr) Release1DArray(m_potSolP);
    if (m_potOrgP != nullptr) Release1DArray(m_potOrgP);
    if (m_potActMinP != nullptr) Release1DArray(m_potActMinP);
    if (m_potStaMinP != nullptr) Release1DArray(m_potStaMinP);
    if (m_potSed != nullptr) Release1DArray(m_potSed);
    if (m_potSand != nullptr) Release1DArray(m_potSand);
    if (m_potSilt != nullptr) Release1DArray(m_potSilt);
    if (m_potClay != nullptr) Release1DArray(m_potClay);
    if (m_potSag != nullptr) Release1DArray(m_potSag);
    if (m_potLag != nullptr) Release1DArray(m_potLag);
    if (m_potVol != nullptr) Release1DArray(m_potVol);
    if (m_potSeep != nullptr) Release1DArray(m_potSeep);
    if (m_potEvap != nullptr) Release1DArray(m_potEvap);
}

bool IMP_SWAT::CheckInputData() {
    CHECK_POSITIVE(M_IMP_SWAT[0], m_nCells);
    CHECK_POSITIVE(M_IMP_SWAT[0], m_cellWidth);
    CHECK_POSITIVE(M_IMP_SWAT[0], m_maxSoilLyrs);
    CHECK_POSITIVE(M_IMP_SWAT[0], m_nRteLyrs);
    CHECK_POSITIVE(M_IMP_SWAT[0], m_evLAI);
    CHECK_NONNEGATIVE(M_IMP_SWAT[0], m_potTilemm);
    CHECK_NONNEGATIVE(M_IMP_SWAT[0], m_potNo3Decay);
    CHECK_NONNEGATIVE(M_IMP_SWAT[0], m_potSolPDecay);
    return true;
}

void IMP_SWAT::SetValue(const char* key, const FLTPT value) {
    string sk(key);
    if (StringMatch(sk, Tag_CellWidth[0])) {
        m_cellWidth = value;
        m_cellArea = m_cellWidth * m_cellWidth * 1.e-4; // m2 ==> ha
        m_cnv = 10. * m_cellArea;                       // mm/ha => m^3
    } else if (StringMatch(sk, VAR_EVLAI[0])) m_evLAI = value;
    else if (StringMatch(sk, VAR_POT_TILE[0])) m_potTilemm = value;
    else if (StringMatch(sk, VAR_POT_NO3DECAY[0])) m_potNo3Decay = value;
    else if (StringMatch(sk, VAR_POT_SOLPDECAY[0])) m_potSolPDecay = value;
    else if (StringMatch(sk, VAR_KV_PADDY[0])) m_kVolat = value;
    else if (StringMatch(sk, VAR_KN_PADDY[0])) m_kNitri = value;
    else if (StringMatch(sk, VAR_POT_K[0])) m_pot_k = value;
    else {
        throw ModelException(M_IMP_SWAT[0], "SetValue",
                             "Parameter " + sk + " does not exist.");
    }
}

void IMP_SWAT::SetValue(const char* key, const int value) {
    string sk(key);
    if (StringMatch(sk, Tag_TimeStep[0])) m_timestep = value;
    else {
        throw ModelException(M_IMP_SWAT[0], "SetValue",
                             "Integer Parameter " + sk + " does not exist.");
    }
}

void IMP_SWAT::Set1DData(const char* key, const int n, FLTPT* data) {
    string sk(key);
    if (StringMatch(sk, VAR_SBOF[0])) {
        m_surfqToCh = data;
        m_nSubbasins = n - 1; /// TODO, add a checkInputSize2 function
        return;
    } else if (StringMatch(sk, VAR_SED_TO_CH[0])) {
        m_sedToCh = data;
        m_nSubbasins = n - 1;
        return;
    } else if (StringMatch(sk, VAR_SUR_NO3_TOCH[0])) {
        m_surNO3ToCh = data;
        m_nSubbasins = n - 1;
        return;
    } else if (StringMatch(sk, VAR_SUR_NH4_TOCH[0])) {
        m_surNH4ToCh = data;
        m_nSubbasins = n - 1;
        return;
    } else if (StringMatch(sk, VAR_SUR_SOLP_TOCH[0])) {
        m_surSolPToCh = data;
        m_nSubbasins = n - 1;
        return;
    } else if (StringMatch(sk, VAR_SUR_COD_TOCH[0])) {
        m_surCodToCh = data;
        m_nSubbasins = n - 1;
        return;
    } else if (StringMatch(sk, VAR_SEDORGN_TOCH[0])) {
        m_sedOrgNToCh = data;
        m_nSubbasins = n - 1;
        return;
    } else if (StringMatch(sk, VAR_SEDORGP_TOCH[0])) {
        m_sedOrgPToCh = data;
        m_nSubbasins = n - 1;
        return;
    } else if (StringMatch(sk, VAR_SEDMINPA_TOCH[0])) {
        m_sedMinPAToCh = data;
        m_nSubbasins = n - 1;
        return;
    } else if (StringMatch(sk, VAR_SEDMINPS_TOCH[0])) {
        m_sedMinPSToCh = data;
        m_nSubbasins = n - 1;
        return;
    }
    CheckInputSize(M_IMP_SWAT[0], key, n, m_nCells);
    if (StringMatch(sk, VAR_SLOPE[0])) m_slope = data;
    else if (StringMatch(sk, VAR_SOL_SUMAWC[0])) m_sol_sumfc = data;
    else if (StringMatch(sk, VAR_POT_VOLMAXMM[0])) m_potVolMax = data;
    else if (StringMatch(sk, VAR_POT_VOLLOWMM[0])) m_potVolMin = data;
    else if (StringMatch(sk, VAR_SEDYLD[0])) m_sedYield = data;
    else if (StringMatch(sk, VAR_SANDYLD[0])) m_sandYield = data;
    else if (StringMatch(sk, VAR_SILTYLD[0])) m_siltYield = data;
    else if (StringMatch(sk, VAR_CLAYYLD[0])) m_clayYield = data;
    else if (StringMatch(sk, VAR_SAGYLD[0])) m_smaggreYield = data;
    else if (StringMatch(sk, VAR_LAGYLD[0])) m_lgaggreYield = data;
    else if (StringMatch(sk, VAR_LAIDAY[0])) m_LAIDay = data;
    else if (StringMatch(sk, VAR_PET[0])) m_pet = data;
    else if (StringMatch(sk, VAR_SOL_SW[0])) m_soilStorageProfile = data;
    else if (StringMatch(sk, VAR_DEET[0])) m_depEvapor = data;
    else if (StringMatch(sk, VAR_DPST[0])) m_depStorage = data;
    else if (StringMatch(sk, VAR_OLFLOW[0])) m_surfaceRunoff = data;
    else if (StringMatch(sk, VAR_SUR_NO3[0])) m_surqNo3 = data;
    else if (StringMatch(sk, VAR_SUR_NH4[0])) m_surqNH4 = data;
    else if (StringMatch(sk, VAR_SUR_SOLP[0])) m_surqSolP = data;
    else if (StringMatch(sk, VAR_SUR_COD[0])) m_surqCOD = data;
    else if (StringMatch(sk, VAR_SEDORGN[0])) m_sedOrgN = data;
    else if (StringMatch(sk, VAR_SEDORGP[0])) m_sedOrgP = data;
    else if (StringMatch(sk, VAR_SEDMINPA[0])) m_sedActiveMinP = data;
    else if (StringMatch(sk, VAR_SEDMINPS[0])) m_sedStableMinP = data;
    else {
        throw ModelException(M_IMP_SWAT[0], "Set1DData",
                             "Parameter " + sk + " does not exist.");
    }
}


void IMP_SWAT::Set1DData(const char* key, const int n, int* data) {
    string sk(key);
    CheckInputSize(M_IMP_SWAT[0], key, n, m_nCells);
    if (StringMatch(sk, VAR_SOILLAYERS[0])) m_nSoilLyrs = data;
    else if (StringMatch(sk, VAR_SUBBSN[0])) m_subbasin = data;
    else if (StringMatch(sk, VAR_IMPOUND_TRIG[0])) m_impoundTrig = data;
    else {
        throw ModelException(M_IMP_SWAT[0], "Set1DData",
                             "Integer Parameter " + sk + " does not exist.");
    }
}

void IMP_SWAT::Set2DData(const char* key, const int n, const int col, FLTPT** data) {
    string sk(key);
    CheckInputSize2D(M_IMP_SWAT[0], key, n, col, m_nCells, m_maxSoilLyrs);
    if (StringMatch(sk, VAR_CONDUCT[0])) m_ks = data;
    else if (StringMatch(sk, VAR_SOILTHICK[0])) m_soilThick = data;
    else if (StringMatch(sk, VAR_POROST[0])) m_sol_por = data;
    else if (StringMatch(sk, VAR_SOL_ST[0])) m_soilStorage = data;
    else if (StringMatch(sk, VAR_SOL_UL[0])) m_sol_sat = data;
    else {
        throw ModelException(M_IMP_SWAT[0], "Set2DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void IMP_SWAT::Set2DData(const char* key, const int n, const int col, int** data) {
    string sk(key);
    if (StringMatch(sk, Tag_ROUTING_LAYERS[0])) {
        m_nRteLyrs = n;
        m_rteLyrs = data;
        return;
    }
    throw ModelException(M_IMP_SWAT[0], "Set2DData",
                         "Integer Parameter " + sk + " does not exist.");
}

void IMP_SWAT::InitialOutputs() {
    CHECK_POSITIVE(M_IMP_SWAT[0], m_nCells);
    if (m_potArea == nullptr) Initialize1DArray(m_nCells, m_potArea, 0.);
    if (m_potVol == nullptr) Initialize1DArray(m_nCells, m_potVol, 0.);
    if (m_potNo3 == nullptr) Initialize1DArray(m_nCells, m_potNo3, 0.);
    if (m_potNH4 == nullptr) Initialize1DArray(m_nCells, m_potNH4, 0.);
    if (m_potOrgN == nullptr) Initialize1DArray(m_nCells, m_potOrgN, 0.);
    if (m_potSolP == nullptr) Initialize1DArray(m_nCells, m_potSolP, 0.);
    if (m_potOrgP == nullptr) Initialize1DArray(m_nCells, m_potOrgP, 0.);
    if (m_potActMinP == nullptr) Initialize1DArray(m_nCells, m_potActMinP, 0.);
    if (m_potStaMinP == nullptr) Initialize1DArray(m_nCells, m_potStaMinP, 0.);
    if (m_potSed == nullptr) Initialize1DArray(m_nCells, m_potSed, 0.);
    if (m_potSand == nullptr) Initialize1DArray(m_nCells, m_potSand, 0.);
    if (m_potSilt == nullptr) Initialize1DArray(m_nCells, m_potSilt, 0.);
    if (m_potClay == nullptr) Initialize1DArray(m_nCells, m_potClay, 0.);
    if (m_potSag == nullptr) Initialize1DArray(m_nCells, m_potSag, 0.);
    if (m_potLag == nullptr) Initialize1DArray(m_nCells, m_potLag, 0.);
    /// water loss
    if (m_potSeep == nullptr) Initialize1DArray(m_nCells, m_potSeep, 0.);
    if (m_potEvap == nullptr) Initialize1DArray(m_nCells, m_potEvap, 0.);
}

int IMP_SWAT::Execute() {
    CheckInputData();
    InitialOutputs();

    for (int ilyr = 0; ilyr < m_nRteLyrs; ilyr++) {
        // There are not any flow relationship within each routing layer.
        // So parallelization can be done here.
        int ncells = CVT_INT(m_rteLyrs[ilyr][0]);
#pragma omp parallel for
        for (int icell = 1; icell <= ncells; icell++) {
            int id = CVT_INT(m_rteLyrs[ilyr][icell]); // cell index
            if (FloatEqual(m_impoundTrig[id], 0.)) {
                /// if impounding trigger on
                PotholeSimulate(id);
            } else {
                ReleaseWater(id);
            }
        }
    }
    /// reCalculate the surface runoff, sediment, nutrient etc. that into the channel
#pragma omp parallel for
    for (int i = 0; i <= m_nSubbasins; i++) {
        m_surfqToCh[i] = 0.;
        m_sedToCh[i] = 0.;
        m_surNO3ToCh[i] = 0.;
        m_surNH4ToCh[i] = 0.;
        m_surSolPToCh[i] = 0.;
        m_surCodToCh[i] = 0.;
        m_sedOrgNToCh[i] = 0.;
        m_sedOrgPToCh[i] = 0.;
        m_sedMinPAToCh[i] = 0.;
        m_sedMinPSToCh[i] = 0.;
    }
    // See https://github.com/lreis2415/SEIMS/issues/36 for more descriptions. By lj
#pragma omp parallel
    {
        FLTPT* tmp_surfq2ch = new(nothrow) FLTPT[m_nSubbasins + 1];
        FLTPT* tmp_sed2ch = new(nothrow) FLTPT[m_nSubbasins + 1];
        FLTPT* tmp_sno32ch = new(nothrow) FLTPT[m_nSubbasins + 1];
        FLTPT* tmp_snh42ch = new(nothrow) FLTPT[m_nSubbasins + 1];
        FLTPT* tmp_solp2ch = new(nothrow) FLTPT[m_nSubbasins + 1];
        FLTPT* tmp_cod2ch = new(nothrow) FLTPT[m_nSubbasins + 1];
        FLTPT* tmp_orgn2ch = new(nothrow) FLTPT[m_nSubbasins + 1];
        FLTPT* tmp_orgp2ch = new(nothrow) FLTPT[m_nSubbasins + 1];
        FLTPT* tmp_minpa2ch = new(nothrow) FLTPT[m_nSubbasins + 1];
        FLTPT* tmp_minps2ch = new(nothrow) FLTPT[m_nSubbasins + 1];
        for (int i = 0; i <= m_nSubbasins; i++) {
            tmp_surfq2ch[i] = 0.;
            tmp_sed2ch[i] = 0.;
            tmp_sno32ch[i] = 0.;
            tmp_snh42ch[i] = 0.;
            tmp_solp2ch[i] = 0.;
            tmp_cod2ch[i] = 0.;
            tmp_orgn2ch[i] = 0.;
            tmp_orgp2ch[i] = 0.;
            tmp_minpa2ch[i] = 0.;
            tmp_minps2ch[i] = 0.;
        }
#pragma omp for
        for (int i = 0; i < m_nCells; i++) {
            int subi = CVT_INT(m_subbasin[i]);
            tmp_surfq2ch[subi] += m_surfaceRunoff[i] * 10. / m_timestep; /// (* m_cellArea, later) mm -> m3/s
            tmp_sed2ch[subi] += m_sedYield[i];
            tmp_sno32ch[subi] += m_surqNo3[i];
            tmp_snh42ch[subi] += m_surqNH4[i];
            tmp_solp2ch[subi] += m_surqSolP[i];
            tmp_cod2ch[subi] += m_surqCOD[i];
            tmp_orgn2ch[subi] += m_sedOrgN[i];
            tmp_orgp2ch[subi] += m_sedOrgP[i];
            tmp_minpa2ch[subi] += m_sedActiveMinP[i];
            tmp_minps2ch[subi] += m_sedStableMinP[i];
        }
#pragma omp critical
        {
            for (int i = 1; i <= m_nSubbasins; i++) {
                m_surfqToCh[i] += tmp_surfq2ch[i] * m_cellArea;
                m_sedToCh[i] += tmp_sed2ch[i];
                m_surNO3ToCh[i] += tmp_sno32ch[i] * m_cellArea;
                m_surNH4ToCh[i] += tmp_snh42ch[i] * m_cellArea;
                m_surSolPToCh[i] += tmp_solp2ch[i] * m_cellArea;
                m_surCodToCh[i] += tmp_cod2ch[i] * m_cellArea;
                m_sedOrgNToCh[i] += tmp_orgn2ch[i] * m_cellArea;
                m_sedOrgPToCh[i] += tmp_orgp2ch[i] * m_cellArea;
                m_sedMinPAToCh[i] += tmp_minpa2ch[i] * m_cellArea;
                m_sedMinPSToCh[i] += tmp_minps2ch[i] * m_cellArea;
            }
        }
        delete[] tmp_surfq2ch;
        delete[] tmp_sed2ch;
        delete[] tmp_sno32ch;
        delete[] tmp_snh42ch;
        delete[] tmp_solp2ch;
        delete[] tmp_cod2ch;
        delete[] tmp_orgn2ch;
        delete[] tmp_orgp2ch;
        delete[] tmp_minpa2ch;
        delete[] tmp_minps2ch;
    } /* END of #pragma omp parallel */

    for (int i = 1; i <= m_nSubbasins; i++) {
        m_surfqToCh[0] += m_surfqToCh[i];
        m_sedToCh[0] += m_sedToCh[i];
        m_surNO3ToCh[0] += m_surNO3ToCh[i];
        m_surNH4ToCh[0] += m_surNH4ToCh[i];
        m_surSolPToCh[0] += m_surSolPToCh[i];
        m_surCodToCh[0] += m_surCodToCh[i];
        m_sedOrgNToCh[0] += m_sedOrgNToCh[i];
        m_sedOrgPToCh[0] += m_sedOrgPToCh[i];
        m_sedMinPAToCh[0] += m_sedMinPAToCh[i];
        m_sedMinPSToCh[0] += m_sedMinPSToCh[i];
    }
    return true;
}

void IMP_SWAT::PotholeSimulate(const int id) {
    /// initialize temporary variables
    FLTPT tileo = 0.; /// m^3, amount of water released to the main channel from the water body by drainage tiles
    //FLTPT potevmm = 0.; /// mm, volume of water evaporated from pothole expressed as depth
    FLTPT potev = 0.; /// m^3, evaporation from impounded water body
    FLTPT spillo = 0.;
    /// m^3, amount of water released to the main channel from impounded water body due to spill-over

    /// potpcpmm and potpcp should be implicitly included in (m_depStorage + m_depEvapor) if stated
    //FLTPT potpcpmm = 0.; /// mm, precipitation falling on pothole water body expressed as depth
    //FLTPT potpcp = 0.; /// m^3, precipitation falling on water body

    //FLTPT potsepmm = 0.; // mm, seepage from impounded water body expressed as depth
    FLTPT potsep = 0.; /// m^3, seepage from impounded water body
    //FLTPT sumo = 0.; /// m^3, sum of all releases from water body on current day
    //FLTPT potflwo = 0.; /// mm, discharge from pothole expressed as depth
    FLTPT potsedo = 0.;  /// kg, sediment leaving pothole on day
    FLTPT potsano = 0.;  /// kg, sand content in sediment leaving pothole on day
    FLTPT potsilo = 0.;  /// kg, silt content
    FLTPT potclao = 0.;  /// kg, clay content
    FLTPT potsago = 0.;  /// kg, small aggregate
    FLTPT potlago = 0.;  /// kg, large aggregate
    FLTPT potno3o = 0.;  /// kg, no3 amount out of pothole
    FLTPT potnh4o = 0.;  /// kg, nh4 amount out of pothole
    FLTPT potsolpo = 0.; /// kg, soluble phosphorus out of pothole
    FLTPT potorgno = 0.; /// kg, orgN out
    FLTPT potorgpo = 0.; /// kg, orgP out
    FLTPT potmpso = 0.;  /// kg, stable mineral phosphorus out
    FLTPT potmpao = 0.;  /// kg, active mineral phosphorus out
    //FLTPT potvol_ini = 0.; /// m^3, pothole volume at the begin of the day
    //FLTPT potsa_ini = 0.; /// ha, surface area of impounded water body at the begin of the day
    FLTPT sedloss = 0.; /// kg, amount of sediment settling out of water during day
    FLTPT sanloss = 0.;
    FLTPT silloss = 0.;
    FLTPT claloss = 0.;
    FLTPT sagloss = 0.;
    FLTPT lagloss = 0.;
    FLTPT no3loss = 0.;   /// kg, amount of nitrate lost from water body
    FLTPT nh4loss = 0.;   /// kg, amount of ammonian lost
    FLTPT solploss = 0.;  /// kg, amount of solP lost
    FLTPT orgnloss = 0.;  /// kg, amount of orgN lost
    FLTPT orgploss = 0.;  /// kg, amount of orgP lost
    FLTPT minpsloss = 0.; /// kg, amount of stable minP lost
    FLTPT minpaloss = 0.; /// kg, amount of active minP lost
    /* pot_fr is the fraction of the cell draining into the pothole
     * the remainder (1-pot_fr) goes directly to runoff
     * currently, we assumed that the entire cell is pothole/impounded area
     */
    FLTPT pot_fr = 1.;
    FLTPT qIn = m_surfaceRunoff[id] * pot_fr;
    /// inflow = surface flow, not include lateral flow, groundwater, etc.
    FLTPT qdayTmp = m_surfaceRunoff[id] * (1 - pot_fr); /// qdayTmp is the actual surface runoff generated
    if (m_depStorage != nullptr && m_depStorage[id] > 0.) {
        qIn += m_depStorage[id]; /// depression storage should be added
        m_depStorage[id] = 0.;
    }
    if (m_depEvapor != nullptr && m_depEvapor[id] > 0.) {
        qIn += m_depEvapor[id]; /// since the evaporation will be calculated below, the m_depEvapor should be added
        m_depEvapor[id] = 0.;
    }
    /// update volume of water in pothole
    m_potVol[id] += qIn;
    //m_potFlowIn[id] += qIn; // TODO, this should be routing cell by cell. by lj

    /* compute surface area of pothole
     * SWAT assuming a cone shape, ha
     * i.e., potholeSurfaceArea(id);
     * However, currently, we assume it is cell area
     */
    m_potArea[id] = m_cellArea;
    //potvol_ini = m_potVol[id];
    //potsa_ini = m_potSurfaceArea[id];

    /// update sediment in pothole
    m_potSed[id] += m_sedYield[id] * pot_fr;
    // FLTPT m_potSedIn = m_potSed[id];
    m_potSand[id] += m_sandYield[id] * pot_fr;
    FLTPT m_potSandIn = m_potSand[id];
    m_potSilt[id] += m_siltYield[id] * pot_fr;
    FLTPT m_potSiltIn = m_potSilt[id];
    m_potClay[id] += m_clayYield[id] * pot_fr;
    FLTPT m_potClayIn = m_potClay[id];
    m_potSag[id] += m_smaggreYield[id] * pot_fr;
    FLTPT m_potSagIn = m_potSag[id];
    m_potLag[id] += m_lgaggreYield[id] * pot_fr;
    FLTPT m_potLagIn = m_potLag[id];

    /// update sediment yields
    FLTPT yy = 1. - pot_fr;
    m_sedYield[id] *= yy;
    m_sandYield[id] *= yy;
    m_siltYield[id] *= yy;
    m_clayYield[id] *= yy;
    m_smaggreYield[id] *= yy;
    m_lgaggreYield[id] *= yy;
    // if(id == 46364) cout<<"pre surq no3: "<<m_surqNo3[id];
    // if(id == 46364) cout<<"pre orgp: "<<m_sedOrgP[id];
    /// update forms of N and P in pothole
    FLTPT xx = pot_fr * m_cellArea;
    m_potNo3[id] += m_surqNo3[id] * xx; // kg/ha * ha ==> kg
    m_potNH4[id] += m_surqNH4[id] * xx;
    m_potOrgN[id] += m_sedOrgN[id] * xx;
    m_potSolP[id] += m_surqSolP[id] * xx;
    m_potOrgP[id] += m_sedOrgP[id] * xx;
    m_potActMinP[id] += m_sedActiveMinP[id] * xx;
    m_potStaMinP[id] += m_sedStableMinP[id] * xx;
    /// update forms of N and P in surface runoff
    m_surqNo3[id] *= yy;
    // if (id == 46364) cout<<", *=yy: "<<m_surqNo3[id];
    m_surqNH4[id] *= yy;
    m_sedOrgN[id] *= yy;
    m_surqSolP[id] *= yy;
    m_sedOrgP[id] *= yy;
    m_sedActiveMinP[id] *= yy;
    m_sedStableMinP[id] *= yy;

    /// if overflow, then send the overflow to the cell's surface flow
    if (m_potVol[id] > m_potVolMax[id]) {
        qdayTmp += m_potVol[id] - m_potVolMax[id];
        spillo = m_potVol[id] - m_potVolMax[id];
        m_potVol[id] = m_potVolMax[id];
        if (spillo + m_potVolMax[id] < UTIL_ZERO) {
            // this should not happen
            xx = 0.;
        } else {
            xx = spillo / (spillo + m_potVolMax[id]);
        }
        potsedo += m_potSed[id] * xx;
        potsano += m_potSand[id] * xx;
        potsilo += m_potSilt[id] * xx;
        potclao += m_potClay[id] * xx;
        potsago += m_potSag[id] * xx;
        potlago += m_potLag[id] * xx;
        potno3o += m_potNo3[id] * xx;
        // if (id == 46364) cout<<"xx: "<<xx<<", potno3o: "<<potno3o<<endl;
        potnh4o += m_potNH4[id] * xx;
        potorgno += m_potOrgN[id] * xx;
        potsolpo += m_potSolP[id] * xx;
        potorgpo += m_potOrgP[id] * xx;
        potmpao += m_potActMinP[id] * xx;
        potmpso += m_potStaMinP[id] * xx;

        m_potSed[id] -= potsedo;
        m_potSand[id] -= potsano;
        m_potSilt[id] -= potsilo;
        m_potClay[id] -= potclao;
        m_potSag[id] -= potsago;
        m_potLag[id] -= potlago;
        m_potNo3[id] -= potno3o;
        m_potNH4[id] -= potnh4o;
        m_potOrgN[id] -= potorgno;
        m_potSolP[id] -= potsolpo;
        m_potOrgP[id] -= potorgpo;
        m_potStaMinP[id] -= potmpso;
        m_potActMinP[id] -= potmpao;

        m_sedYield[id] += potsedo / m_cellArea;
        m_sandYield[id] += potsano / m_cellArea;
        m_siltYield[id] += potsilo / m_cellArea;
        m_clayYield[id] += potclao / m_cellArea;
        m_smaggreYield[id] += potsago / m_cellArea;
        m_lgaggreYield[id] += potlago / m_cellArea;
        m_surqNo3[id] += potno3o / m_cellArea;
        // if (id == 46364) cout<<", +=potno3o: "<<m_surqNo3[id];
        m_surqNH4[id] += potnh4o / m_cellArea;
        m_sedOrgN[id] += potorgno / m_cellArea;
        m_surqSolP[id] += potsolpo / m_cellArea;
        m_sedOrgP[id] += potorgpo / m_cellArea;
        m_sedStableMinP[id] += potmpso / m_cellArea;
        m_sedActiveMinP[id] += potmpao / m_cellArea;
    } /// end if overflow

    /// if no overflow, compute settling and losses, surface inlet tile
    /// flow, evap, seepage, and redistribute soil water
    if (m_potVol[id] > UTIL_ZERO) {
        /// compute settling  -clay and silt based on fall velocity (v = 411*d2) d=mm, v=m/hr
        FLTPT pot_depth = m_potVol[id];
        FLTPT drcla = 0., drsil = 0., drtot = 0.;
        if (pot_depth > 10.) {
            /// assume clay v(fall) = 10 mm/d
            drcla = 1. - 0.5 * 10. / pot_depth;
        } else {
            drcla = 0.5 * pot_depth / 10.;
        }
        m_potClay[id] *= drcla;

        if (pot_depth > 1000.) {
            /// assume silt v(fall) = 1000 mm/d
            drsil = 1. - 0.5 * 1000. / pot_depth;
        } else {
            drsil = 0.5 * pot_depth * 0.001;
        }
        m_potSilt[id] *= drsil;

        /// assume complete settling of all other size (dr = 0)
        m_potSand[id] = 0.;
        m_potSag[id] = 0.;
        m_potLag[id] = 0.;

        /// compute total delivery ratio for pot_sed
        FLTPT allSedPart = m_potClayIn + m_potSiltIn + m_potSandIn + m_potSagIn + m_potLagIn;

        if (allSedPart < UTIL_ZERO) {
            drtot = 0.;
        } else {
            drtot = (m_potClay[id] + m_potSilt[id] + m_potSand[id] + m_potSag[id] + m_potLag[id]) / allSedPart;
        }
        m_potSed[id] *= drtot;

        /// compute organic settling assuming an enrichment ratio of 3 on clay (0.75)
        /// delivery of organics is 0.75 * dr(clay)- assuming dr on all non-clay = 1
        m_potOrgN[id] *= 0.75 * drcla;
        m_potOrgP[id] *= 0.75 * drcla;
        m_potActMinP[id] *= 0.75 * drcla;
        m_potStaMinP[id] *= 0.75 * drcla;
        ///m_potNo3[id] *= (1. - m_potNo3Decay);
        m_potSolP[id] *= 1. - m_potSolPDecay;
        /*
         * first-order kinetics is adopted from Chowdary et al., 2004
         * to account for volatilization, nitrification, and denitrification in impounded water
         */
        // 1. / 86400. = 1.1574074074074073e-05
        FLTPT nh3V = m_potNH4[id] * (1. - CalExp(-m_kVolat * m_timestep * 1.1574074074074073e-05));
        FLTPT no3N = m_potNH4[id] * (1. - CalExp(-m_kNitri * m_timestep * 1.1574074074074073e-05));
        /// update
        m_potNH4[id] -= nh3V + no3N;
        m_potNo3[id] += no3N;

        m_potNH4[id] = Max(m_potNH4[id], UTIL_ZERO);
        m_potNo3[id] = Max(m_potNo3[id], UTIL_ZERO);

        /// compute flow from surface inlet tile
        tileo = Min(m_potTilemm, m_potVol[id]);
        FLTPT potvol_tile = m_potVol[id];
        m_potVol[id] -= tileo;
        qdayTmp += tileo;

        /// limit seepage into soil if profile is near field capacity
        /* m_pot_k: hydraulic conductivity of soil surface of pothole
         * [defaults to conductivity of upper soil (0.01--10.) layer]
         * set as input parameters from database
         */
        if (m_pot_k > 0.) {
            yy = m_pot_k;
        } else {
            yy = m_ks[id][0];
        }
        /// calculate seepage into soil
        potsep = yy * m_potArea[id] * 240. / m_cnv; /// mm/hr*ha/240=m3/cnv=mm
        potsep = Min(potsep, m_potVol[id]);
        FLTPT potvol_sep = m_potVol[id];
        m_potVol[id] -= potsep;
        m_potSeep[id] += potsep;
        m_soilStorage[id][0] += potsep; /// this will be handled in the next time step, added by LJ

        ///// force the soil water storage to field capacity
        //for (int ly = 0; ly < (int)m_soilLayers[id]; ly++)
        //{
        //	FLTPT dep2cap = m_sol_sat[id][ly] - m_soilStorage[id][ly];
        //	if (dep2cap > 0.)
        //	{
        //		dep2cap = Min(dep2cap, m_potVol[id]);
        //		m_soilStorage[id][ly] += dep2cap;
        //		m_potVol[id] -= dep2cap;
        //	}
        //}
        //if (m_potVol[id] < m_potVolMin[id])
        //	m_potVol[id] = m_potVolMin[id]; /// force to reach the lowest depth.
        ///// recompute total soil water storage
        //m_soilStorageProfile[id] = 0.;
        //for (int ly = 0; ly < (int)m_soilLayers[id]; ly++)
        //	m_soilStorageProfile[id] += m_soilStorage[id][ly];

        /// compute evaporation from water surface
        if (m_LAIDay[id] < m_evLAI) {
            potev = (1. - m_LAIDay[id] / m_evLAI) * m_pet[id];
            // if (id == 46364) cout<<"pet: "<<m_pet[id]<<", laiday: "<<m_LAIDay[id]<<", potEvap: "<<potev<<", ";
            potev = Min(potev, m_potVol[id]);
            m_potVol[id] -= potev;
            m_potEvap[id] += potev;
        }
        if (potvol_tile > UTIL_ZERO) {
            sedloss = m_potSed[id] * tileo / potvol_tile;
            sedloss = Min(sedloss, m_potSed[id]);
            m_potSed[id] -= sedloss;
            potsedo += sedloss;
            m_sedYield[id] += sedloss;

            no3loss = m_potNo3[id] * tileo / potvol_tile;
            no3loss = Min(no3loss, m_potNo3[id]);
            m_potNo3[id] -= no3loss;
            m_surqNo3[id] += no3loss / m_cellArea;
            // if (id == 46364) cout<<", += tile loss: "<<m_surqNo3[id];
            nh4loss = m_potNH4[id] * tileo / potvol_tile;
            nh4loss = Min(nh4loss, m_potNH4[id]);
            m_potNH4[id] -= nh4loss;
            m_surqNH4[id] += nh4loss / m_cellArea;

            solploss = m_potSolP[id] * tileo / potvol_tile;
            solploss = Min(solploss, m_potSolP[id]);
            m_potSolP[id] -= solploss;
            m_surqSolP[id] += solploss / m_cellArea;

            orgnloss = m_potOrgN[id] * tileo / potvol_tile;
            orgnloss = Min(orgnloss, m_potOrgN[id]);
            m_potOrgN[id] -= orgnloss;
            m_sedOrgN[id] += orgnloss / m_cellArea;

            orgploss = m_potOrgP[id] * tileo / potvol_tile;
            orgploss = Min(orgploss, m_potOrgP[id]);
            m_potOrgP[id] -= orgploss;
            m_sedOrgP[id] += orgploss / m_cellArea;

            minpsloss = m_potStaMinP[id] * tileo / potvol_tile;
            minpsloss = Min(minpsloss, m_potStaMinP[id]);
            m_potStaMinP[id] -= minpsloss;
            m_sedStableMinP[id] += minpsloss / m_cellArea;

            minpaloss = m_potActMinP[id] * tileo / potvol_tile;
            minpaloss = Min(minpaloss, m_potActMinP[id]);
            m_potActMinP[id] -= minpaloss;
            m_sedActiveMinP[id] += minpaloss / m_cellArea;

            sanloss = m_potSand[id] * tileo / potvol_tile;
            m_potSand[id] -= sanloss;
            potsano += sanloss;
            m_sandYield[id] += sanloss;

            silloss = m_potSilt[id] * tileo / potvol_tile;
            m_potSilt[id] -= silloss;
            potsilo += silloss;
            m_siltYield[id] += silloss;

            claloss = m_potClay[id] * tileo / potvol_tile;
            m_potClay[id] -= claloss;
            potclao += claloss;
            m_clayYield[id] += claloss;

            sagloss = m_potSag[id] * tileo / potvol_tile;
            m_potSag[id] -= sagloss;
            potsago += sagloss;
            m_smaggreYield[id] += sagloss;

            lagloss = m_potLag[id] * tileo / potvol_tile;
            m_potLag[id] -= lagloss;
            potlago += lagloss;
            m_lgaggreYield[id] += lagloss;
        }
        if (potvol_sep > UTIL_ZERO) {
            FLTPT lossRatio = potsep / potvol_sep;
            sedloss = m_potSed[id] * lossRatio;
            sedloss = Min(sedloss, m_potSed[id]);
            m_potSed[id] -= sedloss;

            no3loss = m_potNo3[id] * lossRatio;
            no3loss = Min(no3loss, m_potNo3[id]);
            m_potNo3[id] -= no3loss;
            // if (id == 46364) cout<<", loss ratio: "<<lossRatio<<", no3 loss from seepage: "<<no3loss<<endl;
            nh4loss = m_potNH4[id] * lossRatio;
            nh4loss = Min(nh4loss, m_potNH4[id]);
            m_potNH4[id] -= nh4loss;

            solploss = m_potSolP[id] * lossRatio;
            solploss = Min(solploss, m_potSolP[id]);
            m_potSolP[id] -= solploss;

            orgnloss = m_potOrgN[id] * lossRatio;
            orgnloss = Min(orgnloss, m_potOrgN[id]);
            m_potOrgN[id] -= orgnloss;

            orgploss = m_potOrgP[id] * lossRatio;
            orgploss = Min(orgploss, m_potOrgP[id]);
            m_potOrgP[id] -= orgploss;

            minpsloss = m_potStaMinP[id] * lossRatio;
            minpsloss = Min(minpsloss, m_potStaMinP[id]);
            m_potStaMinP[id] -= minpsloss;

            minpaloss = m_potActMinP[id] * lossRatio;
            minpaloss = Min(minpaloss, m_potActMinP[id]);
            m_potActMinP[id] -= minpaloss;

            sanloss = m_potSand[id] * lossRatio;
            m_potSand[id] -= sanloss;

            silloss = m_potSilt[id] * lossRatio;
            m_potSilt[id] -= silloss;

            claloss = m_potClay[id] * lossRatio;
            m_potClay[id] -= claloss;

            sagloss = m_potSag[id] * lossRatio;
            m_potSag[id] -= sagloss;

            lagloss = m_potLag[id] * lossRatio;
            m_potLag[id] -= lagloss;
        }
    }
    // force to auto-irrigation at the end of the day, added by LJ.
    if (m_potVol[id] < UTIL_ZERO) {
        m_potVol[id] = m_potVolMin[id];
    }
    //potholeSurfaceArea(id);
    m_surfaceRunoff[id] = qdayTmp;
    //if (id == 46364)  /// dianbu 46364, dianbu2 1085
    //	cout<<"surfaceQ: "<<m_surfaceRunoff[id]<<", potVol: "<<m_potVol[id]<<
    //	", potNh4: "<<m_potNH4[id]<<", surqNh4: "<<m_surqNH4[id]<<endl;
}

void IMP_SWAT::PotholeSurfaceArea(const int id) {
    /// compute surface area assuming a cone shape, ha
    FLTPT potVol_m3 = m_potVol[id] * m_cnv;
    m_potArea[id] = (FLTPT)PI * CalPow(3. * potVol_m3 / (PI * m_slope[id]), 0.6666);
    m_potArea[id] *= 0.0001; /// convert to ha
    if (m_potArea[id] <= UTIL_ZERO) {
        m_potArea[id] = 0.001;
    }
    if (m_potArea[id] > m_cellArea) {
        m_potArea[id] = m_cellArea;
    }
}

void IMP_SWAT::ReleaseWater(const int id) {
    FLTPT proption = 1.;
    FLTPT xx = proption * m_cellArea;
    if (m_potVol[id] < UTIL_ZERO) {
        return;
    }
    m_surfaceRunoff[id] += m_potVol[id] * proption;
    m_potVol[id] *= 1. - proption;
    if (m_potSed[id] < UTIL_ZERO) {
        m_potSed[id] = 0.;
        m_sandYield[id] = 0.;
        m_siltYield[id] = 0.;
        m_clayYield[id] = 0.;
        m_smaggreYield[id] = 0.;
        m_lgaggreYield[id] = 0.;
    } else {
        m_sedYield[id] += m_potSed[id] * proption;
        m_sandYield[id] += m_potSand[id] * proption;
        m_siltYield[id] += m_potSilt[id] * proption;
        m_clayYield[id] += m_potClay[id] * proption;
        m_smaggreYield[id] += m_potSag[id] * proption;
        m_lgaggreYield[id] += m_potLag[id] * proption;
        m_potSed[id] *= 1. - proption;
        m_potSand[id] *= 1. - proption;
        m_potSilt[id] *= 1. - proption;
        m_potClay[id] *= 1. - proption;
        m_potSag[id] *= 1. - proption;
        m_potLag[id] *= 1. - proption;
    }
    if (m_potNo3[id] < UTIL_ZERO) {
        m_potNo3[id] = 0.;
    } else {
        m_surqNo3[id] += m_potNo3[id] * xx;
        // if (id == 46364) cout<<", release: "<<m_surqNo3[id]<<endl;
        m_potNo3[id] *= 1. - proption;
    }
    if (m_potNH4[id] < UTIL_ZERO) {
        m_potNH4[id] = 0.;
    } else {
        m_surqNH4[id] += m_potNH4[id] * xx;
        m_potNH4[id] *= 1. - proption;
    }
    if (m_potSolP[id] < UTIL_ZERO) {
        m_potSolP[id] = 0.;
    } else {
        m_surqSolP[id] += m_potSolP[id] * xx;
        m_potSolP[id] *= 1. - proption;
    }
    if (m_potOrgN[id] < UTIL_ZERO) {
        m_potOrgN[id] = 0.;
    } else {
        m_sedOrgN[id] += m_potOrgN[id] * xx;
        m_potOrgN[id] *= 1. - proption;
    }
    if (m_potOrgP[id] < UTIL_ZERO) {
        m_potOrgP[id] = 0.;
    } else {
        m_sedOrgP[id] += m_potOrgP[id] * xx;
        m_potOrgP[id] *= 1. - proption;
    }
    if (m_potActMinP[id] < UTIL_ZERO) {
        m_potActMinP[id] = 0.;
    } else {
        m_sedActiveMinP[id] += m_potActMinP[id] * xx;
        m_potActMinP[id] *= 1. - proption;
    }
    if (m_potStaMinP[id] < UTIL_ZERO) {
        m_potStaMinP[id] = 0.;
    } else {
        m_sedStableMinP[id] += m_potStaMinP[id] * xx;
        m_potStaMinP[id] *= 1. - proption;
    }
    /// Debugging: dianbu 46364, dianbu2 1085
    // if (id == 46364) cout<<"releaseWater, "<<m_surfaceRunoff[id]<<", "<<m_potVol[id]<<", surqNh4: "<<m_surqNH4[id]<<endl;
}

void IMP_SWAT::Get1DData(const char* key, int* n, FLTPT** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_POT_VOL[0])) *data = m_potVol;
    else if (StringMatch(sk, VAR_POT_SA[0])) *data = m_potArea;
    else if (StringMatch(sk, VAR_POT_NO3[0])) *data = m_potNo3;
    else if (StringMatch(sk, VAR_POT_NH4[0])) *data = m_potNH4;
    else if (StringMatch(sk, VAR_POT_SOLP[0])) *data = m_potSolP;
    else {
        throw ModelException(M_IMP_SWAT[0], "Get1DData",
                             "Parameter" + sk + "does not exist.");
    }
    *n = m_nCells;
}
