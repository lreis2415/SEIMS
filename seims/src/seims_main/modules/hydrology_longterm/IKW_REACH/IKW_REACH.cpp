/*----------------------------------------------------------------------
*	Purpose: 	Overland routing using 4-point implicit finite difference method
*
*	Created:	Junzhi Liu
*	Date:		23-Febrary-2011
*
*	Revision:
*   Date:
*---------------------------------------------------------------------*/
//#include "vld.h"
#include "IKW_REACH.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "util.h"
#include <omp.h>
#include <cmath>
#include <iostream>
#include <set>
#include <sstream>
#include <algorithm> 
#include <omp.h>

//#define MINI_SLOPE 0.0001f
//#define NODATA_VALUE -9999  defined in util.h

#define MIN_FLUX 1e-12f
#define MAX_ITERS_CH 10

const float _23 = 2.0f / 3.0f;
const float SQ2 = sqrt(2.f);
using namespace std;

IKW_REACH::IKW_REACH(void) : m_dt(-1), m_nreach(-1), m_Kchb(NODATA_VALUE),
                             m_Kbank(NODATA_VALUE), m_Epch(NODATA_VALUE), m_Bnk0(NODATA_VALUE), m_Chs0(NODATA_VALUE), m_aBank(NODATA_VALUE),
                             m_bBank(NODATA_VALUE), m_subbasin(NULL), m_qsSub(NULL),
                             m_qiSub(NULL), m_qgSub(NULL), m_petCh(NULL), m_gwStorage(NULL), m_area(NULL),
                             m_Vseep0(0.f), m_chManning(NULL), m_chSlope(NULL),
                              m_bankStorage(NULL), m_seepage(NULL), m_chOrder(NULL),
                             m_qsCh(NULL), m_qiCh(NULL), m_qgCh(NULL),
                             m_x(0.2f), m_co1(0.7f), m_qIn(NULL), m_chStorage(NULL), m_manningScalingFactor(1.0f),
                             m_qUpReach(0.f), m_deepGroudwater(0.f)  //90.f for fenkeng; 0 for Lyg;
{
    m_chWTdepth = NULL;

    //m_vScalingFactor(2.5f)
	//m_Vdiv(NULL), m_Vpoint(NULL),
}


IKW_REACH::~IKW_REACH(void)
{
    if (m_area != NULL) delete[] m_area;
    if (m_chStorage != NULL) delete[] m_chStorage;
    if (m_qOut != NULL) delete[] m_qOut;
    if (m_bankStorage != NULL)
        delete[] m_bankStorage;
    if (m_seepage != NULL)
        delete[] m_seepage;
    if (m_chStorage != NULL)
        delete[] m_chStorage;
    if (m_qsCh != NULL)
        delete[] m_qsCh;
    if (m_qiCh != NULL)
        delete[] m_qiCh;
    if (m_qgCh != NULL)
        delete[] m_qgCh;
    if (m_chWTdepth != NULL)
        delete[] m_chWTdepth;
}

bool IKW_REACH::CheckInputData(void)
{
    if (m_dt < 0)
    {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: m_dt has not been set.");
    }

    if (m_nreach < 0)
    {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: m_nreach has not been set.");
    }

    if (FloatEqual(m_Kchb, NODATA_VALUE))
    {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: K_chb has not been set.");
    }
    if (FloatEqual(m_Kbank, NODATA_VALUE))
    {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: K_bank has not been set.");
    }
    if (FloatEqual(m_Epch, NODATA_VALUE))
    {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: Ep_ch has not been set.");
    }
    if (FloatEqual(m_Bnk0, NODATA_VALUE))
    {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: Bnk0 has not been set.");
    }
    if (FloatEqual(m_Chs0, NODATA_VALUE))
    {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: Chs0 has not been set.");
    }
    if (FloatEqual(m_aBank, NODATA_VALUE))
    {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: A_bnk has not been set.");
    }
    if (FloatEqual(m_bBank, NODATA_VALUE))
    {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: B_bnk has not been set.");
    }
    if (FloatEqual(m_Vseep0, NODATA_VALUE))
    {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: m_Vseep0 has not been set.");
    }
	if (this->m_manningScalingFactor <= 0)
	{
		throw ModelException(MID_CH_DW, "CheckInputData", "You have not set the manning scaling factor variable.");
	}
    if (m_subbasin == NULL)
    {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: m_subbasin has not been set.");
    }
    if (m_qsSub == NULL)
    {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: Q_SBOF has not been set.");
    }
    //if (m_qiSub == NULL)
    //{
    //	throw ModelException("IKW_REACH","CheckInputData","The parameter: Q_SBIF has not been set.");
    //}
    //if (m_qgSub == NULL)
    //{
    //	throw ModelException("IKW_REACH","CheckInputData","The parameter: QG_sub has not been set.");
    //}
/*	if (m_petCh == NULL)
	{
		throw ModelException("IKW_REACH","CheckInputData","The parameter: SBPET has not been set.");
	}*/
    //if (m_gwStorage == NULL)
    //{
    //	throw ModelException("IKW_REACH","CheckInputData","The parameter: GW_sub has not been set.");
    //}
    if (m_chWidth == NULL)
    {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: RchParam has not been set.");
    }
    return true;
}

void  IKW_REACH::initialOutputs()
{
    if (m_nreach <= 0)
        throw ModelException("IKW_REACH", "initialOutputs", "The cell number of the input can not be less than zero.");

    if (m_reachLayers.empty())
    {
        CheckInputData();
        for (int i = 1; i <= m_nreach; i++)
        {
            int order = (int) m_chOrder[i];
            m_reachLayers[order].push_back(i);
        }
    }

    //initial channel storage
    if (m_chStorage == NULL)
    {
        m_chStorage = new float[m_nreach + 1];
        m_qIn = new float[m_nreach + 1];
        m_qOut = new float[m_nreach + 1];
        m_bankStorage = new float[m_nreach + 1];
        m_seepage = new float[m_nreach + 1];
        m_qsCh = new float[m_nreach + 1];
        m_qiCh = new float[m_nreach + 1];
        m_qgCh = new float[m_nreach + 1];
        m_chWTdepth = new float[m_nreach + 1];

#pragma omp parallel for
        for (int i = 1; i <= m_nreach; i++)
        {
            float qiSub = 0.f;
            float qgSub = 0.f;
            if (m_qiSub != NULL)
                qiSub = m_qiSub[i];
            if (m_qgSub != NULL)
                qgSub = m_qgSub[i];
            m_seepage[i] = 0.f;
            m_bankStorage[i] = m_Bnk0 * m_chLen[i];
            m_chStorage[i] = m_Chs0 * m_chLen[i];
            m_qIn[i] = 0.f;
            m_qOut[i] = m_qsSub[i] + qiSub + qgSub;
            m_qsCh[i] = m_qsSub[i];
            m_qiCh[i] = qiSub;
            m_qgCh[i] = qgSub;
            m_chWTdepth[i] = 0.f;

        }
    }
}

int IKW_REACH::Execute()
{
    initialOutputs();

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
            ChannelFlow(reachIndex);
        }
    }

    return 0;
}

bool IKW_REACH::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        //StatusMsg("Input data for "+string(key) +" is invalid. The size could not be less than zero.");
        return false;
    }
#ifdef STORM_MODEL
    if(m_nreach != n-1)
    {
        if(m_nreach <=0)
            m_nreach = n-1;
        else
        {
            //StatusMsg("Input data for "+string(key) +" is invalid. All the input data should have same size.");
            ostringstream oss;
            oss << "Input data for "+string(key) << " is invalid with size: " << n << ". The origin size is " << m_nreach << ".\n";
            throw ModelException("IKW_REACH","CheckInputSize",oss.str());
        }
    }
#else
    if (m_nreach != n - 1)
    {
        if (m_nreach <= 0)
            m_nreach = n - 1;
        else
        {
            //StatusMsg("Input data for "+string(key) +" is invalid. All the input data should have same size.");
            ostringstream oss;
            oss << "Input data for " + string(key) << " is invalid with size: " << n << ". The origin size is " <<
            m_nreach << ".\n";
            throw ModelException("IKW_REACH", "CheckInputSize", oss.str());
        }
    }
#endif
    return true;
}

//bool IKW_REACH::CheckInputSizeChannel(const char* key, int n)
//{
//	if(n <= 0)
//	{
//		//StatusMsg("Input data for "+string(key) +" is invalid. The size could not be less than zero.");
//		return false;
//	}
//	if(m_chNumber != n)
//	{
//		if(m_chNumber <=0) m_chNumber = n;
//		else
//		{
//			//StatusMsg("Input data for "+string(key) +" is invalid. All the input data should have same size.");
//			return false;
//		}
//	}
//
//	return true;
//}

//void IKW_REACH::GetValue(const char* key, float* value)
//{
//	string sk(key);
//	if (StringMatch(sk, "QOUTLET"))
//	{
//		map<int, vector<int> >::iterator it = m_reachLayers.end();
//		it--;
//		int reachId = it->second[0];
//		int iLastCell = m_reachs[reachId].size() - 1;
//		*value = m_qCh[reachId][iLastCell];
//		//*value = m_hToChannel[m_idOutlet];
//		//*value = m_qsSub[m_idOutlet];
//		//*value = m_qsSub[m_idOutlet] + m_qCh[reachId][iLastCell];
//	}
//}

void IKW_REACH::SetValue(const char *key, float value)
{
    string sk(key);

    if (StringMatch(sk, VAR_QUPREACH))
        m_qUpReach = value;
    else if (StringMatch(sk, Tag_ChannelTimeStep))
    {
        m_dt = (int) value;
    }
    else if (StringMatch(sk, VAR_OMP_THREADNUM))
    {
        omp_set_num_threads((int) value);
    }
    else if (StringMatch(sk, VAR_K_CHB))
    {
        m_Kchb = value;
    }
    else if (StringMatch(sk, VAR_K_BANK))
    {
        m_Kbank = value;
    }
	else if (StringMatch(sk, VAR_CH_MANNING_FACTOR))
	{
		m_manningScalingFactor = value;
	}
    else if (StringMatch(sk, VAR_EP_CH))
    {
        m_Epch = value;
    }
    else if (StringMatch(sk, VAR_BNK0))
    {
        m_Bnk0 = value;
    }
    else if (StringMatch(sk, VAR_CHS0))
    {
        m_Chs0 = value;
    }
    else if (StringMatch(sk, VAR_VSEEP0))
    {
        m_Vseep0 = value;
    }
    else if (StringMatch(sk, VAR_A_BNK))
    {
        m_aBank = value;
    }
    else if (StringMatch(sk, VAR_B_BNK))
    {
        m_bBank = value;
    }
    else if (StringMatch(sk, VAR_MSK_X))
    {
        m_x = value;
    }
    else if (StringMatch(sk, VAR_MSK_CO1))
    {
        m_co1 = value;
    }
    else
        throw ModelException("IKW_REACH", "SetSingleData", "Parameter " + sk
                                                           + " does not exist. Please contact the module developer.");

}

void IKW_REACH::Set1DData(const char *key, int n, float *value)
{
    string sk(key);
    //check the input data
    if (StringMatch(sk, VAR_SUBBSN))
    {
        m_subbasin = value;   //m_subbasin
    }
    else if (StringMatch(sk, VAR_SBOF))
    {
        CheckInputSize(key, n);
        m_qsSub = value;
    }
    else if (StringMatch(sk, VAR_SBIF))
    {
        CheckInputSize(key, n);
        m_qiSub = value;
    }
    else if (StringMatch(sk, VAR_SBQG))
    {
        m_qgSub = value;
    }
    else if (StringMatch(sk, VAR_SBPET))
    {
        m_petCh = value;
    }
    else if (StringMatch(sk, VAR_SBGS))
    {
        m_gwStorage = value;
    }
    else
        throw ModelException("IKW_REACH", "Set1DData", "Parameter " + sk
                                                       + " does not exist. Please contact the module developer.");

}

void IKW_REACH::GetValue(const char *key, float *value)
{
    string sk(key);
    int iOutlet = m_reachLayers.rbegin()->second[0];
    if (StringMatch(sk, VAR_QOUTLET))
    {
        //*value = m_qsCh[iOutlet];
        m_qOut[0] = m_qOut[iOutlet] + m_deepGroudwater;
        *value = m_qOut[0];
    }
    else if (StringMatch(sk, VAR_QSOUTLET))
    {
        *value = m_qsCh[iOutlet];
    }

}

void IKW_REACH::Get1DData(const char *key, int *n, float **data)
{
    string sk(key);
    *n = m_nreach + 1;
    int iOutlet = m_reachLayers.rbegin()->second[0];
    if (StringMatch(sk, VAR_QRECH))
    {
        m_qOut[0] = m_qOut[iOutlet] + m_deepGroudwater;
        *data = m_qOut;
    }
    else if (StringMatch(sk, VAR_QS))
    {
        m_qsCh[0] = m_qsCh[iOutlet];
        *data = m_qsCh;
    }
    else if (StringMatch(sk, VAR_QI))
    {
        m_qiCh[0] = m_qiCh[iOutlet];
        *data = m_qiCh;
    }
    else if (StringMatch(sk, VAR_QG))
    {
        m_qgCh[0] = m_qgCh[iOutlet];
        *data = m_qgCh;
    }
    else if (StringMatch(sk, VAR_BKST))
    {
        m_bankStorage[0] = m_bankStorage[iOutlet];
        *data = m_bankStorage;
    }
    else if (StringMatch(sk, VAR_CHST))
    {
        m_chStorage[0] = m_chStorage[iOutlet];
        *data = m_chStorage;
    }
    else if (StringMatch(sk, VAR_SEEPAGE))
    {
        m_seepage[0] = m_seepage[iOutlet];
        *data = m_seepage;
    }
    else if (StringMatch(sk, VAR_CHWTDEPTH))
    {
        m_chWTdepth[0] = m_chWTdepth[iOutlet];
        *data = m_chWTdepth;
    }
    else
        throw ModelException("IKW_REACH", "Get1DData", "Output " + sk
                                                       +
                                                       " does not exist in the IKW_REACH module. Please contact the module developer.");

}

void IKW_REACH::Get2DData(const char *key, int *nRows, int *nCols, float ***data)
{
    string sk(key);
    throw ModelException("IKW_REACH", "Get2DData", "Output " + sk
                                                   +
                                                   " does not exist in the IKW_REACH module. Please contact the module developer.");

}

void IKW_REACH::Set2DData(const char *key, int nrows, int ncols, float **data)
{
    string sk(key);

    if (StringMatch(sk, Tag_RchParam))
    {
        m_nreach = ncols - 1;

        m_reachId = data[0];
        m_reachDownStream = data[1];
        m_chOrder = data[2];
        m_chWidth = data[3];
        m_chLen = data[4];
        m_chDepth = data[5];
        m_chVel = data[6];
        m_area = data[7];
        m_chManning = data[8];
        m_chSlope = data[9];

        m_reachUpStream.resize(m_nreach + 1);
        if (m_nreach > 1)
        {
            for (int i = 1; i <= m_nreach; i++)// index of the reach is the equal to streamlink ID(1 to nReaches)
            {
                int downStreamId = int(m_reachDownStream[i]);
                if (downStreamId <= 0)
                    continue;
                m_reachUpStream[downStreamId].push_back(i);
            }
        }
    }
    else
        throw ModelException("IKW_REACH", "Set2DData", "Parameter " + sk
                                                       + " does not exist. Please contact the module developer.");

}

//---------------------------------------------------------------------------
// modified from OpenLISEM
/** Newton Rapson iteration for new water flux in cell, based on Ven Te Chow 1987
\param qIn      summed Q new from upstream
\param qLast    current discharge in the cell
\param surplus        infiltration surplus flux (in m2/s), has value <= 0
\param alpha    alpha calculated in LISEM from before kinematic wave
\param dt   timestep
\param dx   length of the cell corrected for slope
*/
float IKW_REACH::GetNewQ(float qIn, float qLast, float surplus, float alpha, float dt, float dx)
{
    /* Using Newton-Raphson Method */
    float ab_pQ, dtX, C;  //auxillary vars
    int count;
    float Qkx; //iterated discharge, becomes Qnew
    float fQkx; //function
    float dfQkx;  //derivative
    const float _epsilon = 1e-12f;
    const float beta = 0.6f;

    /* if no input then output = 0 */
    if ((qIn + qLast) <= -surplus * dx)//0)
    {
        //itercount = -1;
        return (0);
    }

    /* common terms */
    ab_pQ = alpha * beta * pow(((qLast + qIn) / 2), beta - 1);
    // derivative of diagonal average (space-time)

    dtX = dt / dx;
    C = dtX * qIn + alpha * pow(qLast, beta) + dt * surplus;
    //dt/dx*Q = m3/s*s/m=m2; a*Q^b = A = m2; surplus*dt = s*m2/s = m2
    //C is unit volume of water
    // first gues Qkx
    Qkx = (dtX * qIn + qLast * ab_pQ + dt * surplus) / (dtX + ab_pQ);

    // VJ 050704, 060830 infil so big all flux is gone
    //VJ 110114 without this de iteration cannot be solved for very small values
    if (Qkx < MIN_FLUX)
    {
        //itercount = -2;
        return (0);
    }

    Qkx = max(Qkx, MIN_FLUX);

    count = 0;
    do
    {
        fQkx = dtX * Qkx + alpha * pow(Qkx, beta) - C;   /* Current k */
        dfQkx = dtX + alpha * beta * pow(Qkx, beta - 1);  /* Current k */
        Qkx -= fQkx / dfQkx;                                /* Next k */
        Qkx = max(Qkx, MIN_FLUX);
        count++;
        //qDebug() << count << fQkx << Qkx;
    } while (fabs(fQkx) > _epsilon && count < MAX_ITERS_CH);

    if (Qkx != Qkx)
        throw ModelException("IKW_OL", "GetNewQ", "Error in iteration!");

    //itercount = count;
    return Qkx;
}

void IKW_REACH::ChannelFlow(int i)
{
    float st0 = m_chStorage[i];

    float qiSub = 0.f;
    if (m_qiSub != NULL)
        qiSub = m_qiSub[i];
    float qgSub = 0.f;
    if (m_qgSub != NULL)
        qgSub = m_qgSub[i];

    //////////////////////////////////////////////////////////////////////////
    // first add all the inflow water
    // 1. water from this subbasin
    float qIn = m_qsSub[i] + qiSub + qgSub;

    // 2. water from upstream reaches
    float qsUp = 0.f;
    float qiUp = 0.f;
    float qgUp = 0.f;
    for (size_t j = 0; j < m_reachUpStream[i].size(); ++j)
    {
        int upReachId = m_reachUpStream[i][j];
        qsUp += m_qsCh[upReachId];
        qiUp += m_qiCh[upReachId];
        qgUp += m_qgCh[upReachId];
    }
    qIn += qsUp + qiUp + qgUp;
    qIn += m_qUpReach; // m_qUpReach is zero for not-parallel program and qsUp, qiUp and qgUp are zero for parallel computing

    // 3. water from bank storage
    float bankOut = m_bankStorage[i] * (1 - exp(-m_aBank));

    m_bankStorage[i] -= bankOut;
    qIn += bankOut / m_dt;

    // add inflow water to storage
    m_chStorage[i] += qIn * m_dt;

    //////////////////////////////////////////////////////////////////////////
    // then subtract all the outflow water
    // 1. transmission losses to deep aquifer, which is lost from the system
    // the unit of kchb is mm/hr
    float seepage = m_Kchb / 1000.f / 3600.f * m_chWidth[i] * m_chLen[i] * m_dt;
    if (qgSub < 0.001f)
    {
        if (m_chStorage[i] > seepage)
        {
            m_seepage[i] = seepage;
            m_chStorage[i] -= seepage;
        }
        else
        {
            m_seepage[i] = m_chStorage[i];
            m_chStorage[i] = 0.f;
            m_qOut[i] = 0.f;
            m_qsCh[i] = 0.f;
            m_qiCh[i] = 0.f;
            m_qgCh[i] = 0.f;
            return;
        }
    }
    else
    {
        m_seepage[i] = 0.f;
    }

    // 2. calculate transmission losses to bank storage
    float dch = m_chStorage[i] / (m_chWidth[i] * m_chLen[i]);
    float bankInLoss = 2 * m_Kbank / 1000.f / 3600.f * dch * m_chLen[i] * m_dt;   // m3/s
    bankInLoss = 0.f;
    if (m_chStorage[i] > bankInLoss)
    {
        m_chStorage[i] -= bankInLoss;
    }
    else
    {
        bankInLoss = m_chStorage[i];
        m_chStorage[i] = 0.f;
    }
    // water balance of the bank storage
    // loss the water from bank storage to the adjacent unsaturated zone and groundwater storage
    float bankOutGw = m_bankStorage[i] * (1 - exp(-m_bBank));
    bankOutGw = 0.f;
    m_bankStorage[i] = m_bankStorage[i] + bankInLoss - bankOutGw;
    if (m_gwStorage != NULL)
        m_gwStorage[i] += bankOutGw / m_area[i] * 1000.f;   // updated groundwater storage

    if (m_chStorage[i] <= 0.f)
    {
        m_qOut[i] = 0.f;
        m_qsCh[i] = 0.f;
        m_qiCh[i] = 0.f;
        m_qgCh[i] = 0.f;
        return;
    }

    // 3. evaporation losses
    float et = 0.f;
    if (m_petCh != NULL)
    {
        et = m_Epch * m_petCh[i] / 1000.0f * m_chWidth[i] * m_chLen[i];    //m3
        if (m_chStorage[i] > et)
        {
            m_chStorage[i] -= et;
        }
        else
        {
            et = m_chStorage[i];
            m_chStorage[i] = 0.f;
            m_qOut[i] = 0.f;
            m_qsCh[i] = 0.f;
            m_qiCh[i] = 0.f;
            m_qgCh[i] = 0.f;
            return;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // routing, there are water in the channel after inflow and transmission loss
    float totalLoss = m_seepage[i] + bankInLoss + et;

    if (m_chStorage[i] >= 0.f)
    {
        //qIn -= totalLoss/m_dt;
        m_chStorage[i] = st0;

        float h = m_chStorage[i] / m_chWidth[i] / m_chLen[i];
        float Perim = 2.f * h + m_chWidth[i];
        float sSin = sqrt(sin(m_chSlope[i]));
        float alpha = pow((m_manningScalingFactor * m_chManning[i]) / sSin * pow(Perim, _23), 0.6f);

        float lossRate = -totalLoss / m_dt / m_chWidth[i];
        //lossRate = 0.f;
        m_qOut[i] = GetNewQ(qIn, m_qOut[i], lossRate, alpha, m_dt, m_chLen[i]);

        float hNew = (alpha * pow(m_qOut[i], 0.6f)) / m_chWidth[i]; // unit m

        m_chStorage[i] += (qIn - m_qOut[i]) * m_dt;
        //float hTest = h + (qIn-m_qOut[i])*m_dt/m_chWidth[i]/m_chLen[i];

        if (m_chStorage[i] < 0.f)
        {
            m_qOut[i] = qIn;
            m_chStorage[i] = 0.f;
        }
    }
    else
    {
        m_qOut[i] = 0.f;
        m_chStorage[i] = 0.f;
        qIn = 0.f;
    }

    float qInSum = m_qsSub[i] + qiSub + qgSub + qsUp + qiUp + qgUp;
    m_qsCh[i] = m_qOut[i] * (m_qsSub[i] + qsUp) / qInSum;
    m_qiCh[i] = m_qOut[i] * (qiSub + qiUp) / qInSum;
    m_qgCh[i] = m_qOut[i] * (qgSub + qgUp) / qInSum;

    // set variables for next time step
    m_qIn[i] = qIn;

    m_chWTdepth[i] = dch;
}
