/*!
 * \brief
 * \author Alex Storey, Junzhi Liu
 * \date May 2013
 *
 * 
 */
#include "Interception.h"
#include "MetadataInfo.h"
#include "api.h"
#include "ClimateParams.h"

#define _USE_MATH_DEFINES

#include <sstream>
#include <fstream>
#include <math.h>
#include <cmath>
#include <time.h>
#include "util.h"
#include "ModelException.h"
#include "omp.h"
#include <iostream>

//! Constructor
clsPI_STORM::clsPI_STORM(void) : m_s0(NULL), m_P(NULL), m_maxInterception(NULL),
                                 m_minInterception(NULL), m_interceptionLast(NULL), m_interceptionLoss(NULL)
{
    m_date = -1;
    m_nCells = -1;
    m_dt = -1;
    this->m_Pi_b = 1.35f;
    //this->m_K_pet = -1.0f;
    this->m_dateLastTimeStep = -1;
    this->m_Init_IS = 0.0f;
}

//! Destructor
clsPI_STORM::~clsPI_STORM(void)
{
    if (this->m_interceptionLoss != NULL) delete[] this->m_interceptionLoss;
    if (this->m_interceptionLast != NULL) delete[] this->m_interceptionLast;
    if (this->m_netPrecipitation != NULL) delete[] this->m_netPrecipitation;
    //if(this->m_evaporation != NULL) delete [] this->m_evaporation;
}

void clsPI_STORM::SetDate(time_t date)
{
    this->m_dateLastTimeStep = this->m_date;
    this->m_date = date;
}

void clsPI_STORM::Set1DData(const char *key, int n, float *data)
{
    this->CheckInputSize(key, n);

    string s(key);
    if (StringMatch(s, VAR_PCP))
    {
        this->m_P = data;

        //Output1DArray(m_nCells, m_P, "f:\\p.txt");
    }
    else if (StringMatch(key, VAR_SLOPE))
        m_s0 = data;
        //else if(StringMatch(s,"D_PET"))			this->m_PET = data;
    else if (StringMatch(s, VAR_INTERC_MAX))
    {
        this->m_maxInterception = data;
    }
    else if (StringMatch(s, VAR_INTERC_MIN)) this->m_minInterception = data;
    else
        throw ModelException(MID_PI_STORM, "Set1DData", "Parameter " + s +
                                                        " does not exist in current module. Please contact the module developer.");
}

void clsPI_STORM::SetValue(const char *key, float data)
{
    string s(key);
    if (StringMatch(s, VAR_PI_B))
        this->m_Pi_b = data;
    else if (StringMatch(s, Tag_HillSlopeTimeStep))
        m_dt = data;
        //else if(StringMatch(s,"K_pet"))			this->m_K_pet = data;
    else if (StringMatch(s, VAR_INIT_IS))
        this->m_Init_IS = data;
    else if (StringMatch(s, VAR_OMP_THREADNUM))
    {
        omp_set_num_threads((int) data);
    }
    else
        throw ModelException(MID_PI_STORM, "SetValue", "Parameter " + s +
                                                       " does not exist in current module. Please contact the module developer.");
}

void clsPI_STORM::Get1DData(const char *key, int *n, float **data)
{
	initialOutputs();
    string s(key);
    if (StringMatch(s, VAR_INLO)) *data = this->m_interceptionLoss;
    else if (StringMatch(s, VAR_NEPR)) *data = this->m_netPrecipitation;
    else
        throw ModelException(MID_PI_STORM, "Get1DData", "Result " + s + " does not exist.");

    *n = this->m_nCells;
}

//! 
int clsPI_STORM::Execute()
{
#pragma omp parallel for
    for (int i = 0; i < m_nCells; ++i)
    {
        //m_P[i] = m_P[i] * m_dt / 3600;
        if (m_P[i] > 0.f)
            m_P[i] = m_P[i] * m_dt / 3600.f * cos(atan(m_s0[i]));
        else
            m_P[i] = 0.f;
    }

    //initial the state variable
    if (this->m_interceptionLast == NULL)
    {
        CheckInputData();

        m_interceptionLast = new float[m_nCells];
        //this->m_evaporation = new float[m_nCells];
        this->m_interceptionLoss = new float[m_nCells];
        this->m_netPrecipitation = new float[m_nCells];
#pragma omp parallel for
        for (int i = 0; i < this->m_nCells; i++)
        {
            m_interceptionLast[i] = this->m_Init_IS;
        }
    }
    int jday = JulianDay(this->m_date);
#pragma omp parallel for
    for (int i = 0; i < this->m_nCells; i++)
    {
        //float PE = this->m_PET[i] * this->m_K_pet;

        //evaporation
        //if(this->m_interceptionLast[i] <= 0)			this->m_evaporation[i] = 0.0f;
        //else if(this->m_interceptionLast[i] > PE)	this->m_evaporation[i] = PE;
        //else														this->m_evaporation[i] = this->m_interceptionLast[i];

        //interception storage capacity
        //int julian = 100;
        double degree = 2 * PI * (jday - 87) / 365.f;
        float min = this->m_minInterception[i];
        float max = this->m_maxInterception[i];
        double capacity = min + (max - min) * pow(0.5 + 0.5 * sin(degree), double(this->m_Pi_b));

        //interception
        //double availableSpace = capacity - this->m_interceptionLast[i] + this->m_evaporation[i];
        double availableSpace = capacity - this->m_interceptionLast[i];
        if (availableSpace < this->m_P[i])
            this->m_interceptionLoss[i] = float(availableSpace);
        else
            this->m_interceptionLoss[i] = this->m_P[i];

        //net precipitation
        this->m_netPrecipitation[i] = m_P[i] - m_interceptionLoss[i];

        //override the interception storage of last time step
        //this->m_interceptionLast[i] += this->m_interceptionLoss[i] - this->m_evaporation[i];
        m_interceptionLast[i] += m_interceptionLoss[i];
    }
    return 0;
}

bool clsPI_STORM::CheckInputData()
{
    if (this->m_date == -1)
    {
        throw ModelException(MID_PI_STORM, "CheckInputData", "You have not set the time.");
        return false;
    }

    if (m_nCells <= 0)
    {
        throw ModelException(MID_PI_STORM, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
        return false;
    }

    if (this->m_P == NULL)
    {
        throw ModelException(MID_PI_STORM, "CheckInputData", "The precipitation data can not be NULL.");
        return false;
    }

    //if(this->m_PET == NULL)
    //{
    //	throw ModelException(MID_PI_STORM,"CheckInputData","The PET data can not be NULL.");
    //	return false;
    //}

    if (this->m_maxInterception == NULL)
    {
        throw ModelException(MID_PI_STORM, "CheckInputData",
                             "The maximum interception storage capacity can not be NULL.");
        return false;
    }

    if (this->m_minInterception == NULL)
    {
        throw ModelException(MID_PI_STORM, "CheckInputData",
                             "The minimum interception storage capacity can not be NULL.");
        return false;
    }

    //if(this->m_K_pet > 1.3 || this->m_K_pet < 0.7)
    //{
    //	throw ModelException(MID_PI_STORM,"CheckInputData","The correction factor of PET can not be " + clsPI_MSM::toString(this->m_K_pet) + ". It should between 0.7 and 1.3.");
    //	return false;
    //}

    if (this->m_Pi_b > 1.5 || this->m_Pi_b < 0.5)
    {
        throw ModelException(MID_PI_STORM, "CheckInputData",
                             "The interception storage capacity exponent can not be " + ValueToString(this->m_Pi_b) +
                             ". It should between 0.5 and 1.5.");
        return false;
    }

    if (this->m_Init_IS > 1 || this->m_Init_IS < 0)
    {
        throw ModelException(MID_PI_STORM, "CheckInputData",
                             "The Initial interception storage can not be " + ValueToString(this->m_Init_IS) +
                             ". It should between 0 and 1.");
        return false;
    }

    return true;
}

bool clsPI_STORM::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        throw ModelException(MID_PI_STORM, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (this->m_nCells != n)
    {
        if (this->m_nCells <= 0) this->m_nCells = n;
        else
        {
            throw ModelException(MID_PI_STORM, "CheckInputSize", "Input data for " + string(key) +
                                                                 " is invalid. All the input data should have same size.");
            ostringstream oss;
            oss << "Input data for " + string(key) << " is invalid with size: " << n << ". The origin size is " <<
            m_nCells << ".\n";
            throw ModelException(MID_PI_STORM, "CheckInputSize", oss.str());
        }
    }
    return true;
}



