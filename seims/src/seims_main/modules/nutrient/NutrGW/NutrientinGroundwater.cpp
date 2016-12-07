#include "NutrientinGroundwater.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <omp.h>
#include "util.h"

using namespace std;

NutrientinGroundwater::NutrientinGroundwater(void) :
//input
        m_TimeStep(-1), m_nCells(-1), m_cellWidth(-1), m_gwno3Con(NULL), m_gwSolCon(NULL), m_gw_q(NULL),
		m_nSubbasins(-1), m_subbasin(NULL), m_subbasinsInfo(NULL), m_gwStor(NULL), 
		m_perco_no3_gw(NULL), m_perco_solp_gw(NULL), m_soilLayers(NULL), m_nSoilLayers(-1), m_sol_no3(NULL),
		m_gwNO3(NULL), m_gwSolP(NULL), m_gw0(NODATA_VALUE),
        //output
        m_solpGwToCh(NULL), m_no3GwToCh(NULL)
{

}

NutrientinGroundwater::~NutrientinGroundwater(void)
{
	if (m_no3GwToCh != NULL) Release1DArray(m_no3GwToCh);
    if (m_solpGwToCh != NULL) Release1DArray(m_solpGwToCh);
	if (m_gwNO3 != NULL) Release1DArray(m_gwNO3);
	if (m_gwSolP != NULL) Release1DArray(m_gwSolP);
}

void NutrientinGroundwater::SetSubbasins(clsSubbasins *subbasins)
{
	if(m_subbasinsInfo == NULL)
	{
		m_subbasinsInfo = subbasins;
		m_nSubbasins = m_subbasinsInfo->GetSubbasinNumber();
		m_subbasinIDs = m_subbasinsInfo->GetSubbasinIDs();
	}
}

bool NutrientinGroundwater::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        throw ModelException(MID_NUTRGW, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_nCells != n)
    {
        if (m_nCells <= 0)
        {
            m_nCells = n;
        } 
		else
        {
            //StatusMsg("Input data for "+string(key) +" is invalid. All the input data should have same size.");
            ostringstream oss;
            oss << "Input data for " + string(key) << " is invalid with size: " << n << ". The origin size is " <<
            m_nCells << ".\n";
            throw ModelException(MID_NUTRGW, "CheckInputSize", oss.str());
        }
    }
    return true;
}

bool NutrientinGroundwater::CheckInputData()
{
    if (m_nCells <= 0)
        throw ModelException(MID_NUTRGW, "CheckInputData", "The cells number can not be less than zero.");
	if (m_TimeStep <= 0)
		throw ModelException(MID_NUTRGW, "CheckInputData", "The parameter: m_TimeStep has not been set.");
    if (m_cellWidth < 0)
        throw ModelException(MID_NUTRGW, "CheckInputData", "The cell width can not be less than zero.");
    if (m_gw0 < 0)
		throw ModelException(MID_NUTRGW, "CheckInputData", "The initial groundwater storage can not be less than zero.");
	if (m_gw_q == NULL)
        throw ModelException(MID_NUTRGW, "CheckInputData", "The groundwater contribution to stream flow data can not be NULL.");
	if (m_gwStor == NULL)
		throw ModelException(MID_NUTRGW, "CheckInputData", "The groundwater storage can not be NULL.");
	if (m_perco_no3_gw == NULL)
		throw ModelException(MID_NUTRGW, "CheckInputData", "The NO3 percolation to groundwater can not be NULL.");
	if (m_perco_solp_gw == NULL)
		throw ModelException(MID_NUTRGW, "CheckInputData", "The solute P percolation to groundwater can not be NULL.");
	if (m_soilLayers == NULL)
		throw ModelException(MID_NUTRGW, "CheckInputData", "The soil layers number can not be NULL.");
	if (m_sol_no3 == NULL)
		throw ModelException(MID_NUTRGW, "CheckInputData", "m_sol_no3 can not be NULL.");
    return true;
}

void NutrientinGroundwater::SetValue(const char *key, float value)
{
    string sk(key);
	if (StringMatch(sk, Tag_TimeStep))
		m_TimeStep = int(value);
    else if (StringMatch(sk, VAR_OMP_THREADNUM))
    {
        omp_set_num_threads((int) value);
    }
    else if (StringMatch(sk, Tag_CellWidth))
    {
        m_cellWidth = value;
    }
	else if (StringMatch(sk, VAR_GW0))
		m_gw0 = value;
    else
    {
        throw ModelException(MID_NUTRGW, "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void NutrientinGroundwater::Set1DData(const char *key, int n, float *data)
{
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSN))
	{
		if (!CheckInputSize(key, n)) return;
		m_subbasin = data;
	}
    else if (StringMatch(sk, VAR_SBQG))
        m_gw_q = data;
	else if (StringMatch(sk, VAR_SBGS))
		m_gwStor = data;
	else if (StringMatch(sk, VAR_PERCO_N_GW))
		m_perco_no3_gw = data;
	else if (StringMatch(sk, VAR_PERCO_P_GW))
		m_perco_solp_gw = data;
	else if (StringMatch(sk, VAR_SOILLAYERS))
	{
		if (!CheckInputSize(key, n)) return;
		m_soilLayers = data;
	}
    else
        throw ModelException(MID_NUTRGW, "Set1DData", "Parameter " + sk + " does not exist.");
}

void NutrientinGroundwater::Set2DData(const char *key, int nRows, int nCols, float **data)
{
	if (!this->CheckInputSize(key, nRows)) return;
	string sk(key);

	m_nSoilLayers = nCols;
	if (StringMatch(sk, VAR_SOL_NO3)) 
		m_sol_no3 = data;
	else if (StringMatch(sk, VAR_SOL_SOLP))
		m_sol_solp = data;
	else
		throw ModelException(MID_NUTRGW, "Set2DData", "Parameter " + sk + " does not exist.");
}

void NutrientinGroundwater::SetReaches(clsReaches *reaches)
{
	if(reaches != NULL)
	{
		m_nSubbasins = reaches->GetReachNumber();
		vector<int> m_reachId = reaches->GetReachIDs();
		if (m_gwSolCon == NULL)
		{
			Initialize1DArray(m_nSubbasins+1, m_gwSolCon, 0.f);
			Initialize1DArray(m_nSubbasins+1, m_gwno3Con, 0.f);
		}
		
		for (vector<int>::iterator it = m_reachId.begin(); it != m_reachId.end(); it++)
		{
			clsReach* tmpReach = reaches->GetReachByID(*it);
			m_gwno3Con[*it] = tmpReach->GetGWNO3();
			m_gwSolCon[*it] = tmpReach->GetGWSolP();
		}
	}
	else
		throw ModelException(MID_NUTRGW, "SetReaches", "The reaches input can not to be NULL.");
}
void NutrientinGroundwater::initialOutputs()
{
    if (this->m_nCells <= 0)
        throw ModelException(MID_NUTRGW, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    // allocate the output variables
	if (m_no3GwToCh == NULL)
	{
		Initialize1DArray(m_nSubbasins+1, m_no3GwToCh, 0.f);
		Initialize1DArray(m_nSubbasins+1, m_solpGwToCh, 0.f);
	}
	if (m_gwNO3 == NULL)
	{	/// initial nutrient amount stored in groundwater
		m_gwNO3 = new float[m_nSubbasins+1];
		m_gwSolP = new float[m_nSubbasins+1];
		for (int i = 1; i <= m_nSubbasins; i++)
		{
			Subbasin *subbasin = m_subbasinsInfo->GetSubbasinByID(i);
			float subArea = subbasin->getCellCount() * m_cellWidth * m_cellWidth;	//m2
			m_gwNO3[i] = m_gw0 * m_gwno3Con[i] * subArea / 1000000.f; /// mm * mg/L * m2 = 10^-6 kg 
			m_gwSolP[i] = m_gw0 * m_gwSolCon[i] * subArea / 1000000.f;
		}
	}
}


int NutrientinGroundwater::Execute()
{
    CheckInputData();
	initialOutputs();
	//int cellid = 18605;
	//cout<<"NutrGW, pre solno3: ";
	//for (int j = 0; j < (int)m_soilLayers[cellid]; j++)
	//	cout<<j<<", "<<m_sol_no3[cellid][j]<<", ";
	//cout<<endl;
	for(vector<int>::iterator iter=m_subbasinIDs.begin(); iter != m_subbasinIDs.end(); iter++)
    {
		int id = *iter;
		Subbasin *subbasin = m_subbasinsInfo->GetSubbasinByID(id);
		int nCells = subbasin->getCellCount();
		float subArea = nCells * m_cellWidth * m_cellWidth;	// m^2
		float revap = subbasin->getEG();
		/// 1. firstly, restore the groundwater storage during current day
		///    since the m_gwStor has involved percolation water, just need add revap and runoff water
		float gwqVol = m_gw_q[id] * m_TimeStep;	// m^3, water volume flow out
		float reVapVol = revap * subArea / 1000.f; // m^3
		float tmpGwStorage = m_gwStor[id] * subArea / 1000.f + gwqVol + reVapVol;
		/// 2. secondly, update nutrient concentration
		m_gwNO3[id] += m_perco_no3_gw[id]; /// nutrient amount, kg
		m_gwSolP[id] += m_perco_solp_gw[id];
		m_gwno3Con[id] = m_gwNO3[id] / tmpGwStorage * 1000.f; // kg / m^3 * 1000. = mg/L
		m_gwSolCon[id] = m_gwSolP[id] / tmpGwStorage * 1000.f;
		/// 3. thirdly, calculate nutrient in groundwater runoff
		//cout<<"subID: "<<id<<", gwQ: "<<m_gw_q[id] << ", ";
		m_no3GwToCh[id] = m_gwno3Con[id] * gwqVol / 1000.f;	// g/m3 * m3 / 1000 = kg
		m_solpGwToCh[id] = m_gwSolCon[id] * gwqVol / 1000.f;
		//cout<<"subID: "<<id<<", gwno3Con: "<<m_gwno3Con[id] << ", no3gwToCh: "<<m_no3gwToCh[id] << ", ";
		/// 4. fourthly, calculate nutrient loss loss through revep and update no3 in the bottom soil layer
		float no3ToSoil = revap / 1000.f * m_gwno3Con[id] * 10.f;	// kg/ha  (m*10*g/m3=kg/ha)
		float solpToSoil = revap / 1000.f * m_gwSolCon[id] * 10.f;
		float no3ToSoil_kg = no3ToSoil * subArea / 10000.f; /// kg/ha * m^2 / 10000.f = kg
		float solpToSoil_kg = solpToSoil * subArea / 10000.f;
		int *cells = subbasin->getCells();
		int index = 0;
		for (int i = 0; i < nCells; i++)
		{
			index = cells[i];
			//if (index == cellid)
			//	cout<<"revap: "<<revap<<", gwNo3: "<<m_gwNO3[id]<<", gwStorg: "<<
			//	tmpGwStorage<<", percoNo3Gw: "<<m_perco_no3_gw[id]<<
			//	", gwNo3Con: "<<m_gwno3Con[id]<<", no3ToSoil: "<<no3ToSoil<<endl;
			m_sol_no3[index][(int)m_soilLayers[index] - 1] += no3ToSoil;
			m_sol_solp[index][(int)m_soilLayers[index] - 1] += solpToSoil;
		}
		/// finally, update nutrient amount
		m_gwNO3[id] -= (m_no3GwToCh[id] + no3ToSoil_kg);
		m_gwSolP[id] -= (m_solpGwToCh[id] + solpToSoil_kg);

		//float gwVol = subArea * m_gwStor[id] / 1000.f; //m3, memo, this m_gwStor is the resulted status of the current time step
		//m_gwno3Con[id] += (m_perco_no3_gw[id] - m_no3GwToCh[id]) * 1000.f / gwVol;
		//m_gwSolCon[id] += (m_perco_solp_gw[id] - m_solpGwToCh[id]) * 1000.f / gwVol;
		//cout<<"subID: "<<id<<", percoNo3: "<<m_perco_no3_gw[id]<<", gwStorage: "<<m_gwStor[id]<<", new gwno3Con: "<<m_gwno3Con[id] << ", ";
    }
	//cout<<"NutrGW, after solno3: ";
	//for (int j = 0; j < (int)m_soilLayers[cellid]; j++)
	//	cout<<j<<", "<<m_sol_no3[cellid][j]<<", ";
	//cout<<endl;
    return 0;
}

void NutrientinGroundwater::Get1DData(const char *key, int *n, float **data)
{
	initialOutputs();
    string sk(key);
    *n = m_nSubbasins + 1;
    if (StringMatch(sk, VAR_NO3GW_TOCH))
        *data = m_no3GwToCh;
    else if (StringMatch(sk, VAR_MINPGW_TOCH))
		*data = m_solpGwToCh;
	else if (StringMatch(sk, VAR_GWNO3_CONC))
		*data = m_gwno3Con;
	else if (StringMatch(sk, VAR_GWSOLP_CONC))
		*data = m_gwSolCon;
	else if (StringMatch(sk, VAR_GWNO3))
		*data = m_gwNO3;
	else if (StringMatch(sk, VAR_GWSOLP))
		*data = m_gwSolP;
    else
        throw ModelException(MID_NUTRGW, "Get1DData", "Parameter " + sk + " does not exist.");
}
