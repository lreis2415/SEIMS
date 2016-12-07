/*!
 * \file NUTR_CH.cpp
 * \brief Sediment routing using variable channel dimension(VCD) method at daily time scale
 * \author Hui Wu
 * \date Jul. 2012

 * \revised LiangJun Zhu
 * \date May / 2016

 * \revised Junzhi Liu
 * \date August / 2016
 */

#include "NUTR_CH.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "util.h"
#include <omp.h>
#include <cmath>
#include <iostream>
#include <set>
#include <sstream>
#include <algorithm> 
#include <omp.h>

#define CHECK_POINTER(moduleName, varName) if (varName == NULL) throw ModelException(moduleName, "CheckInputData", "The parameter: varName has not been set.");
#define CHECK_POSITIVE(moduleName, varName) if (varName > 0) 	throw ModelException(moduleName, "CheckInputData", "The parameter: varName has not been set.");

using namespace std;

NUTR_CH::NUTR_CH(void) : m_dt(-1), m_nreach(-1), m_Chs0(NODATA_VALUE), m_Vdiv(NULL), m_Vpoint(NULL), m_chStorage(NULL), 
                         m_chOrder(NULL), m_qchOut(NULL), m_latno3ToCh(NULL), m_sur_no3ToCh(NULL), m_sur_solpToCh(NULL),
						 m_sedorgnToCh(NULL), m_sedorgpToCh(NULL), m_no3gwToCh(NULL), m_minpgwToCh(NULL),
						 m_chOrgN(NULL), m_chNo3(NULL), m_chOrgP(NULL), m_chSolP(NULL)
{
}


NUTR_CH::~NUTR_CH(void)
{
	if(m_chOrgN != NULL) Release1DArray(m_chSolP);
	if(m_chNo3 != NULL)  Release1DArray(m_chSolP);
	if(m_chOrgP != NULL) Release1DArray(m_chSolP);
	if(m_chSolP != NULL) Release1DArray(m_chSolP);
}

bool NUTR_CH::CheckInputData(void)
{
	CHECK_POSITIVE(MID_NUTR_CH, m_dt)
	CHECK_POSITIVE(MID_NUTR_CH, m_nreach)
	CHECK_POSITIVE(MID_NUTR_CH, m_Chs0)

    CHECK_POINTER(MID_NUTR_CH, m_chWidth)
	CHECK_POINTER(MID_NUTR_CH, m_chStorage)
    CHECK_POINTER(MID_NUTR_CH, m_qchOut)
	CHECK_POINTER(MID_NUTR_CH, m_latno3ToCh)
	CHECK_POINTER(MID_NUTR_CH, m_sur_no3ToCh)
	CHECK_POINTER(MID_NUTR_CH, m_sur_solpToCh)
	CHECK_POINTER(MID_NUTR_CH, m_sedorgnToCh)
	CHECK_POINTER(MID_NUTR_CH, m_sedorgpToCh)
	CHECK_POINTER(MID_NUTR_CH, m_no3gwToCh)
	CHECK_POINTER(MID_NUTR_CH, m_minpgwToCh)

    return true;
}

void  NUTR_CH::initialOutputs()
{

    if (m_nreach <= 0)
        throw ModelException(MID_NUTR_CH, "initialOutputs", "The cell number of the input can not be less than zero.");

    if (m_reachLayers.empty())
    {
        CheckInputData();
        for (int i = 1; i <= m_nreach; i++)
        {
            int order = (int) m_chOrder[i];
            m_reachLayers[order].push_back(i);
        }
    }

	if (m_latno3ToCh == NULL)
	{
		Initialize1DArray(m_nreach+1, m_chOrgN, 0.f);
		Initialize1DArray(m_nreach+1, m_chNo3, 0.f);
		Initialize1DArray(m_nreach+1, m_chOrgP, 0.f);
		Initialize1DArray(m_nreach+1, m_chSolP, 0.f);
	}
}

int NUTR_CH::Execute()
{
    //check the data
    CheckInputData();
    initialOutputs();

    map<int, vector<int> >::iterator it;
    for (it = m_reachLayers.begin(); it != m_reachLayers.end(); it++)
    {
        // There are not any flow relationship within each routing layer.
        // So parallelization can be done here.
        int nReaches = it->second.size();
        // the size of m_reachLayers (map) is equal to the maximum stream order
#pragma omp parallel for
        for (int i = 0; i < nReaches; ++i)
        {
            int reachIndex = it->second[i]; // index in the array

            SedChannelRouting(reachIndex);
        }
    }

    return 0;
}

bool NUTR_CH::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        //this->StatusMsg("Input data for "+string(key) +" is invalid. The size could not be less than zero.");
        return false;
    }
    if (this->m_nreach != n)
    {
        if (this->m_nreach <= 0) this->m_nreach = n;
        else
        {
            //this->StatusMsg("Input data for "+string(key) +" is invalid. All the input data should have same size.");
            ostringstream oss;
            oss << "Input data for " + string(key) << " is invalid with size: " << n << ". The origin size is " <<
            m_nreach << ".\n";
            throw ModelException(MID_NUTR_CH, "CheckInputSize", oss.str());
        }
    }

    return true;
}

void NUTR_CH::GetValue(const char *key, float *value)
{
    string sk(key);
    int iOutlet = m_reachLayers.rbegin()->second[0];
    if (StringMatch(sk, VAR_SED_OUTLET))
    {
    }
}

void NUTR_CH::SetValue(const char *key, float value)
{
    string sk(key);

    if (StringMatch(sk, VAR_OMP_THREADNUM))
    {
        omp_set_num_threads((int) value);
    }
#ifdef STORM_MODEL
        else if (StringMatch(sk, Tag_ChannelTimeStep))
        {
            m_dt = (int) value;
        }
#else
    else if (StringMatch(sk, Tag_TimeStep))
    {
        m_dt = (int) value;
    }
#endif
    else if (StringMatch(sk, VAR_CHS0))
    {
        m_Chs0 = value;
    }
    else
        throw ModelException(MID_NUTR_CH, "SetValue", "Parameter " + sk + " does not exist.");
}

void NUTR_CH::Set1DData(const char *key, int n, float *value)
{
    string sk(key);

    //check the input data
    if (StringMatch(sk, VAR_CHST))              m_chStorage = value;
	else if(StringMatch(sk, VAR_SUR_NO3_TOCH))  m_sur_no3ToCh = value;
	else if(StringMatch(sk, VAR_SUR_SOLP_TOCH)) m_sur_solpToCh = value;
	else if(StringMatch(sk, VAR_SEDORGN_TOCH))  m_sedorgnToCh = value;
	else if(StringMatch(sk, VAR_SEDORGP_TOCH))  m_sedorgpToCh = value;
	else if(StringMatch(sk, VAR_LATNO3_TOCH))   m_latno3ToCh = value;
	else if(StringMatch(sk, VAR_NO3GW_TOCH))    m_no3gwToCh = value;
	else if(StringMatch(sk, VAR_MINPGW_TOCH))   m_minpgwToCh = value;
    else
        throw ModelException(MID_NUTR_CH, "Set1DData", "Parameter " + sk + " does not exist.");
}

void NUTR_CH::Get1DData(const char *key, int *n, float **data)
{
	initialOutputs();
    string sk(key);
    *n = m_nreach + 1;
    int iOutlet = m_reachLayers.rbegin()->second[0];

	if (StringMatch(sk, VAR_CH_NO3))        *data = m_chNo3; 
	//else if (StringMatch(sk, VAR_CH_NH4))   *data = m_chNH4;
	else if (StringMatch(sk, VAR_CH_SOLP))  *data = m_chSolP; 
	else if (StringMatch(sk, VAR_CH_ORGN))  *data = m_chOrgN; 
	else if (StringMatch(sk, VAR_CH_ORGP))  *data = m_chOrgP; 
    else
        throw ModelException(MID_NUTR_CH, "Get1DData", "Output " + sk + " does not exist.");
}

void NUTR_CH::Get2DData(const char *key, int *nRows, int *nCols, float ***data)
{
    string sk(key);
    /*if (StringMatch(sk,"T_CHSB"))
    {
    *data = this->m_T_CHSB;
    *nRows = this->m_nreach;
    *nCols = SEDIMENT_CHANNEL_ROUTING_RESULT_DISCHARGE_COLUMN_COUNT;
    }
    else
        throw ModelException(MID_NUTR_CH, "Get2DData", "Output " + sk
        + " does not exist in the NUTR_CH module. Please contact the module developer.");*/
}

void NUTR_CH::Set2DData(const char *key, int nrows, int ncols, float **data)
{
    string sk(key);

    if (StringMatch(sk, Tag_RchParam))
    {
        m_nreach = ncols - 1;

        m_reachId = data[0];
        m_reachDownStream = data[1];
        m_chOrder = data[2];
        m_chWidth = data[3];
        m_chLen = data[4];
        m_chDepth = data[5];
        //m_chVel = data[6];
        //m_area = data[7];
        //m_chSlope = data[9];

        m_reachUpStream.resize(m_nreach + 1);
        if (m_nreach > 1)
        {
            for (int i = 1; i <= m_nreach; i++)// index of the reach is the equal to streamlink ID(1 to nReaches)
            {
                int downStreamId = int(m_reachDownStream[i]);
                if (downStreamId <= 0)
                    continue;
                m_reachUpStream[downStreamId].push_back(i);
            }
        }
    }
    else
        throw ModelException(MID_NUTR_CH, "Set2DData", "Parameter " + sk + " does not exist.");
}

void NUTR_CH::SedChannelRouting(int i)
{
    // 1 .whether is no water in channel
  //  if (m_qchOut[i] < 0.0001f)
  //  {
		//m_sedDeg[i] = 0.0f;
		//m_sedDep[i] = 0.0f;
  //      m_SedOut[i] = 0.f;
  //  }
  //  else
  //  {   // 2. sediment from upstream reaches
  //      float sedUp = 0;
  //      for (size_t j = 0; j < m_reachUpStream[i].size(); ++j)
  //      {
  //          int upReachId = m_reachUpStream[i][j];
  //          sedUp += m_SedOut[upReachId];
  //      }
  //      float allSediment = sedUp + m_sedtoCh[i] + m_sedStorage[i];

  //      //get peak channel velocity (m/s)
  //      float peakFlowRate = m_qchOut[i] * m_prf;
  //      float crossarea = m_chStorage[i] / m_chLen[i];            // SWAT, eq. 7:1.2.3
  //      float peakVelocity = peakFlowRate / crossarea;
  //      if (peakVelocity > 5) peakVelocity = 5.0f;
  //      //max concentration
  //      float maxCon = m_spcon * pow(peakVelocity, m_spexp);
  //      //initial concentration,mix sufficiently

  //      float qOutV = m_qchOut[i] * m_dt;
  //      float allWater = m_chStorage[i] + qOutV;

  //      if (allWater < 0.001f)
  //      {
		//	m_sedDeg[i] = 0.0f;
		//	m_sedDep[i] = 0.0f;
  //          m_SedOut[i] = 0.0f;
  //          return;
  //      }

		////deposition and degradation
  //      float initCon = allSediment / allWater;
  //      float sedDeposition = allWater * (initCon - maxCon);
  //      //if (abs(sedDeposition) < 1.0e-6f)
  //      //    sedDeposition = 0.0f;
  //      if (peakVelocity < m_vcrit)
  //          sedDeposition = 0.0f;

  //      float sedDegradation = 0.0f;
  //      float sedDegradation1 = 0.0f;
  //      float sedDegradation2 = 0.0f;
		////get tbase
		//float tbase = m_chLen[i] / (m_dt * peakVelocity);
		//if (tbase > 1) tbase = 1.0f;
  //      if (sedDeposition < 0.f)    //degradation
  //      {
  //          sedDegradation = -sedDeposition * tbase;
		//	// first the deposited material will be degraded before channel bed
  //          if (sedDegradation >= m_sedDep[i])
  //          {
  //              sedDegradation1 = m_sedDep[i];
  //              sedDegradation2 = (sedDegradation - sedDegradation1) * m_erodibilityFactor * m_coverFactor;
  //          }
  //          else
  //          {
  //              sedDegradation1 = sedDegradation;
  //          }

  //          sedDeposition = 0.0f;
  //      }

  //      //update sed deposition
  //      m_sedDep[i] += sedDeposition - sedDegradation1;
  //      m_sedDeg[i] += sedDegradation1 + sedDegradation2;

  //      //get sediment after deposition and degradation
  //      allSediment += sedDegradation1 + sedDegradation2 - sedDeposition;

  //      //get out flow water fraction
  //      float outFraction = qOutV / allWater;
  //      m_SedOut[i] = allSediment * outFraction;

  //      //update sed storage
  //      m_sedStorage[i] = allSediment - m_SedOut[i];

  //      //get final sediment in water, cannot large than 0.848t/m3
  //      float maxSedinWt = 0.848f * qOutV;
  //      if (m_SedOut[i] > maxSedinWt)
  //      {
  //          m_sedDep[i] += m_SedOut[i] - maxSedinWt;
  //          m_SedOut[i] = maxSedinWt;
  //      }


  //  }

}

