/*//
 * \file NutrientOLRoute.cpp
 * \ingroup NutrOL
 * \author Huiran Gao
 * \date Jun 2016
 */

#include <iostream>
#include "NutrOL_IKW.h"
#include "MetadataInfo.h"
#include <cmath>
#include <fstream>
#include "ModelException.h"
#include "util.h"
#include <omp.h>

using namespace std;

NutrientOL_IKW::NutrientOL_IKW(void):
//input
        m_nCells(-1), m_cellWidth(-1), m_nLayers(-1), m_TimeStep(NODATA_VALUE),
        m_routingLayers(NULL), m_flowInIndex(NULL), m_FlowWidth(NULL), m_chWidth(NULL), m_streamLink(NULL),
        m_latno3(NULL), m_surqno3(NULL), m_surqsolp(NULL), m_no3gw(NULL),
        m_minpgw(NULL), m_sedorgn(NULL), m_sedorgp(NULL), m_sedminpa(NULL), m_sedminps(NULL),
        m_ChV(NULL), m_QV(NULL), m_fract(NULL), m_cod(NULL),
        //output
        m_surqno3ToCh(NULL), m_latno3ToCh(NULL), m_no3gwToCh(NULL), m_surqsolpToCh(NULL), m_minpgwToCh(NULL),
        m_sedorgnToCh(NULL), m_sedorgpToCh(NULL), m_sedminpaToCh(NULL), m_sedminpsToCh(NULL),
        m_ammoToCh(NULL), m_nitriteToCh(NULL), m_codToCh(NULL)
{

}

NutrientOL_IKW::~NutrientOL_IKW(void)
{
	if (m_QV != NULL) Release1DArray(m_QV);
	if (m_ChV != NULL) Release1DArray(m_ChV);
	if (m_fract != NULL) Release1DArray(m_fract);
	if (m_surqno3ToCh != NULL) Release1DArray(m_surqno3ToCh);
	if (m_latno3ToCh != NULL) Release1DArray(m_latno3ToCh);
	if (m_no3gwToCh != NULL) Release1DArray(m_no3gwToCh);
	if (m_surqsolpToCh != NULL) Release1DArray(m_surqsolpToCh);
	if (m_minpgwToCh != NULL) Release1DArray(m_minpgwToCh);
	if (m_sedorgnToCh != NULL) Release1DArray(m_sedorgnToCh);
	if (m_sedorgpToCh != NULL) Release1DArray(m_sedorgpToCh);
	if (m_sedminpaToCh != NULL) Release1DArray(m_sedminpaToCh);
	if (m_sedminpsToCh != NULL) Release1DArray(m_sedminpsToCh);
	if (m_ammoToCh != NULL) Release1DArray(m_ammoToCh);
	if (m_nitriteToCh != NULL) Release1DArray(m_nitriteToCh);
	if (m_codToCh != NULL) Release1DArray(m_codToCh);
}

bool NutrientOL_IKW::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        throw ModelException(MID_NutOLRout, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_nCells != n)
    {
        if (m_nCells <= 0)
        {
            m_nCells = n;
        } else
        {
            //StatusMsg("Input data for "+string(key) +" is invalid. All the input data should have same size.");
            ostringstream oss;
            oss << "Input data for " + string(key) << " is invalid with size: " << n << ". The origin size is " <<
            m_nCells << ".\n";
            throw ModelException(MID_NutOLRout, "CheckInputSize", oss.str());
        }
    }
    return true;
}

bool NutrientOL_IKW::CheckInputData()
{
    if (this->m_nCells <= 0)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The cells number can not be less than zero.");
    }
    if (this->m_cellWidth <= 0)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The cell width can not be less than zero.");
    }
    if (this->m_nLayers <= 0)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The layers of reaches data can not be less than zero.");
    }
    if (this->m_TimeStep == NODATA_VALUE)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The time can not be zero.");
    }
    if (this->m_routingLayers == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "Routing layers can not be NULL.");
    }
    if (this->m_flowInIndex == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The 2d array of flow in cells data can not be NULL.");
    }
    if (this->m_sedminps == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The amount of stable mineral phosphorus absorbed to sediment in surface runoff can not be NULL.");
    }
    if (this->m_sedminpa == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The amount of active mineral phosphorus absorbed to sediment in surface runoff can not be NULL.");
    }
    if (this->m_sedorgp == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The amount of organic phosphorus in surface runoff can not be NULL.");
    }
    if (this->m_sedorgn == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The amount of organic nitrogen in surface runoff can not be NULL.");
    }
    if (this->m_minpgw == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The soluble P loading to reach in groundwater can not be NULL.");
    }
    if (this->m_no3gw == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The nitrate loading to reach in groundwater can not be NULL.");
    }
    if (this->m_surqsolp == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The amount of soluble phosphorus in surface runoff can not be NULL.");
    }
    if (this->m_surqno3 == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The amount of nitrate transported with surface runoff can not be NULL.");
    }
    if (this->m_latno3 == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The amount of nitrate transported with lateral flow can not be NULL.");
    }
    if (this->m_streamLink == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The stream link data can not be NULL.");
    }
    if (this->m_chWidth == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The channel width can not be NULL.");
    }
    if (this->m_FlowWidth == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The flow width of overland plane can not be NULL.");
    }
    if (this->m_cod == NULL)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData", "The carbonaceous oxygen demand of surface runoff can not be NULL.");
    }
    return true;
}

void NutrientOL_IKW::SetValue(const char *key, float value)
{
    string sk(key);
    if (StringMatch(sk, VAR_OMP_THREADNUM))
    {
        omp_set_num_threads((int) value);
    }
    else if (StringMatch(sk, Tag_CellSize)) { this->m_nCells = (int)value; }
    else if (StringMatch(sk, Tag_CellWidth)) { this->m_cellWidth = value; }
    else if (StringMatch(sk, Tag_HillSlopeTimeStep)) { this->m_TimeStep = value; }
    else
    {
        throw ModelException(MID_NutOLRout, "SetValue", "Parameter " + sk +
                                                      " does not exist in Nutrient method. Please contact the module developer.");
    }
}

void NutrientOL_IKW::Set1DData(const char *key, int n, float *data)
{
    if (!this->CheckInputSize(key, n)) return;
    string sk(key);
    if (StringMatch(sk, VAR_STREAM_LINK)) { this->m_streamLink = data; }
    else if (StringMatch(sk, VAR_CHWIDTH)) { this->m_chWidth = data; }
    else if (StringMatch(sk, VAR_FLOWWIDTH)) { this->m_FlowWidth = data; }
    else if (StringMatch(sk, VAR_LATNO3)) { this->m_latno3 = data; }
    else if (StringMatch(sk, VAR_SURQNO3)) { this->m_surqno3 = data; }
        //else if(StringMatch(sk, VAR_AMMONIAN)) {this -> m_ammo = data;}
    else if (StringMatch(sk, VAR_SURQSOLP)) { this->m_surqsolp = data; }
    else if (StringMatch(sk, VAR_NO3GW)) { this->m_no3gw = data; }
    else if (StringMatch(sk, VAR_MINPGW)) { this->m_minpgw = data; }
    else if (StringMatch(sk, VAR_SEDORGN)) { this->m_sedorgn = data; }
    else if (StringMatch(sk, VAR_SEDORGP)) { this->m_sedorgp = data; }
    else if (StringMatch(sk, VAR_SEDMINPA)) { this->m_sedminpa = data; }
    else if (StringMatch(sk, VAR_SEDMINPS)) { this->m_sedminps = data; }
    else if (StringMatch(sk, VAR_COD)) { this->m_cod = data; }
    else
    {
        throw ModelException(MID_NutOLRout, "SetValue", "Parameter " + sk +
                                                      " does not exist in CLIMATE module. Please contact the module developer.");
    }
}

void NutrientOL_IKW::Set2DData(const char *key, int nrows, int ncols, float **data)
{
	if (!this->CheckInputSize(key, nrows)) return;
    string sk(key);
    if (StringMatch(sk, Tag_ROUTING_LAYERS))
    {
        m_routingLayers = data;
        m_nLayers = nrows;
    }
    else if (StringMatch(sk, Tag_FLOWIN_INDEX_D8))
    {
        m_flowInIndex = data;
    } else
    {
        throw ModelException(MID_NutOLRout, "Set2DData",
                             "Parameter " + sk + " does not exist. Please contact the module developer.");
    }
}

void NutrientOL_IKW::initialOutputs()
{
    if (this->m_nCells <= 0)
    {
        throw ModelException(MID_NutOLRout, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    }
    // allocate the output variables
    if (m_surqno3ToCh == NULL)
    {
        for (int i = 0; i < m_nCells; i++)
        {
            m_surqno3ToCh[i] = 0.f;
            m_latno3ToCh[i] = 0.f;
            m_no3gwToCh[i] = 0.f;
            m_surqsolpToCh[i] = 0.f;
            m_minpgwToCh[i] = 0.f;
            m_sedorgnToCh[i] = 0.f;
            m_sedorgpToCh[i] = 0.f;
            m_sedminpaToCh[i] = 0.f;
            m_sedminpsToCh[i] = 0.f;
            m_ammoToCh[i] = 0.f;
            m_nitriteToCh[i] = 0.f;
            m_codToCh[i] = 0.f;
        }
    }
}

int NutrientOL_IKW::Execute()
{
    if (!this->CheckInputData())
    {
        return false;
    }
    this->initial();
    this->initialOutputs();
#pragma omp parallel for
    for (int iLayer = 0; iLayer < m_nLayers; ++iLayer)
    {
        // There are not any flow relationship within each routing layer.
        int nCells = (int) m_routingLayers[iLayer][0];
        for (int iCell = 1; iCell <= nCells; ++iCell)
        {
            int id = (int) m_routingLayers[iLayer][iCell];
            NutrientinOverland(id);
        }
    }
    //return ??
    return 0;
}

void NutrientOL_IKW::initial()
{
    if (m_QV == NULL)
    {
        m_QV = new float[m_nCells];
        m_ChV = new float[m_nCells];
        m_fract = new float[m_nCells];
        for (int i = 0; i < m_nCells; i++)
        {
            m_QV[i] = 0.f;
            m_ChV[i] = 0.f;
            m_fract[i] = 0.f;
        }
    }
}
float NutrientOL_IKW::CalculateSedinFlowFraction(int id)
{
	//TODO
	return 0.f;
}
void NutrientOL_IKW::NutrientinOverland(int i)
{
	// Calculate the ratio between sediment in flow and soil loss caused by water erosion
	float fractionSed = 0.f;
	fractionSed = CalculateSedinFlowFraction(i);
	//TODO

    //sum the nutrients of the upstream overland flow
    float flowwidth = m_FlowWidth[i];
    float cellArea = m_cellWidth * m_cellWidth / 10000.f; // /10000.f m2 -> ha
    float surqno3 = 0.f;    // sum of surqno3 flow in
    float latno3 = 0.f;    // sum of latno3 flow in
    float ammo = 0.f;    // sum of ammonium flow in
    float no3gw = 0.f;    // sum of no3gw flow in
    float surqsolp = 0.f;    // sum of surqsolp flow in
    float minpgw = 0.f;    // sum of minpgw flow in
    float sedorgn = 0.f;    // sum of sedorgn flow in
    float sedorgp = 0.f;    // sum of sedorgp flow in
    float sedminpa = 0.f;    // sum of sedminpa flow in
    float sedminps = 0.f;    // sum of sedminps flow in
    float cod = 0.f;    // sum of cod flow in

    for (int k = 1; k <= (int) m_flowInIndex[i][0]; ++k)
    {
        int flowInID = (int) m_flowInIndex[i][k];
        // Calculate amount of nutrients in surface runoff
        m_surqno3[flowInID] = max(1.e-12f, m_surqno3[flowInID]);
        m_latno3[flowInID] = max(1.e-12f, m_latno3[flowInID]);
        m_no3gw[flowInID] = max(1.e-12f, m_no3gw[flowInID]);
        m_surqsolp[flowInID] = max(1.e-12f, m_surqsolp[flowInID]);
        m_minpgw[flowInID] = max(1.e-12f, m_minpgw[flowInID]);
        m_sedorgn[flowInID] = max(1.e-12f, m_sedorgn[flowInID]);
        m_sedorgp[flowInID] = max(1.e-12f, m_sedorgp[flowInID]);
        m_sedminpa[flowInID] = max(1.e-12f, m_sedminpa[flowInID]);
        m_sedminps[flowInID] = max(1.e-12f, m_sedminps[flowInID]);
        m_cod[flowInID] = max(1.e-12f, m_cod[flowInID]);
        //Sum
		// if the upstream cell is not a channel cell (kg/ha)
		 if (m_streamLink[flowInID] < 0) 
		 {
			surqno3 = surqno3 + m_surqno3[flowInID];
			latno3 = latno3 + m_latno3[flowInID];
			no3gw = no3gw + m_no3gw[flowInID];
			surqsolp = surqsolp + m_surqsolp[flowInID];
			minpgw = minpgw + m_minpgw[flowInID];
			sedorgn = sedorgn + m_sedorgn[flowInID];
			sedorgp = sedorgp + m_sedorgp[flowInID];
			sedminpa = sedminpa + m_sedminpa[flowInID];
			sedminps = sedminps + m_sedminps[flowInID];
			cod = cod + m_cod[flowInID];
		 }
    }
    // if the channel width is greater than the cell width
    if (m_streamLink[i] >= 0 && flowwidth <= 0)
    {
        m_surqno3ToCh[i] = surqno3 * cellArea;
        m_latno3ToCh[i] = latno3 * cellArea;
        m_no3gwToCh[i] = no3gw * cellArea;
        m_surqsolpToCh[i] = surqsolp * cellArea;
        m_minpgwToCh[i] = minpgw * cellArea;
        m_sedorgnToCh[i] = sedorgn * cellArea;
        m_sedorgpToCh[i] = sedorgp * cellArea;
        m_sedminpaToCh[i] = sedminpa * cellArea;
        m_sedminpsToCh[i] = sedminps * cellArea;
        m_cod[i] = cod * cellArea; // TODO calculate volume
        return;
    }

    // Calculate nutrients to channel
    m_surqno3ToCh[i] = NutToChannel(i, m_surqno3ToCh[i]);
    m_latno3ToCh[i] = NutToChannel(i, m_latno3ToCh[i]);
    m_no3gwToCh[i] = NutToChannel(i, m_no3gwToCh[i]);
    m_surqsolpToCh[i] = NutToChannel(i, m_surqsolpToCh[i]);
    m_minpgwToCh[i] = NutToChannel(i, m_minpgwToCh[i]);
    m_sedorgnToCh[i] = NutToChannel(i, m_sedorgnToCh[i]);
    m_sedorgpToCh[i] = NutToChannel(i, m_sedorgpToCh[i]);
    m_sedminpaToCh[i] = NutToChannel(i, m_sedminpaToCh[i]);
    m_sedminpsToCh[i] = NutToChannel(i, m_sedminpsToCh[i]);
    m_codToCh[i] = NutToChannel(i, m_codToCh[i]);
    m_ammoToCh[i] = 0.f;
    m_nitriteToCh[i] = 0.f;
    m_codToCh[i] = m_codToCh[i] / 1.e6f; //mg to kg
}

float NutrientOL_IKW::NutToChannel(int id, float nut)
{
        float fractiontochannel = 0.f;
        if (m_chWidth[id] > 0)
        {
            float tem = m_ChV[id] * m_TimeStep;
            fractiontochannel = 2.f * tem / (m_cellWidth - m_chWidth[id]);
            fractiontochannel = min(fractiontochannel, 1.f);
        }
        float nuttoch = fractiontochannel * nut;
        return nuttoch;
}
void NutrientOL_IKW::Get1DData(const char *key, int *n, float **data)
{
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_SUR_NO3_CH)) { *data = this->m_surqno3ToCh; }
    else if (StringMatch(sk, VAR_LATNO3_CH)) { *data = this->m_latno3ToCh; }
    else if (StringMatch(sk, VAR_NO3GW_CH)) { *data = this->m_no3gwToCh; }
    else if (StringMatch(sk, VAR_SUR_SOLP_CH)) { *data = this->m_surqsolpToCh; }
    else if (StringMatch(sk, VAR_MINPGW_CH)) { *data = this->m_minpgwToCh; }
    else if (StringMatch(sk, VAR_SEDORGN_CH)) { *data = this->m_sedorgnToCh; }
    else if (StringMatch(sk, VAR_SEDORGP_CH)) { *data = this->m_sedorgpToCh; }
    else if (StringMatch(sk, VAR_SEDMINPA_CH)) { *data = this->m_sedminpaToCh; }
    else if (StringMatch(sk, VAR_SEDMINPS_CH)) { *data = this->m_sedminpsToCh; }
    else if (StringMatch(sk, VAR_AMMO_CH)) { *data = this->m_ammoToCh; }
    else if (StringMatch(sk, VAR_NITRITE_CH)) { *data = this->m_nitriteToCh; }
    else if (StringMatch(sk, VAR_COD_CH)) { *data = this->m_codToCh; }
    else
    {
        throw ModelException(MID_NutOLRout, "GetValue",
                             "Parameter " + sk + " does not exist. Please contact the module developer.");
    }
}
