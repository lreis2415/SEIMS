#include "SNO_WB.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include <math.h>
#include <cmath>
#include "util.h"
#include <omp.h>

SNO_WB::SNO_WB(void)
{
    // set default values for member variables
    //this->m_Date = -1;
    this->m_nCells = -1;
    this->m_t0 = NODATA_VALUE;
    this->m_tsnow = NODATA_VALUE;
    this->m_kblow = NODATA_VALUE;
    this->m_swe0 = NODATA_VALUE;
    this->m_SA = NULL;
    this->m_Pnet = NULL;
    this->m_tMax = NULL;
    this->m_SR = NULL;
    this->m_SE = NULL;
    this->m_SM = NULL;
    /*this->m_subbasinList = NULL;
    this->m_subbasinSelectedCount = -1;
    this->m_subbasinSelected = NULL;
    this->m_subbasin = NULL;
    this->m_snowWaterBalance = NULL;*/
    this->m_WindSpeed = NULL;
    this->m_P = NULL;
    ///this->m_nCells = -1;
    ///m_isInitial = true;
}

SNO_WB::~SNO_WB(void)
{
    if (this->m_SA != NULL) delete[] this->m_SA;

    /*if(this->m_subbasinList != NULL)
    {
        map<int,subbasin*>::iterator it;
        for(it=this->m_subbasinList->begin();it!=this->m_subbasinList->end();it++)
        {
            if(it->second != NULL) delete it->second;
        }
        delete this->m_subbasinList;
    }*/

    /*if(m_snowWaterBalance!=NULL)
    {
        for(int i=0;i<this->m_subbasinSelectedCount;i++)
        {
            if(m_snowWaterBalance[i] != NULL) delete [] m_snowWaterBalance[i];
        }
        delete [] m_snowWaterBalance;
    }*/
}

bool SNO_WB::CheckInputData(void)
{
    if (this->m_date <= 0) throw ModelException(MID_SNO_WB, "CheckInputData", "You have not set the time.");
    if (this->m_nCells <= 0)
        throw ModelException(MID_SNO_WB, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    if (this->m_P == NULL)
        throw ModelException(MID_SNO_WB, "CheckInputData", "The precipitation data can not be NULL.");
    if (this->m_tMean == NULL)
        throw ModelException(MID_SNO_WB, "CheckInputData", "The mean temperature data can not be NULL.");
    if (this->m_tMax == NULL)
        throw ModelException(MID_SNO_WB, "CheckInputData", "The max temperature data can not be NULL.");
    if (this->m_WindSpeed == NULL)
        throw ModelException(MID_SNO_WB, "CheckInputData", "The wind speed data can not be NULL.");
    if (this->m_kblow == NODATA_VALUE)
        throw ModelException(MID_SNO_WB, "CheckInputData",
                             "The fraction coefficient of snow blowing into or out of the watershed can not be NULL.");
    if (this->m_t0 == NODATA_VALUE)
        throw ModelException(MID_SNO_WB, "CheckInputData", "The snowmelt threshold temperature can not be NULL.");
    if (this->m_tsnow == NODATA_VALUE)
        throw ModelException(MID_SNO_WB, "CheckInputData", "The snowfall threshold temperature can not be NULL.");
    if (this->m_swe0 == NODATA_VALUE)
        throw ModelException(MID_SNO_WB, "CheckInputData", "The Initial snow water equivalent can not be NULL.");
    //if(this->m_subbasinSelected == NULL)	throw ModelException(MID_SNO_WB,"CheckInputData","The subbasin selected can not be NULL.");
    //if(this->m_subbasinSelectedCount < 0)	throw ModelException(MID_SNO_WB,"CheckInputData","The number of subbasin selected can not be lower than 0.");
    //if(this->m_subbasin == NULL)	throw ModelException(MID_SNO_WB,"CheckInputData","The subbasin data can not be NULL.");
    if (this->m_Pnet == NULL)
        throw ModelException(MID_SNO_WB, "CheckInputData", "The net precipitation data can not be NULL.");
    if (this->m_SM == NULL) throw ModelException(MID_SNO_WB, "CheckInputData", "The snow melt data can not be NULL.");
    //if(this->m_SR == NULL)			throw ModelException(MID_SNO_WB,"CheckInputData","The snow redistribution data can not be NULL.");
    if (this->m_SE == NULL)
        throw ModelException(MID_SNO_WB, "CheckInputData", "The snow sublimation data can not be NULL.");
    return true;
}

void SNO_WB::initialOutputs()
{
    if (m_nCells <= 0)
        throw ModelException(MID_SNO_WB, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    /// m_isInitial should be removed and replaced by initialOutputs.By LJ.
    if (this->m_SA == NULL)
    {
        this->m_SA = new float[this->m_nCells];
        for (int i = 0; i < this->m_nCells; i++)
        {
            m_SA[i] = 0.0f;
            //if(this->m_tMean[i] < this->m_tsnow)	this->m_SA[i] = this->m_swe0;	//winter
            //else
            //	this->m_SA[i] = 0.0f;			// other seasons
        }
    }
    if (m_SWE == NODATA_VALUE)
        m_SWE = 0.f;
}

int SNO_WB::Execute()
{
    this->CheckInputData();

    this->initialOutputs();

    /*if(m_subbasinList == NULL && this->m_subbasinSelected != NULL && this->m_subbasinSelectedCount > 0)
    {
        getSubbasinList(this->m_nCells,this->m_subbasin,this->m_subbasinSelectedCount ,this->m_subbasinSelected);
    }*/

    if (m_SR == NULL)  /// the initialization should be removed when snow redistribution module is accomplished. LJ
    {
        m_SR = new float[m_nCells];
        for (int i = 0; i < this->m_nCells; i++)
            m_SR[i] = 0.f;
    }

    ///this->m_SWE = 0.0f;/// move to initialOutputs()
    for (int rw = 0; rw < this->m_nCells; rw++)
    {
        float dT = this->m_tMean[rw];
        float dPnet = this->m_Pnet[rw];
        float dSE = this->m_SE[rw];
        float dSR = this->m_SR[rw];
        float dSA = this->m_SA[rw];
        float dSM = this->m_SM[rw];

        float dtmp2 = dSA + dSR - dSE - dSM;
        if (dT <= this->m_tsnow)        //snowfall
        {
            dtmp2 += (1 + this->m_kblow) * dPnet;
        }
        else if (dT > this->m_tsnow && dT < this->m_t0 && dSA >
                                                          dPnet) //rain on snow, if the snow accumulation is larger than net precipitation, assume all the precipitation is accumulated in snow pack.
        {
            dtmp2 += dPnet;
        }

        this->m_SA[rw] = max(dtmp2, 0.0f);

        this->m_SWE += this->m_SA[rw];
    }

    this->m_SWE /= this->m_nCells;
    return 0;
}

bool SNO_WB::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        throw ModelException(MID_SNO_WB, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (this->m_nCells != n)
    {
        if (this->m_nCells <= 0) this->m_nCells = n;
        else
        {
            throw ModelException(MID_SNO_WB, "CheckInputSize", "Input data for " + string(key) +
                                                               " is invalid. All the input data should have same size.");
            return false;
        }
    }
    return true;
}

//void SNO_WB::getSubbasinList(int cellCount, float* subbasinGrid, int subbasinSelectedCount, float* subbasinSelected)
//{
//	if(this->m_subbasinList != NULL) return;
//	if(subbasinSelected == NULL) return;
//
//	map<int,bool> selected;
//	bool isAllNeedStatistc = false;
//	for(int i = 0;i < subbasinSelectedCount; i++)
//	{
//		int subid = int(subbasinSelected[i]);
//		selected[subid] = true;
//		if(subid == 0) isAllNeedStatistc = true;		
//	}
//
//	//map<int,subbasin*> list;	
//	this->m_subbasinList = new map<int,subbasin*>();
//	if(isAllNeedStatistc) (*m_subbasinList)[0] = new subbasin(0); 
//	for(int i = 0;i < cellCount; i++)
//	{
//		if(isAllNeedStatistc) (*m_subbasinList)[0]->addCell(i);
//
//		int subid = int(subbasinGrid[i]);
//		if(!selected[subid]) continue;		
//
//		map<int,subbasin*>::iterator it = (*m_subbasinList).find(subid);
//		if(it == (*m_subbasinList).end()) (*m_subbasinList)[subid] = new subbasin(subid);
//		(*m_subbasinList)[subid]->addCell(i);
//	}
//}

void SNO_WB::SetValue(const char *key, float data)
{
    string s(key);
    if (StringMatch(s, VAR_K_BLOW)) this->m_kblow = data;
    else if (StringMatch(s, VAR_T0)) this->m_t0 = data;
    else if (StringMatch(s, VAR_T_SNOW)) this->m_tsnow = data;
    else if (StringMatch(s, VAR_SWE0)) this->m_swe0 = data;
    else if (StringMatch(s, VAR_OMP_THREADNUM))omp_set_num_threads((int) data);
    else if (StringMatch(s, Tag_CellSize)) this->m_nCells = (int) data;
    else
        throw ModelException(MID_SNO_WB, "SetValue", "Parameter " + s
                                                     +
                                                     " does not exist in current module. Please contact the module developer.");
}

void SNO_WB::Set1DData(const char *key, int n, float *data)
{
    string s(key);
    /*if(StringMatch(s,Tag_SubbasinSelected))
    {
        this->m_subbasinSelected = data;
        this->m_subbasinSelectedCount = n;
        return;
    }*/

    //if(StringMatch(s, VAR_WS))
    //{
    //	this->m_WindSpeed = data;
    //	this->m_nCells = n;
    //	return;
    //}
    this->CheckInputSize(key, n);
    if (StringMatch(s, VAR_NEPR)) this->m_Pnet = data;
    else if (StringMatch(s, VAR_SNRD)) this->m_SR = data;
    else if (StringMatch(s, VAR_SNSB)) this->m_SE = data;
    else if (StringMatch(s, VAR_SNME)) this->m_SM = data;
    else if (StringMatch(s, VAR_WS)) this->m_WindSpeed = data;
    else if (StringMatch(s, VAR_TMAX)) this->m_tMax = data;
    else if (StringMatch(s, VAR_TMEAN)) this->m_tMean = data;
    else if (StringMatch(s, VAR_PCP)) this->m_P = data;
        //else if(StringMatch(s, VAR_SUBBSN))		this->m_subbasin = data;
    else
        throw ModelException(MID_SNO_WB, "Set1DData", "Parameter " + s
                                                      +
                                                      " does not exist in current module. Please contact the module developer.");

}

void SNO_WB::Get1DData(const char *key, int *n, float **data)
{
    initialOutputs();
    string s(key);
    if (StringMatch(s, VAR_SNAC))
    {
        /// move these code to initialOutputs(). By LJ.
        /*if(this->m_SA == NULL)
        {
            if(this->m_nCells <=0) throw ModelException(MID_SNO_WB,"Get1DData","The dimension of the input data can not be less than zero.");
            this->m_SA = new float[this->m_nCells];
            for(int i=0;i<this->m_nCells;i++)
            {
                this->m_SA[i] = 0.0f;
            }
        }	*/
        *data = this->m_SA;
        *n = this->m_nCells;
    }
    else
        throw ModelException(MID_SNO_WB, "Get1DData",
                             "Result " + s + " does not exist in current module. Please contact the module developer.");
}

//void SNO_WB::Get2DData(const char* key, int* nRows, int* nCols, float*** data)
//{
//	string s(key);
//	if(StringMatch(s, VAR_SNWB))
//	{
//		setValueToSubbasin();
//		*nRows = this->m_subbasinSelectedCount;
//		*nCols = 9;
//		*data = this->m_snowWaterBalance;
//	}
//	else throw ModelException(MID_SNO_WB,"Get2DData","Result " + s + " does not exist in current module. Please contact the module developer.");
//}

void SNO_WB::GetValue(const char *key, float *data)
{
    initialOutputs();
    string s(key);
    if (StringMatch(s, VAR_SWE)) *data = this->m_SWE;
    else
        throw ModelException(MID_SNO_WB, "GetValue", "Result " + s +
                                                     " does not exist in current module. Please contact the module developer.");
}

//void SNO_WB::setValueToSubbasin()
//{
//
//	if(this->m_subbasinList != NULL)
//	{
//		if(m_snowWaterBalance == NULL)
//		{
//			m_snowWaterBalance = new float*[this->m_subbasinSelectedCount];
//			for(int i=0;i<this->m_subbasinSelectedCount;i++)
//			{
//				m_snowWaterBalance[i] = new float[9];
//			}
//		}
//
//		float u = 0.0f;									//average wind speed
//		for (int rw = 0; rw < this->m_nCells; rw++) u+=this->m_WindSpeed[rw];
//		u /= this->m_nCells;
//
//		int index = 0;
//		map<int,subbasin*>::iterator it;
//		for(it=this->m_subbasinList->begin();it!=this->m_subbasinList->end();it++)
//		{
//			it->second->clear();
//			vector<int>* cells = it->second->getCells();
//			vector<int>::iterator itCells;
//			for(itCells=cells->begin();itCells<cells->end();itCells++)
//			{
//				int cell = *itCells;
//				it->second->addP(m_P[cell]);
//				it->second->addPnet(this->m_Pnet[cell]);
//				it->second->addPblow(this->m_kblow * this->m_Pnet[cell]);
//				it->second->addT(m_tMean[cell]);
//				//it->second->addWind(u);
//				it->second->addSR(m_SR[cell]);
//				it->second->addSE(m_SE[cell]);
//				it->second->addSM(m_SM[cell]);
//				it->second->addSA(m_SA[cell]);				
//			}
//
//			m_snowWaterBalance[index][0] = it->second->getAverage("P");
//			m_snowWaterBalance[index][1] = it->second->getAverage("P_net");
//			m_snowWaterBalance[index][2] = it->second->getAverage("P_blow");
//			m_snowWaterBalance[index][3] = it->second->getAverage("T");
//			m_snowWaterBalance[index][4] = u;//it->second->getAverage("Wind");
//			m_snowWaterBalance[index][5] = it->second->getAverage("SR");
//			m_snowWaterBalance[index][6] = it->second->getAverage("SE");
//			m_snowWaterBalance[index][7] = it->second->getAverage("SM");
//			m_snowWaterBalance[index][8] = it->second->getAverage("SA");
//
//			//cout<< "subbasin:" << it->second->getId() << endl;
//			//cout<< "P:" << m_snowWaterBalance[index][0] << endl;
//			//cout<< "P_net:" << m_snowWaterBalance[index][1] << endl;
//			//cout<< "P_blow:" << m_snowWaterBalance[index][2] << endl;
//			//cout<< "T:" << m_snowWaterBalance[index][3] << endl;
//			//cout<< "Wind:" << m_snowWaterBalance[index][4] << endl;
//			//cout<< "SR:" << m_snowWaterBalance[index][5] << endl;
//			//cout<< "SE:" << m_snowWaterBalance[index][6] << endl;
//			//cout<< "SM:" << m_snowWaterBalance[index][7] << endl;
//			//cout<< "SA:" << m_snowWaterBalance[index][8] << endl;
//
//			index++;
//		}
//	}
//}