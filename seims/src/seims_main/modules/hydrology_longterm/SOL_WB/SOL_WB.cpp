#include "SOL_WB.h"
#include "MetadataInfo.h"
#include "util.h"
#include "ModelException.h"
#include <cmath>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <omp.h>

SOL_WB::SOL_WB(void) :m_nCells(-1), m_nSoilLayers(-1), m_soilLayers(NULL),m_soilThick(NULL),m_soilZMX(NULL),
	m_pNet(NULL),m_Infil(NULL),m_ES(NULL),m_Revap(NULL),
	m_RI(NULL),m_Perco(NULL),m_soilStorage(NULL),
	m_subbasinsInfo(NULL),m_nSubbasins(-1),
	m_PCP(NULL),m_Interc(NULL),m_EI(NULL),m_Dep(NULL),m_ED(NULL),
	m_RS(NULL),m_RG(NULL),m_SE(NULL),m_tMean(NULL),m_SoilT(NULL),
	m_soilWaterBalance(NULL)
{
}

SOL_WB::~SOL_WB(void)
{
    if (m_soilWaterBalance != NULL) Release2DArray(m_nSubbasins+1,m_soilWaterBalance);
}
void SOL_WB::initialOutputs()
{
	if(m_soilWaterBalance == NULL) Initialize2DArray(m_nSubbasins+1,16,m_soilWaterBalance,0.f);
}
int SOL_WB::Execute()
{
    CheckInputData();
    return 0;
}

void SOL_WB::SetValue(const char *key, float data)
{
    string s(key);
    if (StringMatch(s, VAR_OMP_THREADNUM))omp_set_num_threads((int) data);
    else
        throw ModelException(MID_SOL_WB, "SetValue", "Parameter " + s
                             +" does not exist. Please contact the module developer.");
}

void SOL_WB::setValueToSubbasins()
{
	if(m_subbasinsInfo != NULL)
	{
		vector<int>::iterator it;
        for(it = m_subbasinIDs.begin(); it != m_subbasinIDs.end(); it++)
		{
			Subbasin *curSub = m_subbasinsInfo->GetSubbasinByID(*it);
			int *cells = curSub->getCells();
			int cellsNum = curSub->getCellCount();
			float ri = 0.f; // total subsurface runoff of soil profile (mm)
			float sm = 0.f; // total soil moisture of soil profile (mm)
			float pcp = 0.f, netPcp = 0.f, depET = 0.f, infil = 0.f;
			float itpET = 0.f, netPerc = 0.f, r = 0.f, revap = 0.f;
			float rs = 0.f, meanT = 0.f, soilT = 0.f, es = 0.f, totalET = 0.f;
			
			for(int i = 0; i < cellsNum; i++) // loop cells of current subbasin
            {
				int cell = cells[i];
                ri = 0.f;
                sm = 0.f;
                for (int j = 0; j < (int)m_soilLayers[cell]; j++)
                {
                    ri += m_RI[cell][j]/ float(cellsNum);
					sm += m_soilStorage[cell][j]/ float(cellsNum);
                }
				pcp += m_PCP[cell] / float(cellsNum);
				meanT += m_tMean[cell]/ float(cellsNum);
				soilT += m_SoilT[cell]/ float(cellsNum);
				netPcp += m_pNet[cell]/ float(cellsNum);
				itpET += m_EI[cell]/ float(cellsNum);
				depET +=m_ED[cell]/ float(cellsNum);
				infil += m_Infil[cell]/ float(cellsNum);
				totalET += (m_EI[cell] + m_ED[cell] + m_ES[cell])/ float(cellsNum); // add plant et?
				es += m_ES[cell]/ float(cellsNum);
				netPerc += (m_Perco[cell][(int)m_soilLayers[cell] - 1] - m_Revap[cell])/ float(cellsNum);
				revap += m_Revap[cell]/ float(cellsNum);
				rs += m_RS[cell]/ float(cellsNum);
				r += (m_RS[cell] + ri)/ float(cellsNum);
            }
			float rg = m_RG[*it];
			r += rg;

			m_soilWaterBalance[*it][0] = pcp;
			m_soilWaterBalance[*it][1] = meanT;
			m_soilWaterBalance[*it][2] = soilT;
			m_soilWaterBalance[*it][3] = netPcp;
			m_soilWaterBalance[*it][4] = itpET;
			m_soilWaterBalance[*it][5] = depET;
			m_soilWaterBalance[*it][6] = infil;
			m_soilWaterBalance[*it][7] = totalET;
			m_soilWaterBalance[*it][8] = es;
			m_soilWaterBalance[*it][9] = netPerc;
			m_soilWaterBalance[*it][10] = revap;
			m_soilWaterBalance[*it][11] = rs;
			m_soilWaterBalance[*it][12] = ri;
			m_soilWaterBalance[*it][13] = rg;
			m_soilWaterBalance[*it][14] = r;
			m_soilWaterBalance[*it][15] = sm;
        }
    }
}

void SOL_WB::Set1DData(const char *key, int nRows, float *data)
{
    string s(key);
    if (StringMatch(s, VAR_RG))
    {
        m_RG = data;
		if(m_nSubbasins != nRows-1)
			throw ModelException(MID_SOL_WB, "Set1DData", "The size of groundwater runoff should be equal to (subbasin number + 1)!");
        return;
    }
    CheckInputSize(key, nRows);
	if (StringMatch(s, VAR_SOILLAYERS))
		m_soilLayers = data;
	else if (StringMatch(s, VAR_SOL_ZMX))
		m_soilZMX = data;
	else if (StringMatch(s, VAR_NEPR))
		m_pNet = data;
    else if (StringMatch(s, VAR_INFIL))
        m_Infil = data;
    else if (StringMatch(s, VAR_SOET))
        m_ES = data;
    else if (StringMatch(s, VAR_REVAP))
        m_Revap = data;
    else if (StringMatch(s, VAR_PCP))
        m_PCP = data;
	else if (StringMatch(s, VAR_INLO))
		m_Interc = data;
    else if (StringMatch(s, VAR_INET))
        m_EI = data;
    else if (StringMatch(s, VAR_DEET))
        m_ED = data;
    else if (StringMatch(s, VAR_DPST))
        m_Dep = data;
    else if (StringMatch(s, VAR_SURU))
		m_RS = data;
    else if (StringMatch(s, VAR_SNSB))
		m_SE = data;
	else if (StringMatch(s, VAR_TMEAN))
		m_tMean = data;
    else if (StringMatch(s, VAR_SOTE))
        m_SoilT = data;
    else
        throw ModelException(MID_SOL_WB, "Set1DData", "Parameter " + s + " does not exist in current module.");
}

void SOL_WB::Set2DData(const char *key, int nrows, int ncols, float **data)
{
    string s(key);
    CheckInputSize(key, nrows);
    m_nSoilLayers = ncols;
    if (StringMatch(s, VAR_PERCO))
        m_Perco = data;
    else if (StringMatch(s, VAR_SSRU))
        m_RI = data;
    else if (StringMatch(s, VAR_SOL_ST))
		m_soilStorage = data;
	else if (StringMatch(s, VAR_SOILTHICK))
		m_soilThick = data;
    else
        throw ModelException(MID_SOL_WB, "Set2DData", "Parameter " + s + " does not exist in current module.");
}
void SOL_WB::SetSubbasins(clsSubbasins *subbasins)
{
	if(m_subbasinsInfo == NULL){
		m_subbasinsInfo = subbasins;
		m_nSubbasins = m_subbasinsInfo->GetSubbasinNumber();
		m_subbasinIDs = m_subbasinsInfo->GetSubbasinIDs();
	}
}
void SOL_WB::Get2DData(const char *key, int *nRows, int *nCols, float ***data)
{
	initialOutputs();
    string s(key);
    if (StringMatch(s, VAR_SOWB))
    {
        setValueToSubbasins();
        *nRows = m_nSubbasins + 1;
        *nCols = 16;
        *data = m_soilWaterBalance;
    }
    else
        throw ModelException(MID_SOL_WB, "Get2DData", "Result " + s + " does not exist in current module.");
}

void SOL_WB::CheckInputData()
{
    if (m_date <= 0) throw ModelException(MID_SOL_WB, "CheckInputData", "You have not set the time.");
    if (m_nCells <= 0)
		throw ModelException(MID_SOL_WB, "CheckInputData", "The dimension of the input data can not be less than zero.");
    if (m_soilLayers == NULL)
        throw ModelException(MID_SOL_WB, "CheckInputData", "The soil layers number can not be NULL.");
	if (m_soilZMX == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The root depth can not be NULL.");
	if (m_soilThick == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The soil thickness can not be NULL.");
	if (m_pNet == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The net precipitation can not be NULL.");
    if (m_Infil == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The infiltration can not be NULL.");
    if (m_ES == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The evaporation of soil can not be NULL.");
	if (m_Revap == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The revaporization can not be NULL.");
	if (m_RI == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The subsurface runoff can not be NULL.");
    if (m_Perco == NULL)
		throw ModelException(MID_SOL_WB, "CheckInputData", "The percolation data can not be NULL.");
	if (m_soilStorage == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The soil moisture can not be NULL.");
	//if (m_subbasin == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The subbasion can not be NULL.");
	if (m_PCP == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The precipitation can not be NULL.");
    if (m_Interc == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The interception can not be NULL.");
    if (m_Dep == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The depression data can not be NULL.");
    if (m_ED == NULL)
        throw ModelException(MID_SOL_WB, "CheckInputData", "The evaporation of depression can not be NULL.");
    if (m_EI == NULL)
        throw ModelException(MID_SOL_WB, "CheckInputData", "The evaporation of interception can not be NULL.");
    if (m_RG == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The runoff of groundwater can not be NULL.");
    if (m_RS == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The runoff of surface can not be NULL.");
    //if(m_SE == NULL) throw ModelException(MID_SOL_WB,"CheckInputData","The snow sublimation can not be NULL."); /// OPTIONAL
	if (m_tMean == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The mean temperature can not be NULL.");
	//if (m_tMin == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The min temperature can not be NULL.");
    //if (m_tMax == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The max temperature can not be NULL.");
    if (m_SoilT == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The soil temperature can not be NULL.");

	if (m_nSubbasins <= 0) throw ModelException(MID_SOL_WB, "CheckInputData", "The subbasins number must be greater than 0.");
	if (m_subbasinIDs.empty()) throw ModelException(MID_SOL_WB, "CheckInputData", "The subbasin IDs can not be EMPTY.");
	if (m_subbasinsInfo == NULL) throw ModelException(MID_SOL_WB, "CheckInputData", "The subbasins information can not be NULL.");
}

bool SOL_WB::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
        throw ModelException(MID_SOL_WB, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    if (m_nCells != n)
    {
        if (m_nCells <= 0) m_nCells = n;
        else
            throw ModelException(MID_SOL_WB, "CheckInputSize", "Input data for " + string(key) +
                                                             " is invalid. All the input data should have same size.");
    }
    return true;
}