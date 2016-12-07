#include "SNO_SP.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include <math.h>
#include <cmath>
#include "util.h"
#include "ClimateParams.h"
#include "PlantGrowthCommon.h"
#include <omp.h>

SNO_SP::SNO_SP(void): m_nCells(-1), m_t0(NODATA_VALUE), m_kblow(NODATA_VALUE), 
	m_tsnow(NODATA_VALUE), m_lagSnow(NODATA_VALUE), m_csnow6(NODATA_VALUE), m_csnow12(NODATA_VALUE), 
	m_snowCoverMax(NODATA_VALUE), m_snowCover50(NODATA_VALUE),
	m_snowCoverCoef1(NODATA_VALUE), m_snowCoverCoef2(NODATA_VALUE),
	m_Pnet(NULL), m_tMean(NULL), m_tMax(NULL), m_SE(NULL), 
	m_SR(NULL), m_SM(NULL), m_SA(NULL), m_packT(NULL)
{//m_swe(NODATA_VALUE), m_lastSWE(NODATA_VALUE), m_swe0(NODATA_VALUE),
}

SNO_SP::~SNO_SP(void)
{
    if (this->m_SM != NULL) Release1DArray(this->m_SM);
	if (this->m_SA != NULL) Release1DArray(this->m_SA);
    if (this->m_packT != NULL) Release1DArray(this->m_packT);
}

bool SNO_SP::CheckInputData(void)
{
    if (this->m_date <= 0) throw ModelException(MID_SNO_SP, "CheckInputData", "You have not set the time.");
    if (this->m_nCells <= 0)
        throw ModelException(MID_SNO_SP, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    if (this->m_t0 == NODATA_VALUE) throw ModelException(MID_SNO_SP, "CheckInputData", "The Snow melt temperature can not be NODATA.");
    if (this->m_kblow == NODATA_VALUE) throw ModelException(MID_SNO_SP, "CheckInputData",
                             "The fraction coefficient of precipitation as snow can not be NODATA.");
    //if (this->m_swe == NODATA_VALUE) throw ModelException(MID_SNO_SP, "CheckInputData", "The average snow accumulation of watershed can not be NODATA.");
	//if (this->m_swe0 == NODATA_VALUE) throw ModelException(MID_SNO_SP, "CheckInputData", "The swe0 can not be NODATA.");
	if (this->m_tsnow == NODATA_VALUE) throw ModelException(MID_SNO_SP, "CheckInputData", "The snow fall temperature can not be NODATA.");
    if (this->m_lagSnow == NODATA_VALUE) throw ModelException(MID_SNO_SP, "CheckInputData", "The lag snow can not be NODATA.");
    if (this->m_csnow6 == NODATA_VALUE) throw ModelException(MID_SNO_SP, "CheckInputData", "The csnow6 can not be NODATA.");
    if (this->m_csnow12 == NODATA_VALUE) throw ModelException(MID_SNO_SP, "CheckInputData", "The csnow12 can not be NODATA.");
	if (this->m_snowCoverMax == NODATA_VALUE) throw ModelException(MID_SNO_SP, "CheckInputData", "The snowCoverMax can not be NODATA.");
	if (this->m_snowCover50 == NODATA_VALUE) throw ModelException(MID_SNO_SP, "CheckInputData", "The snowCover50 can not be NODATA.");
    if (this->m_tMean == NULL)
        throw ModelException(MID_SNO_SP, "CheckInputData", "The mean temperature data can not be NULL.");
    if (this->m_tMax == NULL)
        throw ModelException(MID_SNO_SP, "CheckInputData", "The max temperature data can not be NULL.");
	if (this->m_Pnet == NULL)
        throw ModelException(MID_SNO_SP, "CheckInputData", "The net precipitation data can not be NULL.");
    //if (this->m_SE == NULL) throw ModelException(MID_SNO_SP, "CheckInputData", "The snow sublimation data can not be NULL.");
    return true;
}

void SNO_SP::initialOutputs()
{
    if (m_nCells <= 0)
        throw ModelException(MID_SNO_SP, "CheckInputData", "The dimension of the input data can not be less than zero.");
	if(m_SM == NULL) Initialize1DArray(m_nCells, m_SM, 0.f);
	if(m_SA == NULL) Initialize1DArray(m_nCells, m_SA, 0.f);
	if(m_packT == NULL) Initialize1DArray(m_nCells, m_packT, 0.f);
	//if (this->m_swe == NODATA_VALUE) 
	//	this->m_swe = this->m_swe0;
	//if (this->m_lastSWE == NODATA_VALUE) 
	//	this->m_lastSWE = this->m_swe;
	if (m_SR == NULL)  /// the initialization should be removed when snow redistribution module is accomplished. LJ
		Initialize1DArray(m_nCells, m_SR, 0.f);
	if (m_SE == NULL) /// Snow sublimation will be considered in AET_PTH
		Initialize1DArray(m_nCells, m_SE, 0.f);
}

int SNO_SP::Execute()
{
    this->CheckInputData();
    this->initialOutputs();
	/// determine the shape parameters for the equation which describes area of
	/// snow cover as a function of amount of snow
	if(m_snowCoverCoef1 == NODATA_VALUE || m_snowCoverCoef2 == NODATA_VALUE)
		PGCommon::getScurveShapeParameter(0.5f, 0.95f, m_snowCover50, 0.95f, &m_snowCoverCoef1, &m_snowCoverCoef2);
  //  if (this->m_lastSWE == NODATA_VALUE) /// moved to initialOutputs()
		//this->m_lastSWE = this->m_swe;
	//If all cells have no snow, snow melt is set to zero.
    //if (m_swe < 0.01)
    //{
    //    if (this->m_lastSWE >= 0.01)
    //    {
    //        for (int rw = 0; rw < this->m_nCells; rw++)
    //            m_SM[rw] = 0.f;
    //    }
    //    this->m_lastSWE = this->m_swe;

    //    return 0;
    //}

	/// adjust melt factor for time of year, i.e., smfac in snom.f
	// which only need to computed once.
	int d = JulianDay(this->m_date);
	float sinv = float(sin(2.f * PI / 365.f * (d - 81.f)));
	float cmelt = (m_csnow6 + m_csnow12) / 2.f + (m_csnow6 - m_csnow12) / 2.f * sinv;
#pragma omp parallel for
    for (int rw = 0; rw < m_nCells; rw++)
    {
		/// estimate snow pack temperature
		m_packT[rw] = m_packT[rw] * (1 - m_lagSnow) + m_tMean[rw] * m_lagSnow;
		/// calculate snow fall
        m_SA[rw] = m_SA[rw] + m_SR[rw] - m_SE[rw];
        if (m_tMean[rw] < m_tsnow) /// precipitation will be snow
		{
			m_SA[rw] += m_kblow * m_Pnet[rw];
			m_Pnet[rw] *= (1.f - m_kblow); 
		}

        if (m_SA[rw] < 0.01) 
			m_SM[rw] = 0.f;
        else
        {
            float dt = m_tMax[rw] - m_t0;
            if (dt < 0)
				m_SM[rw] = 0.f;  //if temperature is lower than t0, the snowmelt is 0.
            else
            {
                //calculate using eq. 1:2.5.2 SWAT p58
                m_SM[rw] = cmelt * ((m_packT[rw] + m_tMax[rw]) / 2.f - m_t0);
				// adjust for areal extent of snow cover
				float snowCoverFrac = 0.f; //fraction of HRU area covered with snow
				if (m_SA[rw] < m_snowCoverMax)
				{
					float xx = m_SA[rw] / m_snowCoverMax;
					snowCoverFrac = xx / (xx + exp(m_snowCoverCoef1 = m_snowCoverCoef2 * xx));
				}
				else
					snowCoverFrac = 1.f;
				m_SM[rw] *= snowCoverFrac;
				if(m_SM[rw] < 0.f) m_SM[rw] = 0.f;
				if(m_SM[rw] > m_SA[rw]) m_SM[rw] = m_SA[rw];
				m_SA[rw] -= m_SM[rw];
				m_Pnet[rw] += m_SM[rw];
				if(m_Pnet[rw] < 0.f) m_Pnet[rw] = 0.f;
            }
        }
    }
    //this->m_lastSWE = this->m_swe;
    return 0;
}

bool SNO_SP::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
        throw ModelException(MID_SNO_SP, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    if (this->m_nCells != n)
    {
        if (this->m_nCells <= 0) this->m_nCells = n;
        else
            throw ModelException(MID_SNO_SP, "CheckInputSize", "Input data for " + string(key) +
                                                               " is invalid. All the input data should have same size.");
    }
    return true;
}

void SNO_SP::SetValue(const char *key, float data)
{
    string s(key);
    if (StringMatch(s, VAR_K_BLOW)) this->m_kblow = data;
    else if (StringMatch(s, VAR_T0)) this->m_t0 = data;
    else if (StringMatch(s, VAR_T_SNOW)) this->m_tsnow = data;
    //else if (StringMatch(s, VAR_SWE)) this->m_swe = data;
    //else if (StringMatch(s, VAR_SWE0)) this->m_swe0 = data;
    else if (StringMatch(s, VAR_LAG_SNOW)) this->m_lagSnow = data;
    else if (StringMatch(s, VAR_C_SNOW6)) this->m_csnow6 = data;
    else if (StringMatch(s, VAR_C_SNOW12)) this->m_csnow12 = data;
	else if (StringMatch(s, VAR_SNOCOVMX)) this->m_snowCoverMax = data;
	else if (StringMatch(s, VAR_SNO50COV)) this->m_snowCover50 = data;
    else if (StringMatch(s, VAR_OMP_THREADNUM)) omp_set_num_threads((int) data);
    else
        throw ModelException(MID_SNO_SP, "SetValue", "Parameter " + s
                                                     +
                                                     " does not exist in current module. Please contact the module developer.");
}

void SNO_SP::Set1DData(const char *key, int n, float *data)
{
    //check the input data
    string s(key);
    this->CheckInputSize(key, n);
    if (StringMatch(s, VAR_TMEAN)) this->m_tMean = data;
    else if (StringMatch(s, VAR_TMAX)) this->m_tMax = data;
    else if (StringMatch(s, VAR_NEPR)) this->m_Pnet = data;
    //else if (StringMatch(s, VAR_SNRD)) this->m_SR = data;
    //else if (StringMatch(s, VAR_SNSB)) this->m_SE = data;
    else
        throw ModelException(MID_SNO_SP, "Set1DData", "Parameter " + s +
                                                      " does not exist in current module. Please contact the module developer.");
}

void SNO_SP::Get1DData(const char *key, int *n, float **data)
{
    initialOutputs();
    string s(key);
    if (StringMatch(s, VAR_SNME)) *data = this->m_SM;
    else if (StringMatch(s, VAR_SNAC)) *data = this->m_SR;
    else
        throw ModelException(MID_SNO_SP, "Get1DData", "Result " + s
                                                      +
                                                      " does not exist in current module. Please contact the module developer.");
    *n = this->m_nCells;
}
