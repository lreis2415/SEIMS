#include "ReservoirMethod.h"

#include "text.h"

ReservoirMethod::ReservoirMethod() :
    m_dt(-1), m_nCells(-1), m_maxSoilLyrs(-1),
    m_nSoilLyrs(nullptr), m_soilThk(nullptr),
    m_dp_co(NODATA_VALUE), m_Kg(NODATA_VALUE), m_Base_ex(NODATA_VALUE),
    m_soilPerco(nullptr), m_IntcpET(nullptr), m_deprStoET(nullptr),
    m_soilET(nullptr), m_actPltET(nullptr), m_pet(nullptr),
    m_revap(nullptr), m_GW0(NODATA_VALUE), m_GWMAX(NODATA_VALUE),
    m_petSubbsn(nullptr), m_gwSto(nullptr), m_slope(nullptr), m_soilWtrSto(nullptr),
    m_soilDepth(nullptr),
    m_VgroundwaterFromBankStorage(nullptr), m_T_Perco(nullptr),
    /// intermediate
    m_T_PerDep(nullptr), m_T_RG(nullptr),
    /// outputs
    m_T_QG(nullptr), m_T_Revap(nullptr), m_T_GWWB(nullptr),
    m_nSubbsns(-1), m_inputSubbsnID(-1), m_subbasinsInfo(nullptr) {
}

ReservoirMethod::~ReservoirMethod() {
    if (m_T_Perco != nullptr) Release1DArray(m_T_Perco);
    if (m_T_PerDep != nullptr) Release1DArray(m_T_PerDep);
    if (m_revap != nullptr) Release1DArray(m_revap);
    if (m_T_Revap != nullptr) Release1DArray(m_T_Revap);
    if (m_T_RG != nullptr) Release1DArray(m_T_RG);
    if (m_T_QG != nullptr) Release1DArray(m_T_QG);
    if (m_petSubbsn != nullptr) Release1DArray(m_petSubbsn);
    if (m_gwSto != nullptr) Release1DArray(m_gwSto);
    if (m_T_GWWB != nullptr) Release2DArray(m_T_GWWB);
}

void ReservoirMethod::InitialOutputs() {
    CHECK_POSITIVE(M_GWA_RE[0], m_nSubbsns);
    int nLen = m_nSubbsns + 1;
    if (m_T_Perco == nullptr) Initialize1DArray(nLen, m_T_Perco, 0.);
    if (m_T_Revap == nullptr) Initialize1DArray(nLen, m_T_Revap, 0.);
    if (m_T_PerDep == nullptr) Initialize1DArray(nLen, m_T_PerDep, 0.);
    if (m_T_RG == nullptr) Initialize1DArray(nLen, m_T_RG, 0.);
    if (m_T_QG == nullptr) Initialize1DArray(nLen, m_T_QG, 0.);
    if (m_petSubbsn == nullptr) Initialize1DArray(nLen, m_petSubbsn, 0.);
    if (m_gwSto == nullptr) Initialize1DArray(nLen, m_gwSto, m_GW0);
    if (m_revap == nullptr) Initialize1DArray(m_nCells, m_revap, 0.);
    if (m_T_GWWB == nullptr) Initialize2DArray(nLen, 6, m_T_GWWB, 0.);
}

int ReservoirMethod::Execute() {
    CheckInputData();
    InitialOutputs();
    for (auto it = m_subbasinIDs.begin(); it != m_subbasinIDs.end(); ++it) {
        int subID = *it;
        Subbasin* curSub = m_subbasinsInfo->GetSubbasinByID(subID);
        // get percolation from the bottom soil layer at the subbasin scale
        int curCellsNum = curSub->GetCellCount();
        int* curCells = curSub->GetCells();
        FLTPT perco = 0.;
        FLTPT fPET = 0.;
        FLTPT revap = 0.;
#pragma omp parallel for reduction(+:perco, fPET, revap)
        for (int i = 0; i < curCellsNum; i++) {
            int index = curCells[i];
            FLTPT tmp_perc = m_soilPerco[index][CVT_INT(m_nSoilLyrs[index]) - 1];
            if (tmp_perc > 0) {
                perco += tmp_perc;
            } else {
                m_soilPerco[index][CVT_INT(m_nSoilLyrs[index]) - 1] = 0.;
            }
            if (m_pet[index] > 0.) {
                fPET += m_pet[index];
            }
            m_revap[index] = m_pet[index] - m_IntcpET[index] - m_deprStoET[index] - m_soilET[index] - m_actPltET[index];
            m_revap[index] = Max(m_revap[index], 0.);
            m_revap[index] = m_revap[index] * m_gwSto[subID] / m_GWMAX;
            revap += m_revap[index];
        }
        perco /= curCellsNum; // mean mm
        fPET /= curCellsNum;
        revap /= curCellsNum;
        /// percolated water ==> vadose zone ==> shallow aquifer ==> deep aquifer
        /// currently, for convenience, we assume a small portion of the percolated water
        /// will enter groundwater. By LJ. 2016-9-2
        FLTPT ratio2gw = 1.;
        perco *= ratio2gw;
        FLTPT percoDeep = perco * m_dp_co; ///< deep percolation

        if (revap > m_gwSto[subID]) {
            for (int i = 0; i < curCellsNum; i++) {
                int index = 0;
                index = curCells[i];
                m_revap[index] *= m_gwSto[subID] / revap;
            }
            revap = m_gwSto[subID];
        }

        // groundwater runoff (mm)
        FLTPT slopeCoef = curSub->GetSlopeCoef();
        FLTPT kg = m_Kg * slopeCoef;
        FLTPT groundRunoff = kg * CalPow(m_gwSto[subID], m_Base_ex); // mm
        FLTPT QGConvert = curSub->GetArea() / m_dt * 0.001; // mm ==> m3/s
        FLTPT groundQ = groundRunoff * curCellsNum * QGConvert;     // groundwater discharge (m3/s)
        FLTPT groundStorage = m_gwSto[subID];
        groundStorage += perco - revap - percoDeep - groundRunoff;

        //add the ground water from bank storage, 2011-3-14
        FLTPT gwBank = 0.;
        // at the first time step m_VgroundwaterFromBankStorage is nullptr
        if (m_VgroundwaterFromBankStorage != nullptr) {
            gwBank = m_VgroundwaterFromBankStorage[subID];
        }
        groundStorage += gwBank / curSub->GetArea() * 1000.;

        groundStorage = Max(groundStorage, 0.);
        if (groundStorage > m_GWMAX) {
            groundRunoff += groundStorage - m_GWMAX;
            groundQ = groundRunoff * curCellsNum * QGConvert; // groundwater discharge (m3/s)
            groundStorage = m_GWMAX;
        }

        /**** Set values for current subbasin ****/
        curSub->SetPet(fPET);
        curSub->SetPerco(perco);
        curSub->SetPerde(percoDeep);
        curSub->SetEg(revap);
        curSub->SetRg(groundRunoff);
        curSub->SetQg(groundQ);
        curSub->SetGw(groundStorage);

        if (groundStorage != groundStorage) {
            std::ostringstream oss;
            oss << perco << "\t" << revap << "\t" << percoDeep << "\t" << groundRunoff << "\t" << m_gwSto[subID]
                    << "\t" << m_Kg << "\t" << m_Base_ex << "\t" << slopeCoef << endl;
            throw ModelException("Subbasin", "setInputs", oss.str());
        }
#ifdef PRINT_DEBUG
        cout << "ID: " << subID <<
                ", pet: " << std::fixed << setprecision(6) << fPET <<
                ", perco: " << std::fixed << setprecision(6) << perco <<
                ", percoDeep: " << std::fixed << setprecision(6) << percoDeep <<
                ", revap: " << std::fixed << setprecision(6) << revap <<
                ", groundRunoff: " << std::fixed << setprecision(6) << groundRunoff <<
                ", groundQ: " << std::fixed << setprecision(6) << groundQ <<
                ", gwStore: " << std::fixed << setprecision(6) << groundStorage << endl;
#endif
        m_petSubbsn[subID] = curSub->GetPet();
        m_T_Perco[subID] = curSub->GetPerco();
        m_T_PerDep[subID] = curSub->GetPerde();
        m_T_Revap[subID] = curSub->GetEg();
        m_T_RG[subID] = curSub->GetRg(); //get rg of specific subbasin
        m_T_QG[subID] = curSub->GetQg(); //get qg of specific subbasin
        m_gwSto[subID] = curSub->GetGw();
    }

    m_T_Perco[0] = m_subbasinsInfo->Subbasin2Basin(VAR_PERCO[0]);
    m_T_PerDep[0] = m_subbasinsInfo->Subbasin2Basin(VAR_PERDE[0]);
    m_T_Revap[0] = m_subbasinsInfo->Subbasin2Basin(VAR_REVAP[0]);
    m_T_RG[0] = m_subbasinsInfo->Subbasin2Basin(VAR_RG[0]); // get rg of entire watershed
    m_T_QG[0] = m_subbasinsInfo->Subbasin2Basin(VAR_QG[0]); // get qg of entire watershed
    m_gwSto[0] = m_subbasinsInfo->Subbasin2Basin(VAR_GW_Q[0]);

    // output to GWWB, the sequence is coincident with the header information defined in PrintInfo.cpp, line 528.
    for (int i = 0; i <= m_nSubbsns; i++) {
        m_T_GWWB[i][0] = m_T_Perco[i];
        m_T_GWWB[i][1] = m_T_Revap[i];
        m_T_GWWB[i][2] = m_T_PerDep[i];
        m_T_GWWB[i][3] = m_T_RG[i];
        m_T_GWWB[i][4] = m_gwSto[i];
        m_T_GWWB[i][5] = m_T_QG[i];
    }

    // update soil moisture
    for (auto it = m_subbasinIDs.begin(); it != m_subbasinIDs.end(); ++it) {
        Subbasin* sub = m_subbasinsInfo->GetSubbasinByID(*it);
        int* cells = sub->GetCells();
        int nCells = sub->GetCellCount();
        int index = 0;
#pragma omp parallel for
        for (int i = 0; i < nCells; i++) {
            index = cells[i];
            m_soilWtrSto[cells[i]][m_nSoilLyrs[cells[i]] - 1] += m_revap[index];
#ifdef _DEBUG
            if (isinf(m_soilWtrSto[cells[i]][CVT_INT(m_nSoilLyrs[cells[i]]) - 1]) ||
                isnan(m_soilWtrSto[cells[i]][CVT_INT(m_nSoilLyrs[cells[i]]) - 1]) ||
                m_soilWtrSto[cells[i]][CVT_INT(m_nSoilLyrs[cells[i]]) - 1] < 0.f) {
                cout << "GWA_RE: moisture is less than zero" << m_soilWtrSto[cells[i]][CVT_INT(m_nSoilLyrs[cells[i]]) - 1] << endl;
            }
#endif
            // TODO: Is it need to allocate revap to each soil layers??? By LJ
        }
    }
    return 0;
}

bool ReservoirMethod::CheckInputData() {
    CHECK_POSITIVE(M_GWA_RE[0], m_nCells);
    CHECK_POSITIVE(M_GWA_RE[0], m_nSubbsns);
    CHECK_POSITIVE(M_GWA_RE[0], m_dt);
    CHECK_POSITIVE(M_GWA_RE[0], m_maxSoilLyrs);
    CHECK_NODATA(M_GWA_RE[0], m_dp_co);
    CHECK_NODATA(M_GWA_RE[0], m_Kg);
    CHECK_NODATA(M_GWA_RE[0], m_Base_ex);
    CHECK_POINTER(M_GWA_RE[0], m_soilPerco);
    CHECK_POINTER(M_GWA_RE[0], m_IntcpET);
    CHECK_POINTER(M_GWA_RE[0], m_deprStoET);
    CHECK_POINTER(M_GWA_RE[0], m_soilET);
    CHECK_POINTER(M_GWA_RE[0], m_actPltET);
    CHECK_POINTER(M_GWA_RE[0], m_pet);
    CHECK_POINTER(M_GWA_RE[0], m_slope);
    CHECK_POINTER(M_GWA_RE[0], m_soilWtrSto);
    CHECK_POINTER(M_GWA_RE[0], m_nSoilLyrs);
    CHECK_POINTER(M_GWA_RE[0], m_soilThk);
    CHECK_POINTER(M_GWA_RE[0], m_subbasinsInfo);
    return true;
}

void ReservoirMethod::SetValue(const char* key, const FLTPT value) {
    string sk(key);
    if (StringMatch(sk, VAR_KG[0])) m_Kg = value;
    else if (StringMatch(sk, VAR_Base_ex[0])) m_Base_ex = value;
    else if (StringMatch(sk, VAR_DF_COEF[0])) m_dp_co = value;
    else if (StringMatch(sk, VAR_GW0[0])) m_GW0 = value;
    else if (StringMatch(sk, VAR_GWMAX[0])) m_GWMAX = value;
    else {
        throw ModelException(M_GWA_RE[0], "SetValue",
                             "Parameter " + sk + " does not exist in current module.");
    }
}

void ReservoirMethod::SetValue(const char* key, const int value) {
    string sk(key);
    if (StringMatch(sk, Tag_TimeStep[0])) m_dt = value;
    else if (StringMatch(sk, VAR_SUBBSNID_NUM[0])) m_nSubbsns = value;
    else if (StringMatch(sk, Tag_SubbasinId)) m_inputSubbsnID = value;
    else {
        throw ModelException(M_GWA_RE[0], "SetValue",
                             "Integer Parameter " + sk + " does not exist in current module.");
    }
}

void ReservoirMethod::Set1DData(const char* key, const int n, FLTPT* data) {
    string sk(key);
    if (StringMatch(sk, VAR_GWNEW[0])) {
        m_VgroundwaterFromBankStorage = data;
        return;
    }
    //check the input data
    if (!CheckInputSize(M_GWA_RE[0], key, n, m_nCells)) return;

    //set the value
    if (StringMatch(sk, VAR_INET[0])) {
        m_IntcpET = data;
    } else if (StringMatch(sk, VAR_DEET[0])) {
        m_deprStoET = data;
    } else if (StringMatch(sk, VAR_SOET[0])) {
        m_soilET = data;
    } else if (StringMatch(sk, VAR_AET_PLT[0])) {
        m_actPltET = data;
    } else if (StringMatch(sk, VAR_PET[0])) {
        m_pet = data;
    } else if (StringMatch(sk, VAR_SLOPE[0])) {
        m_slope = data;
    } else {
        throw ModelException(M_GWA_RE[0], "Set1DData",
                             "Parameter " + sk + " does not exist in current module.");
    }
}


void ReservoirMethod::Set1DData(const char* key, const int n, int* data) {
    string sk(key);
    if (!CheckInputSize(M_GWA_RE[0], key, n, m_nCells)) return;
    if (StringMatch(sk, VAR_SOILLAYERS[0])) {
        m_nSoilLyrs = data;
    } else {
        throw ModelException(M_GWA_RE[0], "Set1DData",
                             "Integer Parameter " + sk + " does not exist in current module.");
    }
}

void ReservoirMethod::Set2DData(const char* key, const int nrows, const int ncols, FLTPT** data) {
    string sk(key);
    CheckInputSize2D(M_GWA_RE[0], key, nrows, ncols, m_nCells, m_maxSoilLyrs);

    if (StringMatch(sk, VAR_PERCO[0])) {
        m_soilPerco = data;
    } else if (StringMatch(sk, VAR_SOL_ST[0])) {
        m_soilWtrSto = data;
    } else if (StringMatch(sk, VAR_SOILDEPTH[0])) {
        m_soilDepth = data;
    } else if (StringMatch(sk, VAR_SOILTHICK[0])) {
        m_soilThk = data;
    } else {
        throw ModelException(M_GWA_RE[0], "Set2DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void ReservoirMethod::SetSubbasins(clsSubbasins* subbsns) {
    if (m_subbasinsInfo == nullptr) {
        m_subbasinsInfo = subbsns;
        // m_nSubbasins = m_subbasinsInfo->GetSubbasinNumber(); // Set in SetValue()! lj
        m_subbasinIDs = m_subbasinsInfo->GetSubbasinIDs();
    }
}

void ReservoirMethod::Get1DData(const char* key, int* nrows, FLTPT** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_REVAP[0])) {
        *data = m_revap;
        *nrows = m_nCells;
    } else if (StringMatch(sk, VAR_RG[0])) {
        *data = m_T_RG;
        *nrows = m_nSubbsns + 1;
    } else if (StringMatch(sk, VAR_SBQG[0])) {
        *data = m_T_QG;
        *nrows = m_nSubbsns + 1;
    } else if (StringMatch(sk, VAR_SBGS[0])) {
        *data = m_gwSto;
        *nrows = m_nSubbsns + 1;
    } else if (StringMatch(sk, VAR_SBPET[0])) {
        *data = m_petSubbsn;
        *nrows = m_nSubbsns + 1;
    } else {
        throw ModelException(M_GWA_RE[0], "Get1DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void ReservoirMethod::Get2DData(const char* key, int* nrows, int* ncols, FLTPT*** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_GWWB[0])) {
        *data = m_T_GWWB;
        *nrows = m_nSubbsns + 1;
        *ncols = 6;
    } else {
        throw ModelException(M_GWA_RE[0], "Get2DData",
                             "Parameter " + sk + " does not exist in current module.");
    }
}
