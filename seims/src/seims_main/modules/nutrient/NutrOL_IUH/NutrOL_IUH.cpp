/*//
 * \file NutrOL_IUH.cpp
 * \ingroup NutrOL
 * \author Huiran Gao
 * \date Jun 2016
 */

#include <iostream>
#include "NutrOL_IUH.h"
#include "MetadataInfo.h"
#include <cmath>
#include <fstream>
#include "ModelException.h"
#include "util.h"
#include <omp.h>

using namespace std;

NutrOL_IUH::NutrOL_IUH(void) :
//input
        m_nCells(-1), m_cellWidth(-1), m_TimeStep(NODATA_VALUE), 
        m_latno3(NULL), m_surqno3(NULL), m_surqsolp(NULL), m_no3gw(NULL), m_minpgw(NULL), 
		m_sedorgn(NULL), m_sedorgp(NULL), m_sedminpa(NULL), m_sedminps(NULL), m_cod(NULL),
		m_Soer(NULL), m_Sed_kg(NULL), m_Suru(NULL), m_Q_Flow(NULL),
        //output
        m_surqno3ToCh(NULL), m_latno3ToCh(NULL), m_no3gwToCh(NULL), m_surqsolpToCh(NULL), m_minpgwToCh(NULL),
        m_sedorgnToCh(NULL), m_sedorgpToCh(NULL), m_sedminpaToCh(NULL), m_sedminpsToCh(NULL),
        m_ammoToCh(NULL), m_nitriteToCh(NULL), m_codToCh(NULL)
{

}

NutrOL_IUH::~NutrOL_IUH(void)
{
	if (m_cellsurqno3 != NULL) Release2DArray(m_nCells, m_cellsurqno3);
	if (m_celllatno3 != NULL) Release2DArray(m_nCells, m_celllatno3);
	if (m_cellno3gw != NULL) Release2DArray(m_nCells, m_cellno3gw);
	if (m_cellsurqsolp != NULL) Release2DArray(m_nCells, m_cellsurqsolp);
	if (m_cellminpgw != NULL) Release2DArray(m_nCells, m_cellminpgw);
	if (m_cellsedorgn != NULL) Release2DArray(m_nCells, m_cellsedorgn);
	if (m_cellsedorgp != NULL) Release2DArray(m_nCells, m_cellsedorgp);
	if (m_cellsedminpa != NULL) Release2DArray(m_nCells, m_cellsedminpa);
	if (m_cellsedminps != NULL) Release2DArray(m_nCells, m_cellsedminps);
	//if (m_cellammo != NULL) Release2DArray(m_nCells, m_cellammo);
	//if (m_cellnitrite != NULL) Release2DArray(m_nCells, m_cellnitrite);
	if (m_cellcod != NULL) Release2DArray(m_nCells, m_cellcod);

	if (m_surqno3ToCh != NULL) Release1DArray(m_surqno3ToCh);
	if (m_latno3ToCh != NULL) Release1DArray(m_latno3ToCh);
	if (m_no3gwToCh != NULL) Release1DArray(m_no3gwToCh);
	if (m_surqsolpToCh != NULL) Release1DArray(m_surqsolpToCh);
	if (m_minpgwToCh != NULL) Release1DArray(m_minpgwToCh);
	if (m_sedorgnToCh != NULL) Release1DArray(m_sedorgnToCh);
	if (m_sedorgpToCh != NULL) Release1DArray(m_sedorgpToCh);
	if (m_sedminpaToCh != NULL) Release1DArray(m_sedminpaToCh);
	if (m_sedminpsToCh != NULL) Release1DArray(m_sedminpsToCh);
	if (m_ammoToCh != NULL) Release1DArray(m_ammoToCh);
	if (m_nitriteToCh != NULL) Release1DArray(m_nitriteToCh);
	if (m_codToCh != NULL) Release1DArray(m_codToCh);
}

bool NutrOL_IUH::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        throw ModelException(MID_NutOLRout, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_nCells != n)
    {
        if (m_nCells <= 0)
        {
            m_nCells = n;
        } else
        {
            //StatusMsg("Input data for "+string(key) +" is invalid. All the input data should have same size.");
            ostringstream oss;
            oss << "Input data for " + string(key) << " is invalid with size: " << n << ". The origin size is " <<
            m_nCells << ".\n";
            throw ModelException(MID_NutOLRout, "CheckInputSize", oss.str());
        }
    }
    return true;
}

bool NutrOL_IUH::CheckInputData()
{
    if (this->m_nCells <= 0)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The cells number can not be less than zero.");
    }
    if (this->m_cellWidth <= 0)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The cell width can not be less than zero.");
    }
    if (this->m_TimeStep == NODATA_VALUE)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The time can not be zero.");
	}
	if (m_subbasin == NULL)
	{
		throw ModelException(MID_NutOLRout, "CheckInputData", "The parameter: m_subbasin has not been set.");
	}
	if (m_Suru == NULL)
	{
		throw ModelException(MID_NutOLRout, "CheckInputData", "The input: m_Suru has not been set.");
	}
	if (m_Q_Flow == NULL)
	{
		throw ModelException(MID_NutOLRout, "CheckInputData", "The input: m_Q_Flow has not been set.");
	}
	if (m_Sed_kg == NULL)
	{
		throw ModelException(MID_NutOLRout, "CheckInputData", "The input: m_Sed_kg has not been set.");
	}
	if (m_Soer == NULL)
	{
		throw ModelException(MID_NutOLRout, "CheckInputData", "The input: m_Soer has not been set.");
	}
	if (m_iuhCell == NULL)
	{
		throw ModelException(MID_NutOLRout, "CheckInputData", "The input: m_iuhCell has not been set.");
	}
    if (this->m_sedminps == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The amount of stable mineral phosphorus absorbed to sediment in surface runoff can not be NULL.");
    }
    if (this->m_sedminpa == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The amount of active mineral phosphorus absorbed to sediment in surface runoff can not be NULL.");
    }
    if (this->m_sedorgp == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The amount of organic phosphorus in surface runoff can not be NULL.");
    }
    if (this->m_sedorgn == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The amount of organic nitrogen in surface runoff can not be NULL.");
    }
    if (this->m_minpgw == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The soluble P loading to reach in groundwater can not be NULL.");
    }
    if (this->m_no3gw == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The nitrate loading to reach in groundwater can not be NULL.");
    }
    if (this->m_surqsolp == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The amount of soluble phosphorus in surface runoff can not be NULL.");
    }
    if (this->m_surqno3 == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The amount of nitrate transported with surface runoff can not be NULL.");
    }
    if (this->m_latno3 == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The amount of nitrate transported with lateral flow can not be NULL.");
    }
    if (this->m_cod == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The carbonaceous oxygen demand of surface runoff can not be NULL.");
    }
    return true;
}

void NutrOL_IUH::SetValue(const char *key, float value)
{
    string sk(key);
    if (StringMatch(sk, VAR_OMP_THREADNUM))
    {
        omp_set_num_threads((int) value);
    }
    else if (StringMatch(sk, Tag_CellSize)) { this->m_nCells = (int)value; }
    else if (StringMatch(sk, Tag_CellWidth)) { this->m_cellWidth = value; }
    else if (StringMatch(sk, Tag_HillSlopeTimeStep)) { this->m_TimeStep = value; }
    else
    {
        throw ModelException(MID_NutOLRout, "SetValue", "Parameter " + sk +
                                                      " does not exist in Nutrient method. Please contact the module developer.");
    }
}

void NutrOL_IUH::Set1DData(const char *key, int n, float *data)
{
    if (!this->CheckInputSize(key, n)) return;
	string sk(key);
	if (StringMatch(sk, VAR_SUBBSN)) {m_subbasin = data;}
    else if (StringMatch(sk, VAR_LATNO3)) { this->m_latno3 = data; }
    else if (StringMatch(sk, VAR_SUR_NO3)) { this->m_surqno3 = data; }
    //else if(StringMatch(sk, VAR_AMMONIAN)) {this -> m_ammo = data;}
    else if (StringMatch(sk, VAR_SUR_SOLP)) { this->m_surqsolp = data; }
    else if (StringMatch(sk, VAR_NO3GW)) { this->m_no3gw = data; }
    else if (StringMatch(sk, VAR_MINPGW)) { this->m_minpgw = data; }
    else if (StringMatch(sk, VAR_SEDORGN)) { this->m_sedorgn = data; }
    else if (StringMatch(sk, VAR_SEDORGP)) { this->m_sedorgp = data; }
    else if (StringMatch(sk, VAR_SEDMINPA)) { this->m_sedminpa = data; }
    else if (StringMatch(sk, VAR_SEDMINPS)) { this->m_sedminps = data; }
    else if (StringMatch(sk, VAR_COD)) { this->m_cod = data; }
    else
    {
        throw ModelException(MID_NutOLRout, "SetValue", "Parameter " + sk +
                                                      " does not exist in CLIMATE module. Please contact the module developer.");
    }
}

void NutrOL_IUH::initialOutputs()
{
	if (this->m_nCells <= 0 || this->m_subbasin == NULL)
		throw ModelException("NutrOL_IUH", "CheckInputData", "The dimension of the input data can not be less than zero.");
	// allocate the output variables

	if (m_nsub <= 0)
	{
		map<int, int> subs;
		for (int i = 0; i < this->m_nCells; i++)
		{
			subs[int(this->m_subbasin[i])] += 1;
		}
		this->m_nsub = subs.size();
	}

	//initial some variables
	if (this->m_surqno3ToCh == NULL)
	{
		m_surqno3ToCh = new float[m_nsub + 1];
		for (int i = 0; i <= m_nsub; i++)
		{
			m_surqno3ToCh[i] = 0.f;
			m_latno3ToCh[i] = 0.f;
			m_no3gwToCh[i] = 0.f;
			m_surqsolpToCh[i] = 0.f;
			m_minpgwToCh[i] = 0.f;
			m_sedorgnToCh[i] = 0.f;
			m_sedorgpToCh[i] = 0.f;
			m_sedminpaToCh[i] = 0.f;
			m_sedminpsToCh[i] = 0.f;
			m_ammoToCh[i] = 0.f;
			m_nitriteToCh[i] = 0.f;
			m_codToCh[i] = 0.f;
		}

		for (int i = 0; i < m_nCells; i++)
		{
			m_cellNutrCols = max(int(m_iuhCell[i][1] + 1), m_cellNutrCols);
		}
		// initial temporary variables
		Initialize2DArray(m_nCells, m_cellNutrCols, m_cellsurqno3, 0.f);
		Initialize2DArray(m_nCells, m_cellNutrCols, m_celllatno3, 0.f);
		Initialize2DArray(m_nCells, m_cellNutrCols, m_cellno3gw, 0.f);
		Initialize2DArray(m_nCells, m_cellNutrCols, m_cellsurqsolp, 0.f);
		Initialize2DArray(m_nCells, m_cellNutrCols, m_cellminpgw, 0.f);
		Initialize2DArray(m_nCells, m_cellNutrCols, m_cellsedorgn, 0.f);
		Initialize2DArray(m_nCells, m_cellNutrCols, m_cellsedorgp, 0.f);
		Initialize2DArray(m_nCells, m_cellNutrCols, m_cellsedminpa, 0.f);
		Initialize2DArray(m_nCells, m_cellNutrCols, m_cellsedminps, 0.f);
		//Initialize2DArray(m_nCells, m_cellNutrCols, m_cellammo, 0.f);
		//Initialize2DArray(m_nCells, m_cellNutrCols, m_cellnitrite, 0.f);
		Initialize2DArray(m_nCells, m_cellNutrCols, m_cellcod, 0.f);
	}
}

int NutrOL_IUH::Execute()
{
    if (!this->CheckInputData())
    {
        return false;
    }
    this->initialOutputs();

	#pragma omp parallel for
	for (int n = 0; n < m_nsub + 1; n++)
	{
		// delete value of last time step
		m_surqno3ToCh[n] = 0.f;
		m_latno3ToCh[n] = 0.f;
		m_no3gwToCh[n] = 0.f;
		m_surqsolpToCh[n] = 0.f;
		m_minpgwToCh[n] = 0.f;
		m_sedorgnToCh[n] = 0.f;
		m_sedorgpToCh[n] = 0.f;
		m_sedminpaToCh[n] = 0.f;
		m_sedminpsToCh[n] = 0.f;
		m_ammoToCh[n] = 0.f;
		m_nitriteToCh[n] = 0.f;
		m_codToCh[n] = 0.f;
	}

	float area = m_cellWidth * m_cellWidth;

	#pragma omp parallel for
	for (int i = 0; i < m_nCells; i++)
	{
		// get the ratio that flow and sediment into channel
		float sedFract, flowFract;
		sedFract = CalculateSedinFlowFraction(i);
		flowFract = CalculateFlowFraction(i);

		// calculate new nutrient concentration
		m_cellsurqno3 = GetnewNutrient(i, m_surqno3, m_cellsurqno3, flowFract);
		m_celllatno3 = GetnewNutrient(i, m_latno3, m_celllatno3, 1.f);
		m_cellno3gw = GetnewNutrient(i, m_no3gw, m_cellno3gw, 1.f);
		m_cellsurqsolp = GetnewNutrient(i, m_surqsolp, m_cellsurqsolp, flowFract);
		m_cellminpgw = GetnewNutrient(i, m_minpgw, m_cellminpgw, 1.f);
		m_cellsedorgn = GetnewNutrient(i, m_sedorgn, m_cellsedorgn, sedFract);
		m_cellsedorgp = GetnewNutrient(i, m_sedorgp, m_cellsedorgp, sedFract);
		m_cellsedminpa = GetnewNutrient(i, m_sedminpa, m_cellsedminpa, sedFract);
		m_cellsedminps = GetnewNutrient(i, m_sedminps, m_cellsedminps, sedFract);
		//m_cellammo = GetnewNutrient(i, m_ammo, m_cellammo, 1.f);
		//m_cellnitrite = GetnewNutrient(i, m_nitrite, m_cellnitrite, 1.f);
		m_cellcod = GetnewNutrient(i, m_cod, m_cellcod, flowFract);

 	}

	for (int i = 0; i < m_nCells; i++)
	{
		//add today's nutrient
		int subi = (int) m_subbasin[i];
		if (m_nsub == 1)
		{
			subi = 1;
		}
		else if (subi >= m_nsub + 1)
		{
			ostringstream oss;
			oss << subi;
			throw ModelException("NutrOL_IUH", "Execute", "The subbasin " + oss.str() + " is invalid.");
		}
		m_surqno3ToCh[subi] += m_cellsurqno3[i][0];
		m_latno3ToCh[subi] += m_celllatno3[i][0];
		m_no3gwToCh[subi] += m_cellno3gw[i][0];
		m_surqsolpToCh[subi] += m_cellsurqsolp[i][0];
		m_minpgwToCh[subi] += m_cellminpgw[i][0];
		m_sedorgnToCh[subi] += m_cellsedorgn[i][0];
		m_sedorgpToCh[subi] += m_cellsedorgp[i][0];
		m_sedminpaToCh[subi] += m_cellsedminpa[i][0];
		m_sedminpsToCh[subi] += m_cellsedminps[i][0];
		m_ammoToCh[subi] += 0.f;  // No calculation before
		m_nitriteToCh[subi] += 0.f;  //No calculation before
		m_codToCh[subi] += m_cellcod[i][0];
	}
	//  sum the nutrient in entire watershed.
	m_surqno3ToCh[0] = GettotalNutr(m_surqno3ToCh);
	m_latno3ToCh[0] = GettotalNutr(m_surqno3ToCh);
	m_no3gwToCh[0] = GettotalNutr(m_no3gwToCh);
	m_surqsolpToCh[0] = GettotalNutr(m_surqsolpToCh);
	m_minpgwToCh[0] = GettotalNutr(m_minpgwToCh);
	m_sedorgnToCh[0] = GettotalNutr(m_sedorgnToCh);
	m_sedorgpToCh[0] = GettotalNutr(m_sedorgpToCh);
	m_sedminpaToCh[0] = GettotalNutr(m_sedminpaToCh);
	m_sedminpsToCh[0] = GettotalNutr(m_sedminpsToCh);
	m_ammoToCh[0] = 0.f;  // No calculation before
	m_nitriteToCh[0] = 0.f;  //No calculation before
	m_codToCh[0] = GettotalNutr(m_codToCh);
	
	return 0;
}

float NutrOL_IUH::CalculateSedinFlowFraction(int id)
{
	float sedFract = 0.f;
	sedFract = min(m_Sed_kg[id] / m_Soer[id], 1.f);
	sedFract = max(sedFract, 0.f);
	return sedFract;
}

float NutrOL_IUH::CalculateFlowFraction(int id)
{
	float flowFract = 0.f;
	flowFract = min(m_Q_Flow[id] / m_Suru[id], 1.f);
	flowFract = max(flowFract, 0.f);
	return flowFract;
}

float NutrOL_IUH::GettotalNutr(float *NutrToCh)
{
	float tmp = 0.f;
	#pragma omp parallel for reduction(+:tmp)
	for (int n = 1; n < m_nsub + 1; n++)
	{ 
		//get nutrient for entire watershed.
		tmp += NutrToCh[n];
	}
	return tmp;
}

float** NutrOL_IUH::GetnewNutrient(int id, float *nutr, float **cellNutr, float ratio)
{
	float cellArea = m_cellWidth * m_cellWidth;
	//forward one time step
	for (int j = 0; j < m_cellNutrCols; j++)
	{
		if (j != m_cellNutrCols - 1)
			cellNutr[id][j] = cellNutr[id][j + 1];
		else
			cellNutr[id][j] = 0.f;
	}
	if (nutr[id] > 0.f)
	{
		int min = int(this->m_iuhCell[id][0]);
		int max = int(this->m_iuhCell[id][1]);
		int col = 2;
		for (int k = min; k <= max; k++)
		{
			cellNutr[id][k] += nutr[id] * m_iuhCell[id][col] * cellArea / 10000.f * ratio;	// unit: kg
			col++;
		}
	}
	return cellNutr;
}

void NutrOL_IUH::Get1DData(const char *key, int *n, float **data)
{
	initialOutputs();
    string sk(key);
    *n = m_nsub + 1;
    if (StringMatch(sk, VAR_SUR_NO3_CH)) { *data = this->m_surqno3ToCh; }
    else if (StringMatch(sk, VAR_LATNO3_CH)) { *data = this->m_latno3ToCh; }
    else if (StringMatch(sk, VAR_NO3GW_CH)) { *data = this->m_no3gwToCh; }
    else if (StringMatch(sk, VAR_SUR_SOLP_CH)) { *data = this->m_surqsolpToCh; }
    else if (StringMatch(sk, VAR_MINPGW_CH)) { *data = this->m_minpgwToCh; }
    else if (StringMatch(sk, VAR_SEDORGN_CH)) { *data = this->m_sedorgnToCh; }
    else if (StringMatch(sk, VAR_SEDORGP_CH)) { *data = this->m_sedorgpToCh; }
    else if (StringMatch(sk, VAR_SEDMINPA_CH)) { *data = this->m_sedminpaToCh; }
    else if (StringMatch(sk, VAR_SEDMINPS_CH)) { *data = this->m_sedminpsToCh; }
    else if (StringMatch(sk, VAR_AMMO_CH)) { *data = this->m_ammoToCh; }
    else if (StringMatch(sk, VAR_NITRITE_CH)) { *data = this->m_nitriteToCh; }
    else if (StringMatch(sk, VAR_COD_CH)) { *data = this->m_codToCh; }
    else
    {
        throw ModelException(MID_NutOLRout, "Get1DData",
                             "Parameter " + sk + " does not exist. Please contact the module developer.");
    }
}
