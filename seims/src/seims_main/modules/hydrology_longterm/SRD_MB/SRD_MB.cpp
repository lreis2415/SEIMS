#include "SRD_MB.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include <math.h>
#include <cmath>
#include "util.h"

#include <omp.h>

SRD_MB::SRD_MB(void)
{
    // set default values for member variables
    this->m_Date = -1;
    this->m_t_wind = -99.0f;
    this->m_ut0 = -99.0f;
    this->m_u0 = -99.0f;
    this->m_kblow = -99.0f;
    this->m_shc_crop = -99.0f;
    this->m_k_slope = -99.0f;
    this->m_k_curvature = -99.0f;
    this->m_nCells = -1;
    this->m_wsSize = -1;
    this->m_swe = -99.0f;
    this->m_swe0 = -99.0f;
    this->m_t0 = -99.0f;
    this->m_lastSWE = -99.0f;
    this->m_tsnow = -99.0f;

    this->m_ws = NULL;
    this->m_Pnet = NULL;
    this->m_SA = NULL;
    this->m_slope_wind = NULL;
    this->m_curva_wind = NULL;
    this->m_shc = NULL;
    this->m_tMin = NULL;
    this->m_tMax = NULL;

    this->m_w = NULL;
    this->m_SR = NULL;
    this->m_wt = NULL;

    m_isInitial = true;
}

SRD_MB::~SRD_MB(void)
{
    //// cleanup
    if (this->m_w != NULL) delete[] this->m_w;
    if (this->m_wt != NULL) delete[] this->m_wt;
    if (this->m_SR != NULL) delete[] this->m_SR;
}

bool SRD_MB::CheckInputData(void)
{
    if (this->m_Date <= 0) throw ModelException("SRD_MB", "CheckInputData", "You have not set the time.");
    if (m_nCells <= 0)
        throw ModelException("SRD_MB", "CheckInputData", "The dimension of the input data can not be less than zero.");
    if (this->m_tMin == NULL)
        throw ModelException("SRD_MB", "CheckInputData", "The min temperature data can not be NULL.");
    if (this->m_tMax == NULL)
        throw ModelException("SRD_MB", "CheckInputData", "The max temperature data can not be NULL.");
    if (this->m_ws == NULL) throw ModelException("SRD_MB", "CheckInputData", "The wind speed data can not be NULL.");
    if (this->m_kblow == -99)
        throw ModelException("SRD_MB", "CheckInputData",
                             "The fraction coefficient of snow blowing into or out of the watershed can not be NULL.");
    if (this->m_Pnet == NULL)
        throw ModelException("SRD_MB", "CheckInputData", "The net precipitation data can not be NULL.");
    if (this->m_curva_wind == NULL)
        throw ModelException("SRD_MB", "CheckInputData", "The curvature wind data can not be NULL.");
    if (this->m_slope_wind == NULL)
        throw ModelException("SRD_MB", "CheckInputData", "The slope wind data can not be NULL.");
    if (this->m_shc == NULL)
        throw ModelException("SRD_MB", "CheckInputData", "The snow hold capacity data can not be NULL.");
    if (this->m_SA == NULL)
        throw ModelException("SRD_MB", "CheckInputData", "The snow hold capacity data can not be NULL.");

    if (this->m_tsnow == -99) throw ModelException("SRD_MB", "CheckInputData", "The t_snow can not be -99.");
    if (this->m_ut0 == -99) throw ModelException("SRD_MB", "CheckInputData", "The ut0 can not be -99.");
    if (this->m_u0 == -99) throw ModelException("SRD_MB", "CheckInputData", "The u0 can not be -99.");
    if (this->m_t_wind == -99) throw ModelException("SRD_MB", "CheckInputData", "The t_wind can not be -99.");
    if (this->m_shc_crop == -99) throw ModelException("SRD_MB", "CheckInputData", "The shc_crop can not be -99.");
    if (this->m_k_slope == -99) throw ModelException("SRD_MB", "CheckInputData", "The k_slope can not be -99.");
    if (this->m_k_curvature == -99) throw ModelException("SRD_MB", "CheckInputData", "The k_curvature can not be -99.");
    if (this->m_swe == -99) throw ModelException("SRD_MB", "CheckInputData", "The swe can not be -99.");
    if (this->m_swe0 == -99) throw ModelException("SRD_MB", "CheckInputData", "The swe0 can not be -99.");

    if (this->m_wsSize <= 0)
        throw ModelException("SRD_MB", "CheckInputData",
                             "The dimension of the wind speed data can not be less than zero.");

    return true;
}

void SRD_MB::initialOutputs()
{
    if (m_nCells <= 0)
        throw ModelException("SRD_MB", "CheckInputData", "The dimension of the input data can not be less than zero.");
    if (m_SR == NULL)
    {
        m_SR = new float[this->m_nCells];
        for (int rw = 0; rw < this->m_nCells; rw++)
        {
            m_SR[rw] = 0.0f;
        }
    }
}

int SRD_MB::Execute()
{
    this->CheckInputData();

    this->initialOutputs();

    //the first time
    if (m_isInitial)
    {
        int count = 0;
        for (int i = 0; i < this->m_nCells; i++)
        {
            if ((this->m_tMin[i] + this->m_tMax[i]) / 2 < this->m_tsnow)
            {
                this->m_SA[i] = this->m_swe0;
                count++;
            }    //winter
            else this->m_SA[i] = 0.0f;                        // other seasons
        }

        m_swe = this->m_swe0 * count / this->m_nCells;
        m_isInitial = false;
    }

    if (this->m_lastSWE == -99.0f) this->m_lastSWE = this->m_swe;
    if (m_swe < 0.01)    //all cells have not snow, so snow redistribution is 0.
    {
        if (this->m_lastSWE >= 0.01)
        {
            for (int rw = 0; rw < this->m_nCells; rw++)
                m_SR[rw] = 0.0f;
        }

        this->m_lastSWE = this->m_swe;
        return true;
    }


    //if some cells have snow, start to calculate snow redistribution.
    if (m_w == NULL) m_w = new float[this->m_nCells];
    if (m_wt == NULL)   //get wt. wt is constant for the watershed, so save it in m_wt to save time.
    {
        m_wt = new float[this->m_nCells];
        for (int rw = 0; rw < this->m_nCells; rw++)
            m_wt[rw] = 1.0f + 1 / this->m_k_slope * this->m_slope_wind[rw] +
                       1 / this->m_k_curvature * this->m_curva_wind[rw];
    }

    float totalW = 0.0f;
    for (int rw = 0; rw < this->m_nCells; rw++)
    {
        float wl = 1.0f;
        float shc = this->m_shc[rw] * 1000;
        if (this->m_swe < shc)
        {
            wl = shc / this->m_shc_crop + this->m_swe * (1 - shc / this->m_shc_crop) / shc;
        }

        m_w[rw] = m_wt[rw] * wl;    //save w in the m_w array, this save time for following loop.
        totalW += m_w[rw];
    }

    float weight = this->m_nCells / totalW;        //weight
    float u = 0.0f;                                    //average wind speed
    for (int rw = 0; rw < this->m_wsSize; rw++) u += this->m_ws[rw];
    u /= this->m_wsSize;

    for (int rw = 0; rw < this->m_nCells; rw++)
    {
        float snow = this->m_SA[rw];
        float tmean = (this->m_tMin[rw] + this->m_tMax[rw]) /
                      2; //if the temperature is higher than t0, the redistribution is 0
        if (tmean < this->m_tsnow) snow += (1 + this->m_kblow) * this->m_Pnet[rw];
        if (snow <
            0.01)            // if snow is lower than a very small positive value (0.01), consider it is equal to 0.
        {
            this->m_SR[rw] = 0.0f;
            continue;
        }

        if (tmean >= this->m_t0)
        {
            this->m_SR[rw] = 0.0f;
            continue;
        }

        float wr = m_w[rw] * weight;
        float sap = m_swe * wr;
        float scp = sap - this->m_SA[rw];

        float ut = float(this->m_ut0 + 0.0033 * pow((this->m_tMin[rw] + this->m_tMax[rw]) / 2 - this->m_t_wind, 2.0f));
        float ww = 0.0f;
        if (u - ut >= this->m_u0) ww = 1.0f;
        if (u - ut >= 0 && u - ut < this->m_u0) ww = (u - ut) / this->m_u0;

        float sr = scp * ww;

        if (abs(sr) <= snow) this->m_SR[rw] = sr;
        else this->m_SR[rw] = snow;
    }

    this->m_lastSWE = this->m_swe;
    return 0;
}

bool SRD_MB::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        throw ModelException("SRD_MB", "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (this->m_nCells != n)
    {
        if (this->m_nCells <= 0) this->m_nCells = n;
        else
        {
            throw ModelException("SRD_MB", "CheckInputSize", "Input data for " + string(key) +
                                                             " is invalid. All the input data should have same size.");
            return false;
        }
    }

    return true;
}

void SRD_MB::SetValue(const char *key, float data)
{
    string s(key);
    if (StringMatch(s, "ThreadNum"))
    {
        omp_set_num_threads((int) data);
    }
    else if (StringMatch(s, "K_blow")) this->m_kblow = data;
    else if (StringMatch(s, "shc_crop")) this->m_shc_crop = data;
    else if (StringMatch(s, "k_slope")) this->m_k_slope = data;
    else if (StringMatch(s, "k_curvature")) this->m_k_curvature = data;
    else if (StringMatch(s, "ut0")) this->m_ut0 = data;
    else if (StringMatch(s, "u0")) this->m_u0 = data;
    else if (StringMatch(s, "t_wind")) this->m_t_wind = data;
    else if (StringMatch(s, "SWE")) this->m_swe = data;
    else if (StringMatch(s, "swe0")) this->m_swe0 = data;
    else if (StringMatch(s, "T0")) this->m_t0 = data;
    else if (StringMatch(s, "T_snow")) this->m_tsnow = data;
    else
        throw ModelException("SRD_MB", "SetValue", "Parameter " + s +
                                                   " does not exist in SRD_MB method. Please contact the module developer.");

}

void SRD_MB::Set1DData(const char *key, int n, float *data)
{
    //check the input data
    string s(key);
    if (StringMatch(s, "T_WS"))
    {
        this->m_ws = data;
        this->m_wsSize = n;
        return;
    }

    this->CheckInputSize(key, n);

    if (StringMatch(s, "slope_wind")) this->m_slope_wind = data;
    else if (StringMatch(s, "curva_wind")) this->m_curva_wind = data;
    else if (StringMatch(s, "shc")) this->m_shc = data;
    else if (StringMatch(s, "D_NEPR")) this->m_Pnet = data;
    else if (StringMatch(s, "D_SNAC")) this->m_SA = data;
    else if (StringMatch(s, "D_TMIN")) this->m_tMin = data;
    else if (StringMatch(s, "D_TMAX")) this->m_tMax = data;
    else
        throw ModelException("SRD_MB", "SetValue", "Parameter " + s +
                                                   " does not exist in SRD_MB method. Please contact the module developer.");

}

void SRD_MB::Get1DData(const char *key, int *n, float **data)
{
    string s(key);
    if (StringMatch(s, "SNRD"))
    {
        *data = this->m_SR;
    }
    else
        throw ModelException("SRD_MB", "getResult", "Result " + s +
                                                    " does not exist in SRD_MB method. Please contact the module developer.");

    *n = this->m_nCells;
}

