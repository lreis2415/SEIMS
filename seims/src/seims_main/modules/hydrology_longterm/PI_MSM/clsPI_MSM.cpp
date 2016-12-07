#include "clsPI_MSM.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "api.h"
#include "ClimateParams.h"
#include "util.h"

#define _USE_MATH_DEFINES

#include <sstream>
#include <fstream>
#include <math.h>
#include <cmath>
#include <time.h>
#include <omp.h>

clsPI_MSM::clsPI_MSM(void) : m_nCells(-1), m_Pi_b(-1.f), m_dateLastTimeStep(-1), m_Init_IS(0.f), 
	m_netPrecipitation(NULL), m_evaporation(NULL),m_interceptionLoss(NULL), m_st(NULL)
{
}

clsPI_MSM::~clsPI_MSM(void)
{
    if (this->m_interceptionLoss != NULL) Release1DArray(this->m_interceptionLoss);
    if (this->m_st != NULL) Release1DArray(this->m_st);
    if (this->m_netPrecipitation != NULL) Release1DArray(this->m_netPrecipitation);
    if (this->m_evaporation != NULL) Release1DArray(this->m_evaporation);
}

void clsPI_MSM::Set1DData(const char *key, int nRows, float *data)
{
    this->CheckInputSize(key, nRows);

    string s(key);
    if (StringMatch(s, VAR_PCP))
        m_P = data;
    else if (StringMatch(s, VAR_PET))
        m_PET = data;
    else if (StringMatch(s, VAR_INTERC_MAX))
        m_maxSt = data;
    else if (StringMatch(s, VAR_INTERC_MIN))
        m_minSt = data;
    else
        throw ModelException(MID_PI_MSM, "Set1DData", "Parameter " + s + " does not exist.");
}

void clsPI_MSM::SetValue(const char *key, float data)
{
    string s(key);
    if (StringMatch(s, VAR_PI_B)) this->m_Pi_b = data;
    else if (StringMatch(s, VAR_INIT_IS)) this->m_Init_IS = data;
    else if (StringMatch(s, VAR_OMP_THREADNUM)) omp_set_num_threads((int) data);
    else
        throw ModelException(MID_PI_MSM, "SetValue", "Parameter " + s + " does not exist.");
}

void clsPI_MSM::Get1DData(const char *key, int *nRows, float **data)
{
	initialOutputs();
    string s = key;
    if (StringMatch(s, VAR_INLO))
        *data = m_interceptionLoss;
    else if (StringMatch(s, VAR_INET))
        *data = m_evaporation;
	else if (StringMatch(s, VAR_CANSTOR))
		*data = m_st;
    else if (StringMatch(s, VAR_NEPR))
        *data = m_netPrecipitation;
    else
        throw ModelException(MID_PI_MSM, "Get1DData", "Result " + s + " does not exist.");
    *nRows = this->m_nCells;
}

void clsPI_MSM::initialOutputs()
{
	if(this->m_st == NULL)
		Initialize1DArray(m_nCells, m_st, m_Init_IS);
	if(this->m_evaporation == NULL)
		Initialize1DArray(m_nCells, m_evaporation, 0.f);
	if(this->m_netPrecipitation == NULL)
		Initialize1DArray(m_nCells, m_netPrecipitation, 0.f);
	if(this->m_interceptionLoss == NULL)
		Initialize1DArray(m_nCells, m_interceptionLoss, 0.f);
}

int clsPI_MSM::Execute()
{
    //check input data
    CheckInputData();
	/// initialize outputs
	initialOutputs();

    int julian = JulianDay(m_date);
#pragma omp parallel for
    for (int i = 0; i < this->m_nCells; i++)
    {
        if (m_P[i] > 0.f)
        {
            //interception storage capacity
            float degree = 2.f * PI * (julian - 87.f) / 365.f;
			/// For water, min and max are both 0, then no need for specific handling.
            float min = m_minSt[i];
            float max = m_maxSt[i];
            float capacity = min + (max - min) * pow(0.5f + 0.5f * sin(degree), m_Pi_b);

            //interception, currently, m_st[i] is storage of (t-1) time step 
            float availableSpace = capacity - m_st[i];
            if (availableSpace < 0)
                availableSpace = 0.f;

            if (availableSpace < m_P[i])
                m_interceptionLoss[i] = availableSpace;
            else
                m_interceptionLoss[i] = m_P[i];

            //net precipitation
            m_netPrecipitation[i] = m_P[i] - m_interceptionLoss[i];
            m_st[i] += m_interceptionLoss[i];
        }
        else
        {
            m_interceptionLoss[i] = 0.f;
            m_netPrecipitation[i] = 0.f;
        }
        //evaporation
        if (m_st[i] > m_PET[i])
            m_evaporation[i] = m_PET[i];
        else
            m_evaporation[i] = m_st[i];
        m_st[i] -= m_evaporation[i];
    }
    return 0;
}

bool clsPI_MSM::CheckInputData()
{
    if (this->m_date < 0)
        throw ModelException(MID_PI_MSM, "CheckInputData", "You have not set the time.");

    if (m_nCells <= 0)
        throw ModelException(MID_PI_MSM, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");

    if (this->m_P == NULL)
        throw ModelException(MID_PI_MSM, "CheckInputData", "The precipitation data can not be NULL.");

    if (this->m_PET == NULL)
        throw ModelException(MID_PI_MSM, "CheckInputData", "The PET data can not be NULL.");

    if (this->m_maxSt == NULL)
        throw ModelException(MID_PI_MSM, "CheckInputData",
                             "The maximum interception storage capacity can not be NULL.");

    if (this->m_minSt == NULL)
        throw ModelException(MID_PI_MSM, "CheckInputData",
                             "The minimum interception storage capacity can not be NULL.");

    if (this->m_Pi_b > 1.5 || this->m_Pi_b < 0.5)
        throw ModelException(MID_PI_MSM, "CheckInputData",
                             "The interception storage capacity exponent can not be " + ValueToString(this->m_Pi_b) +
                             ". It should between 0.5 and 1.5.");
    if (this->m_Init_IS > 1.f || this->m_Init_IS < 0.f)
        throw ModelException(MID_PI_MSM, "CheckInputData",
                             "The Initial interception storage can not be " + ValueToString(this->m_Init_IS) +
                             ". It should between 0 and 1.");
    return true;
}

bool clsPI_MSM::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
        throw ModelException(MID_PI_MSM, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    if (this->m_nCells != n)
    {
        if (this->m_nCells <= 0) this->m_nCells = n;
        else
            throw ModelException(MID_PI_MSM, "CheckInputSize", "Input data for " + string(key) +
                                                               " is invalid. All the input data should have same size.");
    }
    return true;
}
