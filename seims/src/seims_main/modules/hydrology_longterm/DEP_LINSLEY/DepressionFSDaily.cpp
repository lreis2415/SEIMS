/*----------------------------------------------------------------------
*	Purpose: 	A simple fill and spill method method to calculate depression storage
*
*	Created:	Junzhi Liu
*	Date:		14-Febrary-2011
*
*	Revision:
*   Date:
*---------------------------------------------------------------------*/

#include "DepressionFSDaily.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "util.h"
#include <cmath>
#include <omp.h>

DepressionFSDaily::DepressionFSDaily(void) : m_nCells(-1),m_depCo(NODATA_VALUE),
                                             m_depCap(NULL), 
											 m_pet(NULL), m_ei(NULL), m_pe(NULL),
                                             m_sd(NULL), m_ed(NULL), m_sr(NULL),
											 m_impoundTriger(NULL),m_potVol(NULL)
{
}

DepressionFSDaily::~DepressionFSDaily(void)
{
    if (m_sd != NULL) Release1DArray(m_sd);
    if (m_ed != NULL) Release1DArray(m_ed);
    if (m_sr != NULL) Release1DArray(m_sr);
}

bool DepressionFSDaily::CheckInputData(void)
{
    if (this->m_date < 0)
        throw ModelException(MID_DEP_LINSLEY, "CheckInputData", "You have not set the time.");
    if (this->m_nCells <= 0)
        throw ModelException(MID_DEP_LINSLEY, "CheckInputData",
                             "The cell number of the input can not be less than zero.");
    if (m_depCo == NODATA_VALUE)
        throw ModelException(MID_DEP_LINSLEY, "CheckInputData",
                             "The parameter: initial depression storage coefficient has not been set.");
    if (m_depCap == NULL)
        throw ModelException(MID_DEP_LINSLEY, "CheckInputData",
                             "The parameter: depression storage capacity has not been set.");
    if (m_pet == NULL)
        throw ModelException(MID_DEP_LINSLEY, "CheckInputData", "The parameter: PET has not been set.");
    if (m_ei == NULL)
        throw ModelException(MID_DEP_LINSLEY, "CheckInputData",
                             "The parameter: evaporation from the interception storage has not been set.");
    if (m_pe == NULL)
        throw ModelException(MID_DEP_LINSLEY, "CheckInputData",
                             "The parameter: excess precipitation has not been set.");
    return true;
}

void  DepressionFSDaily::initialOutputs()
{
    if (this->m_nCells <= 0)
        throw ModelException(MID_DEP_LINSLEY, "initialOutputs",
                             "The cell number of the input can not be less than zero.");

    if (m_sd == NULL && m_depCap != NULL)
    {
        m_sd = new float[m_nCells];
        m_ed = new float[m_nCells];
        m_sr = new float[m_nCells];
#pragma omp parallel for
        for (int i = 0; i < m_nCells; ++i)
        {
            m_sd[i] = m_depCo * m_depCap[i];
            m_ed[i] = 0.f;
            m_sr[i] = 0.f;
        }
    }
}

int DepressionFSDaily::Execute()
{
    //check the data
    CheckInputData();
    initialOutputs();

#pragma omp parallel for
    for (int i = 0; i < m_nCells; ++i)
    {
        //////////////////////////////////////////////////////////////////////////
        // runoff
        if (m_depCap[i] < 0.001f)
        {
            m_sr[i] = m_pe[i];
            m_sd[i] = 0.f;
        }
        else if (m_pe[i] > 0.f)
        {
            float pc = m_pe[i] - m_depCap[i] * log(1.f - m_sd[i] / m_depCap[i]);
            float deltaSd = m_pe[i] * exp(-pc / m_depCap[i]);
            if (deltaSd > m_depCap[i] - m_sd[i])
                deltaSd = m_depCap[i] - m_sd[i];
            m_sd[i] += deltaSd;
            m_sr[i] = m_pe[i] - deltaSd;
        }
        else
        {
            m_sd[i] += m_pe[i];
            m_sr[i] = 0.f;
        }

        //////////////////////////////////////////////////////////////////////////
        // evaporation
        if (m_sd[i] > 0)
        {
			/// TODO: Is this logically right? PET is just potential, which include 
			///       not only ET from surface water, but also from plant and soil.
			///       Please Check the corresponding theory. By LJ.
            // evaporation from depression storage
            if (m_pet[i] - m_ei[i] < m_sd[i])
            {
                m_ed[i] = m_pet[i] - m_ei[i];
            }
            else
            {
                m_ed[i] = m_sd[i];
            }
            m_sd[i] -= m_ed[i];
        }
        else
        {
            m_ed[i] = 0.f;
            m_sd[i] = 0.f;
        }
		if (m_impoundTriger != NULL && FloatEqual(m_impoundTriger[i], 0.f)){
			if (m_potVol != NULL)
			{
				m_potVol[i] += m_sr[i];
				m_potVol[i] += m_sd[i];
				m_sr[i] = 0.f;
				m_sd[i] = 0.f;
			}
		}
    }
    return true;
}

bool DepressionFSDaily::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        return false;
    }
    if (this->m_nCells != n)
    {
        if (this->m_nCells <= 0) this->m_nCells = n;
        else
			throw ModelException(MID_DEP_LINSLEY, "CheckInputSize", "Input data for " + string(key) +
			" is invalid. All the input data should have same size.");
    }
    return true;
}

void DepressionFSDaily::SetValue(const char *key, float data)
{
    string sk(key);
    if (StringMatch(sk, VAR_DEPREIN)) m_depCo = data;
    else if (StringMatch(sk, VAR_OMP_THREADNUM))omp_set_num_threads((int) data); 
    else
        throw ModelException(MID_DEP_LINSLEY, "SetValue", "Parameter " + sk + " does not exist.");
}

void DepressionFSDaily::Set1DData(const char *key, int n, float *data)
{
    //check the input data
    CheckInputSize(key, n);
    string sk(key);
    if (StringMatch(sk, VAR_DEPRESSION))
        m_depCap = data;
    else if (StringMatch(sk, VAR_INET))
        m_ei = data;
    else if (StringMatch(sk, VAR_PET))
        m_pet = data;
    else if (StringMatch(sk, VAR_EXCP))
        m_pe = data;
	else if (StringMatch(sk, VAR_IMPOUND_TRIG)) m_impoundTriger = data;
	else if (StringMatch(sk, VAR_POT_VOL)) m_potVol = data;
    else
        throw ModelException(MID_DEP_LINSLEY, "Set1DData", "Parameter " + sk+" does not exist.");
}

void DepressionFSDaily::Get1DData(const char *key, int *n, float **data)
{
    initialOutputs();
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_DPST))
        *data = m_sd;
    else if (StringMatch(sk, VAR_DEET))
        *data = m_ed;
    else if (StringMatch(sk, VAR_SURU))
        *data = m_sr;
    else
        throw ModelException(MID_DEP_LINSLEY, "Get1DData", "Output " + sk+" does not exist.");
}