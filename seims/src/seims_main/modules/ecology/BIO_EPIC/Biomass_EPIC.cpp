#include <iostream>
#include "Biomass_EPIC.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "utils.h"
#include "util.h"
#include "PlantGrowthCommon.h"
#include "ClimateParams.h"
#include <omp.h>
#include <cmath>

using namespace std;

Biomass_EPIC::Biomass_EPIC(void) : m_nCells(-1), m_nClimDataYrs(-1), m_co2(NODATA_VALUE), m_tMean(NULL), m_tMin(NULL),
                                   m_SR(NULL), m_dayLenMin(NULL), m_dormHr(NULL),
                                   m_soilLayers(-1), m_NUpDis(NODATA_VALUE), m_PUpDis(NODATA_VALUE), m_NFixCoef(NODATA_VALUE),
                                   m_NFixMax(NODATA_VALUE), m_soilRD(NULL), m_tMeanAnn(NULL),
                                   m_nSoilLayers(NULL), m_soilZMX(NULL), m_soilALB(NULL), m_soilDepth(NULL),
                                   m_soilAWC(NULL), m_totSoilAWC(NULL), m_totSoilSat(NULL),
                                   m_soilStorage(NULL), m_soilStorageProfile(NULL), m_sol_rsdin(NULL), m_sol_cov(NULL),m_sol_rsd(NULL),
                                   m_igro(NULL), m_landCoverCls(NULL), m_aLAIMin(NULL), m_BIOE(NULL), m_BIOEHI(NULL),
                                   m_frBioLeafDrop(NULL), m_maxLAI(NULL), m_maxBiomass(NULL),
                                   m_frPlantN1(NULL), m_frPlantN2(NULL), m_frPlantN3(NULL), m_frPlantP1(NULL),
                                   m_frPlantP2(NULL), m_frPlantP3(NULL),
                                   m_chtMax(NULL), m_co2Hi(NULL), m_frDLAI(NULL), m_epco(NULL), m_lightExtCoef(NULL),
                                   m_frGrowOptLAI1(NULL), m_frGrowOptLAI2(NULL),
                                   m_hvstIdx(NULL), m_frMaxLAI1(NULL), m_frMaxLAI2(NULL), m_matYrs(NULL), m_tBase(NULL),
                                   m_tOpt(NULL), m_wavp(NULL),
                                   m_cht(NULL), m_initTreeMatYr(NULL), m_initBiomass(NULL), m_initLAI(NULL),
                                   m_PHUPlt(NULL), m_dormFlag(NULL), m_pltET(NULL), m_pltPET(NULL),
                                   m_dayLen(NULL), m_VPD(NULL), m_PET(NULL), m_ppt(NULL), m_soilESDay(NULL),
                                   m_soilNO3(NULL), m_soilPsol(NULL), m_snowAcc(NULL),
                                   m_LAIDay(NULL), m_frPHUacc(NULL), m_LAIYrMax(NULL), m_hvstIdxAdj(NULL),
                                   m_LAIMaxFr(NULL), m_oLAI(NULL), m_lastSoilRootDepth(NULL),
                                   m_plantEPDay(NULL), m_frRoot(NULL), m_fixN(NULL), 
								   m_plantUpTkN(NULL), m_plantN(NULL), m_frPlantN(NULL),
                                   m_plantUpTkP(NULL), m_plantP(NULL), m_frPlantP(NULL),
                                   m_NO3Defic(NULL), m_frStrsAe(NULL), m_frStrsN(NULL), m_frStrsP(NULL),
                                   m_frStrsTmp(NULL), m_frStrsWa(NULL),
                                   m_biomassDelta(NULL), m_biomass(NULL), m_albedo(NULL),
								   // parameters not allowed to modify
								   ubw(10.f), uobw(NODATA_VALUE)
{
}

Biomass_EPIC::~Biomass_EPIC(void)
{
	if (m_sol_cov != NULL) Release1DArray(m_sol_cov);
	if (m_sol_rsd != NULL) Release2DArray(m_nCells, m_sol_rsd);
    if (m_LAIDay != NULL) Release1DArray(m_LAIDay);
	if (m_LAIYrMax != NULL) Release1DArray(m_LAIYrMax);
	if (m_frPHUacc != NULL) Release1DArray(m_frPHUacc);
    if (m_pltET != NULL) Release1DArray(m_pltET);
    if (m_pltPET != NULL) Release1DArray(m_pltPET);
	if (m_hvstIdxAdj != NULL) Release1DArray(m_hvstIdxAdj);
	if (m_LAIMaxFr != NULL) Release1DArray(m_LAIMaxFr);
	if (m_oLAI != NULL) Release1DArray(m_oLAI);
	if (m_lastSoilRootDepth != NULL) Release1DArray(m_lastSoilRootDepth);
    if (m_plantEPDay != NULL) Release1DArray(m_plantEPDay);
	if (m_frRoot != NULL) Release1DArray(m_frRoot);
    if (m_fixN != NULL) Release1DArray(m_fixN);
	if (m_plantUpTkN != NULL) Release1DArray(m_plantUpTkN);
	if (m_plantUpTkP != NULL) Release1DArray(m_plantUpTkP);
	if (m_plantN != NULL) Release1DArray(m_plantN);
	if (m_plantP != NULL) Release1DArray(m_plantP);
	if (m_frPlantN != NULL) Release1DArray(m_frPlantN);
	if (m_frPlantP != NULL) Release1DArray(m_frPlantP);
	if (m_NO3Defic != NULL) Release1DArray(m_NO3Defic);
    if (m_frStrsAe != NULL) Release1DArray(m_frStrsAe);
    if (m_frStrsN != NULL) Release1DArray(m_frStrsN);
	if (m_frStrsP != NULL) Release1DArray(m_frStrsP);
	if (m_frStrsTmp != NULL) Release1DArray(m_frStrsTmp);
	if (m_frStrsWa != NULL) Release1DArray(m_frStrsWa);
	if (m_biomassDelta != NULL) Release1DArray(m_biomassDelta);
	if (m_biomass != NULL) Release1DArray(m_biomass);
}

void Biomass_EPIC::SetValue(const char *key, float value)
{
    string sk(key);
    if (StringMatch(sk, VAR_CO2)) m_co2 = value;
    else if (StringMatch(sk, VAR_OMP_THREADNUM)) omp_set_num_threads((int) value);
    else if (StringMatch(sk, VAR_NUPDIS)) m_NUpDis = value;
    else if (StringMatch(sk, VAR_PUPDIS)) m_PUpDis = value;
    else if (StringMatch(sk, VAR_NFIXCO)) m_NFixCoef = value;
    else if (StringMatch(sk, VAR_NFIXMX)) m_NFixMax = value;
    else
        throw ModelException(MID_BIO_EPIC, "SetValue", "Parameter " + sk + " does not exist.");
}

bool Biomass_EPIC::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
        throw ModelException(MID_BIO_EPIC, "CheckInputSize", "Input data for " + string(key) +
                                                             " is invalid. The size could not be less than zero.");
    if (m_nCells != n)
    {
        if (m_nCells <= 0)
            m_nCells = n;
        else
            throw ModelException(MID_BIO_EPIC, "CheckInputSize", "Input data for " + string(key) +
                                                                 " is invalid. All the input raster data should have same size.");
    }
    return true;
}

void Biomass_EPIC::Set1DData(const char *key, int n, float *data)
{
    string sk(key);
    CheckInputSize(key, n);
    //// climate
    if (StringMatch(sk, DataType_MeanTemperature)) m_tMean = data;
    else if (StringMatch(sk, DataType_MinimumTemperature)) m_tMin = data;
    else if (StringMatch(sk, DataType_SolarRadiation)) m_SR = data;
    else if (StringMatch(sk, VAR_DAYLEN_MIN)) m_dayLenMin = data;
    else if (StringMatch(sk, VAR_TMEAN_ANN))m_tMeanAnn = data;
    else if (StringMatch(sk, VAR_DORMHR)) m_dormHr = data;
    else if (StringMatch(sk, VAR_DAYLEN)) m_dayLen = data;
    //// soil properties and water related
    else if (StringMatch(sk, VAR_SOILLAYERS)) m_nSoilLayers = data;
    else if (StringMatch(sk, VAR_SOL_ZMX)) m_soilZMX = data;
    else if (StringMatch(sk, VAR_SOL_ALB)) m_soilALB = data;
    else if (StringMatch(sk, VAR_SOL_SW)) m_soilStorageProfile = data;
    else if (StringMatch(sk, VAR_SOL_SUMAWC)) m_totSoilAWC = data;
    else if (StringMatch(sk, VAR_SOL_SUMSAT)) m_totSoilSat = data;
    else if (StringMatch(sk, VAR_PET)) m_PET = data;
    else if (StringMatch(sk, VAR_VPD)) m_VPD = data;
    else if (StringMatch(sk, VAR_PPT)) m_ppt = data;
    else if (StringMatch(sk, VAR_SOET)) m_soilESDay = data;
    else if (StringMatch(sk, VAR_SOL_COV)) m_sol_cov = data;
    else if (StringMatch(sk, VAR_SNAC)) m_snowAcc = data;
    //// land cover
	else if (StringMatch(sk, VAR_SOL_RSDIN)) m_sol_rsdin = data;
    else if (StringMatch(sk, VAR_IGRO)) m_igro = data;
    else if (StringMatch(sk, VAR_IDC)) m_landCoverCls = data;
    else if (StringMatch(sk, VAR_ALAIMIN)) m_aLAIMin = data;
    else if (StringMatch(sk, VAR_BIO_E)) m_BIOE = data;
    else if (StringMatch(sk, VAR_BIOEHI)) m_BIOEHI = data;
    else if (StringMatch(sk, VAR_BIOLEAF)) m_frBioLeafDrop = data;
    else if (StringMatch(sk, VAR_BLAI)) m_maxLAI = data;
    else if (StringMatch(sk, VAR_BMX_TREES)) m_maxBiomass = data;
    else if (StringMatch(sk, VAR_BN1)) m_frPlantN1 = data;
    else if (StringMatch(sk, VAR_BN2)) m_frPlantN2 = data;
    else if (StringMatch(sk, VAR_BN3)) m_frPlantN3 = data;
    else if (StringMatch(sk, VAR_BP1)) m_frPlantP1 = data;
    else if (StringMatch(sk, VAR_BP2)) m_frPlantP2 = data;
    else if (StringMatch(sk, VAR_BP3)) m_frPlantP3 = data;
    else if (StringMatch(sk, VAR_CHTMX)) m_chtMax = data;
    else if (StringMatch(sk, VAR_CO2HI)) m_co2Hi = data;
    else if (StringMatch(sk, VAR_DLAI)) m_frDLAI = data;
    else if (StringMatch(sk, VAR_EXT_COEF)) m_lightExtCoef = data;
    else if (StringMatch(sk, VAR_FRGRW1)) m_frGrowOptLAI1 = data;
    else if (StringMatch(sk, VAR_FRGRW2)) m_frGrowOptLAI2 = data;
    else if (StringMatch(sk, VAR_HVSTI)) m_hvstIdx = data;
    else if (StringMatch(sk, VAR_LAIMX1)) m_frMaxLAI1 = data;
    else if (StringMatch(sk, VAR_LAIMX2)) m_frMaxLAI2 = data;
    else if (StringMatch(sk, VAR_MAT_YRS)) m_matYrs = data;
    else if (StringMatch(sk, VAR_T_BASE)) m_tBase = data;
    else if (StringMatch(sk, VAR_T_OPT)) m_tOpt = data;
    else if (StringMatch(sk, VAR_WAVP)) m_wavp = data;
        //// management
    else if (StringMatch(sk, VAR_EPCO)) m_epco = data;
    else if (StringMatch(sk, VAR_TREEYRS)) m_initTreeMatYr = data;
    else if (StringMatch(sk, VAR_LAIINIT)) m_initLAI = data;
    else if (StringMatch(sk, VAR_BIOINIT)) m_initBiomass = data;
    else if (StringMatch(sk, VAR_PHUPLT)) m_PHUPlt = data;
    else if (StringMatch(sk, VAR_CHT)) m_cht = data;
    else if (StringMatch(sk, VAR_DORMI)) m_dormFlag = data;
    else
        throw ModelException(MID_BIO_EPIC, "Set1DData", "Parameter " + sk + " does not exist.");
}

bool Biomass_EPIC::CheckInputSize2D(const char *key, int n, int col)
{
    CheckInputSize(key, n);
    if (col <= 0)
        throw ModelException(MID_BIO_EPIC, "CheckInputSize2D", "Input data for " + string(key) +
                                                               " is invalid. The layer number could not be less than zero.");
    if (m_soilLayers != col)
    {
        if (m_soilLayers <= 0)
            m_soilLayers = col;
        else
            throw ModelException(MID_BIO_EPIC, "CheckInputSize2D", "Input data for " + string(key) +
                                                                   " is invalid. All the layers of input 2D raster data should have same size.");
    }
    return true;
}

void Biomass_EPIC::Set2DData(const char *key, int nRows, int nCols, float **data)
{
    string sk(key);
    CheckInputSize2D(key, nRows, nCols);
    if (StringMatch(sk, VAR_SOILDEPTH)) m_soilDepth = data;
	else if (StringMatch(sk, VAR_SOILTHICK)) m_soilThick = data;
    else if (StringMatch(sk, VAR_SOL_RSD)) m_sol_rsd = data;
    else if (StringMatch(sk, VAR_SOL_AWC)) m_soilAWC = data;
    else if (StringMatch(sk, VAR_SOL_ST)) m_soilStorage = data;
    else if (StringMatch(sk, VAR_SOL_NO3)) m_soilNO3 = data;
    else if (StringMatch(sk, VAR_SOL_SOLP)) m_soilPsol = data;
    else
        throw ModelException(MID_BIO_EPIC, "Set2DData", "Parameter " + sk + " does not exist.");
}

bool Biomass_EPIC::CheckInputData(void)
{
    /// DT_Single
    if (m_date <= 0) throw ModelException(MID_BIO_EPIC, "CheckInputData", "You have not set the time.");
    if (m_nCells <= 0)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    if (m_soilLayers <= 0)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The layer number of the input 2D raster data can not be less than zero.");
    if (FloatEqual(m_co2, NODATA_VALUE))
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The ambient atmospheric CO2 concentration must be provided.");
    if (FloatEqual(m_NUpDis, NODATA_VALUE))
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The nitrogen uptake distribution parameter must be provided.");
    if (FloatEqual(m_PUpDis, NODATA_VALUE))
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The phosphorus uptake distribution parameter must be provided.");
    if (FloatEqual(m_NFixCoef, NODATA_VALUE))
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The nitrogen fixation coefficient must be provided.");
    if (FloatEqual(m_NFixMax, NODATA_VALUE))
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The maximum daily-N fixation must be provided.");
    if (m_tMeanAnn == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The annual mean air temperature can not be NULL.");
    /// DT_Raster1D
    if (m_tMin == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The min temperature data can not be NULL.");
    if (m_tMean == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The mean temperature data can not be NULL.");
    if (m_SR == NULL) throw ModelException(MID_BIO_EPIC, "CheckInputData", "The solar radiation data can not be NULL.");
    if (m_dayLen == NULL) throw ModelException(MID_BIO_EPIC, "CheckInputData", "The day length can not be NULL.");
    if (m_dayLenMin == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The minimum day length can not be NULL.");
    if (m_dormHr == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The dormancy threshold time can not be NULL.");

    if (m_nSoilLayers == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The soil layers data can not be NULL.");
    if (m_soilZMX == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The maximum rooting depth in soil profile can not be NULL.");
    if (m_soilALB == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The albedo when soil is moist can not be NULL.");
    if (m_soilStorageProfile == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The amount of water stored in the soil profile can not be NULL.");
    if (m_totSoilAWC == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The amount of water held in soil profile at field capacity can not be NULL.");
    if (m_totSoilSat == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The amount of water held in soil profile at saturation can not be NULL.");
    if (m_PET == NULL) throw ModelException(MID_BIO_EPIC, "CheckInputData", "The PET can not be NULL.");
    if (m_VPD == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The Vapor pressure deficit can not be NULL.");
    if (m_ppt == NULL) throw ModelException(MID_BIO_EPIC, "CheckInputData", "The potential plant et can not be NULL.");
    if (m_soilESDay == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The actual soil et can not be NULL.");
	if (this->m_sol_rsdin == NULL)
		throw ModelException(MID_NUTR_TF, "CheckInputData", "The m_sol_rsdin can not be NULL.");
    if (m_igro == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The land cover status code can not be NULL.");
    if (m_landCoverCls == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The land cover/plant classification can not be NULL.");
    if (m_aLAIMin == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The minimum LAI during winter dormant period can not be NULL.");
    if (m_BIOE == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The RUE at ambient atmospheric CO2 concentration data can not be NULL.");
    if (m_BIOEHI == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The  RUE at elevated atmospheric CO2 concentration data can not be NULL.");
    if (m_frBioLeafDrop == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The fraction of biomass that drops during dormancy data can not be NULL.");
    if (m_maxLAI == NULL) throw ModelException(MID_BIO_EPIC, "CheckInputData", "The max LAI data can not be NULL.");
    if (m_maxBiomass == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The maximum biomass for a forest can not be NULL.");
    if (m_frPlantN1 == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The normal fraction of nitrogen in the plant biomass at emergence data can not be NULL.");
    if (m_frPlantN2 == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The normal fraction of nitrogen in the plant biomass at 50% maturity data can not be NULL.");
    if (m_frPlantN3 == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The normal fraction of nitrogen in the plant biomass at maturity data can not be NULL.");
    if (m_frPlantP1 == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The normal fraction of phosphorus in the plant biomass at emergence data can not be NULL.");
    if (m_frPlantP2 == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The normal fraction of phosphorus in the plant biomass at 50% maturity data can not be NULL.");
    if (m_frPlantP3 == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The normal fraction of phosphorus in the plant biomass at maturity data can not be NULL.");
    if (m_chtMax == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The maximum canopy height can not be NULL.");
    if (m_co2Hi == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The elevated atmospheric CO2 concentration data can not be NULL.");
    if (m_frDLAI == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The PHU fraction when senescence becomes dominant data can not be NULL.");
    if (m_lightExtCoef == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The  light extinction coefficient data can not be NULL.");
    if (m_frGrowOptLAI1 == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The PHU fraction for the 1st point data can not be NULL.");
    if (m_frGrowOptLAI2 == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The PHU fraction for the 2nd point data can not be NULL.");
    if (m_hvstIdx == NULL) throw ModelException(MID_BIO_EPIC, "CheckInputData", "The harvest index can not be NULL.");
    if (m_frMaxLAI1 == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The LAI fraction for the 1st point data can not be NULL.");
    if (m_frMaxLAI2 == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "TheLAI fraction for the 2nd point  data can not be NULL.");
    if (m_matYrs == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The full development year data can not be NULL.");
    if (m_tBase == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The base temperature data can not be NULL.");
    if (m_tOpt == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The optimal temperature for plant growth can not be NULL.");
    if (m_wavp == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The rate of decline in rue per unit increase in VPD data can not be NULL.");

    if (m_epco == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The plant water uptake compensation factor can not be NULL.");
    if (m_initTreeMatYr == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The initial age of tress can not be NULL.");
    if (m_initLAI == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The initial leaf area index of transplants can not be NULL.");
    if (m_initBiomass == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The initial dry weight biomass can not be NULL.");
    if (m_PHUPlt == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The total number of heat unites (hours) needed to bring plant to maturity can not be NULL.");
    if (m_cht == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The canopy height for the day can not be NULL.");
    if (m_dormFlag == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The dormancy status code can not be NULL.");

    /// DT_Raster2D
    if (m_soilDepth == NULL)
		throw ModelException(MID_BIO_EPIC, "CheckInputData", "The soil depth data can not be NULL.");
	if (m_soilThick == NULL)
		throw ModelException(MID_BIO_EPIC, "CheckInputData", "The soil thickness data can not be NULL.");
    if (m_soilAWC == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The water available to plants in soil layer at field capacity can not be NULL.");
    if (m_soilStorage == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The soil moisture in soil layers can not be NULL.");
    if (m_soilNO3 == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData",
                             "The nitrogen stored in the nitrate pool in soil layer can not be NULL.");
    if (m_soilPsol == NULL)
        throw ModelException(MID_BIO_EPIC, "CheckInputData", "The phosphorus stored in solution can not be NULL.");
    return true;
}

void Biomass_EPIC::initialOutputs()
{
	if (FloatEqual(uobw, NODATA_VALUE)){
		ubw = 10.f; /// the uptake distribution for water is hardwired, users are not allowed to modify
		uobw = 0.f;
		uobw = PGCommon::getNormalization(ubw);
	}
	if(m_albedo == NULL) Initialize1DArray(m_nCells, m_albedo, 0.f);
	if(m_sol_cov == NULL || m_sol_rsd == NULL)
	{
		Initialize1DArray(m_nCells, m_sol_cov, m_sol_rsdin);
		Initialize2DArray(m_nCells, m_soilLayers, m_sol_rsd, 0.f);
#pragma omp parallel for
		for (int i = 0; i < m_nCells; i++)
			m_sol_rsd[i][0] = m_sol_cov[i];
	}
    if (m_LAIDay == NULL)
	{	
		if(m_initLAI != NULL)
			Initialize1DArray(m_nCells, m_LAIDay, m_initLAI);
		else
			Initialize1DArray(m_nCells, m_LAIDay, 0.f);
	}
    if (m_LAIYrMax == NULL)
    {
        m_LAIYrMax = new float[m_nCells];
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++)
        {
            if (PGCommon::IsTree((int) (m_landCoverCls[i])))
                m_LAIYrMax[i] *= m_initTreeMatYr[i] / m_matYrs[i];
            else
                m_LAIYrMax[i] = m_LAIDay[i];
        }
    }
    if (m_frPHUacc == NULL)
		Initialize1DArray(m_nCells, m_frPHUacc, 0.f);
    if(m_pltET == NULL)
		Initialize1DArray(m_nCells, m_pltET, 0.f);
	if(m_pltPET == NULL)
		Initialize1DArray(m_nCells, m_pltPET, 0.f);
    if (m_hvstIdxAdj == NULL)
		Initialize1DArray(m_nCells, m_hvstIdxAdj, 0.f);
	if(m_LAIMaxFr == NULL)
		Initialize1DArray(m_nCells, m_LAIMaxFr, 0.f);
	if (m_oLAI == NULL)
		Initialize1DArray(m_nCells, m_oLAI, 0.f);
    if (m_lastSoilRootDepth == NULL)
		Initialize1DArray(m_nCells, m_lastSoilRootDepth, 10.f);
	if (m_soilRD == NULL)
		Initialize1DArray(m_nCells, m_soilRD, 10.f);
	if(m_plantEPDay == NULL)
		Initialize1DArray(m_nCells, m_plantEPDay, 0.f);
	if (m_frRoot == NULL)
		Initialize1DArray(m_nCells, m_frRoot, 0.f);
	if(m_fixN == NULL)
		Initialize1DArray(m_nCells, m_fixN, 0.f);
	if (m_plantUpTkN == NULL)
		Initialize1DArray(m_nCells, m_plantUpTkN, 0.f);
	if(m_plantUpTkP == NULL)
		Initialize1DArray(m_nCells, m_plantUpTkP, 0.f);
	if (m_plantN == NULL)
		Initialize1DArray(m_nCells, m_plantN, 0.f);
	if (m_plantP == NULL)
		Initialize1DArray(m_nCells, m_plantP, 0.f);
	if (m_frPlantN == NULL)
		Initialize1DArray(m_nCells, m_frPlantN, 0.f);
	if (m_frPlantP == NULL)
		Initialize1DArray(m_nCells, m_frPlantP, 0.f);

	if(m_NO3Defic == NULL)
		Initialize1DArray(m_nCells, m_NO3Defic, 0.f);
	// initialize these stress factors according to sim_iniday.f of SWAT
	if (m_frStrsAe == NULL)
		Initialize1DArray(m_nCells, m_frStrsAe, 1.f);
	if(m_frStrsN == NULL)
		Initialize1DArray(m_nCells, m_frStrsN, 1.f);
	if (m_frStrsP == NULL)
		Initialize1DArray(m_nCells, m_frStrsP, 1.f);
	if (m_frStrsTmp == NULL)
		Initialize1DArray(m_nCells, m_frStrsTmp, 1.f);
	// according to zero.f of SWAT
	if (m_frStrsWa == NULL)
		Initialize1DArray(m_nCells, m_frStrsWa, 1.f);
	if (m_biomassDelta == NULL)
		Initialize1DArray(m_nCells, m_biomassDelta, 0.f);
	if (m_biomass == NULL){
		if (m_initBiomass != NULL)
			Initialize1DArray(m_nCells, m_biomass, m_initBiomass);
		else
			Initialize1DArray(m_nCells, m_biomass, 0.f);
	}
}

void Biomass_EPIC::DistributePlantET(int i)
{	/// swu.f of SWAT
    float sum = 0.f, sump = 0.f, gx = 0.f;
    /// fraction of water uptake by plants achieved
    /// where the reduction is caused by low water content
    float reduc = 0.f;
    /// water uptake by plants in each soil layer
	/*
	 * Initialize1DArray should not be used inside a OMP codeblock
	 * In VS this would be fine, but in linux, may be problematic.*/
	float *wuse(NULL);
	Initialize1DArray((int) m_nSoilLayers[i], wuse, 0.f);

	//float *wuse = new float[(int) m_nSoilLayers[i]];
	//for (int j = 0; j < (int) m_nSoilLayers[i]; j++)
	//	wuse[j] = 0.f;
    /// water uptake by plants from all layers
    float xx = 0.f;
    int ir = -1;
    int idc = int(m_landCoverCls[i]);
    if (idc == 1 || idc == 2 || idc == 4 || idc == 5)
    {
        m_soilRD[i] = 2.5f * m_frPHUacc[i] * m_soilZMX[i];
        if (m_soilRD[i] > m_soilZMX[i]) m_soilRD[i] = m_soilZMX[i];
        if (m_soilRD[i] < 10.f) m_soilRD[i] = 10.f;   /// minimum root depth is 10mm
    }
    else
        m_soilRD[i] = m_soilZMX[i];
    m_lastSoilRootDepth[i] = m_soilRD[i];
    if (m_ppt[i] <= 0.01f)
        m_frStrsWa[i] = 1.f;
    else
    {
        /// initialize variables
        gx = 0.f;
        ir = 0;
        sump = 0.f;
        xx = 0.f;
		// update soil storage profile just in case
		m_soilStorageProfile[i] = 0.f;
		for (int ly = 0; ly < (int) m_nSoilLayers[i]; ly++)
			m_soilStorageProfile[i] += m_soilStorage[i][ly];
		/// compute aeration stress
		if (m_soilStorageProfile[i] >= m_totSoilAWC[i]) // mm
		{
			float satco = (m_soilStorageProfile[i] - m_totSoilAWC[i]) / (m_totSoilSat[i] - m_totSoilAWC[i]);
			float pl_aerfac = 0.85f;
			float scparm = 100.f * (satco - pl_aerfac) / (1.0001f - pl_aerfac);
			if (scparm > 0.f)
				m_frStrsAe[i] = 1.f - (scparm / (scparm + exp(2.9014f - 0.03867f * scparm)));
			else
				m_frStrsAe[i] = 1.f;
		}
		for (int j = 0; j < (int) m_nSoilLayers[i]; j++)
		{
			if (ir > 0) break;
			if (m_soilRD[i] <= m_soilDepth[i][j])
			{
				gx = m_soilRD[i];
				ir = j;
			}
			else
				gx = m_soilDepth[i][j];
			sum = 0.f;
			if (m_soilRD[i] <= 0.01f)
				sum = m_ppt[i] / uobw;
			else
				sum = m_ppt[i] * (1.f - exp(-ubw * gx / m_soilRD[i])) / uobw;
			wuse[j] = sum - sump + 1.f * m_epco[i];
			wuse[j] = sum - sump + (sump - xx) * m_epco[i];
			sump = sum;
			/// adjust uptake if sw is less than 25% of plant available water
			reduc = 0.f;
			if (m_soilStorage[i][j] < m_soilAWC[i][j] / 4.f)
				reduc = exp(5.f * (4.f * m_soilStorage[i][j] / m_soilAWC[i][j] - 1.f));
			else
				reduc = 1.f;
			// reduc = 1.f;  /// TODO, Is SWAT wrong here? by LJ
			wuse[j] *= reduc;
			if (m_soilStorage[i][j] < wuse[j])
				wuse[j] = m_soilStorage[i][j];
			m_soilStorage[i][j] = max(UTIL_ZERO, m_soilStorage[i][j] - wuse[j]);
			xx += wuse[j];
		}
		/// update total soil water in profile
		m_soilStorageProfile[i] = 0.f;
		for (int ly = 0; ly < (int)m_nSoilLayers[i]; ly++)
			m_soilStorageProfile[i] += m_soilStorage[i][ly];
		m_frStrsWa[i] = xx / m_ppt[i];
		m_plantEPDay[i] = xx;
	}
	Release1DArray(wuse);
	//delete[] wuse;
	//wuse = NULL;
}

void Biomass_EPIC::CalTempStress(int i)
{
    float tgx = 0.f, rto = 0.f;
    tgx = m_tMean[i] - m_tBase[i];
    if (tgx <= 0.f)
        m_frStrsTmp[i] = 0.f;
    else if (m_tMean[i] > m_tOpt[i])
        tgx = 2.f * m_tOpt[i] - m_tBase[i] - m_tMean[i];
    rto = (m_tOpt[i] - m_tBase[i]) / pow((tgx + UTIL_ZERO), 2.f);
    if (rto <= 200.f && tgx > 0.f)
        m_frStrsTmp[i] = exp(-0.1054f * rto);
    else
        m_frStrsTmp[i] = 0.f;
    if (m_tMin[i] <= m_tMeanAnn[i] - 15.f)
        m_frStrsTmp[i] = 0.f;
}

void Biomass_EPIC::AdjustPlantGrowth(int i)
{
    /// Update accumulated heat units for the plant
    float delg = 0.f;
    if (m_PHUPlt[i] > 0.1)
        delg = (m_tMean[i] - m_tBase[i]) / m_PHUPlt[i];
    if (delg < 0.f)
        delg = 0.f;
	m_frPHUacc[i] += delg;
	//if(i == 5) cout << m_biomassDelta[i] << ", \n";
	//if(i == 0) cout << m_frPHUacc[i] << ", \n";
    /// If plant hasn't reached maturity
    if (m_frPHUacc[i] <= 1.f)
    {
        ///compute temperature stress - strstmp(j) , tstr.f in SWAT
        CalTempStress(i);
        /// Calculate optimal biomass
        /// 1. calculate photosynthetically active radiation
        float activeRadiation = 0.f;
        activeRadiation = 0.5f * m_SR[i] * (1.f - exp(-m_lightExtCoef[i] * (m_LAIDay[i] + 0.05f)));
        /// 2. Adjust radiation-use efficiency for CO2
        ////  determine shape parameters for the radiation use efficiency equation, readplant.f in SWAT
        if (FloatEqual(m_co2Hi[i], 330.0f)) m_co2Hi[i] = 660.f;
        float m_RadUseEffiShpCoef1 = 0.f;
        float m_RadUseEffiShpCoef2 = 0.f;
        PGCommon::getScurveShapeParameter(m_BIOE[i] * 0.01f, m_BIOEHI[i] * 0.01f, m_co2, m_co2Hi[i],
                                          &m_RadUseEffiShpCoef1, &m_RadUseEffiShpCoef2);

        float beadj = 0.f;
        if (m_co2 > 330.f)
            beadj = 100.f * m_co2 / (m_co2 + exp(m_RadUseEffiShpCoef1 - m_co2 * m_RadUseEffiShpCoef2));
        else
            beadj = m_BIOE[i];
        /// 3. adjust radiation-use efficiency for vapor pressure deficit
        ///     assumes vapor pressure threshold of 1.0 kPa
        float ruedecl = 0.f;
        if (m_VPD[i] > 1.0)
        {
            ruedecl = m_VPD[i] - 1.f;
            beadj -= m_wavp[i] * ruedecl;
            beadj = max(beadj, 0.27f * m_BIOE[i]);
        }
        m_biomassDelta[i] = max(0.f, beadj * activeRadiation);
        /// 4. Calculate plant uptake of N and P to make sure no plant N and P uptake under temperature, water and aeration stress   
		/// m_frStrsWa and m_frStrsAe are derived from DistributePlantET()
		//if (i == 2000 && (m_frStrsWa[i] > 0.f || m_frStrsTmp[i] > 0.f || m_frStrsAe[i] > 0.f))
		//	cout<<"water stress frac: "<<m_frStrsWa[i]<<", tmp: "<<m_frStrsTmp[i]<<", Ae: "<<m_frStrsAe[i]<<endl;
        float reg = min(min(m_frStrsWa[i], m_frStrsTmp[i]), m_frStrsAe[i]);
        if (reg < 0.f) reg = 0.f;
        if (reg > 0.)
        {
			//cout<<"Begin Uptake N and P"<<endl;
            /// call nup to calculates plant nitrogen uptake
            PlantNitrogenUptake(i);
            /// call npup to calculates plant phosphorus uptake
            PlantPhosphorusUptake(i);
        }
        else
        {
            m_frStrsN[i] = 1.f;
            m_frStrsP[i] = 1.f;
        }
        /// 5. auto fertilization-nitrogen demand (non-legumes only)
        int idc = int(m_landCoverCls[i]);
        //if((idc == 4 || idc == 5 || idc == 6 || idc == 7) && auto_nstrs[i] > 0.)
        /// call anfert
        ///////// TODO: Finish auto fertilization-nitrogen later. by LJ
        /// 6. reduce predicted biomass due to stress on plant
        reg = min(m_frStrsWa[i], min(m_frStrsTmp[i], min(m_frStrsN[i], m_frStrsP[i])));
        if (reg < 0.f) reg = 0.f;
        if (reg > 1.f) reg = 1.f;
        //// TODO bio_targ in SWAT is not incorporated in SEIMS.
		m_biomass[i] += m_biomassDelta[i] * reg;
        float rto = 1.f;
        if (idc == CROP_IDC_TREES)
        {
            if (m_matYrs[i] > 0.)
            {
                float curYrMat = m_initTreeMatYr[i] + m_yearIdx;
                rto = curYrMat / m_matYrs[i];
                m_biomass[i] = min(m_biomass[i], rto * m_maxBiomass[i] * 1000.f);  /// convert tons/ha -> kg/ha
            }
            else
                rto = 1.f;
        }
		m_biomass[i] = max(m_biomass[i], 0.f);
		//if(i == 5) cout << m_biomass[i] << ", \n";
        /// 7. calculate fraction of total biomass that is in the roots
        float m_rootShootRatio1 = 0.4f;
        float m_rootShootRatio2 = 0.2f;
        m_frRoot[i] = m_rootShootRatio1 * (m_rootShootRatio1 - m_rootShootRatio2) * m_frPHUacc[i];
        float LAIShpCoef1 = 0.f, LAIShpCoef2 = 0.f;
        PGCommon::getScurveShapeParameter(m_frMaxLAI1[i], m_frMaxLAI2[i], m_frGrowOptLAI1[i], m_frGrowOptLAI2[i],
                                          &LAIShpCoef1, &LAIShpCoef2);
        float f = m_frPHUacc[i] / (m_frPHUacc[i] + exp(LAIShpCoef1 - LAIShpCoef2 * m_frPHUacc[i]));
        float ff = f - m_LAIMaxFr[i];
        m_LAIMaxFr[i] = f;
        /// 8. calculate new canopy height
        if (idc == CROP_IDC_TREES)
            m_cht[i] = rto * m_chtMax[i];
        else
            m_cht[i] = m_chtMax[i] * sqrt(f);
        /// 9. calculate new leaf area index (LAI)
        float laiMax = 0.f;
        float laiDelta = 0.f;
        if (m_frPHUacc[i] <= m_frDLAI[i])
        {
            laiMax = 0.f;
            laiDelta = 0.f;
            if (idc == CROP_IDC_TREES)
                laiMax = rto * m_maxLAI[i];
            else
                laiMax = m_maxLAI[i];
            if (m_LAIDay[i] > laiMax) m_LAIDay[i] = laiMax;
            laiDelta = ff * laiMax * (1.f - exp(5.f * (m_LAIDay[i] - laiMax))) * sqrt(reg);
            m_LAIDay[i] += laiDelta;
            if (m_LAIDay[i] > laiMax) m_LAIDay[i] = laiMax;
            m_oLAI[i] = m_LAIDay[i];
            if (m_LAIDay[i] > m_LAIYrMax[i]) m_LAIYrMax[i] = m_LAIDay[i];
        }
        else
            m_LAIDay[i] = m_oLAI[i] * (1.f - m_frPHUacc[i]) / (1.f - m_frDLAI[i]);
        if (m_LAIDay[i] < m_aLAIMin[i])
            m_LAIDay[i] = m_aLAIMin[i];
        /// 10. calculate plant ET values
        if (m_frPHUacc[i] > 0.5f && m_frPHUacc[i] < m_frDLAI[i])
        {
            m_pltET[i] += (m_plantEPDay[i] + m_soilESDay[i]);
            m_pltPET[i] += m_PET[i];
        }
        m_hvstIdxAdj[i] = m_hvstIdx[i] * 100.f * m_frPHUacc[i] / (100.f * m_frPHUacc[i] +
                                                                  exp(11.1f - 10.f * m_frPHUacc[i]));
    }
    else
    {
        if (m_frDLAI[i] > 1.f)
        {
            if (m_frPHUacc[i] > m_frDLAI[i])
                m_LAIDay[i] = m_oLAI[i] * (1.f - (m_frPHUacc[i] - m_frDLAI[i]) / (1.2f - m_frDLAI[i]));
        }
        if (m_LAIDay[i] < 0.f) m_LAIDay[i] = 0.f;
    }
}

void Biomass_EPIC::PlantNitrogenUptake(int i)
{
    float uobn = PGCommon::getNormalization(m_NUpDis);
    float n_reduc = 300.f; /// nitrogen uptake reduction factor (not currently used; defaulted 300.)
    float tno3 = 0.f;
    for (int l = 0; l < (int) m_nSoilLayers[i]; l++)
        tno3 += m_soilNO3[i][l];
    tno3 /= n_reduc;
    // float up_reduc = tno3 / (tno3 + exp(1.56f - 4.5f * tno3)); /// However, up_reduc is not used hereafter.
    /// icrop is land cover code in SWAT.
    /// in SEIMS, it is no need to use it.
    //// determine shape parameters for plant nitrogen uptake equation, from readplant.f
    m_frPlantN[i] = PGCommon::NPBiomassFraction(m_frPlantN1[i], m_frPlantN2[i], m_frPlantN3[i], m_frPHUacc[i]);
    float un2 = 0.f; /// ideal (or optimal) plant nitrogen content (kg/ha)
    un2 = m_frPlantN[i] * m_biomass[i];
    if (un2 < m_plantN[i]) un2 = m_plantN[i];
    m_NO3Defic[i] = un2 - m_plantN[i];
    m_NO3Defic[i] = min(4.f * m_frPlantN3[i] * m_biomassDelta[i], m_NO3Defic[i]);
    m_frStrsN[i] = 1.f;
    int ir = 0;
    if (m_NO3Defic[i] < UTIL_ZERO)
		return;
    for (int l = 0; l < (int)m_nSoilLayers[i]; l++)
    {
		if (ir > 0)
			break;
        float gx = 0.f;
        if (m_soilRD[i] <= m_soilDepth[i][l])
        {
            gx = m_soilRD[i];
            ir = 1;
        }
        else
            gx = m_soilDepth[i][l];
        float unmx = 0.f;
        float uno3l = 0.f; /// plant nitrogen demand (kg/ha)
        unmx = m_NO3Defic[i] * (1.f - exp(-m_NUpDis * gx / m_soilRD[i])) / uobn;
        uno3l = min(unmx - m_plantUpTkN[i], m_soilNO3[i][l]);
		//if (uno3l != uno3l)
		//	cout<<"cellid: "<<i<<", lyr: "<<l<<",m_NO3Defic: "<<m_NO3Defic[i]<<
		//	", UpDis: "<<m_NUpDis<<", gx: "<<gx<<", msoilrd:"<<m_soilRD[i]<<
		//	", uobn:"<<uobn<<", plantUpTkN: "<<m_plantUpTkN[i]<<
		//	", soilNo3: "<<m_soilNO3[i][l]<<", unmx: "<<unmx<<
		//	", uno3l: "<<uno3l<<endl;
        m_plantUpTkN[i] += uno3l;
        m_soilNO3[i][l] -= uno3l;
    }
    if (m_plantUpTkN[i] < 0.f) m_plantUpTkN[i] = 0.f;
    /// If crop is a legume, call nitrogen fixation routine
    if (FloatEqual(m_landCoverCls[i], 1.f) || FloatEqual(m_landCoverCls[i], 2.f) ||
        FloatEqual(m_landCoverCls[i], 3.f))
        PlantNitrogenFixed(i);
    m_plantUpTkN[i] += m_fixN[i];
    m_plantN[i] += m_plantUpTkN[i];
	//if (m_plantN[i] > 0.f)
	//	cout<<"cell ID: "<<i<<", plantN: "<<m_plantN[i]<<endl;
    /// compute nitrogen stress
    if (FloatEqual(m_landCoverCls[i], 1.f) || FloatEqual(m_landCoverCls[i], 2.f) ||
        FloatEqual(m_landCoverCls[i], 3.f))
        m_frStrsN[i] = 1.f;
    else
    {
        PGCommon::calPlantStressByLimitedNP(m_plantN[i], un2, &m_frStrsN[i]);
        float xx = 0.f;
        if (m_NO3Defic[i] > 1.e-5f)
            xx = m_plantUpTkN[i] / m_NO3Defic[i];
        else
            xx = 1.f;
        m_frStrsN[i] = max(m_frStrsN[i], xx);
        m_frStrsN[i] = min(m_frStrsN[i], 1.f);
    }
}

void Biomass_EPIC::PlantNitrogenFixed(int i)
{
    /// compute the difference between supply and demand
    float uno3l = 0.f;
    if (m_NO3Defic[i] > m_plantUpTkN[i])
        uno3l = m_NO3Defic[i] - m_plantUpTkN[i];
    else /// if supply is being met, fixation = 0 and return
    {
        m_fixN[i] = 0.f;
        return;
    }
    /// compute fixation as a function of no3, soil water, and growth stage
    //// 1. compute soil water factor
    float fxw = 0.f;
    fxw = m_soilStorageProfile[i] / (0.85f * m_totSoilAWC[i]);
    //// 2. compute no3 factor
    float sumn = 0.f; /// total amount of nitrate stored in soil profile (kg/ha)
    float fxn = 0.f;
    for (int l = 0; l < m_nSoilLayers[i]; l++)
        sumn += m_soilNO3[i][l];
    if (sumn > 300.f) fxn = 0.f;
    if (sumn > 100.f && sumn <= 300.f) fxn = 1.5f - 0.0005f * sumn;
    if (sumn <= 100.f) fxn = 1.f;
    //// 3. compute growth stage factor
    float fxg = 0.f;
    if (m_frPHUacc[i] > 0.15f && m_frPHUacc[i] <= 0.30f)
        fxg = 6.67f * m_frPHUacc[i] - 1.f;
    if (m_frPHUacc[i] > 0.30f && m_frPHUacc[i] <= 0.55f)
        fxg = 1.f;
    if (m_frPHUacc[i] > 0.55f && m_frPHUacc[i] <= 0.75f)
        fxg = 3.75f - 5.f * m_frPHUacc[i];
    float fxr = min(1.f, min(fxw, fxn)) * fxg;
    fxr = max(0.f, fxr);
    if (m_NFixCoef <= 0.f) m_NFixCoef = 0.5f;
    if (m_NFixMax <= 0.f) m_NFixMax = 20.f;
    m_fixN[i] = min(6.f, fxr * m_NO3Defic[i]);
    m_fixN[i] = m_NFixCoef * m_fixN[i] + (1.f - m_NFixCoef) * uno3l;
    m_fixN[i] = min(m_fixN[i], uno3l);
    m_fixN[i] = min(m_NFixMax, m_fixN[i]);
}

void Biomass_EPIC::PlantPhosphorusUptake(int i)
{
    float uobp = PGCommon::getNormalization(m_PUpDis);
    //// determine shape parameters for plant phosphorus uptake equation, from readplant.f
    m_frPlantP[i] = PGCommon::NPBiomassFraction(m_frPlantP1[i], m_frPlantP2[i], m_frPlantP3[i], m_frPHUacc[i]);
    float up2 = 0.f; /// optimal plant phosphorus content
    float uapd = 0.f; /// plant demand of phosphorus
    float upmx = 0.f; /// maximum amount of phosphorus that can be removed from the soil layer
    float uapl = 0.f; /// amount of phosphorus removed from layer
    float gx = 0.f; /// lowest depth in layer from which phosphorus may be removed
    up2 = m_frPlantP[i] * m_biomass[i];
    if (up2 < m_plantP[i]) up2 = m_plantP[i];
    uapd = up2 - m_plantP[i];
    uapd *= 1.5f;   /// luxury p uptake
    m_frStrsP[i] = 1.f;
    int ir = 0;
    if (uapd < UTIL_ZERO) return;
    for (int l = 0; l < m_nSoilLayers[i]; l++)
    {
        if (ir > 0) break;
        if (m_soilRD[i] <= m_soilDepth[i][l])
        {
            gx = m_soilRD[i];
            ir = 1;
        }
        else
            gx = m_soilDepth[i][l];
        upmx = uapd * (1.f - exp(-m_PUpDis * gx / m_soilRD[i])) / uobp;
        uapl = min(upmx - m_plantUpTkP[i], m_soilPsol[i][l]);
        m_plantUpTkP[i] += uapl;
        m_soilPsol[i][l] -= uapl;
    }
    if (m_plantUpTkP[i] < 0.f) m_plantUpTkP[i] = 0.f;
    m_plantP[i] += m_plantUpTkP[i];
    /// compute phosphorus stress
    PGCommon::calPlantStressByLimitedNP(m_plantP[i], up2, &m_frStrsP[i]);
}

void Biomass_EPIC::CheckDormantStatus(int i)
{
	/// TODO
	return;
}

int Biomass_EPIC::Execute()
{
    CheckInputData();
	initialOutputs();
	//cout<<"BIOEPIC, pre solno3: ";
	//for (int i = 0; i < m_nCells; i++)
	//{
	//	for (int j = 0; j < (int)m_nSoilLayers[i]; j++){
	//		if (m_soilNO3[i][j] != m_soilNO3[i][j])
	//			cout<<"cellid: "<<i<<"lyr: "<<j<<", "<<m_soilNO3[i][j]<<endl;
	//	}
	//}
	//cout<<endl;
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++)
    {
		/// calculate albedo in current day, albedo.f of SWAT
		float cej = -5.e-5f, eaj = 0.f;
		eaj = exp(cej * (m_sol_cov[i] + 0.1f));
		if (m_snowAcc[i] < 0.5f)
		{
			m_albedo[i] = m_soilALB[i];
			if (m_LAIDay[i] > 0.f)
				m_albedo[i] = 0.23f * (1.f - eaj) + m_soilALB[i] * eaj;
		}
		else
			m_albedo[i] = 0.8f;
		/// reWrite from plantmod.f of SWAT
        /// calculate residue on soil surface for current day
        m_sol_cov[i] = max(0.8f * m_biomass[i] + m_sol_rsd[i][0], 0.f);
        if (FloatEqual(m_igro[i], 1.f))            /// land cover growing
        {
            DistributePlantET(i);                  /// swu.f
            if (FloatEqual(m_dormFlag[i], 0.f))    /// plant will not undergo stress if dormant
                AdjustPlantGrowth(i);              /// plantmod.f
            CheckDormantStatus(i);                 /// dormant.f
        }
	}
	//cout<<"BIOEPIC, after solno3: ";
	//for (int i = 0; i < m_nCells; i++)
	//{
	//	for (int j = 0; j < (int)m_nSoilLayers[i]; j++){
	//		if (m_soilNO3[i][j] != m_soilNO3[i][j])
	//			cout<<"cellid: "<<i<<"lyr: "<<j<<", "<<m_soilNO3[i][j]<<endl;
	//	}
	//}
	//cout<<endl;
    return 0;
}

void Biomass_EPIC::Get1DData(const char *key, int *n, float **data)
{
    initialOutputs();
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_BIOMASS)) *data = m_biomass;
    else if (StringMatch(sk, VAR_LAST_SOILRD)) *data = m_lastSoilRootDepth;
    else if (StringMatch(sk, VAR_PLANT_P)) *data = m_plantP;
    else if (StringMatch(sk, VAR_PLANT_N)) *data = m_plantN;
	else if (StringMatch(sk, VAR_FR_PLANT_N)) *data = m_frPlantN;
	else if (StringMatch(sk, VAR_FR_PLANT_P)) *data = m_frPlantP;
    else if (StringMatch(sk, VAR_AET_PLT)) *data = m_plantEPDay;
    else if (StringMatch(sk, VAR_PLTPET_TOT)) *data = m_pltPET;
    else if (StringMatch(sk, VAR_PLTET_TOT)) *data = m_pltET;
    else if (StringMatch(sk, VAR_FR_PHU_ACC)) *data = m_frPHUacc;
    else if (StringMatch(sk, VAR_LAIDAY)) *data = m_LAIDay;
	else if (StringMatch(sk, VAR_LAIYRMAX)) *data = m_LAIYrMax;
	else if (StringMatch(sk, VAR_LAIMAXFR)) *data = m_LAIMaxFr;
	else if (StringMatch(sk, VAR_OLAI)) *data = m_oLAI;
    else if (StringMatch(sk, VAR_ALBDAY)) *data = m_albedo;
    else if (StringMatch(sk, VAR_DORMI)) *data = m_dormFlag;
	else if (StringMatch(sk, VAR_IGRO)) *data = m_igro;
	else if(StringMatch(sk,VAR_HVSTI_ADJ)) *data = m_hvstIdxAdj;
    else if (StringMatch(sk, VAR_CHT)) *data = m_cht;
	else if(StringMatch(sk, VAR_FR_ROOT)) *data = m_frRoot;
	else if (StringMatch(sk, VAR_FR_STRSWTR)) *data = m_frStrsWa;
    else if (StringMatch(sk, VAR_SOL_COV)) *data = m_sol_cov;
    else if (StringMatch(sk, VAR_SOL_SW)) *data = m_soilStorageProfile;
    else
        throw ModelException(MID_BIO_EPIC, "Get1DData", "Result " + sk +
                                                        " does not exist in current module. Please contact the module developer.");
}

void Biomass_EPIC::Get2DData(const char *key, int *nRows, int *nCols, float ***data)
{
	initialOutputs();
	string sk(key);
	*nRows = m_nCells;
	*nCols = m_soilLayers;
	if (StringMatch(sk, VAR_SOL_RSD)) *data = m_sol_rsd;
	else
		throw ModelException(MID_BIO_EPIC, "Get2DData", "Result " + sk + " does not exist.");
}