#include "SSM_PE.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "util.h"

#include <omp.h>

SSM_PE::SSM_PE(void)
{
    // set default values for member variables
    this->m_nCells = -1;

    this->m_t0 = NODATA_VALUE;
    this->m_ksubli = NODATA_VALUE;
    this->m_kblow = NODATA_VALUE;
    this->m_swe = NODATA_VALUE;
    this->m_swe0 = NODATA_VALUE;
    this->m_lastSWE = NODATA_VALUE;
    this->m_tsnow = NODATA_VALUE;

    this->m_PET = NULL;
    this->m_Pnet = NULL;
    this->m_SA = NULL;
    this->m_SR = NULL;
    this->m_SE = NULL;
    this->m_tMean = NULL;
    m_isInitial = true;
}

SSM_PE::~SSM_PE(void)
{
    //// cleanup
    if (this->m_SE != NULL) delete[] this->m_SE;
}

bool SSM_PE::CheckInputData(void)
{
    if (m_nCells <= 0)
        throw ModelException(MID_SSM_PE, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    if (this->m_kblow == NODATA_VALUE)
        throw ModelException(MID_SSM_PE, "CheckInputData",
                             "The fraction coefficient of snow blowing into or out of the watershed can not be NULL.");
    if (this->m_Pnet == NULL)
        throw ModelException(MID_SSM_PE, "CheckInputData", "The net precipitation data can not be NULL.");
    if (this->m_PET == NULL) throw ModelException(MID_SSM_PE, "CheckInputData", "The PET data can not be NULL.");
    if (this->m_SA == NULL)
        throw ModelException(MID_SSM_PE, "CheckInputData", "The snow accumulation data can not be NULL.");
    // Currently, no module's outputs have SNRD, this should be on TODO LIST. LJ
    //if(this->m_SR == NULL)			throw ModelException(MID_SSM_PE,"CheckInputData","The snow redistribution data can not be NULL.");
    if (this->m_swe == NODATA_VALUE) throw ModelException(MID_SSM_PE, "CheckInputData", "The swe can not be NODATA.");
    if (this->m_swe0 == NODATA_VALUE) throw ModelException(MID_SSM_PE, "CheckInputData", "The swe0 can not be NODATA.");
    if (this->m_ksubli == NODATA_VALUE) throw ModelException(MID_SSM_PE, "CheckInputData", "The k_subli can not be NODATA.");
    if (this->m_tMean == NULL)
        throw ModelException(MID_SSM_PE, "CheckInputData", "The mean air temperature data can not be NULL.");
    if (this->m_t0 == NODATA_VALUE)
        throw ModelException(MID_SSM_PE, "CheckInputData", "The Snowmelt temperature can not be NODATA.");
    if (this->m_tsnow == NODATA_VALUE)
        throw ModelException(MID_SSM_PE, "CheckInputData", "The snow fall temperature can not be NODATA.");
    return true;
}

void SSM_PE::initialOutputs()
{
    if (m_nCells <= 0)
        throw ModelException(MID_SSM_PE, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    if (m_SE == NULL)
    {
        m_SE = new float[this->m_nCells];
        for (int rw = 0; rw < this->m_nCells; rw++)
            m_SE[rw] = 0.0f;
    }
}

int SSM_PE::Execute()
{
    this->CheckInputData();

    this->initialOutputs();
    if (m_SR == NULL)  /// the initialization should be removed when snow redistribution module is accomplished. LJ
    {
        m_SR = new float[m_nCells];
        for (int i = 0; i < this->m_nCells; i++)
            m_SR[i] = 0.f;
    }
    if (m_isInitial)
    {
        int count = 0;
        for (int i = 0; i < this->m_nCells; i++)
        {
            if (this->m_tMean[i] < this->m_tsnow)
            {
                this->m_SA[i] = this->m_swe0;
                count++;
            }    //winter
            else this->m_SA[i] = 0.0f;                        // other seasons
        }

        m_swe = this->m_swe0 * count / this->m_nCells;
        m_isInitial = false;
    }

    if (this->m_lastSWE == NODATA_VALUE) this->m_lastSWE = this->m_swe;
    if (m_swe < 0.01)    //all cells have not snow, so snow sublimation is 0.
    {
        if (this->m_lastSWE >= 0.01)
        {
            for (int rw = 0; rw < this->m_nCells; rw++)
                m_SE[rw] = 0.0f;
        }
        this->m_lastSWE = this->m_swe;
        return true;
    }

    for (int rw = 0; rw < this->m_nCells; rw++)
    {
        float snow = this->m_SA[rw] + this->m_SR[rw];
        float tmean = this->m_tMean[rw];
        if (tmean < this->m_tsnow) snow += (1 + this->m_kblow) * this->m_Pnet[rw];
        if (snow < 0.01) this->m_SE[rw] = 0.0f;
        else
        {
            if (tmean >= this->m_t0) this->m_SE[rw] = 0.0f;   //if temperature is higher than t0, the sublimation is 0.
            else
            {
                float se = this->m_ksubli * this->m_PET[rw];
                this->m_SE[rw] = min(snow, se);
            }
        }
    }
    this->m_lastSWE = this->m_swe;
    return true;
    return 0;
}

bool SSM_PE::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        throw ModelException(MID_SSM_PE, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (this->m_nCells != n)
    {
        if (this->m_nCells <= 0) this->m_nCells = n;
        else
        {
            throw ModelException(MID_SSM_PE, "CheckInputSize", "Input data for " + string(key) +
                                                               " is invalid. All the input data should have same size.");
            return false;
        }
    }
    return true;
}

void SSM_PE::SetValue(const char *key, float data)
{
    string s(key);
    if (StringMatch(s, VAR_OMP_THREADNUM))
    {
        omp_set_num_threads((int) data);
    }
    else if (StringMatch(s, VAR_K_BLOW)) this->m_kblow = data;
    else if (StringMatch(s, VAR_K_SUBLI)) this->m_ksubli = data;
    else if (StringMatch(s, VAR_SWE)) this->m_swe = data;
    else if (StringMatch(s, VAR_SWE0)) this->m_swe0 = data;
    else if (StringMatch(s, VAR_T0)) this->m_t0 = data;
    else if (StringMatch(s, VAR_T_SNOW)) this->m_tsnow = data;
    else
        throw ModelException(MID_SSM_PE, "SetValue", "Parameter " + s
                                                     +
                                                     " does not exist in current module. Please contact the module developer.");

}

void SSM_PE::Set1DData(const char *key, int n, float *data)
{
    //check the input data
    string s(key);

    this->CheckInputSize(key, n);

    if (StringMatch(s, VAR_PET)) this->m_PET = data;
    else if (StringMatch(s, VAR_NEPR)) this->m_Pnet = data;
    else if (StringMatch(s, VAR_SNAC)) this->m_SA = data;
    else if (StringMatch(s, VAR_SNRD)) this->m_SR = data;
    else if (StringMatch(s, VAR_TMEAN)) this->m_tMean = data;
    else
        throw ModelException(MID_SSM_PE, "Set1DData", "Parameter " + s +
                                                      " does not exist in current module. Please contact the module developer.");
}

void SSM_PE::Get1DData(const char *key, int *n, float **data)
{
    initialOutputs();
    string s(key);
    if (StringMatch(s, VAR_SNSB))
        *data = this->m_SE;
    else
        throw ModelException(MID_SSM_PE, "Get1DData", "Result " + s +
                                                      " does not exist in current module. Please contact the module developer.");
    *n = this->m_nCells;
}

