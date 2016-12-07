/*!
 * \brief Kinematic wave method for channel flow erosion and deposition
 * \author Hui Wu
 * \date Feb. 2012
 * \revised LiangJun Zhu
 * \revised date May. 2016
 */
#include "KinWavSed_CH.h"
#include "MetadataInfo.h"
#include "util.h"
#include "ModelException.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

#include <omp.h>

using namespace std;

KinWavSed_CH::KinWavSed_CH(void) : m_CellWith(-1), m_nCells(-1), m_TimeStep(NODATA_VALUE), m_chNumber(-1), m_Slope(NULL),
                                   m_chWidth(NULL), m_ChannelWH(NULL), m_flowInIndex(NULL), m_flowOutIndex(NULL),
                                   m_reachId(NULL), m_streamOrder(NULL),
                                   m_sourceCellIds(NULL), m_streamLink(NULL), m_ChQkin(NULL), m_ChVol(NULL),
                                   m_Qsn(NULL), m_CHDETFlow(NULL), m_CHSedDep(NULL),
                                   m_CHSed_kg(NULL), m_ChDetCo(NODATA_VALUE), m_USLE_K(NULL), m_SedToChannel(NULL),
                                   m_ChV(NULL),
                                   m_ChManningN(NULL), m_ChTcCo(NODATA_VALUE), m_CHSedConc(NULL), m_depCh(NULL)
{//m_SedSubbasin(NULL), deprecated by LJ
}

KinWavSed_CH::~KinWavSed_CH(void)
{
    if (m_CHDETFlow != NULL)
    {
        for (int i = 0; i < m_chNumber; ++i)
            delete[] m_CHDETFlow[i];
        delete[] m_CHDETFlow;
    }
    if (m_CHSedDep != NULL)
    {
        for (int i = 0; i < m_chNumber; ++i)
            delete[] m_CHSedDep[i];
        delete[] m_CHSedDep;
    }
    if (m_CHSed_kg != NULL)
    {
        for (int i = 0; i < m_chNumber; ++i)
            delete[] m_CHSed_kg[i];
        delete[] m_CHSed_kg;
    }
    if (m_CHSedConc != NULL)
    {
        for (int i = 0; i < m_chNumber; ++i)
            delete[] m_CHSedConc[i];
        delete[] m_CHSedConc;
    }
    if (m_Qsn != NULL)
    {
        for (int i = 0; i < m_chNumber; ++i)
            delete[] m_Qsn[i];
        delete[] m_Qsn;
    }
    if (m_ChVol != NULL)
    {
        for (int i = 0; i < m_chNumber; ++i)
            delete[] m_ChVol[i];
        delete[] m_ChVol;
    }
    if (m_ChV != NULL)
    {
        for (int i = 0; i < m_chNumber; ++i)
            delete[] m_ChV[i];
        delete[] m_ChV;
    }
    if (m_sourceCellIds != NULL)
        delete[] m_sourceCellIds;
    //if(m_SedSubbasin !=NULL)
    //	delete[] m_SedSubbasin;

    //test
    if (m_detCH != NULL)
        delete[] m_detCH;
    if (m_depCh != NULL)
        delete[] m_depCh;
    if (m_routQs != NULL)
        delete[] m_routQs;
    if (m_cap != NULL)
        delete[] m_cap;
    if (m_chanV != NULL)
        delete[] m_chanV;
    if (m_chanVol != NULL)
        delete[] m_chanVol;
}

void KinWavSed_CH::SetValue(const char *key, float data)
{
    string s(key);
    if (StringMatch(s, Tag_CellWidth)) m_CellWith = data;
    else if (StringMatch(s, Tag_CellSize)) m_nCells = (int) data;
    else if (StringMatch(s, Tag_HillSlopeTimeStep)) m_TimeStep = data;
    else if (StringMatch(s, VAR_CH_TCCO)) m_ChTcCo = data;
    else if (StringMatch(s, VAR_CH_DETCO)) m_ChDetCo = data;
    else if (StringMatch(s, VAR_OMP_THREADNUM)) omp_set_num_threads((int) data);
    else
        throw ModelException(MID_KINWAVSED_CH, "SetValue", "Parameter " + s + " does not exist in current module.\n");
}

void KinWavSed_CH::GetValue(const char *key, float *value)
{
    string sk(key);
    if (StringMatch(sk, VAR_SED_OUTLET))
    {
        map<int, vector<int> >::iterator it = m_reachLayers.end();
        it--;
        int reachId = it->second[0];
        int iLastCell = m_reachs[reachId].size() - 1;
        *value = m_Qsn[reachId][iLastCell];                  ///1000;    //kg -> ton
        //*value = m_CHSedConc[reachId][iLastCell];  //kg/m3
    }
    else
        throw ModelException(MID_KINWAVSED_CH, "GetValue", "Output " + sk
                                                           +
                                                           " does not exist in the current module. Please contact the module developer.");
}

void KinWavSed_CH::Set1DData(const char *key, int nRows, float *data)
{
    string s(key);

    CheckInputSize(key, nRows);

    if (StringMatch(s, VAR_SLOPE)) m_Slope = data;
    else if (StringMatch(s, VAR_CHWIDTH)) m_chWidth = data;
    else if (StringMatch(s, VAR_STREAM_LINK)) m_streamLink = data;
    else if (StringMatch(s, VAR_USLE_K)) m_USLE_K = data;
    else if (StringMatch(s, Tag_FLOWOUT_INDEX_D8)) m_flowOutIndex = data;
    else if (StringMatch(s, VAR_SED_TO_CH)) m_SedToChannel = data;

    else
        throw ModelException(MID_KINWAVSED_CH, "SetValue", "Parameter " + s +
                                                           " does not exist in current module. Please contact the module developer.");
}

void KinWavSed_CH::Get1DData(const char *key, int *n, float **data)
{
    string sk(key);
    *n = m_nCells;
    /*if (StringMatch(sk, "SEDSUBBASIN"))
    {
    *data = m_SedSubbasin;
    }*/
    if (StringMatch(sk, VAR_CH_DEP))
    {
        *data = m_depCh;
    }
    else if (StringMatch(sk, VAR_CH_DET))
    {
        *data = m_detCH;
    }
    else if (StringMatch(sk, VAR_CH_SEDRATE))
    {
        *data = m_routQs;
    }
    else if (StringMatch(sk, VAR_CH_FLOWCAP))
    {
        *data = m_cap;
    }
    else if (StringMatch(sk, VAR_CH_V))
    {
        *data = m_chanV;  // this is the problem
    }
    else if (StringMatch(sk, VAR_CH_VOL))
    {
        *data = m_chanVol;  // this is the problem
    }
    else
        throw ModelException(MID_KINWAVSED_CH, "Get1DData", "Output " + sk
                                                            +
                                                            " does not exist in the KinWavSed_CH module. Please contact the module developer.");
}

void KinWavSed_CH::Set2DData(const char *key, int nrows, int ncols, float **data)
{
    string sk(key);
    if (StringMatch(sk, Tag_ReachParameter))
    {
        m_chNumber = ncols; // overland is nrows;
        m_reachId = data[0];
        m_streamOrder = data[1];
        m_reachDownStream = data[2];
        m_ChManningN = data[3];          //  "Manning_nCh"
        for (int i = 0; i < m_chNumber; i++)
            m_idToIndex[(int) m_reachId[i]] = i;

        m_reachUpStream.resize(m_chNumber);
        for (int i = 0; i < m_chNumber; i++)
        {
            int downStreamId = int(m_reachDownStream[i]);
            if (downStreamId == 0)
                continue;
            int downStreamIndex = m_idToIndex[downStreamId];
            m_reachUpStream[downStreamIndex].push_back(i);
        }

    }
    else if (StringMatch(sk, Tag_FLOWIN_INDEX_D8))
        m_flowInIndex = data;
    else if (StringMatch(sk, VAR_HCH))
    {
        m_ChannelWH = data;
    }
    else if (StringMatch(sk, VAR_QRECH))
    {
        m_ChQkin = data;
        cout << m_ChQkin << endl;
    }
    else
        throw ModelException(MID_KINWAVSED_CH, "Set2DData", "Parameter " + sk
                                                            + " does not exist. Please contact the module developer.");

}

void KinWavSed_CH::Get2DData(const char *key, int *nRows, int *nCols, float ***data)
{
    /*string sk(key);
    *nRows = m_chNumber;
    if (StringMatch(sk, "SEDCONC"))
        *data = m_ChQ;
    else if (StringMatch(sk, "SEDINFLOW"))
        *data = m_CHSed_kg;
    else
        throw ModelException(MID_KINWAVSED_CH, "Get2DData", "Output " + sk
        + " does not exist in the KinWavSed_CH module. Please contact the module developer.");*/

}

bool KinWavSed_CH::CheckInputData()
{
    if (m_flowInIndex == NULL)
    {
        throw ModelException(MID_KINWAVSED_CH, "CheckInputData", "The parameter: flow in index has not been set.");
        return false;
    }
    if (m_date < 0)
    {
        throw ModelException(MID_KINWAVSED_CH, "CheckInputData", "You have not set the time.");
        return false;
    }
    if (m_CellWith <= 0)
    {
        throw ModelException(MID_KINWAVSED_CH, "CheckInputData", "The cell width can not be less than zero.");
        return false;
    }
    if (m_nCells <= 0)
    {
        throw ModelException(MID_KINWAVSED_CH, "CheckInputData", "The cell number can not be less than zero.");
        return false;
    }
    if (m_chNumber <= 0)
    {
        throw ModelException(MID_KINWAVSED_CH, "CheckInputData", "The channel reach number can not be less than zero.");
        return false;
    }
    if (m_TimeStep < 0)
    {
        throw ModelException(MID_KINWAVSED_CH, "CheckInputData", "You have not set the time step.");
        return false;
    }
    if (m_ChTcCo < 0)
    {
        throw ModelException(MID_KINWAVSED_CH, "CheckInputData",
                             "You have not set calibration coefficient of transport capacity.");
        return false;
    }
    if (m_ChDetCo < 0)
    {
        throw ModelException(MID_KINWAVSED_CH, "CheckInputData",
                             "You have not set calibration coefficient of channel flow detachment.");
        return false;
    }
    if (m_Slope == NULL)
    {
        throw ModelException(MID_KINWAVSED_CH, "CheckInputData", "The slope��%��can not be NULL.");
        return false;
    }
    if (m_ChManningN == NULL)
    {
        throw ModelException(MID_KINWAVSED_CH, "CheckInputData", "Manning N can not be NULL.");
        return false;
    }
    if (m_chWidth == NULL)
    {
        throw ModelException(MID_KINWAVSED_CH, "CheckInputData", "Channel width can not be NULL.");
        return false;
    }
    if (m_ChannelWH == NULL)
    {
        throw ModelException(MID_KINWAVSED_CH, "CheckInputData", "The channel water depth can not be NULL.");
        return false;
    }
    if (m_ChQkin == NULL)
    {
        throw ModelException(MID_KINWAVSED_CH, "CheckInputData", "The channel flow can not be NULL.");
        return false;
    }
    if (m_streamLink == NULL)
    {
        throw ModelException(MID_KINWAVSED_CH, "CheckInputData", "The stream link can not be NULL.");
        return false;
    }
    if (m_flowOutIndex == NULL)
    {
        throw ModelException(MID_KINWAVSED_CH, "CheckInputData", "The flow out index can not be NULL.");
        return false;
    }
    if (m_streamOrder == NULL)
    {
        throw ModelException(MID_KINWAVSED_CH, "CheckInputData",
                             "The stream order of reach parameter can not be NULL.");
        return false;
    }
    if (m_reachDownStream == NULL)
    {
        throw ModelException(MID_KINWAVSED_CH, "CheckInputData",
                             "The downstream of reach in reach parameter can not be NULL.");
        return false;
    }

    return true;
}

bool KinWavSed_CH::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        throw ModelException(MID_KINWAVSED_CH, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_nCells != n)
    {
        if (m_nCells <= 0) m_nCells = n;
        else
        {
            throw ModelException(MID_KINWAVSED_CH, "CheckInputSize", "Input data for " + string(key) +
                                                                     " is invalid. All the input data should have same size.");
            return false;
        }
    }

    return true;
}

void KinWavSed_CH::initial()
{
    if (m_depCh == NULL)
    {
        //test output
        m_depCh = new float[m_nCells];
        m_detCH = new float[m_nCells];
        m_routQs = new float[m_nCells];
        m_cap = new float[m_nCells];
        m_chanV = new float[m_nCells];
        m_chanVol = new float[m_nCells];
        for (int i = 0; i < m_nCells; i++)
        {
            m_depCh[i] = 0.0f;
            m_detCH[i] = 0.0f;
            m_routQs[i] = 0.0f;
            m_cap[i] = 0.0f;
            m_chanV[i] = 0.0f;
            m_chanVol[i] = 0.0f;
        }
        //allocate the output variable
        if (m_nCells <= 0)
            throw ModelException(MID_KINWAVSED_CH, "initialOutputs",
                                 "The cell number of the input can not be less than zero.");
        if (m_chNumber <= 0)
            throw ModelException(MID_KINWAVSED_CH, "initialOutputs",
                                 "The channel number of the input can not be less than zero.");

        if (m_CHSed_kg == NULL)
        {
            // find source cells the reaches
            m_sourceCellIds = new int[m_chNumber];
            for (int i = 0; i < m_chNumber; ++i)
                m_sourceCellIds[i] = -1;
            for (int i = 0; i < m_nCells; i++)
            {
                if (FloatEqual(m_streamLink[i], NODATA_VALUE))
                    continue;
                int reachId = (int) m_streamLink[i];
                bool isSource = true;
                for (int k = 1; k <= (int) m_flowInIndex[i][0]; ++k)
                {
                    int flowInId = (int) m_flowInIndex[i][k];
                    int flowInReachId = (int) m_streamLink[flowInId];
                    if (flowInReachId == reachId)
                    {
                        isSource = false;
                        break;
                    }
                }

                if ((int) m_flowInIndex[i][0] == 0)
                    isSource = true;

                if (isSource)
                {
                    int reachIndex = m_idToIndex[reachId];
                    m_sourceCellIds[reachIndex] = i;
                }
            }
            // get the cells in reaches according to flow direction
            for (int iCh = 0; iCh < m_chNumber; iCh++)
            {
                int iCell = m_sourceCellIds[iCh];
                int reachId = (int) m_streamLink[iCell];
                while ((int) m_streamLink[iCell] == reachId)
                {
                    m_reachs[iCh].push_back(iCell);
                    iCell = (int) m_flowOutIndex[iCell];
                }
            }

            if (m_reachLayers.empty())
            {
                for (int i = 0; i < m_chNumber; i++)
                {
                    int order = (int) m_streamOrder[i];
                    m_reachLayers[order].push_back(i);
                }
            }

            m_CHDETFlow = new float *[m_chNumber];
            m_CHSedDep = new float *[m_chNumber];
            m_CHSedConc = new float *[m_chNumber];
            m_Qsn = new float *[m_chNumber];
            //m_Qlastt = new float[m_chNumber];
            m_CHSed_kg = new float *[m_chNumber];
            //m_SedSubbasin = new float[m_chNumber];
            m_ChVol = new float *[m_chNumber];
            m_ChV = new float *[m_chNumber];
            for (int i = 0; i < m_chNumber; ++i)
            {
                //m_SedSubbasin[i] = 0.0f;
                int n = m_reachs[i].size();
                m_CHDETFlow[i] = new float[n];
                m_CHSedDep[i] = new float[n];
                m_CHSedConc[i] = new float[n];
                m_Qsn[i] = new float[n];
                //m_Qlastt[i] = new float[n];
                m_CHSed_kg[i] = new float[n];
                m_ChVol[i] = new float[n];
                m_ChV[i] = new float[n];
                for (int j = 0; j < n; ++j)
                {
                    m_CHDETFlow[i][j] = 0.f;
                    m_CHSedDep[i][j] = 0.f;
                    m_CHSedConc[i][j] = 0.f;
                    m_Qsn[i][j] = 0.f;
                    //m_Qlastt[i][j] = 0.f;
                    m_CHSed_kg[i][j] = 0.f;
                    m_ChVol[i][j] = 0.f;
                    m_ChV[i][j] = 0.f;

                }
            }
        }
    }
}

//float KinWavSed_CH::CalcuVelocityChannelFlow(int i)
//{
//	const float beta = 0.6f;
//	const float _23 = 2.0f/3;
//	float wh = m_ChannelWH[i]/1000;    // mm to m -> /1000 
//	float FW = m_FlowWidth[i];     
//	float S = sin(atan(m_Slope[i]));   //sine of the slope
//	float grad = sqrt(S);
//	float Perim = 2 * wh + FW;
//	float area = FW * wh;
//	float R = 0.0f; 
//	if(Perim > 0)
//		R = area / Perim;
//	else
//		R = 0.0f;
//	
//	float V = 0.0f;
//	V = pow(R, _23) * grad / m_ChManningN[i];
//	return V;
//}
void KinWavSed_CH::CalcuVelocityChannelFlow(int iReach, int iCell, int id)  //id is the cell id in the 1D array
{
    const float beta = 0.6f;
    const float _23 = 2.0f / 3;
    float wh = m_ChannelWH[iReach][iCell] / 1000;    // mm to m -> /1000
    float FW = m_chWidth[id];
    float S = sin(atan(m_Slope[id]));   //sine of the slope
    float grad = max(0.001f, sqrt(S));
    float Perim = 2 * wh + FW;
    float area = FW * wh;
    float R = 0.0f;
    if (Perim > 0)
        R = area / Perim;
    else
        R = 0.0f;

    //float V = 0.0f;
    m_ChV[iReach][iCell] = pow(R, _23) * grad / m_ChManningN[iReach];

    //test
    m_chanV[id] = wh; //m_ChV[iReach][iCell];
    //return V;
}

//float KinWavSed_CH::MaxConcentration(float watvol, float sedvol)
//{
//	float conc = (watvol > m_CellWith * m_CellWith * 1e-6 ? sedvol / watvol : 0);
//	if (conc > 848)
//	{
//		//dep += min(0, 848 * watvol - sedvol);
//		conc = 848;
//	}
//	//sedvol = conc * watvol;
//
//	return conc;
//}

//float KinWavSed_CH::MaxConcentration(float watvol, float sedvol, int iReach, int iCell)
//{
//	float conc = (watvol > m_CellWith * m_CellWith * 1e-6 ? sedvol / watvol : 0);
//	if (conc > 848)
//	{
//		m_CHSedDep[iReach][iCell] = min(0.f, 848 * watvol - sedvol);
//		conc = 848;
//	}
//	m_CHSed_kg[iReach][iCell] = conc * watvol;
//
//	return conc;
//}

void KinWavSed_CH::WaterVolumeCalc(int iReach, int iCell, int id)
{
    float slope, DX, wh;
    slope = atan(m_Slope[id]);
    DX = m_CellWith / cos(slope);
    wh = m_ChannelWH[iReach][iCell] /
         1000.f;  //mm -> m   : m_ChannelWH[iReach][iCell] => "HCH" from the channel routing module
    m_ChVol[iReach][iCell] = DX * m_chWidth[id] * wh;  // m3
    //test
    m_chanVol[id] = m_ChVol[iReach][iCell];
}

void KinWavSed_CH::CalcuChFlowDetachment(int iReach, int iCell, int id)  //i is the id of the cell in the grid map  
{
    //using simplified Srinivasan and Galvao (1995) equation to calculate channel flow detachment
    // the critical shear stress for sediment was neglected.
    float Df, shearStr, waterden, g, chwdeepth;
    float s = max(0.01f, m_Slope[id]);
    float S0 = sin(atan(s));
    waterden = 1000;
    g = 9.8f;
    chwdeepth = m_ChannelWH[iReach][iCell] / 1000;   // convert to m
    //test
    //m_detCH[id] = m_ChannelWH[iReach][iCell];
    shearStr = waterden * g * chwdeepth * S0;
    // kg/(m2*min)
    Df = m_ChDetCo * m_USLE_K[id] * pow(shearStr, 1.5f);
    ///  kg/(m2*min), convert to kg
    float DX, CHareas;
    DX = m_CellWith / cos(atan(s));
    CHareas = DX * m_chWidth[id];
    m_CHDETFlow[iReach][iCell] = Df * (m_TimeStep / 60) * CHareas;  //kg
    m_detCH[id] = m_CHDETFlow[iReach][iCell];
    /*if (m_detCH[id] >0 )
    {
        int test=0;
    }*/
}

//float KinWavSed_CH::GetTransportCapacity(int iReach, int iCell, int id)
//{
//	float q, S0, K, TranCap;
//	q = m_ChQkin[iReach][iCell] * 60;
//	float s = max(0.01f, m_Slope[id]);
//	S0 = sin(atan(s));
//	K = m_USLE_K[id];
//	
//	TranCap = m_ChTcCo * K * S0 * pow(q, 2.0f) * (m_TimeStep/60);   // convert to kg
//
//	/*if (q > 1)
//	{
//		int i = 0;
//	}*/
//	
//	return TranCap;
//}

float KinWavSed_CH::GetTransportCapacity(int iReach, int iCell, int id)
{
    // using Beasley et al. (1980) equation for transport capacity of sediment in channel flow
    float q, S0, K, TranCap, chVol;
    //WH, CalcuVelocityChannelFlow(iReach, iCell, id);
    //WH = m_ChannelWH[iReach][iCell];
    //q = m_ChV[iReach][iCell]* WH * 60;
    q = m_ChQkin[iReach][iCell] * 60;   // convert to m3/min
    float s = max(0.01f, m_Slope[id]);
    S0 = sin(atan(s));
    K = m_USLE_K[id];
    chVol = m_ChVol[iReach][iCell];
    if (chVol > 0)
        TranCap = m_ChTcCo * K * S0 * pow(q, 2.0f) * (m_TimeStep / 60) / chVol;   // kg/min, convert to kg/m3
        //float threadhold = 0.046f;
        //if(q < threadhold)
        //	TranCap = m_eco1 * K * S0 * sqrt(q) * (m_TimeStep/60);   // convert to kg
        //else
        //	TranCap = m_eco2 * K * S0 * pow(q, 2.0f) * (m_TimeStep/60);
        /*if (q > 1)
        {
            int i = 0;
        }*/
    else
        TranCap = 0.0f;
    return TranCap; //kg/m3
}

void KinWavSed_CH::GetSedimentInFlow(int iReach, int iCell, int id)
{
    float TC, Df, SedtoCh, Deposition, concentration, chVol;
    //float sedinf;
    TC = GetTransportCapacity(iReach, iCell, id);        //kg/m3
    m_cap[id] = TC;
    CalcuChFlowDetachment(iReach, iCell, id);
    Df = m_CHDETFlow[iReach][iCell];       //kg
    SedtoCh = m_SedToChannel[id];        //kg
    m_CHSed_kg[iReach][iCell] += (Df + SedtoCh);   //sediment in flow
    //while the sediment in flow is more than the capacity of the channel flow,
    // then the sediment deposition would happen; if the velocity of flow is reduced, the TC is decreased correspondingly,
    // then, the sediment deposition may take place.
    chVol = m_ChVol[iReach][iCell];
    if (chVol > 0.0f)
        concentration = m_CHSed_kg[iReach][iCell] / chVol;   //kg/m3
    else
        concentration = 0.0f;
    Deposition = max(concentration - TC, 0.0f);   //kg, >0
    if (Deposition > 0)
    {
        m_CHSed_kg[iReach][iCell] = TC * chVol;
    }
    m_CHSedDep[iReach][iCell] = Deposition * chVol; //kg/cell
    m_depCh[id] = m_CHSedDep[iReach][iCell];
}

float KinWavSed_CH::simpleSedCalc(float Qn, float Qin, float Sin, float dt, float vol, float sed)
{
    float Qsn = 0;
    float totsed = sed + Sin * dt;  // add upstream sed to sed present in cell
    float totwater = vol + Qin * dt;   // add upstream water to volume water in cell
    if (totwater <= 1e-10)
        return (Qsn);
    Qsn = min(totsed / dt, Qn * totsed / totwater);
    return (Qsn); // sedoutflow is new concentration * new out flux

}

float KinWavSed_CH::complexSedCalc(float Qj1i1, float Qj1i, float Qji1, float Sj1i, float Sji1, float alpha, float dt,
                                   float dx)
{
    float Sj1i1, Cavg, Qavg, aQb, abQb_1, A, B, C, s = 0;
    const float beta = 0.6f;


    if (Qj1i1 < 1e-6)
        return (0);

    Qavg = 0.5f * (Qji1 + Qj1i);
    if (Qavg <= 1e-6)
        return (0);

    Cavg = (Sj1i + Sji1) / (Qj1i + Qji1);
    aQb = alpha * pow(Qavg, beta);
    abQb_1 = alpha * beta * pow(Qavg, beta - 1);

    A = dt * Sj1i;
    B = -dx * Cavg * abQb_1 * (Qj1i1 - Qji1);
    C = (Qji1 <= 1e-6 ? 0 : dx * aQb * Sji1 / Qji1);
    if (Qj1i1 > 1e-6)
        Sj1i1 = (dx * dt * s + A + C + B) / (dt + dx * aQb / Qj1i1);    //why s = 0 ?
    else
        Sj1i1 = 0;

    return max(0.0f, Sj1i1);
}

void KinWavSed_CH::ChannelflowSedRouting(int iReach, int iCell, int id)
{
    //sum the sediment of the upstream overland flow
    float Sin = 0.f;
    float Qin = 0.f;
    if (iCell == 0)// inflow of this cell is the last cell of the upstream reach
    {
        for (size_t i = 0; i < m_reachUpStream[iReach].size(); ++i)
        {
            int upReachId = m_reachUpStream[iReach][i];
            if (upReachId >= 0)
            {
                int upCellsNum = m_reachs[upReachId].size();
                int upCellId = m_reachs[upReachId][upCellsNum - 1];
                Qin += m_ChQkin[upReachId][upCellsNum - 1];
                Sin += m_Qsn[upReachId][upCellsNum - 1];
            }
        }
    }
    else
    {
        Qin = m_ChQkin[iReach][iCell - 1];  //the iCell-1 is the prior of the iCell
        Sin = m_Qsn[iReach][iCell - 1];
    }
    //m_Qlastt[iReach][iCell] = m_ChQ[iReach][iCell];  // for the next time step

    WaterVolumeCalc(iReach, iCell, id);
    float WtVol = m_ChVol[iReach][iCell];
    // calculate sediment channel routing
    GetSedimentInFlow(iReach, iCell, id);
    //m_routQs[id] = m_ChQkin[iReach][iCell];
    m_Qsn[iReach][iCell] = simpleSedCalc(m_ChQkin[iReach][iCell], Qin, Sin, m_TimeStep, WtVol,
                                         m_CHSed_kg[iReach][iCell]);

    ////---- another method
    //float beta = 0.6f;
    //float beta1 = 1/beta;
    //float _23 = 2.0f/3;
    //float Perim = 2 * m_ChannelWH[iReach][iCell]/1000 + m_chWidth[id];
    //float S = sin(atan(m_Slope[id]));
    //float Alpha = pow(m_ChManningN[id]/sqrt(S) * pow(Perim, _23), beta);
    //float Q, Qs;
    //if(Alpha > 0)
    //	Q = pow(m_chWidth[id]*m_ChannelWH[iReach][iCell] / Alpha, beta1);
    //else
    //	Q = 0;
    //Qs = Q * m_CHSedConc[iReach][iCell];
    //float DX = m_CellWith/cos(asin(S));
    //m_Qsn[iReach][iCell] = complexSedCalc(m_ChQ[iReach][iCell], Qin, Q, Sin, Qs, Alpha, m_TimeStep, DX);
    //----end

    float tem = Sin + m_CHSed_kg[iReach][iCell] / m_TimeStep;    //kg/s
    // no more sediment outflow than total sed in cell
    m_Qsn[iReach][iCell] = min(m_Qsn[iReach][iCell], tem);
    m_routQs[id] = m_Qsn[iReach][iCell];
    tem = Sin * m_TimeStep + m_CHSed_kg[iReach][iCell] - m_Qsn[iReach][iCell] * m_TimeStep;
    // new sed volume based on all fluxes and or sed present
    m_CHSed_kg[iReach][iCell] = max(0.0f, tem);
    float concentration = 0;
    if (WtVol > 0)
    {
        concentration = m_CHSed_kg[iReach][iCell] / WtVol;   //kg/m3
    }
    m_CHSedConc[iReach][iCell] = concentration;
}

//void KinWavSed_CH::setNotAvailableInput()
//{
//	if(m_nCells <= 0)
//	{
//		throw ModelException(MID_KINWAVSED_CH,"CheckInputData","The cell number can not be less than zero.");
//		//exit(-1);
//	}
//	if(m_COH == NULL)
//	{
//		m_COH = new float[m_nCells];
//		m_D50 = new float[m_nCells];
//
//		for(int i=0; i<m_nCells; i++)
//		{
//			m_COH[i] = 0.99f;     //1 is no erosion
//			m_D50[i] = 50;     //240;   //32-300
//
//		}
//	}
//}

int KinWavSed_CH::Execute()
{
    CheckInputData();

    initial();
    //StatusMsg("executing KinWavSed_CH");
    map<int, vector<int> >::iterator it;
    for (it = m_reachLayers.begin(); it != m_reachLayers.end(); it++)
    {
        // There are not any flow relationship within each routing layer.
        // So parallelization can be done here.
        int nReaches = it->second.size();
        // the size of m_reachLayers (map) is equal to the maximum stream order
#pragma omp parallel for
        for (int i = 0; i < nReaches; ++i)
        {
            int reachIndex = it->second[i]; // index in the array
            vector<int> &vecCells = m_reachs[reachIndex];
            int n = vecCells.size();
            for (int iCell = 0; iCell < n; ++iCell)
            {
                ChannelflowSedRouting(reachIndex, iCell, vecCells[iCell]);
            }
            //m_SedSubbasin[reachIndex] = m_Qsn[reachIndex][n-1]/1000;   //kg/s -> ton/s
        }
    }
    //StatusMsg("end of executing KinWavSed_CH");

    return 0;
}