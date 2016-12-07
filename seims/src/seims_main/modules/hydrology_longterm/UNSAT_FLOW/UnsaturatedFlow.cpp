// UnsaturatedFlow.cpp : main project file.

#include "UnsaturatedFlow.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "util.h"
#include <omp.h>

UnsaturatedFlow::UnsaturatedFlow(void)
{
    // set default values for member variables

    this->m_FieldCap = NULL;
    this->m_WiltPoint = NULL;
    this->m_EI = NULL;
    this->m_PET = NULL;
    this->m_ED = NULL;
    this->m_Moist = NULL;
    this->m_SoilT = NULL;
    this->m_ForzenT = -99.0f;
    m_cellSize = -1;

    m_infiltration = NULL;
    m_percolation = NULL;
    m_interflow = NULL;

    this->m_D_SOET = NULL;
    m_rootDepth = NULL;
}

UnsaturatedFlow::~UnsaturatedFlow(void)
{
    if (m_D_SOET != NULL) delete[] this->m_D_SOET;
}


//Execute module
int UnsaturatedFlow::Execute()
{
    this->CheckInputData();

    if (m_D_SOET == NULL)
    {
        m_D_SOET = new float[this->m_cellSize];
    }

#pragma omp parallel for
    for (int i = 0; i < this->m_cellSize; i++)
    {
        if (this->m_SoilT[i] <= this->m_ForzenT)    //if the soil temperature is lower than tFrozen, then ES = 0.
        {
            m_D_SOET[i] = 0.0f;
            continue;
        }

        float f0 = this->m_FieldCap[i];
        float w0 = this->m_WiltPoint[i];
        //float rootDepth = this->m_rootDepth[i];
        //float m0 = this->m_Moist[i] * rootDepth + this->m_infiltration[i] - this->m_percolation[i] - this->m_interflow[i];	//mm
        //m0 /= rootDepth;//mm3/mm3

        if (m_Moist[i] >= f0)
            m_D_SOET[i] = this->m_PET[i] - this->m_EI[i] - this->m_ED[i];
        else if (w0 <= m_Moist[i] && m_Moist[i] < f0)
            m_D_SOET[i] = (this->m_PET[i] - this->m_EI[i] - this->m_ED[i]) * (m_Moist[i] - w0) / (f0 - w0);
        else
            m_D_SOET[i] = 0.0f;   //???

        m_D_SOET[i] = max(m_D_SOET[i], 0.0f);

        float availableWater = (m_Moist[i] - m_WiltPoint[i]) * m_rootDepth[i];
        if (m_D_SOET[i] > availableWater)
        {
            m_D_SOET[i] = availableWater;
            m_Moist[i] = m_WiltPoint[i];
        }
        else
            m_Moist[i] -= m_D_SOET[i] / m_rootDepth[i];
    }

    return 0;

}

void UnsaturatedFlow::Get1DData(const char *key, int *nRows, float **data)
{
    string s(key);
    if (StringMatch(s, "SOET"))
    {
        *data = this->m_D_SOET;
    }
    else
        throw ModelException("UnsaturatedFlow", "getResult", "Result " + s +
                                                             " does not exist in UnsaturatedFlow method. Please contact the module developer.");

    *nRows = this->m_cellSize;
}

void UnsaturatedFlow::SetValue(const char *key, float data)
{
    string s(key);
    if (StringMatch(s, "T_Soil")) this->m_ForzenT = data;
    else if (StringMatch(s, "ThreadNum"))
    {
        omp_set_num_threads((int) data);
    }
    else
        throw ModelException("UnsaturatedFlow", "SetValue", "Parameter " + s +
                                                            " does not exist in UnsaturatedFlow method. Please contact the module developer.");
}

void UnsaturatedFlow::Set1DData(const char *key, int nRows, float *data)
{
    string s(key);

    this->CheckInputSize(key, nRows);

    if (StringMatch(s, "Fieldcap")) this->m_FieldCap = data;
    else if (StringMatch(s, "Wiltingpoint")) this->m_WiltPoint = data;
    else if (StringMatch(s, "RootDepth")) this->m_rootDepth = data;
    else if (StringMatch(s, "D_INET")) this->m_EI = data;
    else if (StringMatch(s, "D_PET")) this->m_PET = data;
    else if (StringMatch(s, "D_DEET")) this->m_ED = data;
    else if (StringMatch(s, "D_SOMO")) this->m_Moist = data;
    else if (StringMatch(s, "D_SOTE")) this->m_SoilT = data;
    else if (StringMatch(s, "D_GRRE")) this->m_percolation = data;
    else if (StringMatch(s, "D_INFIL")) this->m_infiltration = data;
    else if (StringMatch(s, "D_SSRU")) this->m_interflow = data;
    else
        throw ModelException("UnsaturatedFlow", "SetValue", "Parameter " + s +
                                                            " does not exist in UnsaturatedFlow method. Please contact the module developer.");

}

string UnsaturatedFlow::toString(float value)
{
    char s[20];
    strprintf(s, 20, "%f", value);
    return string(s);
}

bool UnsaturatedFlow::CheckInputData()
{
    if (m_cellSize <= 0)
        throw ModelException("UnsaturatedFlow", "CheckInputData",
                             "The dimension of the input data can not be less than zero.");

    if (this->m_FieldCap == NULL)
        throw ModelException("UnsaturatedFlow", "CheckInputData", "The Soil field capacity can not be NULL.");
    if (this->m_WiltPoint == NULL)
        throw ModelException("UnsaturatedFlow", "CheckInputData", "The plant wilting point moisture can not be NULL.");

    if (this->m_EI == NULL) throw ModelException("UnsaturatedFlow", "CheckInputData", "The EI can not be NULL.");
    if (this->m_PET == NULL) throw ModelException("UnsaturatedFlow", "CheckInputData", "The PET can not be NULL.");
    if (this->m_ED == NULL) throw ModelException("UnsaturatedFlow", "CheckInputData", "The ED can not be NULL.");
    if (this->m_Moist == NULL)
        throw ModelException("UnsaturatedFlow", "CheckInputData", "The soil moisture can not be NULL.");
    if (this->m_SoilT == NULL) throw ModelException("PER_PI", "CheckInputData", "The soil temerature can not be NULL.");
    if (this->m_ForzenT == -99.0f)
        throw ModelException("PER_PI", "CheckInputData", "The threshold soil freezing temerature can not be NULL.");

    if (this->m_percolation == NULL)
        throw ModelException("SSR_DA", "CheckInputData", "The percolation can not be NULL.");
    if (this->m_infiltration == NULL)
        throw ModelException("SSR_DA", "CheckInputData", "The infiltration can not be NULL.");
    if (this->m_interflow == NULL) throw ModelException("SSR_DA", "CheckInputData", "The interflow can not be NULL.");

    return true;
}

bool UnsaturatedFlow::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        throw ModelException("UnsaturatedFlow", "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (this->m_cellSize != n)
    {
        if (this->m_cellSize <= 0) this->m_cellSize = n;
        else
        {
            throw ModelException("UnsaturatedFlow", "CheckInputSize", "Input data for " + string(key) +
                                                                      " is invalid. All the input data should have same size.");
            return false;
        }
    }

    return true;
}
