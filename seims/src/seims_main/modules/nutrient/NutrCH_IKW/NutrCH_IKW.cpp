/*//
 * \file NutrientCHRoute.cpp
 * \ingroup NutCHRout
 * \author Huiran Gao
 * \date Jun 2016
 */

#include <iostream>
#include "NutrCH_IKW.h"
#include "MetadataInfo.h"
#include <cmath>
#include <fstream>
#include "ModelException.h"
#include "util.h"
#include <omp.h>

using namespace std;

NutrientCH_IKW::NutrientCH_IKW(void):
//input
        m_CellWith(-1), m_nCells(-1), m_chNumber(-1),  m_dt(-1), m_flowInIndex(NULL), m_flowOutIndex(NULL), m_sourceCellIds(NULL),
		m_streamOrder(NULL),  m_streamLink(NULL), m_aBank(-1), m_qUpReach(-1), m_rnum1(-1), igropt(-1),
        m_ai0(-1), m_ai1(-1), m_ai2(-1), m_ai3(-1), m_ai4(-1), m_ai5(-1), m_ai6(-1), m_lambda0(-1), m_lambda1(-1), m_lambda2(-1),
        m_k_l(-1), m_k_n(-1), m_k_p(-1), m_p_n(-1), tfact(-1), m_mumax(-1), m_rhoq(-1),
        m_daylen(NULL), m_sra(NULL), m_bankStorage(NULL), m_qsSub(NULL), m_qiSub(NULL), m_qgSub(NULL),
        m_qsCh(NULL), m_qiCh(NULL), m_qgCh(NULL), m_chStorage(NULL), m_chWTdepth(NULL), m_wattemp(NULL),
        m_latno3ToCh(NULL), m_surqno3ToCh(NULL), m_surqsolpToCh(NULL), m_no3gwToCh(NULL),
        m_minpgwToCh(NULL), m_sedorgnToCh(NULL), m_sedorgpToCh(NULL), m_sedminpaToCh(NULL),
        m_sedminpsToCh(NULL), m_ammoToCh(NULL), m_nitriteToCh(NULL),
        //output
        m_algae(NULL), m_organicn(NULL), m_organicp(NULL), m_ammonian(NULL), m_nitriten(NULL), m_nitraten(NULL),
        m_disolvp(NULL), m_rch_cod(NULL), m_rch_dox(NULL), m_chlora(NULL), m_soxy(NULL)
{

}

NutrientCH_IKW::~NutrientCH_IKW(void)
{
    if (m_algae != NULL)
    {
		delete[] m_algae;
	}
    if (m_organicn != NULL)
    {
        delete[] m_organicn;
    }
    if (m_organicp != NULL)
    {
        delete[] m_organicp;
    }
    if (m_ammonian != NULL)
    {
        delete[] m_ammonian;
    }
    if (m_nitriten != NULL)
    {
        delete[] m_nitriten;
    }
    if (m_nitraten != NULL)
    {
        delete[] m_nitraten;
    }
    if (m_disolvp != NULL)
    {
        delete[] m_disolvp;
    }
    if (m_rch_cod != NULL)
    {
        delete[] m_rch_cod;
    }
    if (m_rch_dox != NULL)
    {
        delete[] m_rch_dox;
    }
    if (m_chlora != NULL)
    {
        delete[] m_chlora;
    }
}

bool NutrientCH_IKW::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_nReaches != n - 1)
    {
        if (m_nReaches <= 0)
        {
            m_nReaches = n - 1;
        } else
        {
            //StatusMsg("Input data for "+string(key) +" is invalid. All the input data should have same size.");
            ostringstream oss;
            oss << "Input data for " + string(key) << " is invalid with size: " << n << ". The origin size is " <<
            m_nReaches << ".\n";
            throw ModelException(MID_NutCHRout, "CheckInputSize", oss.str());
        }
    }
    return true;
}

bool NutrientCH_IKW::CheckInputData()
{

	if (m_CellWith <= 0)
	{
		throw ModelException(MID_NutCHRout, "CheckInputData", "The cell width can not be less than zero.");
	}
	if (m_nCells <= 0)
	{
		throw ModelException(MID_NutCHRout, "CheckInputData", "The cell number can not be less than zero.");
	}
	if (m_chNumber <= 0)
	{
		throw ModelException(MID_NutCHRout, "CheckInputData", "The channel reach number can not be less than zero.");
	}
	if (m_flowOutIndex == NULL)
	{
		throw ModelException(MID_NutCHRout, "CheckInputData", "The flow out index can not be NULL.");
	}
	if (m_streamOrder == NULL)
	{
		throw ModelException(MID_NutCHRout, "CheckInputData", "The stream order of reach parameter can not be NULL.");
	}
	if (m_streamLink == NULL)
	{
		throw ModelException(MID_NutCHRout, "CheckInputData", "The stream link can not be NULL.");
	}
    if (this->m_dt < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The parameter: m_dt has not been set.");
    }
    if (this->m_nReaches < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The parameter: m_nReaches has not been set.");
    }
    if (this->m_aBank < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The bank flow recession constant can not be less than zero.");
    }
    if (this->m_qUpReach < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The input data qUpReach can not be less than zero.");
    }
    if (this->m_rnum1 < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The fraction of overland flow can not be less than zero.");
    }
    if (this->igropt < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The option for calculating the local specific growth rate of algae can not be less than zero.");
    }
    if (this->m_ai0 < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The ratio of chlorophyll-a to algal biomass can not be less than zero.");
    }
    if (this->m_ai1 < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The fraction of algal biomass that is nitrogen can not be less than zero.");
    }
    if (this->m_ai2 < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The fraction of algal biomass that is phosphorus can not be less than zero.");
    }
    if (this->m_ai3 < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The the rate of oxygen production per unit of algal photosynthesis can not be less than zero.");
    }
    if (this->m_ai4 < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The the rate of oxygen uptake per unit of algae respiration can not be less than zero.");
    }
    if (this->m_ai5 < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The the rate of oxygen uptake per unit of NH3 nitrogen oxidation can not be less than zero.");
    }
    if (this->m_ai6 < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The the rate of oxygen uptake per unit of NO2 nitrogen oxidation can not be less than zero.");
    }
    if (this->m_lambda0 < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The non-algal portion of the light extinction coefficient can not be less than zero.");
    }
    if (this->m_lambda1 < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The linear algal self-shading coefficient can not be less than zero.");
    }
    if (this->m_lambda2 < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The nonlinear algal self-shading coefficient can not be less than zero.");
    }
    if (this->m_k_l < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The half saturation coefficient for light can not be less than zero.");
    }
    if (this->m_k_n < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The half saturation constant for nitrogen can not be less than zero.");
    }
    if (this->m_k_p < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The half saturation constant for phosphorus can not be less than zero.");
    }
    if (this->m_p_n < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The algal preference factor for ammonia can not be less than zero.");
    }
    if (this->tfact < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The fraction of solar radiation computed can not be less than zero.");
    }
    if (this->m_mumax < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The maximum specific algal growth rate at 20 deg C can not be less than zero.");
    }
    if (this->m_rhoq < 0)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The algal respiration rate at 20 deg C can not be less than zero.");
    }
    if (this->m_daylen == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The day length for current day can not be NULL.");
    }
    if (this->m_sra == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The solar radiation for the day data can not be NULL.");
    }
    if (this->m_bankStorage == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The bank storage can not be NULL.");
    }
    if (this->m_qsSub == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The overland flow to streams from each subbasin can not be NULL.");
    }
    if (this->m_qiSub == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The inter-flow to streams from each subbasin can not be NULL.");
    }
    if (this->m_qgSub == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The groundwater flow out of the subbasin can not be NULL.");
    }
    if (this->m_qsCh == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The input qsCh can not be NULL.");
    }
    if (this->m_qiCh == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The input qiCh can not be NULL.");
    }
    if (this->m_qgCh == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The input qgCh can not be NULL.");
    }
    if (this->m_chStorage == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The  reach storage data can not be NULL.");
    }
    if (this->m_chWTdepth == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The channel water depth data can not be NULL.");
    }
    if (this->m_wattemp == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The temperature of water in reach data can not be NULL.");
    }
    if (this->m_latno3ToCh == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The amount of nitrate transported with lateral flow can not be NULL.");
    }
    if (this->m_surqno3ToCh == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The amount of nitrate transported with surface runoff can not be NULL.");
    }
    if (this->m_surqsolpToCh == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The amount of soluble phosphorus in surface runoff can not be NULL.");
    }
    if (this->m_no3gwToCh == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The nitrate loading to reach in groundwater can not be NULL.");
    }
    if (this->m_minpgwToCh == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The soluble P loading to reach in groundwater can not be NULL.");
    }
    if (this->m_sedorgnToCh == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The amount of organic nitrogen in surface runoff can not be NULL.");
    }
    if (this->m_sedorgpToCh == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The amount of organic phosphorus in surface runoff can not be NULL.");
    }
    if (this->m_sedminpaToCh == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The amount of active mineral phosphorus absorbed to sediment in surface runoff can not be NULL.");
    }
    if (this->m_sedminpsToCh == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The amount of stable mineral phosphorus absorbed to sediment in surface runoff can not be NULL.");
    }
    if (this->m_ammoToCh == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The amount of ammonium transported with lateral flow can not be NULL.");
    }
    if (this->m_nitriteToCh == NULL)
    {
        throw ModelException(MID_NutCHRout, "CheckInputData", "The amount of nitrite transported with lateral flow can not be NULL.");
    }
    return true;
}

void NutrientCH_IKW::SetValue(const char *key, float value)
{
    string sk(key);
    if (StringMatch(sk, VAR_OMP_THREADNUM))
    {
        omp_set_num_threads((int) value);
	}
	else if (StringMatch(sk, Tag_CellWidth))  { this->m_CellWith = value;}
	else if (StringMatch(sk, Tag_CellSize))  { this->m_nCells = (int) value;}
    else if (StringMatch(sk, Tag_ChannelTimeStep)) { this->m_dt = (int) value; }
    else if (StringMatch(sk, VAR_A_BNK)) { this->m_aBank = value; }
    else if (StringMatch(sk, VAR_QUPREACH)) { this->m_qUpReach = value; }
    else if (StringMatch(sk, VAR_RNUM1)) { this->m_rnum1 = value; }
    else if (StringMatch(sk, VAR_IGROPT)) { this->igropt = (int)value; }
    else if (StringMatch(sk, VAR_AI0)) { this->m_ai0 = value; }
    else if (StringMatch(sk, VAR_AI1)) { this->m_ai1 = value; }
    else if (StringMatch(sk, VAR_AI2)) { this->m_ai2 = value; }
    else if (StringMatch(sk, VAR_AI3)) { this->m_ai3 = value; }
    else if (StringMatch(sk, VAR_AI4)) { this->m_ai4 = value; }
    else if (StringMatch(sk, VAR_AI5)) { this->m_ai5 = value; }
    else if (StringMatch(sk, VAR_AI6)) { this->m_ai6 = value; }
    else if (StringMatch(sk, VAR_LAMBDA0)) { this->m_lambda0 = value; }
    else if (StringMatch(sk, VAR_LAMBDA1)) { this->m_lambda1 = value; }
    else if (StringMatch(sk, VAR_LAMBDA2)) { this->m_lambda2 = value; }
    else if (StringMatch(sk, VAR_K_L)) { this->m_k_l = value; }
    else if (StringMatch(sk, VAR_K_N)) { this->m_k_n = value; }
    else if (StringMatch(sk, VAR_K_P)) { this->m_k_p = value; }
    else if (StringMatch(sk, VAR_P_N)) { this->m_p_n = value; }
    else if (StringMatch(sk, VAR_TFACT)) { this->tfact = value; }
    else if (StringMatch(sk, VAR_MUMAX)) { this->m_mumax = value; }
    else if (StringMatch(sk, VAR_RHOQ)) { this->m_rhoq = value; }
        //else if(StringMatch(sk, VAR_VSF)) {m_vScalingFactor = value;}
        //else if (StringMatch(sk, VAR_MSK_X)) {m_x = value;}
        //else if (StringMatch(sk, VAR_MSK_CO1)) {m_co1 = value;}
    else
    {
        throw ModelException(MID_NutCHRout, "SetValue", "Parameter " + sk +
                                                      " does not exist in CLIMATE method. Please contact the module developer.");
    }
}

void NutrientCH_IKW::Set1DData(const char *key, int n, float *data)
{
    if (!this->CheckInputSize(key, n))
    {
        return;
    }
    string sk(key);
	if (StringMatch(sk, VAR_DAYLEN)) { this->m_daylen = data; }
	else if (StringMatch(sk, Tag_FLOWOUT_INDEX_D8)) { this->m_flowOutIndex = data;}
	else if (StringMatch(sk, VAR_STREAM_LINK)) { this->m_streamLink = data;}
    else if (StringMatch(sk, VAR_SRA)) { this->m_sra = data; }
    else if (StringMatch(sk, VAR_BKST)) { this->m_bankStorage = data; }
    else if (StringMatch(sk, VAR_SBOF)) { this->m_qsSub = data; }
    else if (StringMatch(sk, VAR_SBIF)) { this->m_qiSub = data; }
    else if (StringMatch(sk, VAR_SBQG)) { this->m_qgSub = data; }
    else if (StringMatch(sk, VAR_QS)) { this->m_qsCh = data; }
    else if (StringMatch(sk, VAR_QI)) { this->m_qiCh = data; }
    else if (StringMatch(sk, VAR_QG)) { this->m_qgCh = data; }
    else if (StringMatch(sk, VAR_CHST)) { this->m_chStorage = data; }
    else if (StringMatch(sk, VAR_CHWTDEPTH)) { this->m_chWTdepth = data; }
    else if (StringMatch(sk, VAR_WATTEMP)) { this->m_wattemp = data; }
    else if (StringMatch(sk, VAR_LATNO3_CH)) { this->m_latno3ToCh = data; }
    else if (StringMatch(sk, VAR_SUR_NO3_CH)) { this->m_surqno3ToCh = data; }
    else if (StringMatch(sk, VAR_SUR_SOLP_CH)) { this->m_surqsolpToCh = data; }
    else if (StringMatch(sk, VAR_NO3GW_CH)) { this->m_no3gwToCh = data; }
    else if (StringMatch(sk, VAR_MINPGW_CH)) { this->m_minpgwToCh = data; }
    else if (StringMatch(sk, VAR_SEDORGN_CH)) { this->m_sedorgnToCh = data; }
    else if (StringMatch(sk, VAR_SEDORGP_CH)) { this->m_sedorgpToCh = data; }
    else if (StringMatch(sk, VAR_SEDMINPA_CH)) { this->m_sedminpaToCh = data; }
    else if (StringMatch(sk, VAR_SEDMINPS_CH)) { this->m_sedminpsToCh = data; }
    else if (StringMatch(sk, VAR_AMMO_CH)) { this->m_ammoToCh = data; }
    else if (StringMatch(sk, VAR_NITRITE_CH)) { this->m_nitriteToCh = data; }
    else
    {
        throw ModelException(MID_NutCHRout, "SetValue", "Parameter " + sk +
                                                      " does not exist in Nutrient module. Please contact the module developer.");
    }
}

void NutrientCH_IKW::Set2DData(const char *key, int nrows, int ncols, float **data)
{
    string sk(key);
    if (StringMatch(sk, VAR_REACH_PARAM))
    {
		m_chNumber = ncols; // overland is nrows;
		m_reachId = data[0];
		m_streamOrder = data[1];
		m_reachDownStream = data[2];
        m_reachUpStream.resize(m_nReaches + 1);
		for (int i = 0; i < m_chNumber; i++)
			m_idToIndex[(int) m_reachId[i]] = i;

		m_reachUpStream.resize(m_chNumber);
		for (int i = 0; i < m_chNumber; i++)
		{
			int downStreamId = int(m_reachDownStream[i]);
			if (downStreamId == 0)
				continue;
			int downStreamIndex = m_idToIndex[downStreamId];
			m_reachUpStream[downStreamIndex].push_back(i);
		}
    }
    else
        throw ModelException("NutCHPout", "Set2DData",
                             "Parameter " + sk + " does not exist. Please contact the module developer.");
}

void  NutrientCH_IKW::initialOutputs()
{
    if (m_nReaches <= 0)
        throw ModelException("NutCHPout", "initialOutputs", "The cell number of the input can not be less than zero.");

    if (m_reachLayers.empty())
    {
        CheckInputData();
        for (int i = 1; i <= m_nReaches; i++)
        {
            int order = (int) m_chOrder[i];
            m_reachLayers[order].push_back(i);
        }
    }

	if (m_organicn == NULL)
	{
		// find source cells the reaches
		m_sourceCellIds = new int[m_chNumber];
		for (int i = 0; i < m_chNumber; ++i)
			m_sourceCellIds[i] = -1;
		for (int i = 0; i < m_nCells; i++)
		{
			if (FloatEqual(m_streamLink[i], NODATA_VALUE))
				continue;
			int reachId = (int) m_streamLink[i];
			bool isSource = true;
			for (int k = 1; k <= (int) m_flowInIndex[i][0]; ++k)
			{
				int flowInId = (int) m_flowInIndex[i][k];
				int flowInReachId = (int) m_streamLink[flowInId];
				if (flowInReachId == reachId)
				{
					isSource = false;
					break;
				}
			}
			if ((int) m_flowInIndex[i][0] == 0)
				isSource = true;

			if (isSource)
			{
				int reachIndex = m_idToIndex[reachId];
				m_sourceCellIds[reachIndex] = i;
			}
		}
		// get the cells in reaches according to flow direction
		for (int iCh = 0; iCh < m_chNumber; iCh++)
		{
			int iCell = m_sourceCellIds[iCh];
			int reachId = (int) m_streamLink[iCell];
			while ((int) m_streamLink[iCell] == reachId)
			{
				m_reachs[iCh].push_back(iCell);
				iCell = (int) m_flowOutIndex[iCell];
			}
		}

		if (m_reachLayers.empty())
		{
			for (int i = 0; i < m_chNumber; i++)
			{
				int order = (int) m_streamOrder[i];
				m_reachLayers[order].push_back(i);
			}
		}
		m_algae = new float *[m_chNumber];
		m_organicn = new float *[m_chNumber];
		m_organicp = new float *[m_chNumber];
		m_ammonian = new float *[m_chNumber];
		m_nitriten = new float *[m_chNumber];
		m_nitraten = new float *[m_chNumber];
		m_disolvp = new float *[m_chNumber];
		m_rch_cod = new float *[m_chNumber];
		m_rch_dox = new float *[m_chNumber];
		m_chlora = new float *[m_chNumber];
		m_soxy = 0.f;
		for (int i = 0; i < m_chNumber; ++i)
		{
			int n = m_reachs[i].size();
			m_algae[i] = new float [n];
			m_organicn[i] = new float [n];
			m_organicp[i] = new float [n];
			m_ammonian[i] = new float [n];
			m_nitriten[i] = new float [n];
			m_nitraten[i] = new float [n];
			m_disolvp[i] = new float [n];
			m_rch_cod[i] = new float [n];
			m_rch_dox[i] = new float [n];
			m_chlora[i] = new float [n];
			for (int j = 0; j < n; ++j)
			{
				m_algae[i][j]  = 0.f;
				m_organicn[i][j]  = 0.f;
				m_organicp[i][j]  = 0.f;
				m_ammonian[i][j]  = 0.f;
				m_nitriten[i][j]  = 0.f;
				m_nitraten[i][j]  = 0.f;
				m_disolvp[i][j]  = 0.f;
				m_rch_cod[i][j]  = 0.f;
				m_rch_dox[i][j]  = 0.f;
				m_chlora[i][j]  = 0.f;

			}
		}
	}
}

int NutrientCH_IKW::Execute()
{
    if (!this->CheckInputData())
    {
        return false;
    }
    this->initialOutputs();
    // Get reach information
    int nReaches = m_reaches->GetReachNumber();
    vector<int> reachIDs = m_reaches->GetReachIDs();
    // Get reach information by subbasin ID
    for (vector<int>::iterator it = reachIDs.begin(); it != reachIDs.end(); it++)
    {
        clsReach *reach = m_reaches->GetReachByID(*it);
        /// Get fields of reach
        m_bc1[*it] = reach->GetBc1();
        m_bc2[*it] = reach->GetBc2();
        m_bc3[*it] = reach->GetBc3();
        m_bc4[*it] = reach->GetBc4();
        m_rk1[*it] = reach->GetRk1();
        m_rk2[*it] = reach->GetRk2();
        m_rk3[*it] = reach->GetRk3();
        m_rk4[*it] = reach->GetRk4();
        m_rs1[*it] = reach->GetRs1();
        m_rs2[*it] = reach->GetRs2();
        m_rs3[*it] = reach->GetRs3();
        m_rs4[*it] = reach->GetRs4();
        m_rs5[*it] = reach->GetRs5();

    }

	map<int, vector<int> >::iterator it;
	for (it = m_reachLayers.begin(); it != m_reachLayers.end(); it++)
	{
		// There are not any flow relationship within each routing layer.
		int nReaches = it->second.size();
		// the size of m_reachLayers (map) is equal to the maximum stream order
		// #pragma omp parallel for
		for (int i = 0; i < nReaches; ++i)
		{
			int reachIndex = it->second[i]; // index in the array
			vector<int> & vecCells = m_reachs[reachIndex];
			int n = vecCells.size();
			for (int iCell = 0; iCell < n; ++iCell)
			{
				NutrientinChannel(reachIndex, iCell, vecCells[iCell]);
			}
			//m_SedSubbasin[reachIndex] = m_Qsn[reachIndex][n-1]/1000;   //kg/s -> ton/s
		}
	}
    //return ??
    return 0;
}

void NutrientCH_IKW::NutrientinChannel(int i, int iCell, int id)
{
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

    // initialize inflow concentrations
    float chlin = 0.f;
    float algin = 0.f;
    float orgnin = 0.f;
    float ammoin = 0.f;
    float nitritin = 0.f;
    float nitratin = 0.f;
    float orgpin = 0.f;
    float dispin = 0.f;
    float codin = 0.f;
    float disoxin = 0.f;
    float cinn = 0.f;
    float heatin = 0.f;
    float cod = 0.f;

    float dalgae = 0.f;
    float dchla = 0.f;
    float dbod = 0.f;
    float ddisox = 0.f;
    float dorgn = 0.f;
    float dnh4 = 0.f;
    float dno2 = 0.f;
    float dno3 = 0.f;
    float dorgp = 0.f;
    float dsolp = 0.f;
    // initialize water flowing into reach
    float wtrIn = 0.f;
    float qiSub = 0.f;
    if (m_qiSub != NULL)
    {
        qiSub = m_qiSub[i];
    }
    float qgSub = 0.f;
    if (m_qgSub != NULL)
    {
        qgSub = m_qgSub[i];
    }
    // 1. water from this subbasin
    wtrIn = m_qsSub[i] + qiSub + qgSub;
    // 2. water from upstream reaches
    float qsUp = 0.f;
    float qiUp = 0.f;
    float qgUp = 0.f;
    for (size_t j = 0; j < m_reachUpStream[i].size(); ++j)
    {
        int upReachId = m_reachUpStream[i][j];
        qsUp += m_qsCh[upReachId];
        qiUp += m_qiCh[upReachId];
        qgUp += m_qgCh[upReachId];
    }
    wtrIn += qsUp + qiUp + qgUp;
    // m_qUpReach is zero for not-parallel program and qsUp, qiUp and qgUp are zero for parallel computing
    wtrIn += m_qUpReach;
    // 3. water from bank storage
    float bankOut = 0.f;
    bankOut = m_bankStorage[i] * (1.f - exp(-m_aBank));
    m_bankStorage[i] -= bankOut;
    wtrIn += bankOut / m_dt;

    //wtrIn = varoute[][inum2)] * (1. - m_rnum1);
    // Calculate flow out of reach
    float wtrOut = 0.f;
    wtrOut = m_qsCh[i] + m_qiCh[i] + m_qgCh[i];

    // Concentrations
    if ((wtrOut / 86400.f) > 0.01f && wtrIn > 0.001f)
    {
        // initialize concentration of nutrient in reach
        // inflow + storage water (wtrTotal m3)
        float wtrTotal = 0.f;
        // initial algal biomass concentration in reach (algcon mg/L)
        float algcon = 0.f;
        // initial organic N concentration in reach (orgncon mg/L)
        float orgncon = 0.f;
        // initial ammonia concentration in reach (nh3con mg/L)
        float nh3con = 0.f;
        // initial nitrite concentration in reach (no2con mg/L)
        float no2con = 0.f;
        // initial nitrate concentration in reach (no3con mg/L)
        float no3con = 0.f;
        // initial organic P concentration in reach (orgpcon  mg/L)
        float orgpcon = 0.f;
        // initial soluble P concentration in reach (solpcon mg/L)
        float solpcon = 0.f;
        // initial carbonaceous biological oxygen demand (cbodcon mg/L)
        float cbodcon = 0.f;
        // initial dissolved oxygen concentration in reach (o2con mg/L)
        float o2con = 0.f;
        wtrTotal = wtrIn + m_chStorage[i];

        algcon = m_algae[i][iCell];
        orgncon = m_organicn[i][iCell];
        nh3con = m_ammonian[i][iCell];
        no2con = m_nitriten[i][iCell];
        no3con = m_nitraten[i][iCell];
        orgpcon = m_organicp[i][iCell];
        solpcon = m_disolvp[i][iCell];
        cbodcon = m_rch_cod[i][iCell];
        o2con = m_rch_dox[i][iCell];
        // temperature of water in reach (wtmp deg C)
        float wtmp = 0.f;
        wtmp = m_wattemp[i];
        // calculate temperature in stream
        if (wtmp <= 0.f)
        {
            wtmp = 0.1f;
        }

        // calculate effective concentration of available nitrogen (cinn)
        float cinn = 0.f;
        cinn = nh3con + no3con;

        // calculate saturation concentration for dissolved oxygen
        float ww = 0.f;    // variable to hold intermediate calculation result
        float xx = 0.f;     // variable to hold intermediate calculation result
        float yy = 0.f;     // variable to hold intermediate calculation result
        float zz = 0.f;     // variable to hold intermediate calculation result
        ww = -139.34410f + (1.575701e05f / (wtmp + 273.15f));
        xx = 6.642308e+07f / pow((wtmp + 273.15f), 2);
        yy = 1.243800e+10f / pow((wtmp + 273.15f), 3);
        zz = 8.621949e+11f / pow((wtmp + 273.15f), 4);
        m_soxy = exp(ww - xx + yy - zz);
        if (m_soxy < 1.e-6f)
        {
            m_soxy = 0.f;
        }

        // O2 impact calculations
        // calculate nitrification rate correction factor for low (cordo)
        float cordo = 0.f;
        float o2con2 = 0.f;
        o2con2 = o2con;
        if (o2con2 <= 0.1f)
        {
            o2con2 = 0.1f;
        }
        if (o2con2 > 30.f)
        {
            o2con2 = 30.f;
        }
        cordo = 1.f - exp(-0.6f * o2con2);
        if (o2con <= 0.001f)
        {
            o2con = 0.001f;
        }
        if (o2con > 30.f)
        {
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
        if (m_ai0 * algcon > 1.e-6f)
        {
            lambda = m_lambda0 + (m_lambda1 * m_ai0 * algcon) + m_lambda2 * pow((m_ai0 * algcon), 0.6666667f);
        }
        else
        {
            lambda = m_lambda0;
        }

        // calculate algal growth limitation factors for nitrogen and phosphorus
        float fnn = 0.f;
        float fpp = 0.f;
        fnn = cinn / (cinn + m_k_n);
        fpp = solpcon / (solpcon + m_k_p);

        // calculate daylight average, photo synthetically active (algi)
        float algi = 0.f;
        dchla = 0.f;
        dbod = 0.f;
        ddisox = 0.f;
        dorgn = 0.f;
        dnh4 = 0.f;
        dno2 = 0.f;
        dno3 = 0.f;
        dorgp = 0.f;
        dsolp = 0.f;
        if (m_daylen[i] > 0.f)
        {
            algi = m_sra[i] * tfact / m_daylen[i];
        } else
        {
            algi = 0.00001f;
        }

        // calculate growth attenuation factor for light, based on daylight average light intensity
        float fl_1 = 0.f;
        float fll = 0.f;
        fl_1 = (1.f / (lambda * m_chWTdepth[i])) *
               log((m_k_l * 24.f + algi) / (m_k_l * 24.f + algi * exp(-lambda * m_chWTdepth[i])));
        fll = 0.92f * (m_daylen[i] / 24.f) * fl_1;

        // calculate local algal growth rate
        float gra = 0.f;
        float dchla = 0.f;
        float dbod = 0.f;
        float ddisox = 0.f;
        float dorgn = 0.f;
        float dnh4 = 0.f;
        float dno2 = 0.f;
        float dno3 = 0.f;
        float dorgp = 0.f;
        float dsolp = 0.f;
        switch (igropt)
        {
            case 1:
                // multiplicative
                gra = m_mumax * fll * fnn * fpp;
            case 2:
                // limiting nutrient
                gra = m_mumax * fll * min(fnn, fpp);
            case 3:
                // harmonic mean
                if (fnn > 1.e-6f && fpp > 1.e-6f)
                {
                    gra = m_mumax * fll * 2.f / ((1.f / fnn) + (1.f / fpp));
                } else
                {
                    gra = 0.f;
                }
        }

        // calculate algal biomass concentration at end of day (phytoplanktonic algae)
        dalgae = 0.f;
        float setl = min(1.f, corTempc(m_rs1[i], thrs1, wtmp) / m_chWTdepth[i]);
        dalgae = algcon +
                 (corTempc(gra, thgra, wtmp) * algcon - corTempc(m_rhoq, thrho, wtmp) * algcon - setl * algcon) * tday;
        if (dalgae < 0.00001f)
        {
            m_algae[i][iCell] = 0.00001f;
        }
        // calculate chlorophyll-a concentration at end of day
        dchla = 0.f;
        dchla = dalgae * m_ai0 / 1000.f;

        // oxygen calculations
        // calculate carbonaceous biological oxygen demand at end of day (dbod)
        float yyy = 0.f;     // variable to hold intermediate calculation result
        float zzz = 0.f;     // variable to hold intermediate calculation result
        yyy = corTempc(m_rk4[i], thm_rk1, wtmp) * cbodcon;
        zzz = corTempc(m_rk3[i], thm_rk3, wtmp) * cbodcon;
        dbod = 0.f;
        dbod = cbodcon - (yyy + zzz) * tday;
        if (dbod < 0.00001f)
        {
            dbod = 0.00001f;
        }

        // calculate dissolved oxygen concentration if reach at end of day (ddisox)
        float uu = 0.f;     // variable to hold intermediate calculation result
        float vv = 0.f;      // variable to hold intermediate calculation result
        ww = 0.f;    // variable to hold intermediate calculation result
        xx = 0.f;     // variable to hold intermediate calculation result
        yy = 0.f;     // variable to hold intermediate calculation result
        zz = 0.f;     // variable to hold intermediate calculation result
        float hh = corTempc(m_rk2[i], thm_rk2, wtmp);
        uu = corTempc(m_rk2[i], thm_rk2, wtmp) * (m_soxy - o2con);
        if (algcon > 0.001f)
        {
            vv = (m_ai3 * corTempc(gra, thgra, wtmp) - m_ai4 * corTempc(m_rhoq, thrho, wtmp)) * algcon;
        }
        else
        {
            algcon = 0.001f;
        }
        ww = corTempc(m_rk1[i], thm_rk1, wtmp) * cbodcon;
        if (m_chWTdepth[i] > 0.001f)
        {
            xx = corTempc(m_rk4[i], thm_rk4, wtmp) / (m_chWTdepth[i] * 1000.f);
        }
        if (nh3con > 0.001f)
        {
            yy = m_ai5 * corTempc(bc1mod, thbc1, wtmp) * nh3con;
        }
        else
        {
            nh3con = 0.001f;
        }
        if (no2con > 0.001f)
        {
            zz = m_ai6 * corTempc(bc2mod, thbc2, wtmp) * no2con;
        }
        else
        {
            no2con = 0.001f;
        }
        ddisox = 0.f;
        ddisox = o2con + (uu + vv - ww - xx - yy - zz) * tday;
        //o2proc = o2con - ddisox;   // not found variable "o2proc"
        if (ddisox < 0.00001f)
        {
            ddisox = 0.00001f;
        }

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
        if (dorgn < 0.00001f)
        {
            dorgn = 0.00001f;
        }

        // calculate fraction of algal nitrogen uptake from ammonia pool
        float f1 = 0.f;
        f1 = m_p_n * nh3con / (m_p_n * nh3con + (1.f - m_p_n) * no3con + 1.e-6f);

        // calculate ammonia nitrogen concentration at end of day (dnh4)
        ww = 0.f;
        xx = 0.f;
        yy = 0.f;
        zz = 0.f;
        ww = corTempc(m_bc3[i], thbc3, wtmp) * orgncon;
        xx = corTempc(bc1mod, thbc1, wtmp) * nh3con;
        yy = corTempc(m_rs3[i], thrs3, wtmp) / (m_chWTdepth[i] * 1000.f);
        zz = f1 * m_ai1 * algcon * corTempc(gra, thgra, wtmp);
        dnh4 = 0.f;
        dnh4 = nh3con + (ww - xx + yy - zz) * tday;
        if (dnh4 < 1.e-6f)
        {
            dnh4 = 0.f;
        }

        // calculate concentration of nitrite at end of day (dno2)
        yy = 0.f;
        zz = 0.f;
        yy = corTempc(bc1mod, thbc1, wtmp) * nh3con;
        zz = corTempc(bc2mod, thbc2, wtmp) * no2con;
        dno2 = 0.f;
        dno2 = no2con + (yy - zz) * tday;
        if (dno2 < 1.e-6f)
        {
            dno2 = 0.f;
        }

        // calculate nitrate concentration at end of day (dno3)
        yy = 0.f;
        zz = 0.f;
        yy = corTempc(bc2mod, thbc2, wtmp) * no2con;
        zz = (1.f - f1) * m_ai1 * algcon * corTempc(gra, thgra, wtmp);
        dno3 = 0.f;
        dno3 = no3con + (yy - zz) * tday;
        if (dno3 < 1.e-6)
        {
            dno3 = 0.f;
        }

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
        if (dorgp < 1.e-6f)
        {
            dorgp = 0.f;
        }

        // calculate dissolved phosphorus concentration at end of day (dsolp)
        xx = 0.f;
        yy = 0.f;
        zz = 0.f;
        xx = corTempc(m_bc4[i], thbc4, wtmp) * orgpcon;
        yy = corTempc(m_rs2[i], thrs2, wtmp) / (m_chWTdepth[i] * 1000.f);
        zz = m_ai2 * corTempc(gra, thgra, wtmp) * algcon;
        dsolp = 0.f;
        dsolp = solpcon + (xx + yy - zz) * tday;
        if (dsolp < 1.e-6)
        {
            dsolp = 0.f;
        }

        wtrTotal = wtrIn + m_chStorage[i];

        if (wtrIn > 0.001f)
        {
            //chlin = 1000. * m_chloraToCh[i] * (1. - m_rnum1) / wtrIn;	//chlorophyll-a
            algin = 1000.f * chlin / m_ai0;
            orgnin = 1000.f * m_sedorgnToCh[i] * (1.f - m_rnum1) / wtrIn;
            ammoin = 1000.f * m_ammoToCh[i] * (1.f - m_rnum1) / wtrIn;
            nitritin = 1000.f * m_nitriteToCh[i] * (1.f - m_rnum1) / wtrIn;
            nitratin = 1000.f * (m_latno3ToCh[i] + m_surqno3ToCh[i] + m_no3gwToCh[i]) * (1.f - m_rnum1) / wtrIn;
            orgpin = 1000.f * (m_sedorgpToCh[i] + m_sedminpsToCh[i] + m_sedminpaToCh[i]) * (1.f - m_rnum1) / wtrIn;
            dispin = 1000.f * (m_surqsolpToCh[i] + m_minpgwToCh[i]) * (1.f - m_rnum1) / wtrIn;
            codin = 1000.f * m_codToCh[i] * (1.f - m_rnum1) / wtrIn;
            //disoxin= 1000. * m_disoxToCh[i] * (1. - m_rnum1) / wtrIn;
            heatin = m_wattemp[i] * (1.f - m_rnum1);
        }

        m_wattemp[i] = (heatin * wtrIn + wtmp * m_chStorage[i]) / wtrTotal;
        m_algae[i][iCell] = (algin * wtrIn + dalgae * m_chStorage[i]) / wtrTotal;
        m_organicn[i][iCell] = (orgnin * wtrIn + dorgn * m_chStorage[i]) / wtrTotal;
        m_ammonian[i][iCell] = (ammoin * wtrIn + dnh4 * m_chStorage[i]) / wtrTotal;
        m_nitriten[i][iCell] = (nitritin * wtrIn + dno2 * m_chStorage[i]) / wtrTotal;
        m_nitraten[i][iCell] = (nitratin * wtrIn + dno3 * m_chStorage[i]) / wtrTotal;
        m_organicp[i][iCell] = (orgpin * wtrIn + dorgp * m_chStorage[i]) / wtrTotal;
        m_disolvp[i][iCell] = (dispin * wtrIn + dsolp * m_chStorage[i]) / wtrTotal;
        m_rch_cod[i][iCell] = (codin * wtrIn + dbod * m_chStorage[i]) / wtrTotal;
        //m_rch_dox[i] = (disoxin * wtrIn +  ddisox * m_chStorage[i]) / wtrTotal;

        // calculate chlorophyll-a concentration at end of day
        m_chlora[i][iCell] = 0.f;
        m_chlora[i][iCell] = m_algae[i][iCell] * m_ai0 / 1000.f;
    } else
    {
        // all water quality variables set to zero when no flow
        algin = 0.f;
        chlin = 0.f;
        orgnin = 0.f;
        ammoin = 0.f;
        nitritin = 0.f;
        nitratin = 0.f;
        orgpin = 0.f;
        dispin = 0.f;
        codin = 0.f;
        disoxin = 0.f;
        m_algae[i][iCell] = 0.f;
        m_chlora[i][iCell] = 0.f;
        m_organicn[i][iCell] = 0.f;
        m_ammonian[i][iCell] = 0.f;
        m_nitriten[i][iCell] = 0.f;
        m_nitraten[i][iCell] = 0.f;
        m_organicp[i][iCell] = 0.f;
        m_disolvp[i][iCell] = 0.f;
        m_rch_cod[i][iCell] = 0.f;
        m_rch_dox[i][iCell] = 0.f;
        dalgae = 0.f;
        dchla = 0.f;
        dorgn = 0.f;
        dnh4 = 0.f;
        dno2 = 0.f;
        dno3 = 0.f;
        dorgp = 0.f;
        dsolp = 0.f;
        dbod = 0.f;
        ddisox = 0.f;
        m_soxy = 0.f;
    }
}

float NutrientCH_IKW::corTempc(float r20, float thk, float tmp)
{
    float theta = 0.f;
    theta = r20 * pow(thk, (tmp - 20.f));
    return theta;
}

void NutrientCH_IKW::GetValue(const char *key, float *value)
{
	string sk(key);
	map<int, vector<int> >::iterator it = m_reachLayers.end();
	it--;
	int reachId = it->second[0];
	int iLastCell = m_reachs[reachId].size() - 1;
    if (StringMatch(sk, VAR_SOXY))
    {
        *value = m_soxy;
    }
	else if (StringMatch(sk, VAR_AL_OUTLET))
    {
        *value = m_algae[reachId][iLastCell];	//mg/L
	}
	else if (StringMatch(sk, VAR_ON_OUTLET))
	{
		*value = m_organicn[reachId][iLastCell];	//mg/L
	}
	else if (StringMatch(sk, VAR_AN_OUTLET))
	{
		*value = m_ammonian[reachId][iLastCell];	//mg/L
	}
	else if (StringMatch(sk, VAR_NIN_OUTLET))
	{
		*value = m_nitriten[reachId][iLastCell];	//mg/L
	}
	else if (StringMatch(sk, VAR_NAN_OUTLET))
	{
		*value = m_nitraten[reachId][iLastCell];	//mg/L
	}
	else if (StringMatch(sk, VAR_OP_OUTLET))
	{
		*value = m_organicp[reachId][iLastCell];	//mg/L
	}
	else if (StringMatch(sk, VAR_DP_OUTLET))
	{
		*value = m_disolvp[reachId][iLastCell];	//mg/L
	}
	else if (StringMatch(sk, VAR_COD_OUTLET))
	{
		*value = m_rch_cod[reachId][iLastCell];	//mg/L
	}
	else if (StringMatch(sk, VAR_CHL_OUTLET))
	{
		*value = m_chlora[reachId][iLastCell];	//mg/L
	}
	else
		throw ModelException(MID_NutCHRout, "GetValue", "Output " + sk
		+
		" does not exist in the current module. Please contact the module developer.");
}

void NutrientCH_IKW::Get2DData(const char *key, int *nRows, int *nCols, float ***data)
{
	string sk(key);
    *nRows = m_nReaches;
	if (StringMatch(sk, VAR_ALGAE)) { *data = this->m_algae; }
	else if (StringMatch(sk, VAR_ORGANICN)) { *data = this->m_organicn; }
	else if (StringMatch(sk, VAR_ORGANICP)) { *data = this->m_organicp; }
	else if (StringMatch(sk, VAR_AMMONIAN)) { *data = this->m_ammonian; }
	else if (StringMatch(sk, VAR_NITRITEN)) { *data = this->m_nitriten; }
	else if (StringMatch(sk, VAR_NITRATEN)) { *data = this->m_nitraten; }
	else if (StringMatch(sk, VAR_DISOLVP)) { *data = this->m_disolvp; }
	else if (StringMatch(sk, VAR_RCH_COD)) { *data = this->m_rch_cod; }
	//else if (StringMatch(sk, VAR_RCH_DOX)) {*data = this -> m_rch_dox;}
	else if (StringMatch(sk, VAR_CHLORA)) { *data = this->m_chlora; }
	else
	{
		throw ModelException(MID_NutCHRout, "GetValue",
			"Parameter " + sk + " does not exist. Please contact the module developer.");
	}
}
