#include <math.h>
#include <iostream>
#include "SUR_MR.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "util.h"
#include <omp.h>

SUR_MR::SUR_MR(void) : m_nCells(-1), m_dt(-1), m_nSoilLayers(-1), m_tFrozen(NODATA_VALUE), 
					   m_kRunoff(NODATA_VALUE),m_pMax(NODATA_VALUE),
                       //m_tSnow(NODATA_VALUE), m_t0(NODATA_VALUE), m_snowAccu(NULL), m_snowMelt(NULL),
                       m_sFrozen(NODATA_VALUE), m_runoffCo(NULL), m_initSoilStorage(NULL), m_tMean(NULL), 
					   // m_soilThick(NULL) ,m_fieldCap(NULL),m_wiltingPoint(NULL), m_porosity(NULL), 
					   m_sol_awc(NULL), m_sol_sumsat(NULL), m_soilLayers(NULL),
                       m_pNet(NULL), m_sd(NULL), m_soilTemp(NULL), m_potVol(NULL),m_impoundTrig(NULL),
                       m_pe(NULL), m_infil(NULL), m_soilStorage(NULL), m_soilStorageProfile(NULL)       
{
}

SUR_MR::~SUR_MR(void)
{
    if (m_pe != NULL) Release1DArray(m_pe);
    if (m_infil != NULL) Release1DArray(m_infil);
    if (m_soilStorage != NULL) Release2DArray(m_nCells, m_soilStorage);
	if (m_soilStorageProfile != NULL) Release1DArray(m_soilStorageProfile);
}

void SUR_MR::CheckInputData(void)
{
    if (m_date < 0)
        throw ModelException(MID_SUR_MR, "CheckInputData", "You have not set the time.");
    if (m_dt < 0)
        throw ModelException(MID_SUR_MR, "CheckInputData", "You have not set the time step.");
    if (m_nCells <= 0)
        throw ModelException(MID_SUR_MR, "CheckInputData", "The cell number of the input can not be less than zero.");
    if (m_initSoilStorage == NULL)
        throw ModelException(MID_SUR_MR, "CheckInputData", "The initial soil moisture can not be NULL.");
    if (m_runoffCo == NULL)
        throw ModelException(MID_SUR_MR, "CheckInputData", "The potential runoff coefficient can not be NULL.");
    if (FloatEqual(m_tFrozen, NODATA_VALUE))
        throw ModelException(MID_SUR_MR, "CheckInputData",
                             "The soil freezing temperature of the input data can not be NODATA.");
    //if (FloatEqual(m_t0, NODATA_VALUE))
    //    throw ModelException(MID_SUR_MR, "CheckInputData",
    //                         "The snow melt threshold temperature of the input data can not be NODATA.");
    //if (FloatEqual(m_tSnow, NODATA_VALUE))
    //    throw ModelException(MID_SUR_MR, "CheckInputData",
    //                         "The snowfall temperature of the input data can not be NODATA.");
    if (FloatEqual(m_kRunoff, NODATA_VALUE))
        throw ModelException(MID_SUR_MR, "CheckInputData", "The runoff exponent of the input data can not be NODATA.");
    if (FloatEqual(m_pMax, NODATA_VALUE))
        throw ModelException(MID_SUR_MR, "CheckInputData",
                             "The maximum P corresponding to runoffCo of the input data can not be NODATA.");
    if (FloatEqual(m_sFrozen, NODATA_VALUE))
        throw ModelException(MID_SUR_MR, "CheckInputData",
                             "The frozen soil moisture of the input data can not be NODATA.");
	if (m_sol_awc == NULL)
		throw ModelException(MID_SUR_MR, "CheckInputData", "The available water amount can not be NULL.");
 //   if (m_fieldCap == NULL)
 //       throw ModelException(MID_SUR_MR, "CheckInputData",
	//	"The water content of soil at field capacity of the input data can not be NULL.");
	//if (m_wiltingPoint == NULL)
	//	throw ModelException(MID_SUR_MR, "CheckInputData",
	//	"The water content of soil at wilting point of the input data can not be NULL.");
	//if (m_soilThick == NULL)
	//	throw ModelException(MID_SUR_MR, "CheckInputData", "the input data: Soil thickness can not be NULL.");
 //   if (m_porosity == NULL)
 //       throw ModelException(MID_SUR_MR, "CheckInputData", "The soil porosity of the input data can not be NULL.");
	if (m_tMean == NULL)
        throw ModelException(MID_SUR_MR, "CheckInputData",
                             "The mean air temperature of the input data can not be NULL.");
    if (m_soilTemp == NULL)
        throw ModelException(MID_SUR_MR, "CheckInputData", "The soil temperature of the input data can not be NULL.");
    if (m_pNet == NULL)
        throw ModelException(MID_SUR_MR, "CheckInputData", "The net precipitation can not be NULL.");
    if (m_sd == NULL)
        throw ModelException(MID_SUR_MR, "CheckInputData", "The depression capacity can not be NULL.");
    //if (m_snowAccu == NULL)
    //    throw ModelException(MID_SUR_MR, "CheckInputData", "The snow accumulation can not be NULL.");
    //if (m_snowMelt == NULL)
    //    throw ModelException(MID_SUR_MR, "CheckInputData", "The snow melt can not be NULL.");
}

void SUR_MR::initialOutputs()
{
    if (m_nCells <= 0)
        throw ModelException(MID_SUR_MR, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    // allocate the output variables
    if (m_pe == NULL)
	{
        m_pe = new float[m_nCells];
        m_infil = new float[m_nCells];
        m_soilStorage = new float *[m_nCells];
		m_soilStorageProfile = new float [m_nCells];
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++)
        {
			Initialize1DArray(m_nSoilLayers, m_soilStorage[i], NODATA_VALUE);
            m_pe[i] = 0.f;
            m_infil[i] = 0.f;
			m_soilStorageProfile[i] = 0.f;
            for (int j = 0; j < (int)m_soilLayers[i]; j++){
				/// mm
                //m_soilStorage[i][j] = m_initSoilStorage[i] * (m_fieldCap[i][j] - m_wiltingPoint[i][j]) * m_soilThick[i][j]; 
				if(m_initSoilStorage[i] >= 0.f && m_initSoilStorage[i] <= 1.f && m_sol_awc[i][j] >= 0.f){
					m_soilStorage[i][j] = m_initSoilStorage[i] * m_sol_awc[i][j];
				}
				else
				{
					m_soilStorage[i][j] = 0.f;
				}
				m_soilStorageProfile[i] += m_soilStorage[i][j];
			}
        }
    }
}

int SUR_MR::Execute()
{
    CheckInputData();
    initialOutputs();
    m_pMax = m_pMax * m_dt / 86400.f;
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++)
    {
		/// Snow melt should be considered in SnowMelt module, this may be redundant. By LJ
        ////account for the effects of snow melt and soil temperature
        //float t = 0.f, snowMelt = 0.f, snowAcc = 0.f, sd = 0.f;
        //if (m_tMean != NULL) t = m_tMean[i];
        //if (m_snowMelt != NULL)snowMelt = m_snowMelt[i];
        //if (m_snowAccu != NULL)snowAcc = m_snowAccu[i];
        //if (m_sd != NULL)sd = m_sd[i];

        //// snow, without snow melt
        //if (m_tMean[i] <= m_tSnow)
        //    hWater = 0.0f;
        //// rain on snow, no snow melt
        //else if (m_tMean[i] > m_tSnow && m_tMean[i] <= m_t0 && snowAcc > m_pNet[i])
        //    hWater = 0.0f;
        //else
        //    hWater = m_pNet[i] + snowMelt + m_sd[i];
        float hWater = 0.f;
		hWater = m_pNet[i] + m_sd[i];
        if (hWater > 0.f)
        {
			/// in the new version, sm is replaced by m_soilStorageProfile. By lj
			/// por is replaced by m_sol_sumsat which is calculated by (sat - wp)
            //float sm = 0.f, por = 0.f;
            //for (int j = 0; j < (int)m_soilLayers[i]; j++)
            //{
            //    sm += m_soilStorage[i][j]; ///  mm H2O
            //    por += m_porosity[i][j] * m_soilThick[i][j]; /// unit can be seen as mm H2O 
            //}
            // float smFraction = min(sm / por, 1.f);

			/// update total soil water content
			m_soilStorageProfile[i] = 0.f;
			for (int ly = 0; ly < (int)m_soilLayers[i]; ly++){
				m_soilStorageProfile[i] += m_soilStorage[i][ly];
			}
			float smFraction = min(m_soilStorageProfile[i]/m_sol_sumsat[i], 1.f);
            // for frozen soil, no infiltration will occur
            if (m_soilTemp[i] <= m_tFrozen && smFraction >= m_sFrozen)
            {
				m_pe[i] = m_pNet[i];
                m_infil[i] = 0.f;
            }
            else
            {
                float alpha = m_kRunoff - (m_kRunoff - 1.f) * hWater / m_pMax;
                if (hWater >= m_pMax)
                    alpha = 1.f;

                //runoff percentage
                float runoffPercentage;
                if (m_runoffCo[i] > 0.99f)
                    runoffPercentage = 1.f;
                else
                    runoffPercentage = m_runoffCo[i] * pow(smFraction, alpha);

                float surfq = hWater * runoffPercentage;
				if (surfq > hWater) surfq = hWater;
                m_infil[i] = hWater - surfq;
                m_pe[i] = surfq;
				
				
				/// TODO: Why calculate surfq first, rather than infiltration first?
				///       I think we should calculate infiltration first, until saturation, 
				///       then surface runoff should be calculated. By LJ.

                // check the output data, In my view, we should avoid this situation to occur. By LJ.
                //if (m_infil[i] != m_infil[i] || m_infil[i] < 0.f)
                //{
                //    //string datestr = getDate(&m_date);
                //    ostringstream oss;
                //    oss << "Cell id:" << i << "\tPrecipitation(mm) = " << m_pNet[i] << "\thwater = " << hWater
                //    << "\tpercentage:" << runoffPercentage << "\tmoisture = " << sm
                //    << "\tInfiltration(mm) = " << m_infil[i] << "\n";
                //    throw ModelException(MID_SUR_MR, "Execute",
                //                         "Output data error: infiltration is less than zero. Where:\n"
                //                         + oss.str() + "Please contact the module developer. ");
                //}
            }
        }
        else
        {
            m_pe[i] = 0.f;
            m_infil[i] = 0.f;
        }
		/// if m_infil > 0., m_soilStorage need to be updated here. By LJ, 2016-9-2
		if (m_infil[i] > 0.f)
		{
			//if (m_potVol != NULL && m_potVol[i] > UTIL_ZERO)
			//{
			//	if (m_impoundTrig != NULL && FloatEqual(m_impoundTrig[i], 0.f))
			//	{
			//		m_potVol[i] += m_infil[i];
			//		/// when impounded, set the maximum infiltration to 2 mm
			//		if (m_potVol[i] > 2.f)
			//			m_infil[i] = 2.f;
			//		else
			//			m_infil[i] = 0.f;
			//		m_potVol[i] -= m_infil[i];
			//	}
			//	//else /// release operation should be considered in IMP_SWAT module
			//	//	m_infil[i] += m_potVol[i];
			//}
			m_soilStorage[i][0] += m_infil[i];
		}
		//if (i == 200)
		//{
		//	cout<<"netRain: "<<m_pNet[i]<<", depStrg: "<<m_sd[i]<<", infil: "<<m_infil[i]<<", surfq: "<<m_pe[i]<<endl;
		//}
    }
    return 0;
}

bool SUR_MR::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        throw ModelException(MID_SUR_MR, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_nCells != n)
    {
        if (m_nCells <= 0) m_nCells = n;
        else
            throw ModelException(MID_SUR_MR, "CheckInputSize", "Input data for " + string(key) +
                                                               " is invalid. All the input data should have same size.");
    }
    return true;
}

void SUR_MR::SetValue(const char *key, float value)
{
    string sk(key);
    if (StringMatch(key, VAR_OMP_THREADNUM))omp_set_num_threads((int) value);
    else if (StringMatch(sk, Tag_HillSlopeTimeStep)) m_dt = value;
	//else if (StringMatch(sk, VAR_T_SNOW))m_tSnow = value;
	//else if (StringMatch(sk, VAR_T0))m_t0 = value;
    else if (StringMatch(sk, VAR_T_SOIL))m_tFrozen = value;
    else if (StringMatch(sk, VAR_K_RUN))m_kRunoff = value;
    else if (StringMatch(sk, VAR_P_MAX))m_pMax = value;
    else if (StringMatch(sk, VAR_S_FROZEN))m_sFrozen = value;
    else
        throw ModelException(MID_SUR_MR, "SetValue", "Parameter " + sk + " does not exist.");
}

void SUR_MR::Set1DData(const char *key, int n, float *data)
{
    CheckInputSize(key, n);
    //set the value
    string sk(key);
    if (StringMatch(sk, VAR_RUNOFF_CO))m_runoffCo = data;
    else if (StringMatch(sk, VAR_NEPR))m_pNet = data;
    else if (StringMatch(sk, VAR_TMEAN))m_tMean = data;
    else if (StringMatch(sk, VAR_MOIST_IN))m_initSoilStorage = data;
    else if (StringMatch(sk, VAR_DPST))m_sd = data;
    else if (StringMatch(sk, VAR_SOTE))m_soilTemp = data;
	else if (StringMatch(sk, VAR_SOILLAYERS))m_soilLayers = data;
	else if (StringMatch(sk, VAR_SOL_SUMSAT))m_sol_sumsat = data;
	else if (StringMatch(sk, VAR_POT_VOL)) m_potVol = data;
	else if (StringMatch(sk, VAR_IMPOUND_TRIG)) m_impoundTrig = data;
    //else if (StringMatch(sk, VAR_SNAC))m_snowAccu = data;
    //else if (StringMatch(sk, VAR_SNME))m_snowMelt = data;
    else
        throw ModelException(MID_SUR_MR, "Set1DData", "Parameter " + sk + " does not exist.");
}

void SUR_MR::Set2DData(const char *key, int nrows, int ncols, float **data)
{
    string sk(key);
    CheckInputSize(key, nrows);
    m_nSoilLayers = ncols;
    if (StringMatch(sk, VAR_SOL_AWC))m_sol_awc = data;
	//else if (StringMatch(sk, VAR_FIELDCAP))m_fieldCap = data;
	//else if (StringMatch(sk, VAR_SOILTHICK))m_soilThick = data;
    //else if (StringMatch(sk, VAR_POROST))m_porosity = data;
    else
        throw ModelException(MID_SUR_MR, "Set2DData", "Parameter " + sk + " does not exist.");
}

void SUR_MR::Get1DData(const char *key, int *n, float **data)
{
    initialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_INFIL)) *data = m_infil;     //infiltration
    else if (StringMatch(sk, VAR_EXCP)) *data = m_pe; // excess precipitation
	else if (StringMatch(sk, VAR_SOL_SW)) *data = m_soilStorageProfile;
    else
        throw ModelException(MID_SUR_MR, "Get1DData", "Result " + sk + " does not exist.");
    *n = m_nCells;
}

void SUR_MR::Get2DData(const char *key, int *nRows, int *nCols, float ***data)
{
    initialOutputs();
    string sk(key);
    *nRows = m_nCells;
    *nCols = m_nSoilLayers;
    if (StringMatch(sk, VAR_SOL_ST))
        *data = m_soilStorage;
    else
        throw ModelException(MID_SUR_MR, "Get2DData", "Output " + sk + " does not exist.");
}
