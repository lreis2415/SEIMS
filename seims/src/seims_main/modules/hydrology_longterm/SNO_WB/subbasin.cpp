#include "subbasin.h"
#include "util.h"

subbasin::subbasin(int id)
{
    this->m_id = id;

    m_P = 0.0f;
    m_P_net = 0.0f;
    m_P_blow = 0.0f;
    m_T = 0.0f;
    m_Wind = 0.0f;
    m_SR = 0.0f;
    m_SE = 0.0f;
    m_SM = 0.0f;
    m_SA = 0.0f;

}

subbasin::~subbasin(void)
{

}

void subbasin::addCell(int cell)
{
    this->m_cells.push_back(cell);
}

void subbasin::clear()
{
    m_P = 0.0f;
    m_P_net = 0.0f;
    m_P_blow = 0.0f;
    m_T = 0.0f;
    m_Wind = 0.0f;
    m_SR = 0.0f;
    m_SE = 0.0f;
    m_SM = 0.0f;
    m_SA = 0.0f;
}

int subbasin::getId()
{
    return this->m_id;
}

float subbasin::getAverage(string key)
{
    if (StringMatch(key, "P")) return m_P / this->m_cells.size();
    else if (StringMatch(key, "P_net")) return m_P_net / this->m_cells.size();
    else if (StringMatch(key, "P_blow")) return m_P_blow / this->m_cells.size();
    else if (StringMatch(key, "T")) return m_T / this->m_cells.size();
    else if (StringMatch(key, "Wind")) return m_Wind / this->m_cells.size();
    else if (StringMatch(key, "SR")) return m_SR / this->m_cells.size();
    else if (StringMatch(key, "SE")) return m_SE / this->m_cells.size();
    else if (StringMatch(key, "SM")) return m_SM / this->m_cells.size();
    else if (StringMatch(key, "SA")) return (m_SA / this->m_cells.size() < 0.01f ? 0.0f : m_SA / this->m_cells.size());
    else return -1.0f;
}

void subbasin::addP(float p)
{
    m_P += p;
}

void subbasin::addPnet(float pnet)
{
    m_P_net += pnet;
}

void subbasin::addPblow(float pblow)
{
    m_P_blow += pblow;
}

void subbasin::addT(float t)
{
    m_T += t;
}

void subbasin::addWind(float wind)
{
    m_Wind += wind;
}

void subbasin::addSR(float sr)
{
    m_SR += sr;
}

void subbasin::addSE(float se)
{
    m_SE += se;
}

void subbasin::addSM(float sm)
{
    m_SM += sm;
}

void subbasin::addSA(float sa)
{
    m_SA += sa;
}

vector<int> *subbasin::getCells()
{
    return &(this->m_cells);
}

//map<int,subbasin*>* subbasin::getSubbasinList(int cellCount, float* subbasinGrid, int subbasinSelectedCount, float* subbasinSelected)
//{
//	map<int,bool> selected;
//	bool isAllNeedStatistc = false;
//	for(int i = 0;i < subbasinSelectedCount; i++)
//	{
//		int subid = int(subbasinSelected[i]);
//		selected[subid] = true;
//		if(subid == 0) isAllNeedStatistc = true;		
//	}
//	
//	map<int,subbasin*> list;	
//	if(isAllNeedStatistc) list[0] = new subbasin(0); 
//	for(int i = 0;i < cellCount; i++)
//	{
//		if(isAllNeedStatistc) list[0]->addCell(i);
//
//		int subid = int(subbasinGrid[i]);
//		if(!selected[subid]) continue;		
//
//		map<int,subbasin*>::iterator it = list.find(subid);
//		if(it == list.end()) list[subid] = new subbasin(subid);
//		list[subid]->addCell(i);
//	}
//
//	return &list;
//}