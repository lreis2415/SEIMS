#include "PETPenmanMonteith.h"
#include "MetadataInfo.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include "ModelException.h"
#include "util.h"
#include "ClimateParams.h"
#include <omp.h>

using namespace std;

PETPenmanMonteith::PETPenmanMonteith(void) : m_tSnow(-1), m_nCells(-1),
                                             m_elev(NULL), m_rhd(NULL), m_sr(NULL), m_tMean(NULL), m_tMin(NULL),
                                             m_tMax(NULL), m_ws(NULL), m_phutot(NULL), 
											 m_growCode(NULL), m_cht(NULL), m_lai(NULL), m_petFactor(1.f), 
                                             m_cellLat(NULL), m_albedo(NULL), m_gsi(NULL), m_vpd2(NULL), m_frgmax(NULL),
                                             m_vpdfr(NULL),
                                             m_pet(NULL), m_vpd(NULL), m_dayLen(NULL), m_phuBase(NULL), m_ppt(NULL)		 
{
}

PETPenmanMonteith::~PETPenmanMonteith(void)
{
    if (this->m_dayLen != NULL) Release1DArray(this->m_dayLen);
    if (this->m_pet != NULL) Release1DArray(this->m_pet);
    if (this->m_vpd != NULL) Release1DArray(this->m_vpd);
    if (this->m_phuBase != NULL) Release1DArray(this->m_phuBase);
}

bool PETPenmanMonteith::CheckInputData()
{
    if (this->m_date < 0)
        throw ModelException(MID_PET_PM, "CheckInputData", "You have not set the time.");
    if (m_nCells <= 0)
        throw ModelException(MID_PET_PM, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    if (this->m_cht == NULL)
        throw ModelException(MID_PET_PM, "CheckInputData", "The canopy height can not be NULL.");
    if (this->m_elev == NULL)
        throw ModelException(MID_PET_PM, "CheckInputData", "The elevation can not be NULL.");
    if (this->m_growCode == NULL)
        throw ModelException(MID_PET_PM, "CheckInputData", "The land cover status code can not be NULL.");
    if (this->m_lai == NULL)
        throw ModelException(MID_PET_PM, "CheckInputData", "The leaf area index can not be NULL.");
    if (this->m_albedo == NULL)
        throw ModelException(MID_PET_PM, "CheckInputData", "The albedo of the current day can not be NULL.");
    if (this->m_rhd == NULL)
        throw ModelException(MID_PET_PM, "CheckInputData", "The relative humidity can not be NULL.");
    if (this->m_sr == NULL)
        throw ModelException(MID_PET_PM, "CheckInputData", "The solar radiation can not be NULL.");
    if (this->m_tMean == NULL)
        throw ModelException(MID_PET_PM, "CheckInputData", "The mean temperature can not be NULL.");
    if (this->m_tMin == NULL)
        throw ModelException(MID_PET_PM, "CheckInputData", "The min temperature can not be NULL.");
    if (this->m_tMax == NULL)
        throw ModelException(MID_PET_PM, "CheckInputData", "The max temperature can not be NULL.");
    if (this->m_ws == NULL)
        throw ModelException(MID_PET_PM, "CheckInputData", "The wind speed can not be NULL.");
    if (this->m_cellLat == NULL)
        throw ModelException(MID_PET_PM, "CheckInputData", "The latitude can not be NULL.");
    if (this->m_phutot == NULL)
        throw ModelException(MID_PET_PM, "CheckInputData", "The PHU0 can not be NULL.");
    if (this->m_gsi == NULL)
        throw ModelException(MID_PET_PM, "CheckInputData", "The maximum stomatal conductance can not be NULL.");
    if (this->m_vpdfr == NULL)
        throw ModelException(MID_PET_PM, "CheckInputData",
                             "The vapor pressure deficit(kPa) corresponding to the second point on the stomatal conductance curve can not be NULL.");
    if (this->m_frgmax == NULL)
        throw ModelException(MID_PET_PM, "CheckInputData",
                             "The fraction of maximum stomatal conductance can not be NULL.");
    return true;
}

bool PETPenmanMonteith::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
        throw ModelException(MID_PET_PM, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    if (this->m_nCells != n)
    {
        if (this->m_nCells <= 0) this->m_nCells = n;
        else
            throw ModelException(MID_PET_PM, "CheckInputSize", "Input data for " + string(key) +
                                                               " is invalid. All the input data should have same size.");
    }
    return true;
}

void PETPenmanMonteith::SetValue(const char *key, float value)
{
    string sk(key);
    if (StringMatch(sk, VAR_CO2)) m_co2 = value;
    else if (StringMatch(sk, VAR_T_SNOW)) m_tSnow = value;
    else if (StringMatch(sk, VAR_K_PET)) m_petFactor = value;
    else if (StringMatch(sk, VAR_OMP_THREADNUM)) omp_set_num_threads((int) value);
    else
        throw ModelException(MID_PET_PM, "SetValue", "Parameter " + sk +
                                                     " does not exist in current module. Please contact the module developer.");
}

void PETPenmanMonteith::Set1DData(const char *key, int n, float *value)
{
    if (!this->CheckInputSize(key, n)) return;
    string sk(key);
    if (StringMatch(sk, DataType_MeanTemperature))
        this->m_tMean = value;
    else if (StringMatch(sk, DataType_MaximumTemperature))
        this->m_tMax = value;
    else if (StringMatch(sk, DataType_MinimumTemperature))
        this->m_tMin = value;
    else if (StringMatch(sk, VAR_CELL_LAT))
        this->m_cellLat = value;
    else if (StringMatch(sk, DataType_RelativeAirMoisture))
        this->m_rhd = value;
    else if (StringMatch(sk, DataType_SolarRadiation))
        this->m_sr = value;
    else if (StringMatch(sk, DataType_WindSpeed))
        this->m_ws = value;
    else if (StringMatch(sk, VAR_DEM))
        this->m_elev = value;
        //from PG model, such as BIO_EPIC
    else if (StringMatch(sk, VAR_CHT))
        this->m_cht = value;
    else if (StringMatch(sk, VAR_ALBDAY))
        this->m_albedo = value;
    else if (StringMatch(sk, VAR_LAIDAY))
        this->m_lai = value;
        //from database
    else if (StringMatch(sk, VAR_PHUTOT))
        this->m_phutot = value;
    else if (StringMatch(sk, VAR_IGRO))
        this->m_growCode = value;
    else if (StringMatch(sk, VAR_GSI))
        this->m_gsi = value;
    else if (StringMatch(sk, VAR_VPDFR))
        this->m_vpdfr = value;
    else if (StringMatch(sk, VAR_FRGMAX))
        this->m_frgmax = value;
    else
        throw ModelException(MID_PET_PM, "Set1DData", "Parameter " + sk +
                                                      " does not exist in current module. Please contact the module developer.");
}

void PETPenmanMonteith::initialOutputs()
{
	if(this->m_vpd == NULL) Initialize1DArray(m_nCells, m_vpd, 0.f);
	if(this->m_dayLen == NULL) Initialize1DArray(m_nCells, m_dayLen, 0.f);
	if(this->m_phuBase == NULL) Initialize1DArray(m_nCells, m_phuBase, 0.f);
	if(this->m_pet == NULL) Initialize1DArray(m_nCells, m_pet, 0.f);
	if(this->m_ppt == NULL) Initialize1DArray(m_nCells, m_ppt, 0.f);
    if (NULL == m_vpd2) this->m_vpd2 = new float[this->m_nCells];
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++)
    {
        if (m_frgmax[i] > 0.f && m_vpdfr[i] > 0.f)
            m_vpd2[i] = (1.f - m_frgmax[i]) / (m_vpdfr[i] - 1.f);
        else
            m_vpd2[i] = 0.f;
    }
}

int PETPenmanMonteith::Execute()
{
    //check the data
    if (!this->CheckInputData()) return false;
    initialOutputs();
    int d = JulianDay(this->m_date);
    //do the execute
#pragma omp parallel for
    for (int j = 0; j < m_nCells; ++j)
    {
        /// update phubase of the simulation year.
        /* update base zero total heat units, src code from SWAT, subbasin.f
        if (tmpav(j) > 0. .and. phutot(hru_sub(j)) > 0.01) then
            phubase(j) = phubase(j) + tmpav(j) / phutot(hru_sub(j))
        end if*/
        if (m_jday == 1) m_phuBase[j] = 0.f;
        if (m_tMean[j] > 0.f && m_phutot[j] > 0.01f)
            m_phuBase[j] += m_tMean[j] / m_phutot[j];

        //////////////////////////////////////////////////////////////////////////
        //               compute net radiation
        //net short-wave radiation for PET (from swat)
        float raShortWave = m_sr[j] * (1.f - 0.23f);
        if (m_tMean[j] < this->m_tSnow)        //if the mean t < snow t, consider the snow depth is larger than 0.5mm.
            raShortWave = m_sr[j] * (1.f - 0.8f);

        //calculate the max solar radiation
        float srMax = 0.f;
        MaxSolarRadiation(d, m_cellLat[j], m_dayLen[j], srMax);
        //calculate net long-wave radiation
        //net emissivity  equation 2.2.20 in SWAT manual
        float satVaporPressure = SaturationVaporPressure(m_tMean[j]);//kPa
        float actualVaporPressure = 0.f;
        if (m_rhd[j] > 1)   /// IF percent unit.
            actualVaporPressure = m_rhd[j] * satVaporPressure * 0.01f;
        else
            actualVaporPressure = m_rhd[j] * satVaporPressure;
        m_vpd[j] = satVaporPressure - actualVaporPressure;
        float rbo = -(0.34f - 0.139f * sqrt(actualVaporPressure)); //P37 1:1.2.22
        //cloud cover factor equation 2.2.19
        float rto = 0.0f;
        if (srMax >= 1.0e-4f)
            rto = 0.9f * (m_sr[j] / srMax) + 0.1f;
        //net long-wave radiation equation 2.2.21
        float tk = m_tMean[j] + 273.15f;
        float raLongWave = rbo * rto * 4.9e-9f * pow(tk, 4.f);
        // calculate net radiation
        float raNet = raShortWave + raLongWave;

        //////////////////////////////////////////////////////////////////////////
        //calculate the slope of the saturation vapor pressure curve
        float dlt = 4098.f * satVaporPressure / pow((m_tMean[j] + 237.3f), 2.f);

        //calculate latent heat of vaporization(MJ/kg, from swat)
        float latentHeat = 2.501f - 0.002361f * m_tMean[j];

        //calculate mean barometric pressure(kPa)
        float pb = 101.3f - m_elev[j] * (0.01152f - 0.544e-6f * m_elev[j]); //p53 1:2.3.8 P
        //psychrometric constant(kPa/deg C)
        float gma = 1.013e-3f * pb / (0.622f * latentHeat);//p53 1:2.3.7

        float rho = 1710.f - 6.85f * m_tMean[j];//p127 2:2.2.19
        //aerodynamic resistance to sensible heat and vapor transfer(s/m)
        float rv = 114.f / (m_ws[j] * pow(170.f / 1000.f, 0.2f)); // P127 2:2.2.20  ??the wind speed need modified?
        //canopy resistance(s/m)
        float rc = 49.f / (1.4f - 0.4f * m_co2 / 330.f);  //P128 2:2.2.22
        float petValue = (dlt * raNet + gma * rho * m_vpd[j] / rv) /
                         (latentHeat * (dlt + gma * (1.f + rc / rv)));  //P122 2:2.2.2
        petValue = m_petFactor * max(0.f, petValue);
        m_pet[j] = petValue;
        //*********************************************************
        //The albedo would be obtained from plant growth module. But now it is assumed to be a constant.
        //After the plant growth module is completed, the following codes should be removed.
        //float albedo = 0.8f;
        //if(m_tMean[j] > this->m_tSnow) albedo = 0.23f;
        //// m_albedo is calculated by BIO_EPIC module. By LJ, Jun., 2016
        //*********************************************************
        // calculate net short-wave radiation for max plant PET
        float raShortWavePlant = m_sr[j] * (1.0f - m_albedo[j]);
        float raNetPlant = raShortWavePlant + raLongWave;

        float epMax = 0.f;
        if (m_growCode[j] > 0) //land cover growing
        {
            //determine wind speed and height of wind speed measurement
            //adjust to 100 cm (1 m) above canopy if necessary
            float zz = 0.f;  //height at which wind speed is determined(cm)
            if (m_cht[j] <= 1.f)
                zz = 170.f;
            else
                zz = m_cht[j] * 100.f + 100.f;
            //wind speed at height zz(m/s)
            float uzz = m_ws[j] * pow(zz / 1000.f, 0.2f);

            //calculate canopy height in cm
            float chz = 0.f;
            if (m_cht[j] < 0.01f)
                chz = 1.0f;
            else
                chz = m_cht[j] * 100.f;

            //calculate roughness length for momentum transfer
            float zom = 0.f;
            if (chz <= 200.f)
                zom = 0.123f * chz;
            else
                zom = 0.058f * pow(chz, 1.19f);

            //calculate roughness length for vapor transfer
            float zov = 0.1f * zom;

            //calculate zero-plane displacement of wind profile
            float d = 0.667f * chz;

            //calculate aerodynamic resistance
            float rv = log((zz - d) / zom) * log((zz - d) / zov);
            rv = rv / (pow(0.41f, 2.f) * uzz);

            //adjust stomatal conductivity for low vapor pressure
            //this adjustment will lower maximum plant ET for plants
            //sensitive to very low vapor pressure
            float xx = m_vpd[j] - 1.f;//difference between vpd and vpthreshold
            //amount of vapor pressure deficit over threshold value
            float fvpd = 1.f;
            if (xx > 0.f)
                fvpd = max(0.1f, 1.f - m_vpd2[j] * xx);
            float gsi_adj = m_gsi[j] * fvpd;


            if (gsi_adj > 1.e-6f)
            {
                //calculate canopy resistance
                float rc = 1.f / gsi_adj;    //single leaf resistance
                rc = rc / (0.5f * (m_lai[j] + 0.01f) * (1.4f - 0.4f * m_co2 / 330.f));
                //calculate maximum plant ET
                epMax = (dlt * raNetPlant + gma * rho * m_vpd[j] / rv) / (latentHeat * (dlt + gma * (1.f + rc / rv)));
                epMax = max(0.f, epMax);
                epMax = min(epMax, petValue);
            }
            else
                epMax = 0.f;
        }
        m_ppt[j] = epMax;
    }
    return 0;
}

void PETPenmanMonteith::Get1DData(const char *key, int *n, float **data)
{
    // CheckInputData(); // Plz avoid putting CheckInputData() in Get1DData, this may cause Set time error! By LJ
    initialOutputs();
	string sk(key);
	*n = this->m_nCells;
    if (StringMatch(sk, VAR_PET)) *data = this->m_pet; 
    else if (StringMatch(sk, VAR_PPT)) *data = this->m_ppt; 
    else if (StringMatch(sk, VAR_VPD)) *data = this->m_vpd; 
	else if (StringMatch(sk, VAR_DAYLEN)) *data = this->m_dayLen; 
    else if (StringMatch(sk, VAR_PHUBASE)) *data = this->m_phuBase; 
}



