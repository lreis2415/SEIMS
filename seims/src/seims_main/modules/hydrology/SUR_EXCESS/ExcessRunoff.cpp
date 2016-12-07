#include "ExcessRunoff.h"
#include "MetadataInfo.h"
#include <cmath>
#include <ctime>
#include <iostream>
#include <fstream>
#include "ModelException.h"
#include "util.h"

#include <omp.h>

using namespace std;

ExcessRunoff::ExcessRunoff(void) : m_infil(NULL), m_pe(NULL), m_accumuDepth(NULL), m_infilCapacitySurplus(NULL),
                                   m_dt(-1), m_nCells(-1), m_fieldCap(NULL), m_porosity(NULL), m_ks(NULL),
                                   m_rootDepth(NULL), m_initSoilMoisture(NULL),
                                   m_sd(NULL), m_soilMoisture(NULL), m_pNet(NULL)//,m_t(NULL),
//m_tSnow(0.0f), m_t0(1.0f),// m_snowAccu(NULL), m_snowMelt(NULL),
//m_tSoilFrozen(-5.0f), m_sFrozen(0.5f), //m_soilTemp(NULL),

{

}

ExcessRunoff::~ExcessRunoff(void)
{
    if (this->m_pe != NULL)
    {
        delete[] m_pe;
    }
    if (this->m_infil != NULL)
    {
        delete[] m_infil;
    }
    if (this->m_infilCapacitySurplus != NULL)
    {
        delete[] m_infilCapacitySurplus;
    }
    if (this->m_accumuDepth != NULL)
    {
        delete[] m_accumuDepth;
    }
    if (m_soilMoisture != NULL)
    {
        delete[] m_soilMoisture;
    }
}

void ExcessRunoff::Get1DData(const char *key, int *n, float **data)
{
    *n = m_nCells;
    string sk(key);
    if (StringMatch(sk, "Infil"))   //infiltration
    {
        *data = this->m_infil;
    }
    else if (StringMatch(sk, "EXCP"))   // excess precipitation
    {
        *data = this->m_pe;
    }
    else if (StringMatch(sk, "INFILCAPSURPLUS"))   // excess precipitation
    {
        *data = this->m_infilCapacitySurplus;
    }
    else if (StringMatch(sk, "AccumuInfil"))   // excess precipitation
    {
        *data = this->m_accumuDepth;
    }
    else if (StringMatch(sk, "SOMO"))   // excess precipitation
    {
        *data = m_soilMoisture;
    }
    else
    {
        throw ModelException("SUR_EXCESS", "Get1DData",
                             "Parameter " + sk + " does not exist. Please contact the module developer.");

    }
}

void ExcessRunoff::clearInputs()
{
    //this->m_date = -1;
}

bool ExcessRunoff::CheckInputData()
{
    if (this->m_date < 0)
    {
        throw ModelException("SUR_EXCESS", "CheckInputData", "You have not set the time.");
        return false;
    }
    if (this->m_dt < 0)
    {
        throw ModelException("SUR_EXCESS", "CheckInputData", "You have not set the time step.");
        return false;
    }
    if (this->m_nCells <= 0)
    {
        throw ModelException("SUR_EXCESS", "CheckInputData", "The cell number can not be less than zero.");
        return false;
    }

    if (this->m_pNet == NULL)
    {
        throw ModelException("SUR_EXCESS", "CheckInputData", "The net precipitation can not be NULL.");
        return false;
    }
    if (this->m_sd == NULL)
    {
        throw ModelException("SUR_EXCESS", "CheckInputData", "The depression storage can not be NULL.");
        return false;
    }

    //if (this->m_soilMoisture == NULL)
    //{
    //	throw ModelException("SUR_EXCESS","CheckInputData","The soil moisture can not be NULL.");
    //	return false;
    //}

    //if (this->m_t == NULL)
    //{
    //	throw ModelException("SUR_EXCESS","CheckInputData","The temperature can not be NULL.");
    //	return false;
    //}
    //if (this->m_snowAccu == NULL)
    //{
    //	throw ModelException("SUR_EXCESS","CheckInputData","The snow accumulation data can not be NULL.");
    //	return false;
    //}
    //if (this->m_snowMelt == NULL)
    //{
    //	throw ModelException("SUR_EXCESS","CheckInputData","The snow melt can not be NULL.");
    //	return false;
    //}
    //if (this->m_soilTemp == NULL)
    //{
    //	throw ModelException("SUR_EXCESS","CheckInputData","The soil temperature can not be NULL.");
    //	return false;
    //}

    if (this->m_porosity == NULL)
    {
        throw ModelException("SUR_EXCESS", "CheckInputData", "The soil porosity can not be NULL.");
        return false;
    }
    if (this->m_ks == NULL)
    {
        throw ModelException("SUR_EXCESS", "CheckInputData", "The hydraulic conductivity can not be NULL.");
        return false;
    }

    if (this->m_fieldCap == NULL)
    {
        throw ModelException("SUR_EXCESS", "CheckInputData", "The Field Capacity can not be NULL.");
        return false;
    }
    if (this->m_rootDepth == NULL)
    {
        throw ModelException("SUR_EXCESS", "CheckInputData", "The root depth can not be NULL.");
        return false;
    }
    if (this->m_initSoilMoisture == NULL)
    {
        throw ModelException("SUR_EXCESS", "CheckInputData", "The initial soil temperature can not be NULL.");
        return false;
    }

    return true;
}

bool ExcessRunoff::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        throw ModelException("SUR_EXCESS", "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (this->m_nCells != n)
    {
        if (this->m_nCells <= 0) this->m_nCells = n;
        else
        {
            throw ModelException("SUR_EXCESS", "CheckInputSize", "Input data for " + string(key) +
                                                                 " is invalid. All the input data should have same size.");
            ostringstream oss;
            oss << "Input data for " + string(key) << " is invalid with size: " << n << ". The origin size is " <<
            m_nCells << ".\n";
            throw ModelException("SUR_EXCESS", "CheckInputSize", oss.str());
        }
    }

    return true;
}

string ExcessRunoff::getDate(time_t *date)
{
    struct tm p;

#ifndef linux
    localtime_s(&p, date);
#else
    localtime_r(date, &p);
#endif

    p.tm_year = p.tm_year + 1900;

    p.tm_mon = p.tm_mon + 1;

    ostringstream oss;
    oss << p.tm_year << "-" << p.tm_mon << "-" << p.tm_mday;

    return oss.str();
}


int ExcessRunoff::Execute(void)
{
    CheckInputData();

    // allocate the output variable
    if (this->m_pe == NULL)
    {
        this->m_pe = new float[this->m_nCells];
        this->m_infil = new float[this->m_nCells];
        m_infilCapacitySurplus = new float[m_nCells];
        m_soilMoisture = new float[m_nCells];
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++)
        {
            m_initSoilMoisture[i] = m_initSoilMoisture[i] * m_fieldCap[i];
            m_soilMoisture[i] = m_initSoilMoisture[i];
        }
    }

    // allocate intermediate variables
    if (this->m_accumuDepth == NULL)
    {
        this->m_accumuDepth = new float[this->m_nCells];
#pragma omp parallel for
        for (int i = 0; i < m_nCells; ++i)
            m_accumuDepth[i] = 0.0f;
    }

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++)
    {
        float pNet = m_pNet[i];
        //account for the effects of snow melt and soil temperature
        //float t = m_t[i];
        // snow, without snow melt
        //if (t <= m_tSnow)
        //{
        //	pNet = 0.0f;
        //}
        // rain on snow, no snow melt
        //else if (t > m_tSnow && t <= m_t0 && m_snowAccu[i] > pNet)
        //{
        //	pNet = 0.0f;
        //}
        //else
        //{
        //pNet = m_pNet[i] + m_snowMelt[i] + m_sd[i];
        pNet = m_pNet[i] + m_sd[i];
        //}

        if (pNet > 0)
        {
            // for frozen soil
            //if (m_soilTemp[i] <= m_tSoilFrozen && m_soilMoisture[i] >= m_sFrozen*m_porosity[i])
            //{
            //	m_pe[i] = pNet;
            //	m_infil[i] = 0.0f;
            //}
            //for saturation overland flow
            //if (m_soilMoisture[i] > m_porosity[i])
            //{
            //	m_pe[i] = pNet;
            //	m_infil[i] = 0.0f;
            //}
            //else
            {
                float ks = m_ks[i] / 3600.f; // mm/h -> mm/s

                if (m_soilMoisture[i] >= m_porosity[i])
                {
                    m_infil[i] = 0.f;
                }
                else if (m_soilMoisture[i] >= m_fieldCap[i])
                {
                    float limitContent = m_rootDepth[i] * (m_porosity[i] - m_soilMoisture[i]);
                    m_infil[i] = min(ks * m_dt, limitContent);
                }
                else
                {
                    m_infil[i] = pNet;
                }

                //check if the infiltration potential exceeds the available water
                if (m_infil[i] > pNet)
                {
                    //limit infiltration rate to available water supply
                    m_infil[i] = pNet;
                    m_infilCapacitySurplus[i] = m_infil[i] - pNet;
                }
                else
                {
                    m_infilCapacitySurplus[i] = 0.f;
                }

                //Compute the cumulative depth of infitration
                m_accumuDepth[i] += m_infil[i];

                // update excess precipitation
                //m_pe[i] = m_pNet[i] + m_snowMelt[i] - m_infil[i];
                m_pe[i] = m_pNet[i] - m_infil[i];

                // adjust soil moisture
                m_soilMoisture[i] += m_infil[i] / m_rootDepth[i];
            }
        }
        else
        {
            m_pe[i] = 0.0f;
            m_infil[i] = 0.0f;
            m_infilCapacitySurplus[i] = 0.f;
        }

        // check the output data
        if (m_infil[i] < 0.0f)
        {
            string datestr = getDate(&m_date);
            ostringstream oss;
            oss << "Date: " << datestr << "\n Precipitation(mm) = " << m_pNet[i] << "\n Infiltration(mm) = " <<
            m_infil[i] << "\n";
            throw ModelException("SUR_EXCESS", "Execute", "Output data error: infiltration is less than zero. Where:\n"
                                                          + oss.str() + "Please contact the module developer. ");
            //return false;
        }
    }

    return 0;
}


// set value
void ExcessRunoff::SetValue(const char *key, float value)
{
    string sk(key);

    //if (StringMatch(sk,"T_snow"))
    //{
    //	m_tSnow = value;
    //}
    //else if (StringMatch(sk,"t_soil"))
    //{
    //	m_tSoilFrozen = value;
    //}
    //else if (StringMatch(sk,"T0"))
    //{
    //	m_t0 = value;
    //}
    //else if (StringMatch(sk,"s_frozen"))
    //{
    //	m_sFrozen = value;
    //}
    //else
    if (StringMatch(sk, "DT_HS"))
    {
        m_dt = value;
    }
    else if (StringMatch(sk, "ThreadNum"))
    {
        omp_set_num_threads((int) value);
    }
    else
    {
        throw ModelException("SUR_EXCESS", "SetValue", "Parameter " + sk + " does not exist in SetValue method.");
    }

}

void ExcessRunoff::Set1DData(const char *key, int n, float *data)
{
    //check the input data
    if (!this->CheckInputSize(key, n)) return;

    //set the value
    string sk(key);
    if (StringMatch(sk, "D_NEPR"))
    {
        m_pNet = data;
    }
        //else if (StringMatch(sk,"D_TEMP"))
        //{
        //	m_t = data;
        //}
    else if (StringMatch(sk, "D_SOMO"))
    {
        m_soilMoisture = data;
    }
    else if (StringMatch(sk, "D_DPST"))
    {
        m_sd = data;
    }
        //else if (StringMatch(sk,"D_SOTE"))
        //{
        //	m_soilTemp = data;
        //}
        //else if (StringMatch(sk,"D_SNAC"))
        //{
        //	m_snowAccu = data;
        //}
        //else if (StringMatch(sk,"D_SNME"))
        //{
        //	m_snowMelt = data;
        //}
    else if (StringMatch(sk, "Moist_in"))
    {
        m_initSoilMoisture = data;
    }
    else if (StringMatch(sk, "Porosity"))
    {
        m_porosity = data;
    }
    else if (StringMatch(sk, "Conductivity"))
    {
        m_ks = data;
    }
    else if (StringMatch(sk, "FieldCap"))
    {
        m_fieldCap = data;
    }
    else if (StringMatch(sk, "RootDepth"))
    {
        m_rootDepth = data;
    }
    else
    {
        throw ModelException("SUR_EXCESS", "SetValue",
                             "Parameter " + sk + " does not exist. Please contact the module developer.");
    }
}
