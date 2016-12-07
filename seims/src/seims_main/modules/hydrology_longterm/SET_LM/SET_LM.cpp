#include "SET_LM.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "util.h"
#include <omp.h>

#include <iostream>

using namespace std;

SET_LM::SET_LM(void) : m_nSoilLayers(2), m_nCells(-1), m_fc(NULL), m_wp(NULL), m_EI(NULL), m_PET(NULL), m_ED(NULL),
                       m_sm(NULL), m_soilT(NULL),
                       m_frozenT(-99.f), m_rootDepth(NULL), m_soilET(NULL), m_upSoilDepth(200.f)
{
}

SET_LM::~SET_LM(void)
{
    if (m_soilET != NULL)
        delete[] m_soilET;
}


//Execute module
int SET_LM::Execute()
{
    CheckInputData();

    if (m_soilET == NULL)
        m_soilET = new float[m_nCells];

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++)
    {
        m_soilET[i] = 0.0f;
        if (m_soilT[i] <= m_frozenT)    //if the soil temperature is lower than tFrozen, then ES = 0.
            continue;

        float et2d[2];
        float depth[2];
        depth[0] = m_upSoilDepth;
        depth[1] = m_rootDepth[i] - m_upSoilDepth;
        if (depth[1] < 0)
        {
            ostringstream oss;
            oss << "The root depth at cell(" << i << ") is " << m_rootDepth[i] <<
            ", and is less than the upper soil depth (" << m_upSoilDepth << endl;
            throw ModelException("SET_LM", "Execute", oss.str());
        }

        float etDeficiency = m_PET[i] - m_EI[i] - m_ED[i];
        for (int j = 0; j < m_nSoilLayers; j++)
        {
            if (etDeficiency <= 0.f)
                break;

            et2d[j] = 0.f;

            if (m_sm[i][j] >= m_fc[i][j])
                et2d[j] = etDeficiency;
            else if (m_sm[i][j] >= m_wp[i][j])
                et2d[j] = etDeficiency * (m_sm[i][j] - m_wp[i][j]) / (m_fc[i][j] - m_wp[i][j]);
            else
                et2d[j] = 0.0f;

            float sm0 = m_sm[i][j];
            float availableWater = (m_sm[i][j] - m_wp[i][j]) * depth[j];
            if (et2d[j] > availableWater)
            {
                et2d[j] = availableWater;
                m_sm[i][j] = m_wp[i][j];
            }
            else
                m_sm[i][j] -= et2d[j] / depth[j];

            if (m_sm[i][j] < 0.f)
            {
                cout << "SET_LM\t" << sm0 << "\t" << m_wp[i][j] << "\t" << m_sm[i][j] << "\t"
                << availableWater / depth[j] << "\t" << et2d[j] / depth[j] << endl;
                throw ModelException("SET_LM", "Execute", "moisture is less than zero.");
            }

            etDeficiency -= et2d[j];

            m_soilET[i] += et2d[j];
        }
    }

    return 0;

}

void SET_LM::Get1DData(const char *key, int *nRows, float **data)
{
    string s(key);
    if (StringMatch(s, VAR_SOET))
        *data = m_soilET;
    else
        throw ModelException("SET_LM", "getResult",
                             "Result " + s + " does not exist in SET_LM method. Please contact the module developer.");

    *nRows = m_nCells;
}

void SET_LM::SetValue(const char *key, float data)
{
    string s(key);
    if (StringMatch(s, VAR_T_SOIL))
        m_frozenT = data;
    else if (StringMatch(s, VAR_OMP_THREADNUM))
        omp_set_num_threads((int) data);
    else
        throw ModelException("SET_LM", "SetValue", "Parameter " + s +
                                                   " does not exist in SET_LM method. Please contact the module developer.");
}

void SET_LM::Set1DData(const char *key, int nRows, float *data)
{
    string s(key);

    CheckInputSize(key, nRows);

    if (StringMatch(s, VAR_SOILDEPTH))
        m_rootDepth = data;
    else if (StringMatch(s, VAR_INET))
        m_EI = data;
    else if (StringMatch(s, VAR_PET))
        m_PET = data;
    else if (StringMatch(s, VAR_DEET))
        m_ED = data;
    else if (StringMatch(s, VAR_SOTE))
        m_soilT = data;
    else
        throw ModelException("SET_LM", "SetValue", "Parameter " + s +
                                                   " does not exist in SET_LM method. Please contact the module developer.");

}

void SET_LM::Set2DData(const char *key, int nrows, int ncols, float **data)
{
    string sk(key);
    CheckInputSize(key, nrows);
    m_nSoilLayers = ncols;

    if (StringMatch(sk, VAR_FIELDCAP))
        m_fc = data;
    else if (StringMatch(sk, VAR_WILTPOINT))
        m_wp = data;
    else if (StringMatch(sk, VAR_SOL_ST))
        m_sm = data;
    else
        throw ModelException("PER_PI", "Set1DData",
                             "Parameter " + sk + " does not exist. Please contact the module developer.");
}

bool SET_LM::CheckInputData()
{
    if (m_nCells <= 0)
        throw ModelException("SET_LM", "CheckInputData", "The dimension of the input data can not be less than zero.");
    if (m_fc == NULL)
        throw ModelException("SET_LM", "CheckInputData", "The Soil field capacity can not be NULL.");
    if (m_wp == NULL)
        throw ModelException("SET_LM", "CheckInputData", "The plant wilting point moisture can not be NULL.");
    if (m_EI == NULL)
        throw ModelException("SET_LM", "CheckInputData", "The EI can not be NULL.");
    if (m_PET == NULL)
        throw ModelException("SET_LM", "CheckInputData", "The PET can not be NULL.");
    if (m_ED == NULL)
        throw ModelException("SET_LM", "CheckInputData", "The ED can not be NULL.");
    if (m_sm == NULL)
        throw ModelException("SET_LM", "CheckInputData", "The soil moisture can not be NULL.");
    if (m_soilT == NULL)
        throw ModelException("SET_LM", "CheckInputData", "The soil temerature can not be NULL.");
    if (m_frozenT < -90.0f)
        throw ModelException("SET_LM", "CheckInputData", "The threshold soil freezing temerature can not be NULL.");
    if (m_nSoilLayers != 2)
        throw ModelException("SET_LM", "CheckInputData", "Only 2 soil layers are supported now.");

    return true;
}

bool SET_LM::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
        throw ModelException("SET_LM", "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");

    if (m_nCells != n)
    {
        if (m_nCells <= 0)
            m_nCells = n;
        else
            throw ModelException("SET_LM", "CheckInputSize", "Input data for " + string(key) +
                                                             " is invalid. All the input data should have same size.");
    }

    return true;
}
