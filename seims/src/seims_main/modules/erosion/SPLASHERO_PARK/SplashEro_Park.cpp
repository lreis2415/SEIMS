#include "SplashEro_Park.h"
#include "MetadataInfo.h"
#include "util.h"
#include "ModelException.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <omp.h>

using namespace std;

SplashEro_Park::SplashEro_Park(void) : m_CellWith(-1), m_nCells(-1), m_TimeStep(NODATA_VALUE), m_Omega(NODATA_VALUE),
                                       m_Slope(NULL),
                                       m_Rain(NULL), m_DETSplash(NULL), m_USLE_C(NULL), m_USLE_K(NULL),
                                       m_Ccoe(NODATA_VALUE), m_Q(NULL), m_sr(NULL), m_depression(NULL)
{
}

SplashEro_Park::~SplashEro_Park(void)
{
    if (m_DETSplash != NULL)
    {
        delete[] m_DETSplash;
    }
}

void SplashEro_Park::Get1DData(const char *key, int *n, float **data)
{
    string s(key);
    *n = m_nCells;
    if (StringMatch(s, VAR_DETSPLASH))
    {
        *data = m_DETSplash;
    }
    else
        throw ModelException(MID_SplashEro_Park, "Get1DData",
                             "Result " + s + " does not exist in current module. Please contact the module developer.");
}

void SplashEro_Park::Set1DData(const char *key, int nRows, float *data)
{
    string s(key);

    CheckInputSize(key, nRows);

    if (StringMatch(s, VAR_SLOPE))
        m_Slope = data;
        //else if(StringMatch(s,"GrassFrac"))		m_GrassFrac = data;
        //else if(StringMatch(s,"CoverFrac"))		m_coverFrac = data;
        //else if(StringMatch(s,"D_SNAC"))		m_SnowCover = data;
        //else if(StringMatch(s,"CHWIDTH"))		m_ChWidth = data;
    else if (StringMatch(s, VAR_DPST))
        m_depression = data;
    else if (StringMatch(s, VAR_SURU))
        m_sr = data;    //SURU is wrong
    else if (StringMatch(s, VAR_QOVERLAND))
        m_Q = data;
    else if (StringMatch(s, VAR_NEPR))
        m_Rain = data;
    else if (StringMatch(s, VAR_USLE_K))
        m_USLE_K = data;
    else if (StringMatch(s, VAR_USLE_C))
        m_USLE_C = data;
    else
        throw ModelException(MID_SplashEro_Park, "SetValue", "Parameter " + s +
                                                             " does not exist in current module. Please contact the module developer.");
}

void SplashEro_Park::SetValue(const char *key, float data)
{
    string s(key);
    if (StringMatch(s, Tag_CellWidth)) m_CellWith = data;
    else if (StringMatch(s, Tag_CellSize)) m_nCells = (int) data;
    else if (StringMatch(s, Tag_HillSlopeTimeStep)) m_TimeStep = data;
    else if (StringMatch(s, VAR_OMEGA)) m_Omega = data;
    else if (StringMatch(s, VAR_OMP_THREADNUM)) omp_set_num_threads((int) data);
    else
        throw ModelException(MID_SplashEro_Park, "SetValue", "Parameter " + s +
                                                             " does not exist in current module. Please contact the module developer.");
}
//
//string SplashEro_Park::toString(float value)
//{
//	ostringstream oss;
//	oss << value;
//	return oss.str();
//}

bool SplashEro_Park::CheckInputData()
{
    if (m_date < 0)
        throw ModelException(MID_SplashEro_Park, "CheckInputData", "You have not set the time.");

    if (m_CellWith <= 0)
        throw ModelException(MID_SplashEro_Park, "CheckInputData", "The cell width can not be less than zero.");

    if (m_nCells <= 0)
        throw ModelException(MID_SplashEro_Park, "CheckInputData", "The cell number can not be less than zero.");

    if (m_TimeStep < 0)
        throw ModelException(MID_SplashEro_Park, "CheckInputData", "You have not set the time step.");

    if (m_Omega < 0)
        throw ModelException(MID_SplashEro_Park, "CheckInputData",
                             "You have not set the calibration coefficient of splash erosion.");

    if (m_Slope == NULL)
        throw ModelException(MID_SplashEro_Park, "CheckInputData", "The slope (%) can not be NULL.");

    /*if (m_GrassFrac == NULL)
    {
        throw ModelException(MID_SplashEro_Park,"CheckInputData","The fraction of grasstrip in a cell can not be NULL.");
        return false;
    }*/
    /*if (m_coverFrac == NULL)
    {
        throw ModelException(MID_SplashEro_Park,"CheckInputData","The fraction of vegetation canopy cover can not be NULL.");
        return false;
    }*/
    /*if (m_ChWidth == NULL)
    {
    throw ModelException(MID_SplashEro_Park,"CheckInputData","The random roughness can not be NULL.");
    return false;
    }*/
    if (m_depression == NULL)
        throw ModelException(MID_SplashEro_Park, "CheckInputData", "The depression storage can not be NULL.");
    if (m_sr == NULL)
        throw ModelException(MID_SplashEro_Park, "CheckInputData", "The surface runoff can not be NULL.");

    if (m_Q == NULL)
        throw ModelException(MID_SplashEro_Park, "CheckInputData", "The water flux of cell can not be NULL.");

    if (m_Rain == NULL)
        throw ModelException(MID_SplashEro_Park, "CheckInputData", "The amount of rainfall can not be NULL.");

    if (m_USLE_C == NULL)
        throw ModelException(MID_SplashEro_Park, "CheckInputData", "The parameter of USLE_C can not be NULL.");

    if (m_USLE_K == NULL)
        throw ModelException(MID_SplashEro_Park, "CheckInputData", "The parameter of USLE_K can not be NULL.");

    /*if (m_SnowCover == NULL)
    {
        throw ModelException(MID_SplashEro_Park,"CheckInputData","The snowmelt cover can not be NULL.");
        return false;
    }*/

    return true;
}

bool SplashEro_Park::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        throw ModelException(MID_SplashEro_Park, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_nCells != n)
    {
        if (m_nCells <= 0) m_nCells = n;
        else
        {
            throw ModelException(MID_SplashEro_Park, "CheckInputSize", "Input data for " + string(key) +
                                                                       " is invalid. All the input data should have same size.");
            return false;
        }
    }

    return true;
}

void SplashEro_Park::initialOutputs()
{
    //allocate the output variable
    if (m_DETSplash == NULL)
    {
        m_DETSplash = new float[m_nCells];
        for (int i = 0; i < m_nCells; i++)
        {
            m_DETSplash[i] = 0.f;
        }
    }
}

int SplashEro_Park::Execute()
{
    //set the input data which was not available right now, this will be delete when the data or module is available
    //setNotAvailableInput();

    CheckInputData();

    initialOutputs();
    //StatusMsg("executing SplashEro_Park");
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++)
    {
        //intensity in mm/h
        float RainInten = m_Rain[i] * 3600.f / m_TimeStep;

        //rainfall between plants in mm  //Rainc->Drc = Rain->Drc * _dx/DX->Drc;
        // correction for slope dx/DX, water spreads out over larger area
        float s = max(0.001f, m_Slope[i]);
        float S0 = sin(atan(s));
        float Dm = 0.00124f * pow(RainInten, 0.182f);
        float waterdepth = (m_sr[i] + m_depression[i]) / 1000.f;   // mm convert to m
        float Fw, Dr;
        if (waterdepth > Dm)
        {
            Fw = exp(1.f - waterdepth / Dm);
        }
        else
        {
            Fw = 1.f;
        }
        // kg/(m2*min)
        Dr = m_Omega * Fw * m_USLE_C[i] * m_USLE_K[i] * pow(RainInten, 2.f) * (2.96f * pow(S0, 0.79f) + 0.56f);
        // convert kg/(m2*min) to kg/cell
        float cellareas = (m_CellWith / cos(atan(s))) * m_CellWith;
        m_DETSplash[i] = Dr * (m_TimeStep / 60.f) * cellareas;
        // // Deal with all exceptions:
        //if (m_SnowCover[i]>0)
        //{
        //	m_DETSplash[i] = (1-m_SnowCover[i]) * m_DETSplash[i];  //no splash on snow deck
        //}

    }
    //StatusMsg("end of executing SplashEro_Park");
    return 0;
}