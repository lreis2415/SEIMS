#include "IUH_NUTR_OL.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "util.h"
#include <map>
#include <omp.h>

using namespace std;

IUH_NUTR_OL::IUH_NUTR_OL(void) : m_TimeStep(-1), m_nCells(-1), m_CellWidth(NODATA_VALUE), m_cellArea(NODATA_VALUE),
	                   m_nSubbasins(-1),  m_subbasin(NULL),m_subbasinsInfo(NULL),
                       m_iuhCell(NULL),  m_iuhCols(-1), m_cellFlowCols(-1),
					   m_sedYield(NULL), m_cellSed(NULL), m_sedtoCh(NULL), m_sedOL(NULL)
{
}

IUH_NUTR_OL::~IUH_NUTR_OL(void)
{
    //// cleanup
	if(m_sedtoCh != NULL) 
		Release1DArray(m_sedtoCh);

	if(m_cellSed != NULL) 
		Release2DArray(m_nCells, m_cellSed);

}

bool IUH_NUTR_OL::CheckInputData(void)
{
    if (m_nCells < 0)
        throw ModelException(MID_IUH_NUTR_OL, "CheckInputData", "The parameter: m_nCells has not been set.");
    if (FloatEqual(m_CellWidth, NODATA_VALUE))
        throw ModelException(MID_IUH_NUTR_OL, "CheckInputData", "The parameter: m_CellWidth has not been set.");
    if (m_TimeStep <= 0)
        throw ModelException(MID_IUH_NUTR_OL, "CheckInputData", "The parameter: m_TimeStep has not been set.");
    if (m_subbasin == NULL)
        throw ModelException(MID_IUH_NUTR_OL, "CheckInputData", "The parameter: m_subbasin has not been set.");

	if (m_nSubbasins <= 0) throw ModelException(MID_IUH_NUTR_OL, "CheckInputData", "The subbasins number must be greater than 0.");
	if (m_subbasinIDs.empty()) throw ModelException(MID_IUH_NUTR_OL, "CheckInputData", "The subbasin IDs can not be EMPTY.");
	if (m_subbasinsInfo == NULL)
		throw ModelException(MID_IUH_NUTR_OL, "CheckInputData", "The parameter: m_subbasinsInfo has not been set.");

    //if (m_uhmaxCell == NULL)
    //{
    //	throw ModelException(MID_IUH_NUTR_OL,"CheckInputData","The parameter: m_uhmax has not been set.");
    //	return false;
    //}
    //if (m_uhminCell == NULL)
    //{
    //	throw ModelException(MID_IUH_NUTR_OL,"CheckInputData","The parameter: m_uhmin has not been set.");
    //	return false;
    //}
    if (m_iuhCell == NULL)
        throw ModelException(MID_IUH_NUTR_OL, "CheckInputData", "The parameter: m_iuhCell has not been set.");
    if (m_date < 0)
        throw ModelException(MID_IUH_NUTR_OL, "CheckInputData", "The parameter: m_date has not been set.");
    return true;
}

void IUH_NUTR_OL::initialOutputs()
{
	// This has been checked in CheckInputData, so deprecated!
    //if (this->m_nCells <= 0 || this->m_subbasin == NULL)
    //    throw ModelException(MID_IUH_NUTR_OL, "CheckInputData", "The dimension of the input data can not be less than zero.");
    // allocate the output variables

    //if (m_nSubbasins <= 0)
    //{
    //    map<int, int> subs;
    //    for (int i = 0; i < this->m_nCells; i++)
    //    {
    //        subs[int(this->m_subbasin[i])] += 1;
    //    }
    //    this->m_nSubbasins = subs.size();
    //}

    //initial some variables
	if(m_cellArea <= 0.f) m_cellArea = m_CellWidth * m_CellWidth;
    if (m_sedtoCh == NULL)
    {
		Initialize1DArray(m_nSubbasins+1, m_sedtoCh, 0.f);
		Initialize1DArray(m_nCells, m_sedOL, 0.f);

        //m_Q_SBOF = new float[m_nSubbasins + 1];
        //for (int i = 0; i <= m_nSubbasins; i++)
        //{
        //    m_Q_SBOF[i] = 0.f;
        //}
        for (int i = 0; i < m_nCells; i++)
            m_cellFlowCols = max(int(m_iuhCell[i][1] + 1), m_cellFlowCols);
        //get m_cellFlowCols, i.e. the maximum of second column of OL_IUH plus 1.

		m_cellSed = new float *[m_nCells];
		Initialize2DArray(m_nCells, m_cellFlowCols, m_cellSed, 0.f);
	}
}

int IUH_NUTR_OL::Execute()
{
    CheckInputData();

    initialOutputs();
	// delete value of last time step
#pragma omp parallel for
    for (int i = 0; i < m_nSubbasins + 1; i++)
        m_sedtoCh[i] = 0.f;

    int nt = 0;
    float qs_cell = 0.f;
    //float area = m_CellWidth * m_CellWidth;

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++)
    {
        //forward one time step
        for (int j = 0; j < m_cellFlowCols; j++)
        {
            if (j != m_cellFlowCols - 1)
			{
                m_cellSed[i][j] = m_cellSed[i][j+1];
			}
            else
			{
                m_cellSed[i][j] = 0.f;
			}
        }

        if (m_sedYield[i] > 0.f)
        {
            int min = int(this->m_iuhCell[i][0]);
            int max = int(this->m_iuhCell[i][1]);
            int col = 2;
            for (int k = min; k <= max; k++)
            {
                m_cellSed[i][k] += m_sedYield[i] * m_iuhCell[i][col];
                col++;
            }
        }
    }

    for (int i = 0; i < m_nCells; i++)
    {
        //add today's flow
        int subi = (int) m_subbasin[i];
        if (m_nSubbasins == 1)
        {
            subi = 1;
        }
        else if (subi >= m_nSubbasins + 1)
        {
            ostringstream oss;
            oss << subi;
            throw ModelException(MID_IUH_NUTR_OL, "Execute", "The subbasin " + oss.str() + " is invalid.");
        }
        m_sedtoCh[subi] += m_cellSed[i][0];

		m_sedOL[i] = m_cellSed[i][0];
    }

    float tmp = 0.f;
	#pragma omp parallel for reduction(+:tmp)
    for (int i = 1; i < m_nSubbasins + 1; i++)
    {
        tmp += m_sedtoCh[i];        //get overland flow routing for entire watershed.
    }
    m_sedtoCh[0] = tmp;

    return 0;
}

bool IUH_NUTR_OL::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
        throw ModelException(MID_IUH_NUTR_OL, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    if (this->m_nCells != n)
    {
        if (this->m_nCells <= 0) this->m_nCells = n;
        else
            throw ModelException(MID_IUH_NUTR_OL, "CheckInputSize", "Input data for " + string(key) +
                                                             " is invalid. All the input data should have same size.");
    }
    return true;
}

void IUH_NUTR_OL::SetValue(const char *key, float value)
{
    string sk(key);

    if (StringMatch(sk, Tag_TimeStep))m_TimeStep = (int) value;
    else if (StringMatch(sk, Tag_CellSize))m_nCells = (int) value;
    else if (StringMatch(sk, Tag_CellWidth))m_CellWidth = value;
    else if (StringMatch(sk, VAR_OMP_THREADNUM))omp_set_num_threads((int) value);
    else
        throw ModelException(MID_IUH_NUTR_OL, "SetValue", "Parameter " + sk + " does not exist in current method.");
}

void IUH_NUTR_OL::Set1DData(const char *key, int n, float *data)
{

    this->CheckInputSize(key, n);

    //set the value
    string sk(key);
	if (StringMatch(sk, VAR_SUBBSN))
		m_subbasin = data;
	else if (StringMatch(sk, VAR_SOER)) 
		m_sedYield = data;
    else
        throw ModelException(MID_IUH_NUTR_OL, "Set1DData", "Parameter " + sk + " does not exist in current method.");
}

void IUH_NUTR_OL::Set2DData(const char *key, int nRows, int nCols, float **data)
{

    string sk(key);
    if (StringMatch(sk, VAR_OL_IUH))
    {
        this->CheckInputSize(VAR_OL_IUH, nRows);
        m_iuhCell = data;
        m_iuhCols = nCols;
    }
    else
        throw ModelException(MID_IUH_NUTR_OL, "Set2DData", "Parameter " + sk + " does not exist in current method.");
}
void IUH_NUTR_OL::SetSubbasins(clsSubbasins *subbasins)
{
	if(m_subbasinsInfo == NULL){
		m_subbasinsInfo = subbasins;
		m_nSubbasins = m_subbasinsInfo->GetSubbasinNumber();
		m_subbasinIDs = m_subbasinsInfo->GetSubbasinIDs();
	}
}

void IUH_NUTR_OL::Get1DData(const char *key, int *n, float **data)
{
	initialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SED_TO_CH))
	{
		*data = m_sedtoCh;   // from each subbasin to channel
		*n = m_nSubbasins + 1;
		return;
	}
	else if (StringMatch(sk, VAR_SED_OL))
	{
		*data = m_sedOL;
		*n = m_nCells;
	}

    else
        throw ModelException(MID_IUH_NUTR_OL, "Get1DData", "Result " + sk + " does not exist in current method.");
}
