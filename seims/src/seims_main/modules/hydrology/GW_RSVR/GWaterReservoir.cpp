#include "GWaterReservoir.h"
#include "text.h"

// using namespace std;  // Avoid this statement! by lj.

GWaterReservoir::GWaterReservoir(void) : m_recharge(NULL), m_storage(NULL), m_recessionCoefficient(-1.f),
                                         m_recessionExponent(1.f), m_cellArea(nullptr),
                                         m_deepCoefficient(0.f), m_nCells(-1), m_nReaches(-1), m_qg(NULL),
                                         m_percSubbasin(NULL), m_subbasin(NULL), m_subbasinID(-1),
                                         m_nCellsSubbasin(NULL), m_initStorage(0.f) {
}

GWaterReservoir::~GWaterReservoir(void) {
    if (m_qg != NULL) Release1DArray(m_qg);
    if (m_nCellsSubbasin != NULL) Release1DArray(m_nCellsSubbasin);
    if (m_percSubbasin != NULL) Release1DArray(m_percSubbasin);
    if (m_storage != NULL) Release1DArray(m_storage);
}

bool GWaterReservoir::CheckInputData() {
    CHECK_NONNEGATIVE(M_GW_RSVR[0], m_subbasinID);
    if (this->m_date < 0) {
        throw ModelException(M_GW_RSVR[0], "CheckInputData", "You have not set the time.");
        return false;
    }
    if (this->m_dt < 0) {
        throw ModelException(M_GW_RSVR[0], "CheckInputData", "You have not set the time step.");
        return false;
    }
    if (this->m_nCells <= 0) {
        throw ModelException(M_GW_RSVR[0], "CheckInputData", "The cell number is not set.");
        return false;
    }

    CHECK_POINTER(M_GW_RSVR[0], m_cellArea);

    if (m_recessionCoefficient <= 0) {
        throw ModelException(M_GW_RSVR[0], "CheckInputData", "The base flow recession coefficient is not set.");
        return false;
    }

    if (m_recharge == NULL) {
        throw ModelException(M_GW_RSVR[0], "CheckInputData", "The percolation is not set.");
        return false;
    }

    return true;
}

bool GWaterReservoir::CheckInputSize(const char *key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}

//void GWaterReservoir::InitOutputs(void) {
    //if (m_qg == NULL) {
        //Initialize1DArray(m_nReaches + 1, m_qg, 0.f);
        //Initialize1DArray(m_nReaches + 1, m_nCellsSubbasin, 0);
        //Initialize1DArray(m_nReaches + 1, m_storage, m_initStorage);
        //Initialize1DArray(m_nReaches + 1, m_storage, m_percSubbasin);

        //if (m_subbasinID == 0) { // deprecate the previously used macro MULTIPLY_REACHES
            //for (int i = 0; i < m_nCells; i++) {
                //m_nCellsSubbasin[(int)m_subbasin[i]] += 1;
            //}
        //} else {
            //m_nCellsSubbasin[1] = m_nCells;
        //}
    //}
//}

void GWaterReservoir::InitOutputs(void) {
	if (m_qg == nullptr) Initialize1DArray(m_nReaches + 1, m_qg, 0.f);
	if (m_nCellsSubbasin == nullptr) Initialize1DArray(m_nReaches + 1, m_nCellsSubbasin, 0);
	if (m_percSubbasin == nullptr) Initialize1DArray(m_nReaches + 1, m_percSubbasin, 0.f);
	if (m_storage == nullptr) Initialize1DArray(m_nReaches + 1, m_storage, 0.f);
	if (m_subbasinID == 0) { // deprecate the previously used macro MULTIPLY_REACHES
		for (int i = 0; i < m_nCells; i++) {
			m_nCellsSubbasin[(int)m_subbasin[i]] += 1;
		}
	}
	else {
		m_nCellsSubbasin[1] = m_nCells;
	}
}

int GWaterReservoir::Execute(void) {
    InitOutputs();
    CheckInputData();
#pragma omp parallel for
    for (int i = 0; i <= m_nReaches; i++) {
        m_percSubbasin[i] = 0.f;
    }

    // get percolation for each subbasin
    for (int i = 0; i < m_nCells; i++) {
        int subbasinIdx = 1; // In MPI version, only one subbasin was calculated
        if (m_subbasinID == 0) { // deprecate the previously used macro MULTIPLY_REACHES
            subbasinIdx = (int)m_subbasin[i];
        }
        m_percSubbasin[subbasinIdx] += m_recharge[i];
    }

    //FLTPT sum = 0.f;
#pragma omp parallel for //reduction(+:sum)
    for (int i = 1; i <= m_nReaches; i++) {
        FLTPT percolation = m_percSubbasin[i] * (1.f - m_deepCoefficient) / m_nCellsSubbasin[i];
        // depth of groundwater runoff(mm)
        FLTPT outFlowDepth = m_recessionCoefficient * CalPow(m_storage[i], m_recessionExponent);
        // groundwater flow out of the subbasin at time t (m3/s)
        m_qg[i] = outFlowDepth / 1000.f * m_nCellsSubbasin[i] * m_cellArea[i] / m_dt;
        //sum = sum + m_qg[i];

        // water balance (mm)
        m_storage[i] += percolation - outFlowDepth;
    }
    //m_qg[0] = sum;

    return 0;
}

// set value
void GWaterReservoir::SetValue(const char *key, FLTPT value) {
    string sk(key);
    if (StringMatch(sk, Tag_HillSlopeTimeStep[0])) {
        m_dt = value;
    } else if (StringMatch(sk, Tag_SubbasinId)) {
        m_subbasinID = int(value);
    } else if (StringMatch(sk, VAR_GW_KG[0])) {
        m_recessionCoefficient = value;
    } else if (StringMatch(sk, VAR_Base_ex[0])) {
        m_recessionExponent = value;
    } else if (StringMatch(sk, VAR_GW0[0])) {
        m_initStorage = value;
    } else if (StringMatch(sk, VAR_GWMAX[0])) {
        m_storageMax = value;
    } else {
        throw ModelException(M_GW_RSVR[0], "SetValue", "Parameter " + sk + " does not exist in SetValue method.");
    }
}

void GWaterReservoir::Set1DData(const char *key, int n, FLTPT *data) {
    string sk(key);
    if (StringMatch(sk, VAR_PERCO[0])) {
        m_recharge = data;
    } else if (StringMatch(sk, VAR_SUBBSN[0])) {
        this->m_subbasin = data;
    } else if (StringMatch(sk, VAR_CELL_AREA[0])) {
        m_cellArea = data;
    } else {
        throw ModelException(M_GW_RSVR[0], "Set1DData",
                             "Parameter " + sk + " does not exist. Please contact the module developer.");
    }
}

void GWaterReservoir::SetReaches(clsReaches *reaches) {
    assert(NULL != reaches);
    m_nReaches = reaches->GetReachNumber();
}

void GWaterReservoir::Get1DData(const char *key, int *n, FLTPT **data) {
    InitOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SBQG[0])) {
        *data = m_qg;
    } else if (StringMatch(sk, VAR_SBGS[0])) {
        *data = m_storage;
    } else {
        throw ModelException(M_GW_RSVR[0], "Get1DData",
                             "Parameter " + sk + " does not exist. Please contact the module developer.");
    }
}
