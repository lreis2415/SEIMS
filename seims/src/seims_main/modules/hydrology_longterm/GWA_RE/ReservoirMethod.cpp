#include "ReservoirMethod.h"

#include "text.h"

ReservoirMethod::ReservoirMethod() :
    m_dt(-1), m_nCells(-1), m_cellWth(NODATA_VALUE), m_maxSoilLyrs(-1),
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
    if (m_T_GWWB != nullptr) Release2DArray(m_nSubbsns + 1, m_T_GWWB);
}

void ReservoirMethod::InitialOutputs() {
    CHECK_POSITIVE(MID_GWA_RE, m_nSubbsns);
    int nLen = m_nSubbsns + 1;
    if (m_T_Perco == nullptr) Initialize1DArray(nLen, m_T_Perco, 0.f);
    if (m_T_Revap == nullptr) Initialize1DArray(nLen, m_T_Revap, 0.f);
    if (m_T_PerDep == nullptr) Initialize1DArray(nLen, m_T_PerDep, 0.f);
    if (m_T_RG == nullptr) Initialize1DArray(nLen, m_T_RG, 0.f);
    if (m_T_QG == nullptr) Initialize1DArray(nLen, m_T_QG, 0.f);
    if (m_petSubbsn == nullptr) Initialize1DArray(nLen, m_petSubbsn, 0.f);
    if (m_gwSto == nullptr) Initialize1DArray(nLen, m_gwSto, m_GW0);
    if (m_revap == nullptr) Initialize1DArray(m_nCells, m_revap, 0.f);
    if (m_T_GWWB == nullptr) Initialize2DArray(nLen, 6, m_T_GWWB, 0.f);
}

int ReservoirMethod::Execute() {
    CheckInputData();
    InitialOutputs();
    float QGConvert = 1.f * m_cellWth * m_cellWth / m_dt * 0.001f; // mm ==> m3/s
    for (auto it = m_subbasinIDs.begin(); it != m_subbasinIDs.end(); ++it) {
        int subID = *it;
        Subbasin* curSub = m_subbasinsInfo->GetSubbasinByID(subID);
        // get percolation from the bottom soil layer at the subbasin scale
        int curCellsNum = curSub->GetCellCount();
        int* curCells = curSub->GetCells();
        float perco = 0.f;
        float fPET = 0.f;
        float revap = 0.f;
#pragma omp parallel for reduction(+:perco, fPET, revap)
        for (int i = 0; i < curCellsNum; i++) {
            int index = curCells[i];
            float tmp_perc = m_soilPerco[index][CVT_INT(m_nSoilLyrs[index]) - 1];
            if (tmp_perc > 0) {
                perco += tmp_perc;
            } else {
                m_soilPerco[index][CVT_INT(m_nSoilLyrs[index]) - 1] = 0.f;
            }
            if (m_pet[index] > 0.f) {
                fPET += m_pet[index];
            }
            m_revap[index] = m_pet[index] - m_IntcpET[index] - m_deprStoET[index] - m_soilET[index] - m_actPltET[index];
            m_revap[index] = Max(m_revap[index], 0.f);
            m_revap[index] = m_revap[index] * m_gwSto[subID] / m_GWMAX;
            revap += m_revap[index];
        }
        perco /= curCellsNum; // mean mm
        fPET /= curCellsNum;
        revap /= curCellsNum;
        /// percolated water ==> vadose zone ==> shallow aquifer ==> deep aquifer
        /// currently, for convenience, we assume a small portion of the percolated water
        /// will enter groundwater. By LJ. 2016-9-2
        float ratio2gw = 1.f;
        perco *= ratio2gw;
        float percoDeep = perco * m_dp_co; ///< deep percolation

        if (revap > m_gwSto[subID]) {
            for (int i = 0; i < curCellsNum; i++) {
                int index = 0;
                index = curCells[i];
                m_revap[index] *= m_gwSto[subID] / revap;
            }
            revap = m_gwSto[subID];
        }

        // groundwater runoff (mm)
        float slopeCoef = curSub->GetSlopeCoef();
        float kg = m_Kg * slopeCoef;
        float groundRunoff = kg * pow(m_gwSto[subID], m_Base_ex); // mm
        float groundQ = groundRunoff * curCellsNum * QGConvert;     // groundwater discharge (m3/s)
        float groundStorage = m_gwSto[subID];
        groundStorage += perco - revap - percoDeep - groundRunoff;

        //add the ground water from bank storage, 2011-3-14
        float gwBank = 0.f;
        // at the first time step m_VgroundwaterFromBankStorage is nullptr
        if (m_VgroundwaterFromBankStorage != nullptr) {
            gwBank = m_VgroundwaterFromBankStorage[subID];
        }
        groundStorage += gwBank / curSub->GetArea() * 1000.f;

        groundStorage = Max(groundStorage, 0.f);
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

    m_T_Perco[0] = m_subbasinsInfo->Subbasin2Basin(VAR_PERCO);
    m_T_PerDep[0] = m_subbasinsInfo->Subbasin2Basin(VAR_PERDE);
    m_T_Revap[0] = m_subbasinsInfo->Subbasin2Basin(VAR_REVAP);
    m_T_RG[0] = m_subbasinsInfo->Subbasin2Basin(VAR_RG); // get rg of entire watershed
    m_T_QG[0] = m_subbasinsInfo->Subbasin2Basin(VAR_QG); // get qg of entire watershed
    m_gwSto[0] = m_subbasinsInfo->Subbasin2Basin(VAR_GW_Q);

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
            m_soilWtrSto[cells[i]][CVT_INT(m_nSoilLyrs[cells[i]]) - 1] += m_revap[index];
            // TODO: Is it need to allocate revap to each soil layers??? By LJ
        }
    }
    return 0;
}

bool ReservoirMethod::CheckInputData() {
    CHECK_POSITIVE(MID_GWA_RE, m_nCells);
    CHECK_POSITIVE(MID_GWA_RE, m_nSubbsns);
    CHECK_POSITIVE(MID_GWA_RE, m_cellWth);
    CHECK_POSITIVE(MID_GWA_RE, m_dt);
    CHECK_POSITIVE(MID_GWA_RE, m_maxSoilLyrs);
    CHECK_NODATA(MID_GWA_RE, m_dp_co);
    CHECK_NODATA(MID_GWA_RE, m_Kg);
    CHECK_NODATA(MID_GWA_RE, m_Base_ex);
    CHECK_POINTER(MID_GWA_RE, m_soilPerco);
    CHECK_POINTER(MID_GWA_RE, m_IntcpET);
    CHECK_POINTER(MID_GWA_RE, m_deprStoET);
    CHECK_POINTER(MID_GWA_RE, m_soilET);
    CHECK_POINTER(MID_GWA_RE, m_actPltET);
    CHECK_POINTER(MID_GWA_RE, m_pet);
    CHECK_POINTER(MID_GWA_RE, m_slope);
    CHECK_POINTER(MID_GWA_RE, m_soilWtrSto);
    CHECK_POINTER(MID_GWA_RE, m_nSoilLyrs);
    CHECK_POINTER(MID_GWA_RE, m_soilThk);
    CHECK_POINTER(MID_GWA_RE, m_subbasinsInfo);
    return true;
}

// set value
void ReservoirMethod::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, Tag_TimeStep)) m_dt = CVT_INT(value);
    else if (StringMatch(sk, VAR_SUBBSNID_NUM)) m_nSubbsns = CVT_INT(value);
    else if (StringMatch(sk, Tag_SubbasinId)) m_inputSubbsnID = CVT_INT(value);
    else if (StringMatch(sk, Tag_CellWidth)) m_cellWth = value;
    else if (StringMatch(sk, VAR_KG)) m_Kg = value;
    else if (StringMatch(sk, VAR_Base_ex)) m_Base_ex = value;
    else if (StringMatch(sk, VAR_DF_COEF)) m_dp_co = value;
    else if (StringMatch(sk, VAR_GW0)) m_GW0 = value;
    else if (StringMatch(sk, VAR_GWMAX)) m_GWMAX = value;
    else {
        throw ModelException(MID_GWA_RE, "SetValue", "Parameter " + sk + " does not exist in current module.");
    }
}

void ReservoirMethod::Set1DData(const char* key, const int n, float* data) {
    string sk(key);
    if (StringMatch(sk, VAR_GWNEW)) {
        m_VgroundwaterFromBankStorage = data;
        return;
    }
    //check the input data
    if (!CheckInputSize(MID_GWA_RE, key, n, m_nCells)) return;

    //set the value
    if (StringMatch(sk, VAR_INET)) {
        m_IntcpET = data;
    } else if (StringMatch(sk, VAR_DEET)) {
        m_deprStoET = data;
    } else if (StringMatch(sk, VAR_SOET)) {
        m_soilET = data;
    } else if (StringMatch(sk, VAR_AET_PLT)) {
        m_actPltET = data;
    } else if (StringMatch(sk, VAR_PET)) {
        m_pet = data;
    } else if (StringMatch(sk, VAR_SLOPE)) {
        m_slope = data;
    } else if (StringMatch(sk, VAR_SOILLAYERS)) {
        m_nSoilLyrs = data;
    } else {
        throw ModelException(MID_GWA_RE, "Set1DData", "Parameter " + sk + " does not exist in current module.");
    }
}

void ReservoirMethod::Set2DData(const char* key, const int nrows, const int ncols, float** data) {
    string sk(key);
    CheckInputSize2D(MID_GWA_RE, key, nrows, ncols, m_nCells, m_maxSoilLyrs);

    if (StringMatch(sk, VAR_PERCO)) {
        m_soilPerco = data;
    } else if (StringMatch(sk, VAR_SOL_ST)) {
        m_soilWtrSto = data;
    } else if (StringMatch(sk, VAR_SOILDEPTH)) {
        m_soilDepth = data;
    } else if (StringMatch(sk, VAR_SOILTHICK)) {
        m_soilThk = data;
    } else {
        throw ModelException(MID_GWA_RE, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void ReservoirMethod::SetSubbasins(clsSubbasins* subbsns) {
    if (m_subbasinsInfo == nullptr) {
        m_subbasinsInfo = subbsns;
        // m_nSubbasins = m_subbasinsInfo->GetSubbasinNumber(); // Set in SetValue()! lj
        m_subbasinIDs = m_subbasinsInfo->GetSubbasinIDs();
    }
}

void ReservoirMethod::Get1DData(const char* key, int* nrows, float** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_REVAP)) {
        *data = m_revap;
        *nrows = m_nCells;
    } else if (StringMatch(sk, VAR_RG)) {
        *data = m_T_RG;
        *nrows = m_nSubbsns + 1;
    } else if (StringMatch(sk, VAR_SBQG)) {
        *data = m_T_QG;
        *nrows = m_nSubbsns + 1;
    } else if (StringMatch(sk, VAR_SBGS)) {
        *data = m_gwSto;
        *nrows = m_nSubbsns + 1;
    } else if (StringMatch(sk, VAR_SBPET)) {
        *data = m_petSubbsn;
        *nrows = m_nSubbsns + 1;
    } else {
        throw ModelException(MID_GWA_RE, "Get1DData", "Parameter " + sk + " does not exist.");
    }
}

void ReservoirMethod::Get2DData(const char* key, int* nrows, int* ncols, float*** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_GWWB)) {
        *data = m_T_GWWB;
        *nrows = m_nSubbsns + 1;
        *ncols = 6;
    } else {
        throw ModelException(MID_GWA_RE, "Get2DData", "Parameter " + sk + " does not exist in current module.");
    }
}
