#include "api.h"
#include <iostream>
#include "pothole_SWAT.h"
#include "MetadataInfo.h"
#include "ModelException.h"

using namespace std;

IMP_SWAT::IMP_SWAT(void) : m_cnv(NODATA_VALUE), m_nCells(-1), m_cellWidth(NODATA_VALUE), m_cellArea(NODATA_VALUE), 
	m_soilLayers(NULL), m_nSoilLayers(-1), m_routingLayers(NULL), m_nRoutingLayers(-1), m_subbasin(NULL),
	m_slope(NULL), m_ks(NULL), m_sol_sat(NULL), m_sol_sumfc(NULL), m_soilThick(NULL), m_sol_por(NULL), 
	m_evLAI(NODATA_VALUE), m_potTilemm(NODATA_VALUE), m_potNo3Decay(NODATA_VALUE), m_potSolPDecay(NODATA_VALUE),
	m_impoundTrig(NULL), 
	m_sedYield(NULL), m_sandYield(NULL), m_siltYield(NULL), m_clayYield(NULL), m_smaggreYield(NULL), m_lgaggreYield(NULL),
	m_depEvapor(NULL), m_depStorage(NULL), m_LAIDay(NULL), m_pet(NULL), m_soilStorage(NULL), m_soilStorageProfile(NULL), 
	m_surfaceRunoff(NULL), m_surqNo3(NULL), m_surqNH4(NULL), m_surqSolP(NULL), m_surqCOD(NULL), m_sedOrgN(NULL), m_sedOrgP(NULL), m_sedActiveMinP(NULL), m_sedStableMinP(NULL),
	m_potNo3(NULL), m_potNH4(NULL), m_potOrgN(NULL), m_potSolP(NULL), m_potOrgP(NULL), m_potActMinP(NULL),
	m_potStaMinP(NULL), m_potSed(NULL), m_potSand(NULL), m_potSilt(NULL), m_potClay(NULL), m_potSag(NULL), m_potLag(NULL), 
	m_potVol(NULL), m_potVolMax(NULL), m_potVolMin(NULL), m_potSeep(NULL), m_potEvap(NULL), m_potSurfaceArea(NULL),
	m_kVolat(NODATA_VALUE), m_kNitri(NODATA_VALUE), m_pot_k(NODATA_VALUE),
	/// overland to channel
	m_surfqToCh(NULL), m_sedToCh(NULL), m_surNO3ToCh(NULL), m_surNH4ToCh(NULL), m_surSolPToCh(NULL), m_surCodToCh(NULL), 
	m_sedOrgNToCh(NULL), m_sedOrgPToCh(NULL), m_sedMinPAToCh(NULL), m_sedMinPSToCh(NULL)
{
	//m_potSedIn(NULL), m_potSandIn(NULL), m_potSiltIn(NULL), m_potClayIn(NULL), m_potSagIn(NULL), m_potLagIn(NULL),
}


IMP_SWAT::~IMP_SWAT(void)
{
	if (m_potSurfaceArea != NULL) Release1DArray(m_potSurfaceArea);
	if (m_potNo3 != NULL) Release1DArray(m_potNo3);
	if (m_potNH4 != NULL) Release1DArray(m_potNH4);
	if (m_potOrgN != NULL) Release1DArray(m_potOrgN);
	if (m_potSolP != NULL) Release1DArray(m_potSolP);
	if (m_potOrgP != NULL) Release1DArray(m_potOrgP);
	if (m_potActMinP != NULL) Release1DArray(m_potActMinP);
	if (m_potStaMinP != NULL) Release1DArray(m_potStaMinP);
	if (m_potSed != NULL) Release1DArray(m_potSed);
	if (m_potSand != NULL) Release1DArray(m_potSand);
	if (m_potSilt != NULL) Release1DArray(m_potSilt);
	if (m_potClay != NULL) Release1DArray(m_potClay);
	if (m_potSag != NULL) Release1DArray(m_potSag);
	if (m_potLag != NULL) Release1DArray(m_potLag);
	if (m_potVol != NULL) Release1DArray(m_potVol);
	if (m_potSeep != NULL) Release1DArray(m_potSeep);
	if (m_potEvap != NULL) Release1DArray(m_potEvap);
}

bool IMP_SWAT::CheckInputSize(const char *key, int n)
{
	if (n <= 0)
		throw ModelException(MID_IMP_SWAT, "CheckInputSize",
		"Input data for " + string(key) + " is invalid. The size could not be less than zero.");
	if (this->m_nCells != n)
	{
		if (this->m_nCells <= 0) this->m_nCells = n;
		else
			throw ModelException(MID_IMP_SWAT, "CheckInputSize", "Input data for " + string(key) +
			" is invalid. All the input data should have same size.");
	}
	return true;
}

bool IMP_SWAT::CheckInputSize2D(const char *key, int n, int col)
{
	CheckInputSize(key, n);
	if (col <= 0)
		throw ModelException(MID_IMP_SWAT, "CheckInputSize2D", "Input data for " + string(key) +
		" is invalid. The layer number could not be less than zero.");
	if (m_nSoilLayers != col)
	{
		if (m_nSoilLayers <= 0)
			m_nSoilLayers = col;
		else
		{
			throw ModelException(MID_IMP_SWAT, "CheckInputSize2D", "Input data for " + string(key) +
				" is invalid. All the layers of input 2D raster data should have same size of " +
				ValueToString(m_nSoilLayers) + " instead of " + ValueToString(col) + ".");
			return false;
		}
	}
	return true;
}

bool IMP_SWAT::CheckInputData()
{
	if (m_cellWidth <= 0)
		throw ModelException(MID_IMP_SWAT, "CheckInputData", "The cell width can not be less than zero.");
	if (m_nCells <= 0)
		throw ModelException(MID_IMP_SWAT, "CheckInputData", "The dimension of the input data can not be less than zero.");
	if (m_nSoilLayers <= 0)
		throw ModelException(MID_IMP_SWAT, "CheckInputData", "The soil layers number can not be less than zero.");
	if (m_nRoutingLayers <= 0)
		throw ModelException(MID_IMP_SWAT, "CheckInputData", "The routing layers number can not be less than zero.");
	if (m_evLAI <= 0)
		throw ModelException(MID_IMP_SWAT, "CheckInputData", "The evLAI can not be less than zero.");
	if (m_potTilemm < 0)
		throw ModelException(MID_IMP_SWAT, "CheckInputData", "The m_potTilemm number can not be less than zero.");
	if (m_potNo3Decay < 0)
		throw ModelException(MID_IMP_SWAT, "CheckInputData", "The m_potNo3Decay can not be less than zero.");
	if (m_potSolPDecay < 0)
		throw ModelException(MID_IMP_SWAT, "CheckInputData", "The m_potSolPDecay can not be less than zero.");

	return true;
}

void IMP_SWAT::SetValue(const char *key, float value)
{
	string sk(key);
	if(StringMatch(sk, Tag_CellWidth)){
		m_cellWidth = value;
		m_cellArea = m_cellWidth * m_cellWidth * 1.e-4f; // m2 ==> ha
		m_cnv = 10.f * m_cellArea; // mm/ha => m^3
	}
	else if (StringMatch(sk, Tag_TimeStep)) m_timestep = value;
	else if (StringMatch(sk, VAR_EVLAI)) m_evLAI = value;
	else if (StringMatch(sk, VAR_POT_TILEMM)) m_potTilemm = value;
	else if (StringMatch(sk, VAR_POT_NO3DECAY)) m_potNo3Decay = value;
	else if (StringMatch(sk, VAR_POT_SOLPDECAY)) m_potSolPDecay = value;
	else if (StringMatch(sk, VAR_KV_PADDY)) m_kVolat = value;
	else if (StringMatch(sk, VAR_KN_PADDY)) m_kNitri = value;
	else if (StringMatch(sk, VAR_POT_K)) m_pot_k = value;
	else
		throw ModelException(MID_IMP_SWAT, "SetValue", "Parameter " + sk + " does not exist.");
}

void IMP_SWAT::Set1DData(const char *key, int n, float *data)
{
	string sk(key);

	if (StringMatch(sk, VAR_SBOF)){
		m_surfqToCh = data;
		m_subbasinNum = n - 1; /// TODO, add a checkInputSize2 function
		return;
	}
	else if (StringMatch(sk, VAR_SED_TO_CH)){
		m_sedToCh = data;
		m_subbasinNum = n - 1;
		return;
	}
	else if (StringMatch(sk, VAR_SUR_NO3_TOCH)){
		m_surNO3ToCh = data;
		m_subbasinNum = n - 1;
		return;
	}
	else if (StringMatch(sk, VAR_SUR_NH4_TOCH)){
		m_surNH4ToCh = data;
		m_subbasinNum = n - 1;
		return;
	}
	else if (StringMatch(sk, VAR_SUR_SOLP_TOCH)){
		m_surSolPToCh = data;
		m_subbasinNum = n - 1;
		return;
	}
	else if (StringMatch(sk, VAR_SUR_COD_TOCH)){
		m_surCodToCh = data;
		m_subbasinNum = n - 1;
		return;
	}
	else if (StringMatch(sk, VAR_SEDORGN_TOCH)){
		m_sedOrgNToCh = data;
		m_subbasinNum = n - 1;
		return;
	}
	else if (StringMatch(sk, VAR_SEDORGP_TOCH)){
		m_sedOrgPToCh = data;
		m_subbasinNum = n - 1;
		return;
	}
	else if (StringMatch(sk, VAR_SEDMINPA_TOCH)){
		m_sedMinPAToCh = data;
		m_subbasinNum = n - 1;
		return;
	}
	else if (StringMatch(sk, VAR_SEDMINPS_TOCH)){
		m_sedMinPSToCh = data;
		m_subbasinNum = n - 1;
		return;
	}
	CheckInputSize(key, n);
	if (StringMatch(sk, VAR_SLOPE))m_slope = data;
	else if (StringMatch(sk, VAR_SOILLAYERS))m_soilLayers = data;
	else if (StringMatch(sk, VAR_SUBBSN)) m_subbasin = data;
	else if (StringMatch(sk, VAR_SOL_SUMAWC)) m_sol_sumfc = data;
	else if (StringMatch(sk, VAR_IMPOUND_TRIG)) m_impoundTrig = data;
	else if (StringMatch(sk, VAR_POT_VOLMAXMM)) m_potVolMax = data;
	else if (StringMatch(sk, VAR_POT_VOLLOWMM)) m_potVolMin = data;
	else if (StringMatch(sk, VAR_SEDYLD)) m_sedYield = data;
	else if (StringMatch(sk, VAR_SANDYLD)) m_sandYield = data;
	else if (StringMatch(sk, VAR_SILTYLD)) m_siltYield = data;
	else if (StringMatch(sk, VAR_CLAYYLD)) m_clayYield = data;
	else if (StringMatch(sk, VAR_SAGYLD)) m_smaggreYield = data;
	else if (StringMatch(sk, VAR_LAGYLD)) m_lgaggreYield = data;
	else if (StringMatch(sk, VAR_LAIDAY)) m_LAIDay = data;
	else if (StringMatch(sk, VAR_PET)) m_pet = data;
	else if (StringMatch(sk, VAR_SOL_SW)) m_soilStorageProfile = data;
	else if (StringMatch(sk, VAR_DEET)) m_depEvapor = data;
	else if (StringMatch(sk, VAR_DPST)) m_depStorage = data;
	else if (StringMatch(sk, VAR_OLFLOW)) m_surfaceRunoff = data;
	else if (StringMatch(sk, VAR_SUR_NO3)) m_surqNo3 = data;
	else if (StringMatch(sk, VAR_SUR_NH4)) m_surqNH4 = data;
	else if (StringMatch(sk, VAR_SUR_SOLP)) m_surqSolP = data;
	else if (StringMatch(sk, VAR_SUR_COD)) m_surqCOD = data;
	else if (StringMatch(sk, VAR_SEDORGN)) m_sedOrgN = data;
	else if (StringMatch(sk, VAR_SEDORGP)) m_sedOrgP = data;
	else if (StringMatch(sk, VAR_SEDMINPA)) m_sedActiveMinP = data;
	else if (StringMatch(sk, VAR_SEDMINPS)) m_sedStableMinP = data;
	else
		throw ModelException(MID_IMP_SWAT, "Set1DData", "Parameter " + sk + " does not exist.");
}

void IMP_SWAT::Set2DData(const char *key, int n, int col, float **data)
{
	string sk(key);
	if (StringMatch(sk, Tag_ROUTING_LAYERS))
	{
		m_nRoutingLayers = n;
		m_routingLayers = data;
		return;
	}
	CheckInputSize2D(key, n, col);
	if (StringMatch(sk, VAR_CONDUCT))m_ks = data;
	else if (StringMatch(sk, VAR_SOILTHICK)) m_soilThick = data;
	else if (StringMatch(sk, VAR_POROST)) m_sol_por = data;
	else if (StringMatch(sk, VAR_SOL_ST)) m_soilStorage = data;
	else if (StringMatch(sk, VAR_SOL_UL)) m_sol_sat = data;
	else
		throw ModelException(MID_IMP_SWAT, "Set2DData", "Parameter " + sk + " does not exist.");
}

void IMP_SWAT::initialOutputs()
{
	if (m_potSurfaceArea == NULL) Initialize1DArray(m_nCells, m_potSurfaceArea, 0.f);
	if (m_potVol == NULL) Initialize1DArray(m_nCells, m_potVol, 0.f);
	if (m_potNo3 == NULL) Initialize1DArray(m_nCells, m_potNo3, 0.f);
	if (m_potNH4 == NULL) Initialize1DArray(m_nCells, m_potNH4, 0.f);
	if (m_potOrgN == NULL) Initialize1DArray(m_nCells, m_potOrgN, 0.f);
	if (m_potSolP == NULL) Initialize1DArray(m_nCells, m_potSolP, 0.f);
	if (m_potOrgP == NULL) Initialize1DArray(m_nCells, m_potOrgP, 0.f);
	if (m_potActMinP == NULL) Initialize1DArray(m_nCells, m_potActMinP, 0.f);
	if (m_potStaMinP == NULL) Initialize1DArray(m_nCells, m_potStaMinP, 0.f);
	if (m_potSed == NULL) Initialize1DArray(m_nCells, m_potSed, 0.f);
	if (m_potSand == NULL) Initialize1DArray(m_nCells, m_potSand, 0.f);
	if (m_potSilt == NULL) Initialize1DArray(m_nCells, m_potSilt, 0.f);
	if (m_potClay == NULL) Initialize1DArray(m_nCells, m_potClay, 0.f);
	if (m_potSag == NULL) Initialize1DArray(m_nCells, m_potSag, 0.f);
	if (m_potLag == NULL) Initialize1DArray(m_nCells, m_potLag, 0.f);
	/// water loss
	if (m_potSeep == NULL) Initialize1DArray(m_nCells, m_potSeep, 0.f);
	if (m_potEvap == NULL) Initialize1DArray(m_nCells, m_potEvap, 0.f);
}

int IMP_SWAT::Execute()
{
	CheckInputData();
	initialOutputs();
	
	for (int iLayer = 0; iLayer < m_nRoutingLayers; ++iLayer)
	{
		// There are not any flow relationship within each routing layer.
		// So parallelization can be done here.
		int nCells = (int) m_routingLayers[iLayer][0];
#pragma omp parallel for
		for (int iCell = 1; iCell <= nCells; ++iCell)
		{
			int id = (int) m_routingLayers[iLayer][iCell]; // cell index
			if (FloatEqual(m_impoundTrig[id], 0.f)) /// if impounding trigger on
				potholeSimulate(id);
			else{
				releaseWater(id);
			}
		}
	}
	/// reCalculate the surface runoff, sediment, nutrient etc. that into the channel
	// cout<<"pre surq no3 to ch: "<<m_surNO3ToCh[12]<<endl;
	// cout<<"pre surfq to ch: "<<m_surfqToCh[12]<<", orgp to ch: "<<m_sedOrgPToCh[12]<<endl;
#pragma omp parallel for
	for (int i = 0; i < m_subbasinNum + 1; i++)
	{
		m_surfqToCh[i] = 0.f;
		m_sedToCh[i] = 0.f;
		m_surNO3ToCh[i] = 0.f;
		m_surNH4ToCh[i] = 0.f;
		m_surSolPToCh[i] = 0.f;
		m_surCodToCh[i] = 0.f;
		m_sedOrgNToCh[i] = 0.f;
		m_sedOrgPToCh[i] = 0.f;
		m_sedMinPAToCh[i] = 0.f;
		m_sedMinPSToCh[i] = 0.f;
	}
	// cout<<"final orgp: "<<m_sedOrgP[46364]<<endl;
	// cout<<"final surq no3: "<<m_surqNo3[46364]<<endl;
	//float maxno3 = -1.f;
	//int idx = -1;
	//for (int i = 0; i < m_nCells; i++)
	//{
	//	if (m_surqNo3[i] > maxno3)
	//	{
	//		maxno3 = m_surqNo3[i];
	//		idx = i;
	//	}
	//}
	//cout<<"maximum no3 id: "<<idx<<endl;
	float maxsedorgp = -1.f;
	int idx = -1;
	for (int i = 0; i < m_nCells; i++)
	{
		if (m_sedOrgP[i] > maxsedorgp)
		{
			maxsedorgp = m_sedOrgP[i];
			idx = i;
		}
	}
	// cout<<"maximum sedorgp id: "<<idx<< ", surfq: " <<m_surfaceRunoff[idx]<<", sedorgp: "<<m_sedOrgP[idx]<<endl;
#pragma omp parallel for
	for (int i = 0; i < m_nCells; i++)
	{
		int subi = (int) m_subbasin[i]; 
		m_surfqToCh[subi] += m_surfaceRunoff[i] * m_cellArea * 10.f / m_timestep; /// mm -> m3/s
		m_sedToCh[subi] += m_sedYield[i];
		m_surNO3ToCh[subi] += m_surqNo3[i] * m_cellArea;
		m_surNH4ToCh[subi] += m_surqNH4[i] * m_cellArea;
		m_surSolPToCh[subi] += m_surqSolP[i] * m_cellArea;
		m_surCodToCh[subi] += m_surqCOD[i] * m_cellArea;
		m_sedOrgNToCh[subi] += m_sedOrgN[i] * m_cellArea;
		m_sedOrgPToCh[subi] += m_sedOrgP[i] * m_cellArea;
		m_sedMinPAToCh[subi] += m_sedActiveMinP[i] * m_cellArea;
		m_sedMinPSToCh[subi] += m_sedStableMinP[i] * m_cellArea;
	}
#pragma omp parallel for
	for (int i = 1; i < m_subbasinNum + 1; i++)
	{
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
	// cout<<", new: "<<m_sedOrgPToCh[12]<<endl;
    return true;
}

void IMP_SWAT::potholeSimulate(int id)
{
/// initialize temporary variables
	float tileo = 0.f; /// m^3, amount of water released to the main channel from the water body by drainage tiles
	//float potevmm = 0.f; /// mm, volume of water evaporated from pothole expressed as depth
	float potev = 0.f; /// m^3, evaporation from impounded water body
	float spillo = 0.f; /// m^3, amount of water released to the main channel from impounded water body due to spill-over
	
	/// potpcpmm and potpcp should be implicitly included in (m_depStorage + m_depEvapor) if stated
	//float potpcpmm = 0.f; /// mm, precipitation falling on pothole water body expressed as depth
	//float potpcp = 0.f; /// m^3, precipitation falling on water body
	
	//float potsepmm = 0.f; // mm, seepage from impounded water body expressed as depth
	float potsep = 0.f; /// m^3, seepage from impounded water body
	//float sumo = 0.f; /// m^3, sum of all releases from water body on current day
	//float potflwo = 0.f; /// mm, discharge from pothole expressed as depth
	float potsedo = 0.f; /// kg, sediment leaving pothole on day
	float potsano = 0.f; /// kg, sand content in sediment leaving pothole on day
	float potsilo = 0.f; /// kg, silt content
	float potclao = 0.f; /// kg, clay content
	float potsago = 0.f; /// kg, small aggregate
	float potlago = 0.f; /// kg, large aggregate
	float potno3o = 0.f; /// kg, no3 amount out of pothole
	float potnh4o = 0.f; /// kg, nh4 amount out of pothole
	float potsolpo = 0.f; /// kg, soluble phosphorus out of pothole
	float potorgno = 0.f;/// kg, orgN out
	float potorgpo = 0.f; /// kg, orgP out
	float potmpso = 0.f; /// kg, stable mineral phosphorus out
	float potmpao = 0.f; /// kg, active mineral phosphorus out
	//float potvol_ini = 0.f; /// m^3, pothole volume at the begin of the day
	//float potsa_ini = 0.f; /// ha, surface area of impounded water body at the begin of the day
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
	/* pot_fr is the fraction of the cell draining into the pothole
	 * the remainder (1-pot_fr) goes directly to runoff
	 * currently, we assumed that the entire cell is pothole/impounded area
	 */
	float pot_fr = 1.f;
	float qIn = m_surfaceRunoff[id] * pot_fr; /// inflow = surface flow, not include lateral flow, groundwater, etc.
	float qdayTmp = m_surfaceRunoff[id] * (1 - pot_fr); /// qdayTmp is the actual surface runoff generated
	if (m_depStorage != NULL && m_depStorage[id] > 0.f){
		qIn += m_depStorage[id]; /// depression storage should be added
		m_depStorage[id] = 0.f;
	}
	if (m_depEvapor != NULL && m_depEvapor[id] > 0.f){
		qIn += m_depEvapor[id]; /// since the evaporation will be calculated below, the m_depEvapor should be added
		m_depEvapor[id] = 0.f;
	}
	/// update volume of water in pothole
	m_potVol[id] += qIn;
	//m_potFlowIn[id] += qIn; // TODO, this should be routing cell by cell. by lj

	/* compute surface area of pothole
	 * SWAT assuming a cone shape, ha
	 * i.e., potholeSurfaceArea(id);
	 * However, currently, we assume it is cell area
	 */
	m_potSurfaceArea[id] = m_cellArea;
	//potvol_ini = m_potVol[id];
	//potsa_ini = m_potSurfaceArea[id];

	/// update sediment in pothole
	m_potSed[id] += m_sedYield[id] * pot_fr;
	// float m_potSedIn = m_potSed[id];
	m_potSand[id] += m_sandYield[id] * pot_fr;
	float m_potSandIn = m_potSand[id];
	m_potSilt[id] += m_siltYield[id] * pot_fr;
	float m_potSiltIn = m_potSilt[id];
	m_potClay[id] += m_clayYield[id] * pot_fr;
	float m_potClayIn = m_potClay[id];
	m_potSag[id] += m_smaggreYield[id] * pot_fr;
	float m_potSagIn = m_potSag[id];
	m_potLag[id] += m_lgaggreYield[id] * pot_fr;
	float m_potLagIn = m_potLag[id];

	/// update sediment yields
	float yy = 1.f - pot_fr;
	m_sedYield[id] *= yy;
	m_sandYield[id] *= yy;
	m_siltYield[id] *= yy;
	m_clayYield[id] *= yy;
	m_smaggreYield[id] *= yy;
	m_lgaggreYield[id] *= yy;
	// if(id == 46364) cout<<"pre surq no3: "<<m_surqNo3[id];
	// if(id == 46364) cout<<"pre orgp: "<<m_sedOrgP[id];
	/// update forms of N and P in pothole
	float xx = pot_fr * m_cellArea;
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
	if (m_potVol[id] > m_potVolMax[id])
	{
		qdayTmp += (m_potVol[id] - m_potVolMax[id]);
		spillo = m_potVol[id] - m_potVolMax[id];
		m_potVol[id] = m_potVolMax[id];
		if (spillo + m_potVolMax[id] < UTIL_ZERO) // this should not happen
			xx = 0.f;
		else
			xx = spillo / (spillo + m_potVolMax[id]);
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
	if (m_potVol[id] > UTIL_ZERO)
	{
		/// compute settling  -clay and silt based on fall velocity (v = 411*d2) d=mm, v=m/hr
		float pot_depth = m_potVol[id];
		float drcla = 0.f, drsil = 0.f, drtot = 0.f;
		if (pot_depth > 10.f) /// assume clay v(fall) = 10 mm/d
			drcla = 1.f - 0.5f * 10.f / pot_depth;
		else
			drcla = 0.5f * pot_depth / 10.f;
		m_potClay[id] *= drcla;

		if (pot_depth > 1000.f) /// assume silt v(fall) = 1000 mm/d
			drsil = 1.f - 0.5f * 1000.f / pot_depth;
		else
			drsil = 0.5f * pot_depth / 1000.f;
		m_potSilt[id] *= drsil;

		/// assume complete settling of all other size (dr = 0)
		m_potSand[id] = 0.f;
		m_potSag[id] = 0.f;
		m_potLag[id] = 0.f;

		/// compute total delivery ratio for pot_sed
		float allSedPart = m_potClayIn + m_potSiltIn + m_potSandIn + m_potSagIn + m_potLagIn;

		if (allSedPart < UTIL_ZERO)
			drtot = 0.f;
		else
			drtot = (m_potClay[id] + m_potSilt[id] + m_potSand[id] + m_potSag[id] + m_potLag[id]) / allSedPart;
		m_potSed[id] *= drtot;

		/// compute organic settling assuming an enrichment ratio of 3 on clay (0.75)
		/// delivery of organics is 0.75 * dr(clay)- assuming dr on all non-clay = 1
		m_potOrgN[id] *= 0.75f * drcla;
		m_potOrgP[id] *= 0.75f * drcla;
		m_potActMinP[id] *= 0.75f * drcla;
		m_potStaMinP[id] *= 0.75f * drcla;
		///m_potNo3[id] *= (1.f - m_potNo3Decay);
		m_potSolP[id] *= (1.f - m_potSolPDecay);
		/*
		 * first-order kinetics is adopted from Chowdary et al., 2004
		 * to account for volatilization, nitrification, and denitrification in impounded water
		 */
		float nh3V = m_potNH4[id] * (1.f - exp(-m_kVolat * m_timestep / 86400.f));
		float no3N = m_potNH4[id] * (1.f - exp(-m_kNitri * m_timestep / 86400.f));
		/// update
		m_potNH4[id] -= (nh3V + no3N);
		m_potNo3[id] += no3N;

		m_potNH4[id] = max(m_potNH4[id], UTIL_ZERO);
		m_potNo3[id] = max(m_potNo3[id], UTIL_ZERO);

		/// compute flow from surface inlet tile
		tileo = min(m_potTilemm, m_potVol[id]);
		float potvol_tile = m_potVol[id];
		m_potVol[id] -= tileo;
		qdayTmp += tileo;

		/// limit seepage into soil if profile is near field capacity
		/* m_pot_k: hydraulic conductivity of soil surface of pothole
		 * [defaults to conductivity of upper soil (0.01--10.) layer]
		 * set as input parameters from database
		 */
		if (m_pot_k > 0.f)
			yy = m_pot_k;
		else
			yy = m_ks[id][0];
		/// calculate seepage into soil
		potsep = yy * m_potSurfaceArea[id] * 240.f / m_cnv; /// mm/hr*ha/240=m3/cnv=mm
		potsep = min(potsep, m_potVol[id]);
		float potvol_sep = m_potVol[id];
		m_potVol[id] -= potsep;
		m_potSeep[id] += potsep;
		m_soilStorage[id][0] += potsep; /// this will be handled in the next time step, added by LJ

		///// force the soil water storage to field capacity
		//for (int ly = 0; ly < (int)m_soilLayers[id]; ly++)
		//{
		//	float dep2cap = m_sol_sat[id][ly] - m_soilStorage[id][ly];
		//	if (dep2cap > 0.f)
		//	{
		//		dep2cap = min(dep2cap, m_potVol[id]);
		//		m_soilStorage[id][ly] += dep2cap;
		//		m_potVol[id] -= dep2cap;
		//	}
		//}
		//if (m_potVol[id] < m_potVolMin[id])
		//	m_potVol[id] = m_potVolMin[id]; /// force to reach the lowest depth. 
		///// recompute total soil water storage 
		//m_soilStorageProfile[id] = 0.f;
		//for (int ly = 0; ly < (int)m_soilLayers[id]; ly++)
		//	m_soilStorageProfile[id] += m_soilStorage[id][ly];

		/// compute evaporation from water surface
		if (m_LAIDay[id] < m_evLAI)
		{
			potev = (1.f - m_LAIDay[id] / m_evLAI) * m_pet[id];
			// if (id == 46364) cout<<"pet: "<<m_pet[id]<<", laiday: "<<m_LAIDay[id]<<", potEvap: "<<potev<<", ";
			potev = min(potev, m_potVol[id]);
			m_potVol[id] -= potev;
			m_potEvap[id] += potev;
		}
		if (potvol_tile > UTIL_ZERO)
		{
			sedloss = m_potSed[id] * tileo / potvol_tile;
			sedloss = min(sedloss, m_potSed[id]);
			m_potSed[id] -= sedloss;
			potsedo += sedloss;
			m_sedYield[id] += sedloss;

			no3loss = m_potNo3[id] * tileo / potvol_tile;
			no3loss = min(no3loss, m_potNo3[id]);
			m_potNo3[id] -= no3loss;
			m_surqNo3[id] += no3loss / m_cellArea;
			// if (id == 46364) cout<<", += tile loss: "<<m_surqNo3[id];
			nh4loss = m_potNH4[id] * tileo / potvol_tile;
			nh4loss = min(nh4loss, m_potNH4[id]);
			m_potNH4[id] -= nh4loss;
			m_surqNH4[id] += nh4loss / m_cellArea;

			solploss = m_potSolP[id] * tileo / potvol_tile;
			solploss = min(solploss, m_potSolP[id]);
			m_potSolP[id] -= solploss;
			m_surqSolP[id] += solploss / m_cellArea;

			orgnloss = m_potOrgN[id] * tileo / potvol_tile;
			orgnloss = min(orgnloss, m_potOrgN[id]);
			m_potOrgN[id] -= orgnloss;
			m_sedOrgN[id] += orgnloss / m_cellArea;

			orgploss = m_potOrgP[id] * tileo / potvol_tile;
			orgploss = min(orgploss, m_potOrgP[id]);
			m_potOrgP[id] -= orgploss;
			m_sedOrgP[id] += orgploss / m_cellArea;

			minpsloss = m_potStaMinP[id] * tileo / potvol_tile;
			minpsloss = min(minpsloss, m_potStaMinP[id]);
			m_potStaMinP[id] -= minpsloss;
			m_sedStableMinP[id] += minpsloss / m_cellArea;

			minpaloss = m_potActMinP[id] * tileo / potvol_tile;
			minpaloss = min(minpaloss, m_potActMinP[id]);
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
		if (potvol_sep > UTIL_ZERO)
		{
			float lossRatio = potsep / potvol_sep;
			sedloss = m_potSed[id] * lossRatio;
			sedloss = min(sedloss, m_potSed[id]);
			m_potSed[id] -= sedloss;

			no3loss = m_potNo3[id] * lossRatio;
			no3loss = min(no3loss, m_potNo3[id]);
			m_potNo3[id] -= no3loss;
			// if (id == 46364) cout<<", loss ratio: "<<lossRatio<<", no3 loss from seepage: "<<no3loss<<endl;
			nh4loss = m_potNH4[id] * lossRatio;
			nh4loss = min(nh4loss, m_potNH4[id]);
			m_potNH4[id] -= nh4loss;

			solploss = m_potSolP[id] * lossRatio;
			solploss = min(solploss, m_potSolP[id]);
			m_potSolP[id] -= solploss;

			orgnloss = m_potOrgN[id] * lossRatio;
			orgnloss = min(orgnloss, m_potOrgN[id]);
			m_potOrgN[id] -= orgnloss;

			orgploss = m_potOrgP[id] * lossRatio;
			orgploss = min(orgploss, m_potOrgP[id]);
			m_potOrgP[id] -= orgploss;

			minpsloss = m_potStaMinP[id] * lossRatio;
			minpsloss = min(minpsloss, m_potStaMinP[id]);
			m_potStaMinP[id] -= minpsloss;

			minpaloss = m_potActMinP[id] * lossRatio;
			minpaloss = min(minpaloss, m_potActMinP[id]);
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
	if (m_potVol[id] < UTIL_ZERO)
	{
		m_potVol[id] = m_potVolMin[id]; 
	}
	//potholeSurfaceArea(id);
	m_surfaceRunoff[id] = qdayTmp;
	//if (id == 46364)  /// dianbu 46364, dianbu2 1085
	//	cout<<"surfaceQ: "<<m_surfaceRunoff[id]<<", potVol: "<<m_potVol[id]<<
	//	", potNh4: "<<m_potNH4[id]<<", surqNh4: "<<m_surqNH4[id]<<endl;
}

void IMP_SWAT::potholeSurfaceArea(int id)
{
	/// compute surface area assuming a cone shape, ha
	float potVol_m3 = m_potVol[id] * m_cnv;
	m_potSurfaceArea[id] = PI * pow((3.f * potVol_m3 / (PI * m_slope[id])), 0.6666f);
	m_potSurfaceArea[id] /= 10000.f; /// convert to ha
	if (m_potSurfaceArea[id] <= UTIL_ZERO)
		m_potSurfaceArea[id] = 0.001f;
	if (m_potSurfaceArea[id] > m_cellArea)
		m_potSurfaceArea[id] = m_cellArea;
}

void IMP_SWAT::releaseWater(int id)
{
	float proption = 1.f;
	float xx = proption * m_cellArea;
	if (m_potVol[id] < UTIL_ZERO)
		return;
	m_surfaceRunoff[id] += m_potVol[id] * proption;
	m_potVol[id] *= (1.f - proption);
	if (m_potSed[id] < UTIL_ZERO)
	{
		m_potSed[id] = 0.f;
		m_sandYield[id] = 0.f;
		m_siltYield[id] = 0.f;
		m_clayYield[id] = 0.f;
		m_smaggreYield[id] = 0.f;
		m_lgaggreYield[id] = 0.f;
	}
	else
	{
		m_sedYield[id] += m_potSed[id] * proption;
		m_sandYield[id] += m_potSand[id] * proption;
		m_siltYield[id] += m_potSilt[id] * proption;
		m_clayYield[id] += m_potClay[id] * proption;
		m_smaggreYield[id] += m_potSag[id] * proption;
		m_lgaggreYield[id] += m_potLag[id] * proption;
		m_potSed[id] *= (1.f - proption);
		m_potSand[id] *= (1.f - proption);
		m_potSilt[id] *= (1.f - proption);
		m_potClay[id] *= (1.f - proption);
		m_potSag[id] *= (1.f - proption);
		m_potLag[id] *= (1.f - proption);
	}
	if (m_potNo3[id] < UTIL_ZERO)
		m_potNo3[id] = 0.f;
	else
	{
		m_surqNo3[id] += m_potNo3[id] * xx;
		// if (id == 46364) cout<<", release: "<<m_surqNo3[id]<<endl;
		m_potNo3[id] *= (1.f - proption);
	}
	if (m_potNH4[id] < UTIL_ZERO)
		m_potNH4[id] = 0.f;
	else
	{
		m_surqNH4[id] += m_potNH4[id] * xx;
		m_potNH4[id] *= (1.f - proption);
	}
	if (m_potSolP[id] < UTIL_ZERO)
		m_potSolP[id] = 0.f;
	else
	{
		m_surqSolP[id] += m_potSolP[id] * xx;
		m_potSolP[id] *= (1.f - proption);
	}
	if (m_potOrgN[id] < UTIL_ZERO)
		m_potOrgN[id] = 0.f;
	else
	{
		m_sedOrgN[id] += m_potOrgN[id] * xx;
		m_potOrgN[id] *= (1.f - proption);
	}
	if (m_potOrgP[id] < UTIL_ZERO)
		m_potOrgP[id] = 0.f;
	else
	{
		m_sedOrgP[id] += m_potOrgP[id] * xx;
		m_potOrgP[id] *= (1.f - proption);
	}
	if (m_potActMinP[id] < UTIL_ZERO)
		m_potActMinP[id] = 0.f;
	else
	{
		m_sedActiveMinP[id] += m_potActMinP[id] * xx;
		m_potActMinP[id] *= (1.f - proption);
	}
	if (m_potStaMinP[id] < UTIL_ZERO)
		m_potStaMinP[id] = 0.f;
	else
	{
		m_sedStableMinP[id] += m_potStaMinP[id] * xx;
		m_potStaMinP[id] *= (1.f - proption);
	}  
	/// Debugging: dianbu 46364, dianbu2 1085
	// if (id == 46364) cout<<"releaseWater, "<<m_surfaceRunoff[id]<<", "<<m_potVol[id]<<", surqNh4: "<<m_surqNH4[id]<<endl;
}

void IMP_SWAT::Get1DData(const char *key, int *n, float **data)
{
	initialOutputs();
	string sk(key);
	if (StringMatch(sk, VAR_POT_VOL))*data = m_potVol;
	else if (StringMatch(sk, VAR_POT_SA)) *data = m_potSurfaceArea;
	else if (StringMatch(sk, VAR_POT_NO3)) *data = m_potNo3;
	else if (StringMatch(sk, VAR_POT_NH4)) *data = m_potNH4;
	else if (StringMatch(sk, VAR_POT_SOLP)) *data = m_potSolP;
	else
		throw ModelException(MID_IMP_SWAT, "Get1DData","Parameter" + sk + "does not exist.");
	*n = m_nCells;
	return;
}
