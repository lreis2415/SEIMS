#include "AtmosphericDeposition.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "util.h"
#include <omp.h>
#include <cmath>
#include <iostream>

using namespace std;

AtmosphericDeposition::AtmosphericDeposition(void) :
		//input
        m_nCells(-1), m_rcn(-1.f), m_rca(-1.f),m_soiLayers(-1),
        m_preci(NULL), m_drydep_no3(-1.f), m_drydep_nh4(-1.f),
		m_addrno3(-1.f), m_addrnh4(-1.f),
        //output
        m_sol_no3(NULL), m_sol_nh4(NULL),  m_wshd_rno3(-1.f)
{
}

AtmosphericDeposition::~AtmosphericDeposition(void)
{
}

bool AtmosphericDeposition::CheckInputData(void)
{
    if (m_nCells <= 0)
        throw ModelException(MID_ATMDEP, "CheckInputData",
                             "The cell number of the input can not be less than zero.");
    if (this->m_soiLayers < 0)
        throw ModelException(MID_ATMDEP, "CheckInputData", "The maximum soil layers number can not be less than 0.");
    //if (this->m_cellWidth < 0)
    //    throw ModelException(MID_ATMDEP, "CheckInputData", "The m_rca can not be less than 0.");
    if (this->m_rcn < 0)
        throw ModelException(MID_ATMDEP, "CheckInputData", "The m_rca can not be less than 0.");
    if (this->m_rca < 0)
        throw ModelException(MID_ATMDEP, "CheckInputData", "The m_rca can not be less than 0.");
    if (this->m_preci == NULL)
        throw ModelException(MID_ATMDEP, "CheckInputData", "The precipitation can not be NULL.");
    if (this->m_drydep_no3 <0)
        throw ModelException(MID_ATMDEP, "CheckInputData", "The m_drydep_no3 can not be less than 0.");
    if (this->m_drydep_nh4 <0)
        throw ModelException(MID_ATMDEP, "CheckInputData", "The m_drydep_nh4 can not be less than 0.");
	if (this->m_sol_no3 == NULL)
		throw ModelException(MID_ATMDEP, "CheckInputData", "The m_sol_no3 can not be NULL.");
	if (this->m_sol_nh4== NULL)
		throw ModelException(MID_ATMDEP, "CheckInputData", "The m_sol_nh4 can not be NULL.");
    return true;
}

bool AtmosphericDeposition::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
        throw ModelException(MID_ATMDEP, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    if (m_nCells != n)
    {
        if (m_nCells <= 0)
            m_nCells = n;
        else
        {
			throw ModelException(MID_ATMDEP, "CheckInputSize", "Input data for " + string(key) + " is invalid with size: " +ValueToString(n)+". The origin size is "
				+ValueToString(m_nCells)+".\n");
        }
    }
    return true;
}

void AtmosphericDeposition::Set1DData(const char *key, int n, float *data)
{
    string sk(key);
	CheckInputSize(key, n);
    if (StringMatch(sk, VAR_PCP))m_preci = data;
    else
        throw ModelException(MID_ATMDEP, "Set1DData", "Parameter " + sk + " does not exist.");
}

void AtmosphericDeposition::SetValue(const char *key, float value)
{
    string sk(key);
    if (StringMatch(sk, VAR_OMP_THREADNUM)) omp_set_num_threads((int) value);
    //else if (StringMatch(sk, Tag_CellSize)) { m_nCells = value; }
    //else if (StringMatch(sk, Tag_CellWidth)) { m_cellWidth = value; }
    else if (StringMatch(sk, VAR_RCN)) { m_rcn = value; }
    else if (StringMatch(sk, VAR_RCA)) { m_rca = value; }
    else if (StringMatch(sk, VAR_DRYDEP_NO3)) { m_drydep_no3 = value; }
    else if (StringMatch(sk, VAR_DRYDEP_NH4)) { m_drydep_nh4 = value; }
    else
        throw ModelException(MID_ATMDEP, "SetValue",
                             "Parameter " + sk + " does not exist. Please contact the module developer.");
}

void AtmosphericDeposition::Set2DData(const char *key, int nRows, int nCols, float **data)
{
    if (!this->CheckInputSize(key, nRows)) return;
    string sk(key);
    m_soiLayers = nCols;
    if (StringMatch(sk, VAR_SOL_NO3)) this->m_sol_no3 = data;
    else if (StringMatch(sk, VAR_SOL_NH4)) this->m_sol_nh4 = data;
    else
        throw ModelException(MID_ATMDEP, "Set2DData", "Parameter " + sk + " does not exist.");
}

void AtmosphericDeposition::initialOutputs()
{
    if (this->m_nCells <= 0)
        throw ModelException(MID_ATMDEP, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    // allocate the output variables
    if (m_addrnh4 < 0.f)
    {
        m_addrnh4 = 0.f;
        m_addrno3 = 0.f;
    }
	/// initialize m_wshd_rno3 to 0.f at each time step
    if (!FloatEqual(m_wshd_rno3, 0.f)) m_wshd_rno3 = 0.f;
}

int AtmosphericDeposition::Execute()
{
    //check the data
    this->CheckInputData();
    this->initialOutputs();
	#pragma omp parallel for
	for (int i = 0; i < m_nCells; i++)
	{
		if(m_preci[i] > 0.f)
		{
			/// Calculate the amount of nitrite and ammonia added to the soil in rainfall
			/// unit conversion: mg/L * mm = 0.01 * kg/ha (CHECKED)
			m_addrno3 = 0.01f * m_rcn * m_preci[i];
			m_addrnh4 = 0.01f * m_rca * m_preci[i];
			m_sol_no3[i][0] += (m_addrno3 + m_drydep_no3 / 365.f);
			m_sol_nh4[i][0] += (m_addrnh4 + m_drydep_nh4 / 365.f);
			m_wshd_rno3 += (m_addrno3 * (1.f / m_nCells));
		}
	}
    return 0;
}

void AtmosphericDeposition::GetValue(const char *key, float *value)
{
    string sk(key);
    if (StringMatch(sk, VAR_WSHD_RNO3)) *value = m_wshd_rno3; 
    else
		throw ModelException(MID_ATMDEP, "GetValue", "Parameter " + sk + " does not exist.");
}