/*!
 * \brief Kinematic wave routing method for overland erosion and deposition
 * \author Hui Wu
 * \date Feb. 2012
 * \revised LiangJun Zhu
 * \revised date May. 2016
 */
#include "KinWavSed_OL.h"
#include "MetadataInfo.h"
#include "util.h"
#include "ModelException.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <omp.h>

using namespace std;

KinWavSed_OL::KinWavSed_OL(void) : m_CellWidth(-1), m_nCells(-1), m_TimeStep(NODATA_VALUE), m_nLayers(-1),
                                   m_routingLayers(NULL),
                                   m_Slope(NULL), m_DETOverland(NULL), m_USLE_K(NULL), m_Ctrans(NULL), m_Qkin(NULL),
                                   m_FlowWidth(NULL), m_DETSplash(NULL),
                                   m_eco1(NODATA_VALUE), m_eco2(NODATA_VALUE), m_V(NULL), m_Qsn(NULL), m_Vol(NULL), m_SedDep(NULL),
                                   m_Sed_kg(NULL), m_SedToChannel(NULL),
                                   m_ManningN(NULL), m_whtoCh(NULL), m_USLE_C(NULL), m_Ccoe(NODATA_VALUE), m_WH(NULL),
                                   m_streamLink(NULL)
{

}

KinWavSed_OL::~KinWavSed_OL(void)
{
    if (m_Ctrans != NULL)
    {
        delete[] m_Ctrans;
    }
    if (m_DETOverland != NULL)
    {
        delete[] m_DETOverland;
    }
    if (m_SedDep != NULL)
    {
        delete[] m_SedDep;
    }
    if (m_SedToChannel != NULL)
    {
        delete[] m_SedToChannel;
    }
    if (m_Sed_kg != NULL)
    {
        delete[] m_Sed_kg;
    }
    if (m_Vol != NULL)
    {
        delete[] m_Vol;
    }
    if (m_V != NULL)
    {
        delete[] m_V;
    }
    if (m_Qsn != NULL)
    {
        delete[] m_Qsn;
    }
    //test
    if (m_ChV != NULL)
    {
        delete[] m_ChV;
    }
    if (m_QV != NULL)
    {
        delete[] m_QV;
    }
    if (m_fract != NULL)
    {
        delete[] m_fract;
    }
}

void KinWavSed_OL::Get1DData(const char *key, int *n, float **data)
{
    *n = m_nCells;
    string s(key);
    if (StringMatch(s, VAR_OL_DET))
    {
        *data = m_DETOverland;
    }
    else if (StringMatch(s, VAR_SED_DEP))
    {
        *data = m_SedDep;
    }
    else if (StringMatch(s, VAR_SED_FLOW))
    {
        *data = m_Sed_kg;
	}
	else if (StringMatch(s, VAR_SED_FLUX))
	{
		*data = m_Qsn;
	}
	else if (StringMatch(s, VAR_SED_TO_CH))
	{
		*data = m_SedToChannel;
	}
        //else if(StringMatch(s,"TestV"))
        //{
        //	*data = m_ChV; //m_ChV; //m_V;//m_tsetv;
        //}
        //else if(StringMatch(s,"TestQV"))
        //{
        //	*data = m_QV;//m_tsetv;
        //}
    else
        throw ModelException(MID_KINWAVSED_OL, "Get1DData",
                             "Result " + s + " does not exist in current module. Please contact the module developer.");

}

void KinWavSed_OL::Set1DData(const char *key, int nRows, float *data)
{
    string s(key);

    CheckInputSize(key, nRows);

    if (StringMatch(s, VAR_SLOPE)) m_Slope = data;
    else if (StringMatch(s, VAR_MANNING)) m_ManningN = data;
    else if (StringMatch(s, VAR_STREAM_LINK)) m_streamLink = data;
    else if (StringMatch(s, VAR_USLE_K)) m_USLE_K = data;
    else if (StringMatch(s, VAR_USLE_C)) m_USLE_C = data;
    else if (StringMatch(s, VAR_CHWIDTH)) m_chWidth = data;
        //else if(StringMatch(s,"D_HTOCH"))		m_whtoCh = data;
    else if (StringMatch(s, VAR_SURU)) m_WH = data;
    else if (StringMatch(s, "D_QOverland")) m_Qkin = data;
    else if (StringMatch(s, "D_DETSplash")) m_DETSplash = data;
    else if (StringMatch(s, "D_FlowWidth")) m_FlowWidth = data;
    else
        throw ModelException(MID_KINWAVSED_OL, "Set1DData", "Parameter " + s +
                                                            " does not exist in current module. Please contact the module developer.");
}

void KinWavSed_OL::Set2DData(const char *key, int nrows, int ncols, float **data)
{
    //check the input data
    //m_nLayers = nrows;
    string sk(key);
    if (StringMatch(sk, Tag_ROUTING_LAYERS))
    {
        m_routingLayers = data;
        m_nLayers = nrows;
    }
    else if (StringMatch(sk, Tag_FLOWIN_INDEX_D8))
        m_flowInIndex = data;
    else
        throw ModelException(MID_KINWAVSED_OL, "Set2DData", "Parameter " + sk
                                                            + " does not exist. Please contact the module developer.");
}

void KinWavSed_OL::SetValue(const char *key, float data)
{
    string s(key);
    if (StringMatch(s, Tag_CellWidth)) m_CellWidth = data;
    else if (StringMatch(s, Tag_CellSize)) m_nCells = (int) data;
    else if (StringMatch(s, Tag_HillSlopeTimeStep)) m_TimeStep = data;//
    else if (StringMatch(s, VAR_OL_SED_ECO1)) m_eco1 = data;//
    else if (StringMatch(s, VAR_OL_SED_ECO2)) m_eco2 = data;//
    else if (StringMatch(s, VAR_OL_SED_CCOE)) m_Ccoe = data;
    else if (StringMatch(s, VAR_OMP_THREADNUM)) omp_set_num_threads((int) data);
    else
        throw ModelException(MID_KINWAVSED_OL, "SetValue", "Parameter " + s +
                                                           " does not exist in current module. Please contact the module developer.");
}

//string KinWavSed_OL::toString(float value)
//{
//	ostringstream oss;
//	oss << value;
//	return oss.str();
//}

bool KinWavSed_OL::CheckInputData()
{
    if (m_routingLayers == NULL)
        throw ModelException(MID_KINWAVSED_OL, "CheckInputData", "The parameter: routingLayers has not been set.");
    if (m_flowInIndex == NULL)
        throw ModelException(MID_KINWAVSED_OL, "CheckInputData", "The parameter: flow in index has not been set.");
    if (m_date < 0)
    {
        throw ModelException(MID_KINWAVSED_OL, "CheckInputData", "You have not set the time.");
        return false;
    }
    if (m_CellWidth <= 0)
    {
        throw ModelException(MID_KINWAVSED_OL, "CheckInputData", "The cell width can not be less than zero.");
        return false;
    }
    if (m_nCells <= 0)
    {
        throw ModelException(MID_KINWAVSED_OL, "CheckInputData", "The cell number can not be less than zero.");
        return false;
    }
    if (m_TimeStep < 0)
    {
        throw ModelException(MID_KINWAVSED_OL, "CheckInputData", "You have not set the time step.");
        return false;
    }
    if (m_eco1 < 0)
    {
        throw ModelException(MID_KINWAVSED_OL, "CheckInputData", "You have not set the calibration coefficient 1.");
        return false;
    }
    if (m_eco2 < 0)
    {
        throw ModelException(MID_KINWAVSED_OL, "CheckInputData", "You have not set the calibration coefficient 2.");
        return false;
    }
    if (m_Ccoe < 0)
    {
        throw ModelException("Soil_DET", "CheckInputData",
                             "You have not set the calibration coefficient of overland erosion.");
        return false;
    }
    if (m_USLE_K == NULL)
    {
        throw ModelException(MID_KINWAVSED_OL, "CheckInputData", "You have not set the USLE K (Erosion factor).");
        return false;
    }
    if (m_USLE_C == NULL)
    {
        throw ModelException(MID_KINWAVSED_OL, "CheckInputData", "The parameter of USLE_C can not be NULL.");
        return false;
    }
    if (m_Slope == NULL)
    {
        throw ModelException(MID_KINWAVSED_OL, "CheckInputData", "The slope��%��can not be NULL.");
        return false;
    }
    if (m_DETSplash == NULL)
    {
        throw ModelException(MID_KINWAVSED_OL, "CheckInputData",
                             "The distribution of splash detachment can not be NULL.");
        return false;
    }
    if (m_chWidth == NULL)
    {
        throw ModelException(MID_KINWAVSED_OL, "CheckInputData", "Channel width can not be NULL.");
        return false;
    }
    if (m_WH == NULL)
    {
        throw ModelException(MID_KINWAVSED_OL, "CheckInputData",
                             "The depth of the surface water layer can not be NULL.");
        return false;
    }
    //if (m_Runoff == NULL)
    //{
    //	throw ModelException(MID_KINWAVSED_OL,"CheckInputData","The surface runoff can not be NULL.");
    //	return false;
    //}
    //if (m_whtoCh == NULL)
    //{
    //	throw ModelException(MID_KINWAVSED_OL,"CheckInputData","The depth of the channel water layer can not be NULL.");
    //	return false;
    //}
    if (m_Qkin == NULL)
    {
        throw ModelException(MID_KINWAVSED_OL, "CheckInputData", "The kinematic wave flow can not be NULL.");
        return false;
    }
    if (m_FlowWidth == NULL)
    {
        throw ModelException(MID_KINWAVSED_OL, "CheckInputData", "The flow width can not be NULL.");
        return false;
    }
    if (m_ManningN == NULL)
    {
        throw ModelException(MID_KINWAVSED_OL, "CheckInputData", "The Manning N can not be NULL.");
        return false;
    }
    return true;
}

bool KinWavSed_OL::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        throw ModelException(MID_KINWAVSED_OL, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_nCells != n)
    {
        if (m_nCells <= 0) m_nCells = n;
        else
        {
            throw ModelException(MID_KINWAVSED_OL, "CheckInputSize", "Input data for " + string(key) +
                                                                     " is invalid. All the input data should have same size.");
            return false;
        }
    }

    return true;
}

void KinWavSed_OL::initial()
{
    //allocate the output variable
    if (m_SedDep == NULL)
    {
        m_SedDep = new float[m_nCells];
    }
    if (m_Sed_kg == NULL)
    {
        m_Sed_kg = new float[m_nCells];
    }
    if (m_SedToChannel == NULL)
    {
        m_SedToChannel = new float[m_nCells];
    }
    if (m_Qsn == NULL)
    {
        m_Qsn = new float[m_nCells];
    }
    if (m_Vol == NULL)
    {
        m_Vol = new float[m_nCells];
        m_V = new float[m_nCells];
        m_QV = new float[m_nCells];
        m_Ctrans = new float[m_nCells];
        m_DETOverland = new float[m_nCells];
        m_ChV = new float[m_nCells];
        m_fract = new float[m_nCells];
        for (int i = 0; i < m_nCells; i++)
        {
            m_Ctrans[i] = 0.0f;
            m_SedDep[i] = 0.0f;
            m_Sed_kg[i] = 0.0f;
            m_SedToChannel[i] = 0.0f;
            m_Qsn[i] = 0.0f;
            m_Vol[i] = 0.0f;
            m_V[i] = 0.0f;
            m_QV[i] = 0.0f; //test
            m_DETOverland[i] = 0.0f;
            m_ChV[i] = 0.0f;
            m_fract[i] = 0.0f;
        }
    }

    CalcuVelocityOverlandFlow();
    WaterVolumeCalc();
}

void KinWavSed_OL::CalcuVelocityOverlandFlow()
{
    const float beta = 0.6f;
    const float _23 = 2.0f / 3;
    float Perim, R, S, n;

    for (int i = 0; i < m_nCells; i++)
    {
        if (m_WH[i] > 0.0001f)
        {
            Perim = 2 * m_WH[i] / 1000 + m_FlowWidth[i];    // mm to m -> /1000
            if (Perim > 0)
                R = m_WH[i] / 1000 * m_FlowWidth[i] / Perim;
            else
                R = 0.0f;
            S = sin(atan(m_Slope[i]));   //sine of the slope
            S = max(0.001f, S);
            n = m_ManningN[i];  //manning n
            //float Alpha = pow(n/sqrt(S) * pow(Perim, _23), beta);
            //if(Alpha > 0)
            //	m_OverlandQ[i] = pow((m_FlowWidth[i]*m_WH[i]/1000)/Alpha, 1.0f/beta);
            //else
            //	m_OverlandQ[i] = 0;

            m_V[i] = pow(R, _23) * sqrt(S) / n;
        }
        else
            m_V[i] = 0;

        //test QV
        /// temp modified by Junzhi
        //float areaInter = m_Runoff[i]/1000 + m_FlowWidth[i];
        float areaInter = m_WH[i] / 1000 + m_FlowWidth[i];
        if (areaInter != 0)
        {
            m_QV[i] = m_Qkin[i] / areaInter;
        }
        else
            m_QV[i] = 0;

    }
}

void KinWavSed_OL::GetTransportCapacity(int id)
{
    float q, S0, K;
    q = m_V[id] * m_WH[id] *
        60.f;   // m2/s -> m2/min                                 // m_Qkin[id]*60;   // convert to m3/min
    float s = max(0.001f, m_Slope[id]);
    S0 = sin(atan(s));
    K = m_USLE_K[id];
    float threadhold = 0.046f;
    if (q <= 0.f)
        m_Ctrans[id] = 0.f;
    else
    {
        if (q < threadhold)
            m_Ctrans[id] = m_eco1 * K * S0 * sqrt(q);   // kg/(m*min) kg per unit width per minute
        else
            m_Ctrans[id] = m_eco2 * K * S0 * pow(q, 2.0f);
        // kg/(m*min)  -> convert to kg/m3
        m_Ctrans[id] = m_Ctrans[id] / q;
    }
}

void KinWavSed_OL::GetSedimentInFlow(int id)
{
    float TC, Df, Dsp, Deposition, concentration, Vol;
    GetTransportCapacity(id);
    TC = m_Ctrans[id];             //kg/m3
    CalcuFlowDetachment(id);
    Df = m_DETOverland[id];       //kg/cell
    Dsp = m_DETSplash[id];       //kg/cell

    m_Sed_kg[id] += (Df + Dsp);   //sediment in flow
    //while the sediment in flow is more than the capacity of the overland flow,
    // then the sediment deposition would happen; if the velocity of flow is reduced, the TC is decreased correspondingly,
    // then, the sediment deposition may take place.
    //MaxConcentration(m_Vol[id], m_Sed_kg[id], id);
    Vol = m_Vol[id];
    if (Vol > 0)
        concentration = m_Sed_kg[id] / Vol;    // convert to kg/m3
    else
        concentration = 0;
    Deposition = max(concentration - TC, 0.0f);   //kg/m3, >0
    if (Deposition > 0)
    {
        m_Sed_kg[id] = TC * Vol;
    }
    m_SedDep[id] = Deposition * Vol; //kg/cell
}

//void KinWavSed_OL::CalcuVelocityOverlandFlow()
//{
//	const float beta = 0.6f;
//	const float _23 = 2.0f/3;
//	float Perim,R,S,n;
//	
//	for (int i=0; i<m_nCells; i++)
//	{
//		if (m_WH[i] > 0.0001f)
//		{
//			Perim = 2 * m_WH[i]/1000 + m_FlowWidth[i];    // mm to m -> /1000 
//			if (Perim > 0)
//				R = m_WH[i]/1000 * m_FlowWidth[i] / Perim;
//			else
//				R = 0.0f;
//			S = sin(atan(m_Slope[i]));   //sine of the slope
//			n = m_ManningN[i];  //manning n
//			float Alpha = pow(n/sqrt(S) * pow(Perim, _23), beta);
//			if(Alpha > 0)
//				m_OverlandQ[i] = pow((m_FlowWidth[i]*m_WH[i]/1000)/Alpha, 1.0f/beta);
//			else
//				m_OverlandQ[i] = 0;
//
//			m_V[i] = pow(R, _23) * sqrt(S)/n;
//		}
//		else
//			m_V[i] = 0;
//	}
//}

void KinWavSed_OL::MaxConcentration(float watvol, float sedvol, int id)
{
    float conc = (watvol > m_CellWidth * m_CellWidth * 1e-6 ? sedvol / watvol : 0);
    if (conc > 848)
    {
        m_SedDep[id] += max(0.f, sedvol - 848 * watvol);
        //m_SedDep[id] = min(0.f, 848 * watvol - sedvol);
        conc = 848;
    }
    m_Sed_kg[id] = conc * watvol;

    //return conc;
}

//float KinWavSed_OL::WaterVolumeCalc(int id)
//{
//	float Vol, slope, DX, wh;
//	slope = atan(m_Slope[id]);
//	DX = m_cellWith/cos(slope);
//	wh = m_WH[id]/1000;  //mm -> m
//	Vol = DX * m_cellWith * wh;  // m3
//	return Vol;
//}
void KinWavSed_OL::WaterVolumeCalc()
{
    float slope, DX, wh;
    for (int i = 0; i < m_nCells; i++)
    {
        slope = atan(m_Slope[i]);
        DX = m_CellWidth / cos(slope);
        wh = m_WH[i] / 1000;  //mm -> m
        //wh = m_Runoff[i]/1000;  //mm -> m
        m_Vol[i] = DX * m_CellWidth * wh;  // m3
    }
}

void KinWavSed_OL::CalcuFlowDetachment(int i)  //i is the id of the cell in the grid map  
{
    // correction for slope dx/DX, water spreads out over larger area
    float s = max(0.001f, m_Slope[i]);
    float S0 = sin(atan(s));
    float waterdepth = m_WH[i] / 1000.f;   // mm convert to m
    //float waterdepth = m_Runoff[i] / 1000;   // mm convert to m

    //using Foster equation to calculate overland flow detachment
    float Df, waterden, g, shearStr;      //,q
    waterden = 1000;
    g = 9.8f;
    shearStr = waterden * g * waterdepth * S0;
    Df = m_Ccoe * m_USLE_C[i] * m_USLE_K[i] * pow(shearStr, 1.5f);
    /*q = m_Q[i];
    Df = m_Ccoe * m_USLE_C[i] * m_USLE_K[i] * q * S0;*/
    // convert kg/(m2*min) to kg/cell
    float cellareas = (m_CellWidth / cos(atan(s))) * m_CellWidth;
    m_DETOverland[i] = Df * (m_TimeStep / 60) * cellareas;
}

float KinWavSed_OL::SedToChannel(int ID)
{
    float fractiontochannel = 0.0f;
    if (m_chWidth[ID] > 0)
    {
        float tem = m_ChV[ID] * m_TimeStep;
        fractiontochannel = 2 * tem / (m_CellWidth - m_chWidth[ID]);
        fractiontochannel = min(fractiontochannel, 1.0f);
    }
    float sedtoch = fractiontochannel * m_Sed_kg[ID];
    m_Sed_kg[ID] -= sedtoch;
    m_fract[ID] = fractiontochannel;
    /*float sedtoch = 0;
    if (m_chWidth[ID] > 0)
    {
        float sedtoch = m_Qsn[ID] * m_TimeStep;
        m_Sed_kg[ID] -= sedtoch;
    }*/
    return sedtoch;
}

float KinWavSed_OL::simpleSedCalc(float Qn, float Qin, float Sin, float dt, float vol, float sed)
{
    float Qsn = 0;
    float totsed = sed + Sin * dt;  // add upstream sed to sed present in cell
    float totwater = vol + Qin * dt;   // add upstream water to volume water in cell
    if (totwater <= 1e-10)
        return (Qsn);
    Qsn = min(totsed / dt, Qn * totsed / totwater);
    return (Qsn); // outflow is new concentration * new out flux

}

float KinWavSed_OL::complexSedCalc(float Qj1i1, float Qj1i, float Qji1, float Sj1i, float Sji1, float alpha, float dt,
                                   float dx)
{
    float Sj1i1, Cavg, Qavg, aQb, abQb_1, A, B, C, s = 0.f;
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
    Sj1i1 = max(0.f, Sj1i1);
    return Sj1i1;
}

void KinWavSed_OL::OverlandflowSedRouting(int id)
{
    //sum the sediment of the upstream overland flow
    float flowwidth = m_FlowWidth[id];
    float Sin = 0.0f;
    float Qin = 0.0f;
    for (int k = 1; k <= (int) m_flowInIndex[id][0]; ++k)
    {
        int flowInID = (int) m_flowInIndex[id][k];
        Qin += m_Qkin[flowInID];   //m3/s
        Sin += m_Qsn[flowInID];        // kg/s
    }

    // if the channel width is greater than the cell width
    if (m_streamLink[id] >= 0 && flowwidth <= 0)
    {
        m_SedToChannel[id] = Sin * m_TimeStep;
        return;
    }

    //m_Qlastt[id] = m_OverlandQ[id];  // for the next time step
    // calculate sediment overland routing
    float WtVol = m_Vol[id];
    GetSedimentInFlow(id);// get value for m_sed_kg[id]
    m_Qsn[id] = simpleSedCalc(m_Qkin[id], Qin, Sin, m_TimeStep, WtVol, m_Sed_kg[id]);
    ////---- another method
    //float beta = 0.6f;
    //float beta1 = 1/beta;
    //float _23 = 2.0f/3;
    //float Perim = 2 * m_WH[id]/1000 + m_FlowWidth[id];
    //float S = sin(atan(m_Slope[id]));
    //float Alpha = pow(m_ManningN[id]/sqrt(S) * pow(Perim, _23), beta);
    //float Q, Qs;
    //if(Alpha > 0)
    //	Q = pow(m_FlowWidth[id]*m_WH[id] / Alpha, beta1);
    //else
    //	Q = 0;
    //Qs = Q * m_SedConc[id];
    //float DX = m_cellWith/cos(asin(S));
    //m_Qsn[id] = complexSedCalc(m_OverlandQ[id], Qin, Q, Sin, Qs, Alpha, m_TimeStep, DX);
    //----end
    float tem = Sin + m_Sed_kg[id] / m_TimeStep;
    m_Qsn[id] = min(m_Qsn[id], tem);
    tem = Sin * m_TimeStep + m_Sed_kg[id] - m_Qsn[id] * m_TimeStep;
    m_Sed_kg[id] = max(0.0f, tem);
    //m_SedConc[id] = MaxConcentration(WtVol, m_Sed_kg[id], id);
    //calculate sediment to channel
    m_SedToChannel[id] = SedToChannel(id);
    //MaxConcentration(WtVol, m_Sed_kg[id], id);

    //if(m_chWidth[id] > 0)
    //{
    //	//float fractiontochannel = min(m_TimeStep * m_V[id]/(0.5f*(m_cellWith - m_chWidth[id])),1.0f);
    //	float fractiontochannel = m_chWidth[id]/m_cellWith;
    //	m_SedToChannel[id] = m_TimeStep * m_Qsn[id] * fractiontochannel;
    //	m_SedToChannel[id] = min(m_SedToChannel[id], m_Sed_kg[id]);
    //	m_Sed_kg[id] -= m_SedToChannel[id];
    //	/*float VtoCh = m_whtoCh[id]/1000 * DX * m_chWidth[id];
    //	m_SedToChannel[id] = VtoCh * m_SedConc[id];
    //	m_SedToChannel[id] = min(m_SedToChannel[id], m_Sed_kg[id]);
    //	m_Sed_kg[id] -= m_SedToChannel[id];*/
    //}
}

int KinWavSed_OL::Execute()
{
    CheckInputData();

    initial();
    /*for (int i=0;i<m_nCells;i++)
    {
        if (m_chWidth[i] > 0)
        {
            int j = i;
        }
    }*/

    //StatusMsg("executing KinWavSed_OL");
    for (int iLayer = 0; iLayer < m_nLayers; ++iLayer)
    {
        // There are not any flow relationship within each routing layer.
        // So parallelization can be done here.
        int nCells = (int) m_routingLayers[iLayer][0];
        //omp_set_num_threads(2);
#pragma omp parallel for
        for (int iCell = 1; iCell <= nCells; ++iCell)
        {
            int id = (int) m_routingLayers[iLayer][iCell];
            OverlandflowSedRouting(id);
        }
    }
    //StatusMsg("end of executing KinWavSed_OL");

    return 0;
}
