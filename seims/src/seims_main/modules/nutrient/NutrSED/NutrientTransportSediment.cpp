#include <iostream>
#include "NutrientTransportSediment.h"
#include "MetadataInfo.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include "ModelException.h"
#include "util.h"
#include <omp.h>

using namespace std;

NutrientTransportSediment::NutrientTransportSediment(void) :
		//input
        m_nCells(-1), m_cellWidth(-1.f), m_cellArea(-1.f), m_soiLayers(-1), m_nSoilLayers(NULL), 
		m_nSubbasins(-1), m_subbasin(NULL), m_subbasinsInfo(NULL),
        m_sol_bd(NULL), m_soilThick(NULL), m_enratio(NULL),
        m_sol_actp(NULL), m_sol_stap(NULL), m_sol_fop(NULL), m_sol_orgp(NULL), 
        m_sol_orgn(NULL), m_sol_aorgn(NULL), m_sol_fon(NULL),
		m_sedEroded(NULL), m_surfaceRunoff(NULL),
		/// for CENTURY C/N cycling model inputs
		m_CbnModel(0),m_sol_LSN(NULL), m_sol_LMN(NULL), m_sol_HPN(NULL), m_sol_HSN(NULL), 
		m_sol_HPC(NULL), m_sol_HSC(NULL), m_sol_LMC(NULL), m_sol_LSC(NULL), m_sol_LS(NULL), 
		m_sol_LM(NULL), m_sol_LSL(NULL), m_sol_LSLC(NULL), m_sol_LSLNC(NULL), m_sol_BMC(NULL), 
		m_sol_WOC(NULL), m_sol_perco(NULL), m_sol_laterq(NULL),
		/// for C-FARM one carbon model input
		m_sol_mp(NULL),
		/// for CENTURY C/N cycling model outputs
		m_sol_latC(NULL), m_sol_percoC(NULL), m_laterC(NULL), m_percoC(NULL), m_sedCLoss(NULL),
        //outputs
        m_sedorgn(NULL), m_sedorgp(NULL), m_sedminpa(NULL), m_sedminps(NULL),
		m_sedorgnToCh(NULL), m_sedorgpToCh(NULL), m_sedminpaToCh(NULL), m_sedminpsToCh(NULL)
{
}

NutrientTransportSediment::~NutrientTransportSediment(void)
{
	if (m_enratio != NULL) Release1DArray(m_enratio);

	if (m_sedorgp != NULL) Release1DArray(m_sedorgp);
	if (m_sedorgn != NULL) Release1DArray(m_sedorgn);
	if (m_sedminpa != NULL) Release1DArray(m_sedminpa);
	if (m_sedminps != NULL) Release1DArray(m_sedminps);

	if (m_sedorgnToCh != NULL) Release1DArray(m_sedorgnToCh);
	if (m_sedorgpToCh != NULL) Release1DArray(m_sedorgpToCh);
	if (m_sedminpaToCh != NULL) Release1DArray(m_sedminpaToCh);
	if (m_sedminpsToCh != NULL) Release1DArray(m_sedminpsToCh);

	/// for CENTURY C/N cycling model outputs
	if (m_sol_latC != NULL) Release2DArray(m_nCells, m_sol_percoC);
	if (m_sol_percoC != NULL) Release2DArray(m_nCells, m_sol_percoC);
	if (m_laterC != NULL) Release1DArray(m_laterC);
	if (m_percoC != NULL) Release1DArray(m_percoC);
	if (m_sedCLoss != NULL) Release1DArray(m_sedCLoss);
}

bool NutrientTransportSediment::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
		throw ModelException(MID_NUTRSED,"CheckInputSize","Input data for "+string(key) +" is invalid. The size could not be less than zero.\n");
    if (m_nCells != n)
    {
        if (m_nCells <= 0)
            m_nCells = n;
        else
			throw ModelException(MID_NUTRSED, "CheckInputSize", "Input data for " + string(key) + " is invalid with size: " +ValueToString(n)+ 
								". The origin size is " + ValueToString(m_nCells)+".\n");
    }
    return true;
}

bool NutrientTransportSediment::CheckInputData()
{
    if (this->m_nCells <= 0)
    {
        throw ModelException(MID_NUTRSED, "CheckInputData", "The input data can not be less than zero.");
    }
    if (this->m_cellWidth < 0.f)
    {
        throw ModelException(MID_NUTRSED, "CheckInputData", "The cell width can not be less than zero.");
    }
    if (this->m_soiLayers <= 0)
    {
        throw ModelException(MID_NUTRSED, "CheckInputData", "The layer number of the input 2D raster data can not be less than zero.");
    }
    if (this->m_nSoilLayers == NULL)
    {
        throw ModelException(MID_NUTRSED, "CheckInputData", "Soil layers number must not be NULL.");
    }
    if (this->m_sedEroded == NULL)
    {
        throw ModelException(MID_NUTRSED, "CheckInputData", "The distribution of soil loss caused by water erosion can not be NULL.");
    }
    if (this->m_surfaceRunoff == NULL)
    {
        throw ModelException(MID_NUTRSED, "CheckInputData", "The distribution of surface runoff generated data can not be NULL.");
    }
    if (this->m_sol_bd == NULL)
    {
        throw ModelException(MID_NUTRSED, "CheckInputData", "The bulk density of the soil data can not be NULL.");
    }
    if (this->m_soilThick == NULL)
    {
        throw ModelException(MID_NUTRSED, "CheckInputData", "The thickness of soil layer can not be NULL.");
    }
    if (this->m_sol_actp == NULL)
    {
        throw ModelException(MID_NUTRSED, "CheckInputData", "The amount of phosphorus stored in the active mineral phosphorus pool can not be NULL.");
    }
    if (this->m_sol_orgn == NULL)
    {
		throw ModelException(MID_NUTRSED, "CheckInputData", "The amount of nitrogen stored in the stable organic N pool can not be NULL.");
    }
    if (this->m_sol_orgp == NULL)
    {
        throw ModelException(MID_NUTRSED, "CheckInputData", "The amount of phosphorus stored in the organic P pool can not be NULL.");
    }
    if (this->m_sol_stap == NULL)
    {
        throw ModelException(MID_NUTRSED, "CheckInputData", "The amount of phosphorus in the soil layer stored in the stable mineral phosphorus pool can not be NULL.");
    }
    if (this->m_sol_aorgn == NULL)
    {
        throw ModelException(MID_NUTRSED, "CheckInputData", "The amount of nitrogen stored in the active organic nitrogen pool data can not be NULL.");
    }
    if (this->m_sol_fon == NULL)
    {
        throw ModelException(MID_NUTRSED, "CheckInputData", "The amount of nitrogen stored in the fresh organic pool can not be NULL.");
    }
    if (this->m_sol_fop == NULL)
    {
        throw ModelException(MID_NUTRSED, "CheckInputData", "The amount of phosphorus stored in the fresh organic pool can not be NULL.");
    }

	if (m_subbasin == NULL)
		throw ModelException(MID_NUTRSED, "CheckInputData", "The parameter: m_subbasin has not been set.");

	if (m_nSubbasins <= 0) throw ModelException(MID_NUTRSED, "CheckInputData", "The subbasins number must be greater than 0.");
	if (m_subbasinIDs.empty()) throw ModelException(MID_NUTRSED, "CheckInputData", "The subbasin IDs can not be EMPTY.");
	if (m_subbasinsInfo == NULL)
		throw ModelException(MID_NUTRSED, "CheckInputData", "The parameter: m_subbasinsInfo has not been set.");
    return true;
}

bool NutrientTransportSediment::CheckInputData_CENTURY()
{
	if (this->m_sol_LSN == NULL) throw ModelException(MID_NUTRSED, "CheckInputData", "The m_sol_LSN can not be NULL.");
	if (this->m_sol_LMN == NULL) throw ModelException(MID_NUTRSED, "CheckInputData", "The m_sol_LMN can not be NULL.");
	if (this->m_sol_HPN == NULL) throw ModelException(MID_NUTRSED, "CheckInputData", "The m_sol_HPN can not be NULL.");
	if (this->m_sol_HSN == NULL) throw ModelException(MID_NUTRSED, "CheckInputData", "The m_sol_HSN can not be NULL.");
	if (this->m_sol_HPC == NULL) throw ModelException(MID_NUTRSED, "CheckInputData", "The m_sol_HPC can not be NULL.");
	if (this->m_sol_HSC == NULL) throw ModelException(MID_NUTRSED, "CheckInputData", "The m_sol_HSC can not be NULL.");
	if (this->m_sol_LMC == NULL) throw ModelException(MID_NUTRSED, "CheckInputData", "The m_sol_LMC can not be NULL.");
	if (this->m_sol_LSC == NULL) throw ModelException(MID_NUTRSED, "CheckInputData", "The m_sol_LSC can not be NULL.");
	if (this->m_sol_LS == NULL) throw ModelException(MID_NUTRSED, "CheckInputData", "The m_sol_LS can not be NULL.");
	if (this->m_sol_LM == NULL) throw ModelException(MID_NUTRSED, "CheckInputData", "The m_sol_LM can not be NULL.");
	if (this->m_sol_LSL == NULL) throw ModelException(MID_NUTRSED, "CheckInputData", "The m_sol_LSL can not be NULL.");
	if (this->m_sol_LSLC == NULL) throw ModelException(MID_NUTRSED, "CheckInputData", "The m_sol_LSLC can not be NULL.");
	if (this->m_sol_LSLNC == NULL) throw ModelException(MID_NUTRSED, "CheckInputData", "The m_sol_LSLNC can not be NULL.");
	if (this->m_sol_BMC == NULL) throw ModelException(MID_NUTRSED, "CheckInputData", "The m_sol_BMC can not be NULL.");
	if (this->m_sol_WOC == NULL) throw ModelException(MID_NUTRSED, "CheckInputData", "The m_sol_WOC can not be NULL.");
	if (this->m_sol_perco == NULL) throw ModelException(MID_NUTRSED, "CheckInputData", "The m_sol_perco can not be NULL.");
	if (this->m_sol_laterq == NULL) throw ModelException(MID_NUTRSED, "CheckInputData", "The m_sol_laterq can not be NULL.");
	return true;
}

bool NutrientTransportSediment::CheckInputData_CFARM()
{
	if (this->m_sol_mp == NULL) throw ModelException(MID_NUTRSED, "CheckInputData", "The m_sol_mp can not be NULL.");
	return true;
}

void NutrientTransportSediment::SetValue(const char *key, float value)
{
    string sk(key);
    if (StringMatch(sk, VAR_OMP_THREADNUM)) omp_set_num_threads((int) value);
    else if (StringMatch(sk, Tag_CellWidth))m_cellWidth = value;
	else if (StringMatch(sk, VAR_CSWAT)) m_CbnModel = (int)value;
    else
        throw ModelException(MID_NUTRSED, "SetValue", "Parameter " + sk +" does not exist.");
}

void NutrientTransportSediment::Set1DData(const char *key, int n, float *data)
{
    if (!this->CheckInputSize(key, n)) return;

    string sk(key);
	if (StringMatch(sk, VAR_SUBBSN))
		m_subbasin = data;
	else if (StringMatch(sk, VAR_SOILLAYERS))
		m_nSoilLayers = data;
    else if (StringMatch(sk, VAR_SEDYLD))
        this->m_sedEroded = data;
    else if (StringMatch(sk, VAR_OLFLOW))
        this->m_surfaceRunoff = data;
    else
        throw ModelException(MID_NUTRSED, "Set1DData", "Parameter " + sk + " does not exist.");
}

void NutrientTransportSediment::Set2DData(const char *key, int nRows, int nCols, float **data)
{
    if (!this->CheckInputSize(key, nRows)) return;
    string sk(key);
    m_soiLayers = nCols;
    if (StringMatch(sk, VAR_SOILTHICK)) { this->m_soilThick = data; }
    else if (StringMatch(sk, VAR_SOL_BD)) { this->m_sol_bd = data; }
    else if (StringMatch(sk, VAR_SOL_AORGN)) { this->m_sol_aorgn = data; }
    else if (StringMatch(sk, VAR_SOL_SORGN)) { this->m_sol_orgn = data; }
    else if (StringMatch(sk, VAR_SOL_HORGP)) { this->m_sol_orgp = data; }
    else if (StringMatch(sk, VAR_SOL_FORGP)) { this->m_sol_fop = data; }
    else if (StringMatch(sk, VAR_SOL_FORGN)) { this->m_sol_fon = data; }
    else if (StringMatch(sk, VAR_SOL_ACTP)) { this->m_sol_actp = data; }
	else if (StringMatch(sk, VAR_SOL_STAP)) { this->m_sol_stap = data; }
	/// for CENTURY C/Y cycling model, optional inputs
	else if (StringMatch(sk, VAR_ROCK)) m_sol_rock = data;
	else if (StringMatch(sk, VAR_SOL_UL)) m_sol_wsatur = data;
	else if (StringMatch(sk, VAR_SOL_LSN)) m_sol_LSN = data;
	else if (StringMatch(sk, VAR_SOL_LMN)) m_sol_LMN = data;
	else if (StringMatch(sk, VAR_SOL_HPN)) m_sol_HPN = data;
	else if (StringMatch(sk, VAR_SOL_HSN)) m_sol_HSN = data;
	else if (StringMatch(sk, VAR_SOL_HPC)) m_sol_HPC = data;
	else if (StringMatch(sk, VAR_SOL_HSC)) m_sol_HSC = data;
	else if (StringMatch(sk, VAR_SOL_LMC)) m_sol_LMC = data;
	else if (StringMatch(sk, VAR_SOL_LSC)) m_sol_LSC = data;
	else if (StringMatch(sk, VAR_SOL_LS)) m_sol_LS = data;
	else if (StringMatch(sk, VAR_SOL_LM)) m_sol_LM = data;
	else if (StringMatch(sk, VAR_SOL_LSL)) m_sol_LSL = data;
	else if (StringMatch(sk, VAR_SOL_LSLC)) m_sol_LSLC = data;
	else if (StringMatch(sk, VAR_SOL_LSLNC)) m_sol_LSLNC = data;
	else if (StringMatch(sk, VAR_SOL_BMC)) m_sol_BMC = data;
	else if (StringMatch(sk, VAR_SOL_WOC)) m_sol_WOC = data;
	else if (StringMatch(sk, VAR_PERCO)) m_sol_perco = data;
	else if (StringMatch(sk, VAR_SSRU)) m_sol_laterq = data;
	/// for C-FARM one carbon model
	else if (StringMatch(sk, VAR_SOL_MP)) m_sol_mp = data;
    else
        throw ModelException(MID_NUTRSED, "Set2DData", "Parameter " + sk + " does not exist.");
}

void NutrientTransportSediment::initialOutputs()
{
    if (this->m_nCells <= 0)
    {
        throw ModelException(MID_NUTRSED, "CheckInputData", "The dimension of the input data can not be less than zero.");
    }
	// initial enrichment ratio
	if (this->m_enratio == NULL)
		Initialize1DArray(m_nCells, m_enratio, 0.f);
	if (m_cellArea < 0)
	{
		m_cellArea = m_cellWidth * m_cellWidth * 0.0001f; //Unit is ha 
	}
    // allocate the output variables
    if (m_sedorgn == NULL)
    {
		Initialize1DArray(m_nCells, m_sedorgn, 0.f);
		Initialize1DArray(m_nCells, m_sedorgp, 0.f);
		Initialize1DArray(m_nCells, m_sedminpa, 0.f);
		Initialize1DArray(m_nCells, m_sedminps, 0.f);

		Initialize1DArray(m_nSubbasins+1, m_sedorgnToCh, 0.f);
		Initialize1DArray(m_nSubbasins+1, m_sedorgpToCh, 0.f);
		Initialize1DArray(m_nSubbasins+1, m_sedminpaToCh, 0.f);
		Initialize1DArray(m_nSubbasins+1, m_sedminpsToCh, 0.f);
    }
	/// for CENTURY C/N cycling model outputs
	if (m_CbnModel == 2 && m_sol_latC == NULL)
	{
		Initialize2DArray(m_nCells, m_soiLayers, m_sol_latC, 0.f);
		Initialize2DArray(m_nCells, m_soiLayers, m_sol_percoC, 0.f);
		Initialize1DArray(m_nCells, m_laterC, 0.f);
		Initialize1DArray(m_nCells, m_percoC, 0.f);
		Initialize1DArray(m_nCells, m_sedCLoss, 0.f);
	}
}

void NutrientTransportSediment::SetSubbasins(clsSubbasins *subbasins)
{
	if(m_subbasinsInfo == NULL){
		m_subbasinsInfo = subbasins;
		m_nSubbasins = m_subbasinsInfo->GetSubbasinNumber();
		m_subbasinIDs = m_subbasinsInfo->GetSubbasinIDs();
	}
}

int NutrientTransportSediment::Execute()
{
    if (!CheckInputData())return false;
	if (m_CbnModel == 1){
		if (!CheckInputData_CFARM())
			return false;
	}
	if (m_CbnModel == 2){
		if (!CheckInputData_CENTURY())
			return false;
	}
    this->initialOutputs();
	// initial nutrient to channel for each day
#pragma omp parallel for
	for (int i = 0; i < m_nSubbasins+1; i++)
	{
		m_sedorgnToCh[i] = 0.f;
		m_sedorgpToCh[i] = 0.f;
		m_sedminpaToCh[i] = 0.f;
		m_sedminpsToCh[i] = 0.f;
	}

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++)
    {
		if (m_sedEroded[i] < 1.e-4f) m_sedEroded[i] = 0.f;
		// CREAMS method for calculating enrichment ratio
		m_enratio[i] = NutrCommon::CalEnrichmentRatio(m_sedEroded[i], m_surfaceRunoff[i], m_cellArea);
		//if(i == 1000) cout << ""<< m_sedEroded[i]<<","<< m_surfaceRunoff[i] << "," << m_enratio[i]<<endl;
        
		//Calculates the amount of organic nitrogen removed in surface runoff
		if (m_CbnModel == 0)
			OrgNRemovedInRunoff_StaticMethod(i);
		else if (m_CbnModel == 1)
			OrgNRemovedInRunoff_CFARMOneCarbonModel(i);
		else if (m_CbnModel == 2)
			OrgNRemovedInRunoff_CENTURY(i);
        //Calculates the amount of organic and mineral phosphorus attached to sediment in surface runoff. psed.f of SWAT
        OrgPAttachedtoSed(i);
    }

	//float maxsedorgp = -1.f;
	//int idx = -1;
	//for (int i = 0; i < m_nCells; i++)
	//{
	//	if (m_sedorgp[i] > maxsedorgp)
	//	{
	//		maxsedorgp = m_sedorgp[i];
	//		idx = i;
	//	}
	//}
	//cout<<"maximum sedorgp id: "<<idx<< ", surfq: " <<m_surfaceRunoff[idx]<< ", sedYld: "<<m_sedEroded[idx]<<
	//	", eratio: "<<m_enratio[idx]<<", sedorgp: "<<m_sedorgp[idx]<<endl;

	// sum by subbasin
	for (int i = 0; i < m_nCells; i++)
	{
		//add today's flow
		int subi = (int) m_subbasin[i];
		if (m_nSubbasins == 1)
		{
			subi = 1;
		}
		else if (subi >= m_nSubbasins + 1)
		{
			throw ModelException(MID_NUTRSED, "Execute", "The subbasin " + ValueToString(subi) + " is invalid.");
		}
		m_sedorgnToCh[subi] += m_sedorgn[i] * m_cellArea;
		m_sedorgpToCh[subi] += m_sedorgp[i] * m_cellArea;
		m_sedminpaToCh[subi] += m_sedminpa[i] * m_cellArea;
		m_sedminpsToCh[subi] += m_sedminps[i] * m_cellArea;
	}
	//cout << m_sedorgpToCh[12] << endl;
	// sum all the subbasins and put the sum value in the zero-index of the array
	//for (int i = 1; i < m_nSubbasins + 1; i++)
	for (vector<int>::iterator it = m_subbasinIDs.begin(); it != m_subbasinIDs.end(); it++)
	{
		m_sedorgnToCh[0] += m_sedorgnToCh[*it];
		m_sedorgpToCh[0] += m_sedorgpToCh[*it];
		m_sedminpaToCh[0] += m_sedminpaToCh[*it];
		m_sedminpsToCh[0] += m_sedminpsToCh[*it];
	}
    return 0;
}

void NutrientTransportSediment::OrgNRemovedInRunoff_StaticMethod(int i)
{
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
	if (orgninfl > 1.e-6f)
	{
		m_sol_aorgn[i][0] = m_sol_aorgn[i][0] - m_sedorgn[i] * (m_sol_aorgn[i][0] / orgninfl);
		m_sol_orgn[i][0] = m_sol_orgn[i][0] - m_sedorgn[i] * (m_sol_orgn[i][0] / orgninfl);
		m_sol_fon[i][0] = m_sol_fon[i][0] - m_sedorgn[i] * (m_sol_fon[i][0] / orgninfl);
		if (m_sol_aorgn[i][0] < 0.f)
		{
			m_sedorgn[i] = m_sedorgn[i] + m_sol_aorgn[i][0];
			m_sol_aorgn[i][0] = 0.f;
		}
		if (m_sol_orgn[i][0] < 0.f)
		{
			m_sedorgn[i] = m_sedorgn[i] + m_sol_orgn[i][0];
			m_sol_orgn[i][0] = 0.f;
		}
		if (m_sol_fon[i][0] < 0.f)
		{
			m_sedorgn[i] = m_sedorgn[i] + m_sol_fon[i][0];
			m_sol_fon[i][0] = 0.f;
		}
	}
}

void NutrientTransportSediment::OrgNRemovedInRunoff_CFARMOneCarbonModel(int i)
{
	/// TODO
}

void NutrientTransportSediment::OrgNRemovedInRunoff_CENTURY(int i)
{
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
	if (totOrgN_lyr0 > UTIL_ZERO)
	{
		float xx1 = (1.f - m_sedorgn[i] / totOrgN_lyr0);
		m_sol_LSN[i][0] *= xx1;
		m_sol_LMN[i][0] *= xx1;
		m_sol_HPN[i][0] *= xx1;
		m_sol_HSN[i][0] *= xx1;
	}
	/// Calculate runoff and leached C&N from micro-biomass
	sol_mass = m_soilThick[i][0]/1000.f * 10000.f * m_sol_bd[i][0] * 1000.f *
		(1.f - m_sol_rock[i][0]/100.f); /// kg/ha
	/// total organic carbon in layer 1
	TOT = m_sol_HPC[i][0] + m_sol_HSC[i][0] + m_sol_LMC[i][0] + m_sol_LSC[i][0];
	/// fraction of soil erosion of total soil mass
	YEW = min((m_sedEroded[i]/m_cellArea+YW/m_cellArea)/sol_mass, 0.9f);

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

	if (m_sol_BMC[i][0] > 0.01f)
	{
		///KOC FOR CARBON LOSS IN WATER AND SEDIMENT(500._1500.) KD = KOC * C
		PRMT_21 = 1000.f;
		m_sol_WOC[i][0] = m_sol_LSC[i][0] + m_sol_LMC[i][0] + m_sol_HPC[i][0] + m_sol_HSC[i][0] + m_sol_BMC[i][0];
		DK = 0.0001f * PRMT_21 * m_sol_WOC[i][0];
		X1 = m_sol_wsatur[i][0];
		if (X1 <= 0.f) X1 = 0.01f;
		XX = X1 + DK;
		V = m_surfaceRunoff[i] +  m_sol_perco[i][0] + m_sol_laterq[i][0];
		if (V > 1.e-10f)
		{
			X3 = m_sol_BMC[i][0] * (1.f - exp(-V / XX)); /// loss of biomass C
			PRMT_44 = 0.5;
			CO = X3 / (m_sol_perco[i][0] + PRMT_44 * (m_surfaceRunoff[i] + m_sol_laterq[i][0]));
			CS = PRMT_44 * CO;
			VBC = CO * m_sol_perco[i][0];
			m_sol_BMC[i][0] -= X3;
			QBC = CS * (m_surfaceRunoff[i] + m_sol_laterq[i][0]);
			/// Compute WBMC loss with sediment
			if (YEW > 0.f)
			{
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
	for (int k = 1; k < (int)m_nSoilLayers[i]; k++)
	{
		m_sol_WOC[i][k] = m_sol_LSC[i][k] + m_sol_LMC[i][k] + m_sol_HPC[i][k] + m_sol_HSC[i][k];
		float Y1 = m_sol_BMC[i][k] + VBC;
		VBC = 0.f;
		if (Y1 > 0.01f)
		{
			V = m_sol_perco[i][k] + m_sol_laterq[i][k];
			if (V > 0.f)
			{
				VBC = Y1 * (1.f - exp(-V/(m_sol_wsatur[i][k] + 0.0001f * PRMT_21 * m_sol_WOC[i][k])));
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

void NutrientTransportSediment::OrgPAttachedtoSed(int i)
{
	//amount of phosphorus attached to sediment in soil (sol_attp)
	float sol_attp = 0.f;
	//fraction of active mineral/organic/stable mineral phosphorus in soil (sol_attp_o, sol_attp_a, sol_attp_s)
	float sol_attp_o = 0.f;
	float sol_attp_a = 0.f;
	float sol_attp_s = 0.f;
	//Calculate sediment
	sol_attp = m_sol_orgp[i][0] + m_sol_fop[i][0] + m_sol_actp[i][0] + m_sol_stap[i][0];
	if (m_sol_mp != NULL)
		sol_attp += m_sol_mp[i][0];
	if (sol_attp > 1.e-3f)
	{
		sol_attp_o = (m_sol_orgp[i][0] + m_sol_fop[i][0]) / sol_attp;
		if (m_sol_mp != NULL)
			sol_attp_o += m_sol_mp[i][0] / sol_attp;
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
	if (porgg > 1.e-3f)
	{
		m_sol_orgp[i][0] = m_sol_orgp[i][0] - m_sedorgp[i] * (m_sol_orgp[i][0] / porgg);
		m_sol_fop[i][0] = m_sol_fop[i][0] - m_sedorgp[i] * (m_sol_fop[i][0] / porgg);
	}
	m_sol_actp[i][0] = m_sol_actp[i][0] - m_sedminpa[i];
	m_sol_stap[i][0] = m_sol_stap[i][0] - m_sedminps[i];
	if (m_sol_orgp[i][0] < 0.f)
	{
		m_sedorgp[i] = m_sedorgp[i] + m_sol_orgp[i][0];
		m_sol_orgp[i][0] = 0.f;
	}
	if (m_sol_fop[i][0] < 0.f)
	{
		m_sedorgp[i] = m_sedorgp[i] + m_sol_fop[i][0];
		m_sol_fop[i][0] = 0.f;
	}
	if (m_sol_actp[i][0] < 0.f)
	{
		m_sedminpa[i] = m_sedminpa[i] + m_sol_actp[i][0];
		m_sol_actp[i][0] = 0.f;
	}
	if (m_sol_stap[i][0] < 0.f)
	{
		m_sedminps[i] = m_sedminps[i] + m_sol_stap[i][0];
		m_sol_stap[i][0] = 0.f;
	}
	// if (i == 46364) cout << "surfq: " <<m_surfaceRunoff[i]<< ", sedYld: "<<m_sedEroded[i]<<", sedorgp: "<<m_sedorgp[i]<< endl;
}

void NutrientTransportSediment::Get1DData(const char *key, int *n, float **data)
{
	initialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SEDORGN)){ 
		*data = this->m_sedorgn; 
		*n = m_nCells;
	}
    else if (StringMatch(sk, VAR_SEDORGP)){ 
		*data = this->m_sedorgp; 
		*n = m_nCells;
	}
    else if (StringMatch(sk, VAR_SEDMINPA)){ 
		*data = this->m_sedminpa; 
		*n = m_nCells;
	}
    else if (StringMatch(sk, VAR_SEDMINPS)){ 
		*data = this->m_sedminps; 
		*n = m_nCells;
	}
	else if(StringMatch(sk, VAR_SEDORGN_TOCH)){
		*data = m_sedorgnToCh; 
		*n = m_nSubbasins + 1;
	}
	else if(StringMatch(sk, VAR_SEDORGP_TOCH)){
		*data = m_sedorgpToCh; 
		*n = m_nSubbasins + 1;
	}
	else if(StringMatch(sk, VAR_SEDMINPA_TOCH)){
		*data = m_sedminpaToCh; 
		*n = m_nSubbasins + 1;
	}
	else if(StringMatch(sk, VAR_SEDMINPS_TOCH)){
		*data = m_sedminpsToCh; 
		*n = m_nSubbasins + 1;
	}
	/// outputs of CENTURY C/N cycling model
	else if (StringMatch(sk, VAR_LATERAL_C)){ 
		*data = this->m_laterC; 
		*n = m_nCells;
	}
	else if (StringMatch(sk, VAR_PERCO_C)){ 
		*data = this->m_percoC; 
		*n = m_nCells;
	}
	else if (StringMatch(sk, VAR_SEDLOSS_C)){ 
		*data = this->m_sedCLoss; 
		*n = m_nCells;
	}
    else
        throw ModelException(MID_NUTRSED, "Get1DData","Parameter " + sk + " does not exist");
}

void NutrientTransportSediment::Get2DData(const char *key, int *nRows, int *nCols, float ***data)
{
	initialOutputs();
    string sk(key);
    *nRows = m_nCells;
    *nCols = m_soiLayers;
    if (StringMatch(sk, VAR_SOL_AORGN)) { *data = this->m_sol_aorgn; }
    else if (StringMatch(sk, VAR_SOL_FORGN)) { *data = this->m_sol_fon; }
    else if (StringMatch(sk, VAR_SOL_SORGN)) { *data = this->m_sol_orgn; }
    else if (StringMatch(sk, VAR_SOL_HORGP)) { *data = this->m_sol_orgp; }
    else if (StringMatch(sk, VAR_SOL_FORGP)) { *data = this->m_sol_fop; }
    else if (StringMatch(sk, VAR_SOL_STAP)) { *data = this->m_sol_stap; }
    else if (StringMatch(sk, VAR_SOL_ACTP)) { *data = this->m_sol_actp; }
	/// outputs of CENTURY C/N cycling model
	else if (StringMatch(sk, VAR_SOL_LATERAL_C)) { *data = this->m_sol_latC; }
	else if (StringMatch(sk, VAR_SOL_PERCO_C)) { *data = this->m_sol_percoC; }
    else
        throw ModelException(MID_NUTRSED, "Get2DData", "Output " + sk +" does not exist.");
}
