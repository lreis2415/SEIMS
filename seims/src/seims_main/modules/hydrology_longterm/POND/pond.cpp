#include "pond.h"

#include <cmath>
#include "text.h"

POND::POND(void) : m_nCells(-1), m_npond(-1), m_pond(NULL), m_pondVolMax(NULL), m_pondVol(NULL), m_landuse(NULL),
	               m_cellWidth(-1), m_pondSurfaceArea(NULL), m_evap_coe(0.6f), m_pond_k(0.1f),
				   m_pondID1(NULL), m_pondID2(NULL), m_pondID3(NULL), m_reachID(NULL), m_paddyNum(-1),
				   m_chStorage(NULL), m_irrDepth(NULL), m_pet(NULL), m_ks(NULL), m_soilStorage(NULL),
				   m_pondCellEvap(NULL), m_pondCellSeep(NULL), m_flowInIndex(NULL),
				   m_surfaceRunoff(NULL), m_pe(NULL), m_flowOutIndex(NULL), m_pondCellVol(NULL),
				   m_sedYield(NULL), m_sandYield(NULL), m_siltYield(NULL), m_clayYield(NULL), m_smaggreYield(NULL), m_lgaggreYield(NULL),
				   m_pondSed(NULL), m_pondSand(NULL), m_pondSilt(NULL), m_pondClay(NULL), m_pondSag(NULL), m_pondLag(NULL),
				   m_pondNo3(NULL), m_pondNH4(NULL), m_pondOrgN(NULL), m_pondSolP(NULL), m_pondOrgP(NULL), m_pondActMinP(NULL),
				   m_pondStaMinP(NULL),
				   m_surqNo3(NULL), m_surqNH4(NULL), m_surqSolP(NULL), m_surqCOD(NULL), m_sedOrgN(NULL), m_sedOrgP(NULL), 
				   m_sedActiveMinP(NULL), m_sedStableMinP(NULL),
				   m_pondSolPDecay(0.f), m_kVolat(NODATA_VALUE), m_kNitri(NODATA_VALUE), m_timestep(-1.f), m_inputSubbsnID(0), m_flowPond(NULL),
                   m_unitArea(NULL)
{
	
}


POND::~POND(void)
{
	if (m_pondVol != NULL) Release1DArray(m_pondVol);
	if (m_pondSurfaceArea != NULL) Release1DArray(m_pondSurfaceArea);
	
}
	

bool POND::CheckInputSize(const char *key, int n)
{
	if (n <= 0)
	{
		return false;
	}
	if (this->m_nCells != n)
	{
		if (this->m_nCells <= 0) this->m_nCells = n;
		else
			throw ModelException(MID_POND, "CheckInputSize", "Input data for " + string(key) +
			" is invalid. All the input data should have same size.");
	}
	return true;
}


bool POND::CheckInputData(void)
{
	return true;
}

void POND::SetValue(const char *key, float value)
{
	string sk(key);
	if(StringMatch(sk, Tag_CellWidth)){
		m_cellWidth = value;
		m_cellArea = m_cellWidth * m_cellWidth;
	}
	else if (StringMatch(sk, VAR_POND_SOLPDECAY)) m_pondSolPDecay = value;
	else if (StringMatch(sk, VAR_KV_PADDY)) m_kVolat = value;
	else if (StringMatch(sk, VAR_KN_PADDY)) m_kNitri = value;
	else if (StringMatch(sk, Tag_TimeStep)) m_timestep = value;
    else if (StringMatch(sk, Tag_SubbasinId)) m_inputSubbsnID = value;
	else
		throw ModelException(MID_POND, "SetValue", "Parameter " + sk + " does not exist.");
	
}

void POND::Set1DData(const char *key, int n, float *data)
{
	string sk(key);
	CheckInputSize(key, n);
	if (StringMatch(sk, VAR_POND)) { m_pond = data; }
	else if (StringMatch(sk, VAR_IRRDEPTH)) { m_irrDepth = data; }
	//else if (StringMatch(sk, VAR_CHST)) { m_chStorage = data; }		
	else if (StringMatch(sk, VAR_PET)) m_pet = data;
	else if (StringMatch(sk, VAR_OLFLOW)) m_surfaceRunoff = data;
	else if (StringMatch(sk, Tag_FLOWOUT_INDEX_D8))
		m_flowOutIndex = data;
	else if (StringMatch(sk, VAR_EXCP)) m_pe = data;
	else if (StringMatch(sk, VAR_LANDUSE)) m_landuse = data;
	else if (StringMatch(sk, VAR_SEDYLD)) m_sedYield = data;
	else if (StringMatch(sk, VAR_SANDYLD)) m_sandYield = data;
	else if (StringMatch(sk, VAR_SILTYLD)) m_siltYield = data;
	else if (StringMatch(sk, VAR_CLAYYLD)) m_clayYield = data;
	else if (StringMatch(sk, VAR_SAGYLD)) m_smaggreYield = data;
	else if (StringMatch(sk, VAR_LAGYLD)) m_lgaggreYield = data;
	else if (StringMatch(sk, VAR_SUR_NO3)) m_surqNo3 = data;
	else if (StringMatch(sk, VAR_SUR_NH4)) m_surqNH4 = data;
	else if (StringMatch(sk, VAR_SUR_SOLP)) m_surqSolP = data;
	else if (StringMatch(sk, VAR_SUR_COD)) m_surqCOD = data;
	else if (StringMatch(sk, VAR_SEDORGN)) m_sedOrgN = data;
	else if (StringMatch(sk, VAR_SEDORGP)) m_sedOrgP = data;
	else if (StringMatch(sk, VAR_SEDMINPA)) m_sedActiveMinP = data;
	else if (StringMatch(sk, VAR_SEDMINPS)) m_sedStableMinP = data;
    else if (StringMatch(sk, VAR_FLOWPOND)) m_flowPond = data;
    else if (StringMatch(sk, VAR_FIELDAREA)) m_unitArea = data;
	else
		throw ModelException(MID_POND, "Set1DData","Parameter " + sk +" does not exist.");
}

void POND::Set2DData(const char *key, int n, int col, float **data)
{
	string sk(key);
	CheckInputSize(key, n);
	m_soilLayers = col;

	if (StringMatch(sk, VAR_CONDUCT)) m_ks = data;
	else if (StringMatch(sk, VAR_SOL_ST)) m_soilStorage = data;
	else if (StringMatch(sk, Tag_FLOWIN_INDEX_D8))
	{
		CheckInputSize(key, n);
		m_flowInIndex = data;
	}	
}


void POND::initialOutputs()
{
	if (this->m_nCells <= 0)
	{
		throw ModelException(MID_POND, "CheckInputData", "The dimension of the input data can not be less than zero.");
	}
	// count all the pond id according to the pond raster and the grid cell of each pond
	if(m_pondIds.empty()){
		for (int i = 0; i < m_nCells; ++i){
			if ((m_inputSubbsnID != FLD_IN_SUBID && m_pond[i] != NODATA_POND) || (m_inputSubbsnID == FLD_IN_SUBID && CVT_INT(m_pond[i]) >= 0)){
				m_pondIds.push_back(m_pond[i]);
				m_pondIdInfo[m_pond[i]].push_back(i);
			}
		}
		// remove repeated id
		sort(m_pondIds.begin(),m_pondIds.end());
		m_pondIds.erase(unique(m_pondIds.begin(), m_pondIds.end()), m_pondIds.end());
		m_npond = m_pondIds.size();
	}
	
	if (m_npond <= 0)
		throw ModelException(MID_POND, "initialOutputs", "The pond number can not be less than zero.");

	// initialize some parameters,such as pond depth, max depth ...(mm)
	if (m_pondVolMax == NULL) Initialize1DArray(m_npond + 1, m_pondVolMax, 5000.f);
	if (m_pondVol == NULL) Initialize1DArray(m_npond + 1, m_pondVol, 3000.f);
	if (m_pondSurfaceArea == NULL) Initialize1DArray(m_npond + 1, m_pondSurfaceArea, 0.f);
	if (m_pondCellEvap == NULL) Initialize1DArray(m_nCells, m_pondCellEvap, 0.f);
	if (m_pondCellSeep == NULL) Initialize1DArray(m_nCells, m_pondCellSeep, 0.f);
	if (m_pondCellVol == NULL) Initialize1DArray(m_nCells, m_pondCellVol, 3000.f);
	if (m_pondSed == NULL) Initialize1DArray(m_nCells, m_pondSed, 0.f);
	if (m_pondSand == NULL) Initialize1DArray(m_nCells, m_pondSand, 0.f);
	if (m_pondSilt == NULL) Initialize1DArray(m_nCells, m_pondSilt, 0.f);
	if (m_pondClay == NULL) Initialize1DArray(m_nCells, m_pondClay, 0.f);
	if (m_pondSag == NULL) Initialize1DArray(m_nCells, m_pondSag, 0.f);
	if (m_pondLag == NULL) Initialize1DArray(m_nCells, m_pondLag, 0.f);
	if (m_pondNo3 == NULL) Initialize1DArray(m_nCells, m_pondNo3, 0.f);
	if (m_pondNH4 == NULL) Initialize1DArray(m_nCells, m_pondNH4, 0.f);
	if (m_pondOrgN == NULL) Initialize1DArray(m_nCells, m_pondOrgN, 0.f);
	if (m_pondSolP == NULL) Initialize1DArray(m_nCells, m_pondSolP, 0.f);
	if (m_pondOrgP == NULL) Initialize1DArray(m_nCells, m_pondOrgP, 0.f);
	if (m_pondActMinP == NULL) Initialize1DArray(m_nCells, m_pondActMinP, 0.f);
	if (m_pondStaMinP == NULL) Initialize1DArray(m_nCells, m_pondStaMinP, 0.f);

	if (m_pondDownPond.empty() || m_pondFlowInCell.empty()){ // only the raster version execute
		for (vector<int>::iterator it = m_pondIds.begin(); it != m_pondIds.end(); it++){					
			int id = *it;
			for(vector<int>::iterator i = m_pondIdInfo[id].begin(); i != m_pondIdInfo[id].end(); i++){
				int cellId = *i;
				int flowOutId = findFlowOutPond(id, cellId);
                if (m_inputSubbsnID == FLD_IN_SUBID) flowOutId = m_flowPond[cellId];
                if (flowOutId < UTIL_ZERO){
                    continue;
                }
				// for a pond, if the pond is over flow, and if it has downstream pond, then it will flow out to the down stream pond
				// if the pond area is large, maybe itself will be contained in the finding of downstream pond ,so should remove it
				if (m_landuse[flowOutId] == LANDUSE_ID_POND && m_pond[flowOutId] != id){
					m_pondDownPond[id].push_back(m_pond[flowOutId]);
				}
				// find flow in cell
				if (m_inputSubbsnID != FLD_IN_SUBID) findFlowInCell(id, cellId);
			}
			if (m_pondDownPond[id].empty()){
				// if it has no pond,it will direct add to reach
				m_pondDownPond[id].push_back(NODATA_POND);
			}			
		}
        // field version to find the flowincell
        if (m_inputSubbsnID == FLD_IN_SUBID) findFlowInCellFld();
	}
}

int POND::Execute()
{
	CheckInputData();
	initialOutputs();	

	// the module only simulate the pond cell
	// for each pond id, it has >= 1 cells, we simulate evap, seepage .et of each cell
	for (vector<int>::iterator it = m_pondIds.begin(); it != m_pondIds.end(); it++){
		int id = *it;
		if(id == 1){
			bool flag = true;
		}
		float totdepth = 0.f;
		// the over floe of the pond
		float qOut = 0.f;
		// compute each pond area
		pondSurfaceArea(id);
		for(vector<int>::iterator i = m_pondIdInfo[id].begin(); i != m_pondIdInfo[id].end(); i++){
			int cellId = *i;	
			// for each cell grid, the pond depth is equal to the pond itself
			pondSimulate(id, cellId);
			// sum all pond cell depth to update the whole pond depth
			totdepth += m_pondCellVol[cellId];
			// if the cell is over flow, then add to qOut
			qOut += m_surfaceRunoff[cellId];
		}
		// update the pond depth
		m_pondVol[id] = (totdepth + qOut) / m_pondIdInfo[id].size();
		if (m_pondVol[id] > m_pondVolMax[id]){
			pondRelease(id, qOut);
		}
		if(id == 122){
				std::ofstream fout;
				fout.open("d:\\pond.txt", std::ios::app);
				fout <<m_year<<"-"<<m_month<<"-"<<m_day<<","<< m_pondVol[122] << "\n";
				fout << std::flush;
				fout.close();
			}
	}				
    return true;
} 

void POND::pondRelease(int id, float qOut){
	// update qOut and set pond depth to max depth
	qOut = m_pondVol[id] - m_pondVolMax[id];
	m_pondVol[id] = m_pondVolMax[id];	
	// tmp: remove the repeated pond id in the vector
	vector<int > tmp(m_pondDownPond[id]);
	sort(tmp.begin(), tmp.end());
	tmp.erase(unique(tmp.begin(), tmp.end()), tmp.end());
	for (vector<int>::iterator i = tmp.begin(); i != tmp.end(); i++){
		int pondDown = *i;
		// if down stream pond > 1, then compute the overflow water distribution
		int num = count(m_pondDownPond[id].begin(), m_pondDownPond[id].end(), pondDown);
		float per = num / m_pondDownPond[id].size();
		if (pondDown != NODATA_POND){
			// add the overflow to downstream pond depth
			m_pondVol[pondDown] += qOut * m_pondIdInfo[id].size() / m_pondIdInfo[pondDown].size() * per;
		}
		else{
			// if no downstream pond, then set to the cell surface runoff averagely
			for(vector<int>::iterator iter = m_pondIdInfo[id].begin(); iter != m_pondIdInfo[id].end(); iter++){
				int cellId = *iter;	
				m_surfaceRunoff[cellId] = qOut;
			}
		}				
	}
}

void POND::findFlowInCell(int id, int cellId){
	// for a grid cell, find the upstream cell recursively until the cell which has no upstream cell
	int nUpstream = (int) m_flowInIndex[cellId][0];
	
	for (int upIndex = 1; upIndex <= nUpstream; ++upIndex)
	{
		int flowInID = (int) m_flowInIndex[cellId][upIndex];
		// if the flow in cell is a pond cell, then not add
		if (m_pond[flowInID] == NODATA_POND){
			m_pondFlowInCell[cellId].push_back(flowInID);
		}		
		if ((int) m_flowInIndex[flowInID][0] != 0){
			findFlowInCell(id, flowInID);
		}		
	}
}

void POND::findFlowInCellFld() {
    // for a field version, the upstream fieldid can be get in parameter flowpond
//#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) { // if current field is a pond, or the down is river, not add.
        if (m_flowPond[i] < 0 || m_landuse[i] == LANDUSE_ID_POND) continue;
        //cout<<i<<m_flowPond[i]<<endl;
        m_pondFlowInCell[CVT_INT(m_flowPond[i])].push_back(i);
    }
}

int POND::findFlowOutPond(int id, int cellId){
	int flowOutId = (int)m_flowOutIndex[cellId];
    if (flowOutId < UTIL_ZERO){
        return flowOutId;
    }
	// if the flow out cell is a pond cell or a reach  cell, then return the flow cell id
	if(m_landuse[flowOutId] != LANDUSE_ID_POND){
		if( m_landuse[flowOutId] != LANDUSE_ID_WATR){
			findFlowOutPond(id, flowOutId);			
		}
		else{
			return flowOutId;
		}		
	}
	else{
		return flowOutId;
	}		
}

void POND::pondSimulate(int id, int cellId)
{
	/// initialize temporary variables
	float pondsedo = 0.f; /// kg, sediment leaving pond on day
	float pondsano = 0.f; /// kg, sand content in sediment leaving pond on day
	float pondsilo = 0.f; /// kg, silt content
	float pondclao = 0.f; /// kg, clay content
	float pondsago = 0.f; /// kg, small aggregate
	float pondlago = 0.f; /// kg, large aggregate
	float pondno3o = 0.f; /// kg, no3 amount out of pond
	float pondnh4o = 0.f; /// kg, nh4 amount out of pond
	float pondsolpo = 0.f; /// kg, soluble phosphorus out of pond
	float pondorgno = 0.f;/// kg, orgN out
	float pondorgpo = 0.f; /// kg, orgP out
	float pondmpso = 0.f; /// kg, stable mineral phosphorus out
	float pondmpao = 0.f; /// kg, active mineral phosphorus out
	float sedloss = 0.f; /// kg, amount of sediment settling out of water during day
	float sanloss = 0.f;
	float silloss = 0.f;
	float claloss = 0.f;
	float sagloss = 0.f;
	float lagloss = 0.f;
	float no3loss = 0.f; /// kg, amount of nitrate lost from water body
	float nh4loss = 0.f; /// kg, amount of ammonian lost
	float solploss = 0.f; /// kg, amount of solP lost
	float orgnloss = 0.f; /// kg, amount of orgN lost
	float orgploss = 0.f; /// kg, amount of orgP lost
	float minpsloss = 0.f; /// kg, amount of stable minP lost
	float minpaloss = 0.f; /// kg, amount of active minP lost

	/// currently, we assumed that the entire cell is a pond cell
	float pond_fr = 1.f;
	/// update sediment in pond
	m_pondSed[cellId] += m_sedYield[cellId] * pond_fr;
	m_pondSand[cellId] += m_sandYield[cellId] * pond_fr;
	float m_pondSandIn = m_pondSand[cellId];
	m_pondSilt[cellId] += m_siltYield[cellId] * pond_fr;
	float m_pondSiltIn = m_pondSilt[cellId];
	m_pondClay[cellId] += m_clayYield[cellId] * pond_fr;
	float m_pondClayIn = m_pondClay[cellId];
	m_pondSag[cellId] += m_smaggreYield[cellId] * pond_fr;
	float m_pondSagIn = m_pondSag[cellId];
	m_pondLag[cellId] += m_lgaggreYield[cellId] * pond_fr;
	float m_pondLagIn = m_pondLag[cellId];

	/// update sediment yields
	float yy = 1.f - pond_fr;
	m_sedYield[cellId] *= yy;
	m_sandYield[cellId] *= yy;
	m_siltYield[cellId] *= yy;
	m_clayYield[cellId] *= yy;
	m_smaggreYield[cellId] *= yy;
	m_lgaggreYield[cellId] *= yy;

	/// update forms of N and P in pond
	float xx = pond_fr * m_cellArea * 1.e-4f;
    if (m_inputSubbsnID == FLD_IN_SUBID) xx = pond_fr * m_unitArea[cellId] * 1.e-4f;
	m_pondNo3[cellId] += m_surqNo3[cellId] * xx; // kg/ha * ha ==> kg
	m_pondNH4[cellId] += m_surqNH4[cellId] * xx;
	m_pondOrgN[cellId] += m_sedOrgN[cellId] * xx;
	m_pondSolP[cellId] += m_surqSolP[cellId] * xx;
	m_pondOrgP[cellId] += m_sedOrgP[cellId] * xx;
	m_pondActMinP[cellId] += m_sedActiveMinP[cellId] * xx;
	m_pondStaMinP[cellId] += m_sedStableMinP[cellId] * xx;

	/// update forms of N and P in surface runoff
	m_surqNo3[cellId] *= yy;
	m_surqNH4[cellId] *= yy;
	m_sedOrgN[cellId] *= yy;
	m_surqSolP[cellId] *= yy;
	m_sedOrgP[cellId] *= yy;
	m_sedActiveMinP[cellId] *= yy;
	m_sedStableMinP[cellId] *= yy;

	float qIn = 0.f;
	// 1. add m_pe in SUR_MR
	qIn += m_pe[cellId];
	// 2. add surface runoff from upstream cell
	if(m_pondFlowInCell[cellId].size() != 0){
		// compute the surface runoff from upstream cell to the pond cell
		for (vector<int>::iterator iter = m_pondFlowInCell[cellId].begin(); iter != m_pondFlowInCell[cellId].end(); iter++){
			int k = *iter;
			qIn += m_surfaceRunoff[k];
			m_surfaceRunoff[k] = 0.f;
		}
	}
	// 3. update cell depth
	m_pondCellVol[cellId] += qIn;	

	/// if overflow, then send the overflow to the cell's surface flow
	if(m_pondCellVol[cellId] > m_pondVolMax[id]){
		m_surfaceRunoff[cellId] += m_pondCellVol[cellId] - m_pondVolMax[id];
		float qTmp = m_pondCellVol[cellId] - m_pondVolMax[id];
		m_pondCellVol[cellId] = m_pondVolMax[id];
		float xx = qTmp / (qTmp + m_pondVolMax[id]);

		pondsedo += m_pondSed[cellId] * xx;
		pondsano += m_pondSand[cellId] * xx;
		pondsilo += m_pondSilt[cellId] * xx;
		pondclao += m_pondClay[cellId] * xx;
		pondsago += m_pondSag[cellId] * xx;
		pondlago += m_pondLag[cellId] * xx;
		pondno3o += m_pondNo3[cellId] * xx;
		pondnh4o += m_pondNH4[cellId] * xx;
		pondorgno += m_pondOrgN[cellId] * xx;
		pondsolpo += m_pondSolP[cellId] * xx;
		pondorgpo += m_pondOrgP[cellId] * xx;
		pondmpao += m_pondActMinP[cellId] * xx;
		pondmpso += m_pondStaMinP[cellId] * xx;

		/// remove the sediment and N¡¢P with overflow from pond
		m_pondSed[cellId] -= pondsedo;
		m_pondSand[cellId] -= pondsano;
		m_pondSilt[cellId] -= pondsilo;
		m_pondClay[cellId] -= pondclao;
		m_pondSag[cellId] -= pondsago;
		m_pondLag[cellId] -= pondlago;
		m_pondNo3[cellId] -= pondno3o;
		m_pondNH4[cellId] -= pondnh4o;
		m_pondOrgN[cellId] -= pondorgno;
		m_pondSolP[cellId] -= pondsolpo;
		m_pondOrgP[cellId] -= pondorgpo;
		m_pondStaMinP[cellId] -= pondmpso;
		m_pondActMinP[cellId] -= pondmpao;

		// update sediment and forms of N and P in surface runoff
		float area = m_cellArea * 1.e-4f;
        if (m_inputSubbsnID == FLD_IN_SUBID) area = m_unitArea[cellId] * 1.e-4f;
		m_sedYield[cellId] += pondsedo / area;
		m_sandYield[cellId] += pondsano / area;
		m_siltYield[cellId] += pondsilo / area;
		m_clayYield[cellId] += pondclao / area;
		m_smaggreYield[cellId] += pondsago / area;
		m_lgaggreYield[cellId] += pondlago / area;
		m_surqNo3[cellId] += pondno3o / area;
		m_surqNH4[cellId] += pondnh4o / area;
		m_sedOrgN[cellId] += pondorgno / area;
		m_surqSolP[cellId] += pondsolpo / area;
		m_sedOrgP[cellId] += pondorgpo / area;
		m_sedStableMinP[cellId] += pondmpso / area;
		m_sedActiveMinP[cellId] += pondmpao / area;
	}

	/// if no overflow
	if (m_pondCellVol[cellId] > UTIL_ZERO)
	{
		/// compute settling  -clay and silt based on fall velocity (v = 411*d2) d=mm, v=m/hr
		float pond_depth = m_pondVol[cellId];
		float drcla = 0.f, drsil = 0.f, drtot = 0.f;
		if (pond_depth > 10.f) /// assume clay v(fall) = 10 mm/d
			drcla = 1.f - 0.5f * 10.f / pond_depth;
		else
			drcla = 0.5f * pond_depth / 10.f;
		m_pondClay[cellId] *= drcla;

		if (pond_depth > 1000.f) /// assume silt v(fall) = 1000 mm/d
			drsil = 1.f - 0.5f * 1000.f / pond_depth;
		else
			drsil = 0.5f * pond_depth / 1000.f;
		m_pondSilt[cellId] *= drsil;

		/// assume complete settling of all other size (dr = 0)
		m_pondSand[cellId] = 0.f;
		m_pondSag[cellId] = 0.f;
		m_pondLag[cellId] = 0.f;

		/// compute total delivery ratio for pond_sed
		float allSedPart = m_pondClayIn + m_pondSiltIn + m_pondSandIn + m_pondSagIn + m_pondLagIn;

		if (allSedPart < UTIL_ZERO)
			drtot = 0.f;
		else
			drtot = (m_pondClay[cellId] + m_pondSilt[cellId] + m_pondSand[cellId] + m_pondSag[cellId] + m_pondLag[cellId]) / allSedPart;
		m_pondSed[cellId] *= drtot;

		/// compute organic settling assuming an enrichment ratio of 3 on clay (0.75)
		/// delivery of organics is 0.75 * dr(clay)- assuming dr on all non-clay = 1
		m_pondOrgN[cellId] *= 0.75f * drcla;
		m_pondOrgP[cellId] *= 0.75f * drcla;
		m_pondActMinP[cellId] *= 0.75f * drcla;
		m_pondStaMinP[cellId] *= 0.75f * drcla;
		m_pondSolP[cellId] *= (1.f - m_pondSolPDecay);

		/*
		 * first-order kinetics is adopted from Chowdary et al., 2004
		 * to account for volatilization, nitrification, and denitrification in impounded water
		 */
		float nh3V = m_pondNH4[cellId] * (1.f - exp(-m_kVolat * m_timestep / 86400.f));
		float no3N = m_pondNH4[cellId] * (1.f - exp(-m_kNitri * m_timestep / 86400.f));
		/// update
		m_pondNH4[cellId] -= (nh3V + no3N);
		m_pondNo3[cellId] += no3N;

		m_pondNH4[cellId] = max(m_pondNH4[cellId], UTIL_ZERO);
		m_pondNo3[cellId] = max(m_pondNo3[cellId], UTIL_ZERO);

		// for a cell grid, compute evaporation from water surface
		float pondevap = m_evap_coe * m_pet[cellId]; // mm
		pondevap = min(pondevap, m_pondCellVol[cellId]);
		m_pondCellVol[cellId] -= pondevap;
		m_pondCellEvap[cellId] += pondevap;

		// calculate seepage into soil, limit seepage into soil if profile is near field capacity
		float pondsep = m_pond_k * 24.f; // mm
		pondsep = min(pondsep, m_pondCellVol[cellId]);
		float pondVol_ori = m_pondCellVol[cellId];
		m_pondCellVol[cellId] -= pondsep;
		m_pondCellSeep[cellId] += pondsep;
		m_soilStorage[cellId][0] += pondsep; 

		// update the sediment and N¡¢P remove with the seepage
		if (pondsep > UTIL_ZERO)
		{
			float lossRatio = pondsep / pondVol_ori;
			sedloss = m_pondSed[cellId] * lossRatio;
			sedloss = min(sedloss, m_pondSed[cellId]);
			m_pondSed[cellId] -= sedloss;

			no3loss = m_pondNo3[cellId] * lossRatio;
			no3loss = min(no3loss, m_pondNo3[cellId]);
			m_pondNo3[cellId] -= no3loss;
			
			nh4loss = m_pondNH4[cellId] * lossRatio;
			nh4loss = min(nh4loss, m_pondNH4[cellId]);
			m_pondNH4[cellId] -= nh4loss;

			solploss = m_pondSolP[cellId] * lossRatio;
			solploss = min(solploss, m_pondSolP[cellId]);
			m_pondSolP[cellId] -= solploss;

			orgnloss = m_pondOrgN[cellId] * lossRatio;
			orgnloss = min(orgnloss, m_pondOrgN[cellId]);
			m_pondOrgN[cellId] -= orgnloss;

			orgploss = m_pondOrgP[cellId] * lossRatio;
			orgploss = min(orgploss, m_pondOrgP[cellId]);
			m_pondOrgP[cellId] -= orgploss;

			minpsloss = m_pondStaMinP[cellId] * lossRatio;
			minpsloss = min(minpsloss, m_pondStaMinP[cellId]);
			m_pondStaMinP[cellId] -= minpsloss;

			minpaloss = m_pondActMinP[cellId] * lossRatio;
			minpaloss = min(minpaloss, m_pondActMinP[cellId]);
			m_pondActMinP[cellId] -= minpaloss;

			sanloss = m_pondSand[cellId] * lossRatio;
			m_pondSand[cellId] -= sanloss;

			silloss = m_pondSilt[cellId] * lossRatio;
			m_pondSilt[cellId] -= silloss;

			claloss = m_pondClay[cellId] * lossRatio;
			m_pondClay[cellId] -= claloss;

			sagloss = m_pondSag[cellId] * lossRatio;
			m_pondSag[cellId] -= sagloss;

			lagloss = m_pondLag[cellId] * lossRatio;
			m_pondLag[cellId] -= lagloss;
		}
	}
}

void POND::pondSurfaceArea(int id)
{
    if (m_inputSubbsnID != FLD_IN_SUBID){
	    // now, we assume if the cell is pond, then the cell area is pond area, the whole pond area is the sun of all cell area
	    float cellArea = m_cellWidth * m_cellWidth;
	    float cellNum = m_pondIdInfo[id].size();
        m_pondSurfaceArea[id] = cellNum * cellArea;	
    }
    else { // if the field version, read unitarea parameter directly
        m_pondSurfaceArea[id] = m_unitArea[id];	
    }
}

void POND::Get1DData(const char *key, int *n, float **data)
{
	initialOutputs();
	string sk(key);
	if (StringMatch(sk, VAR_POND_VOL))*data = m_pondVol;
	else if (StringMatch(sk, VAR_POND_SA))*data = m_pondSurfaceArea;
	else 
		throw ModelException(MID_POND, "Get1DData","Parameter" + sk + "does not exist.");
	*n = m_nCells;
	
	return;
}
