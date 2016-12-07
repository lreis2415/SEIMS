#include "IUH_IF.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "util.h"
#include <map>
#include <omp.h>

using namespace std;

IUH_IF::IUH_IF(void) : m_TimeStep(-1), m_nCells(-1), m_CellWidth(NODATA_VALUE), m_nsub(-1), m_subbasin(NULL),
                       m_iuhCell(NULL), m_ssru(NULL), m_iuhCols(-1), m_cellFlowCols(-1)
{
    m_Q_SBIF = NULL;
    m_cellFlow = NULL;
}

IUH_IF::~IUH_IF(void)
{
    //// cleanup
    if (m_Q_SBIF != NULL)
        delete[] m_Q_SBIF;
    if (this->m_cellFlow != NULL)
    {
        for (int i = 0; i < this->m_nCells; i++)
        {
            if (this->m_cellFlow[i] != NULL) delete[] this->m_cellFlow[i];
        }
        delete[] this->m_cellFlow;
    }
}

bool IUH_IF::CheckInputData(void)
{
    if (m_nCells < 0)
    {
        throw ModelException("IUH_IF", "CheckInputData", "The parameter: m_nCells has not been set.");
        return false;
    }
    if (FloatEqual(m_CellWidth, NODATA_VALUE))
    {
        throw ModelException("IUH_IF", "CheckInputData", "The parameter: m_CellWidth has not been set.");
        return false;
    }
    if (m_TimeStep <= 0)
    {
        throw ModelException("IUH_IF", "CheckInputData", "The parameter: m_TimeStep has not been set.");
        return false;
    }

    if (m_subbasin == NULL)
    {
        throw ModelException("IUH_IF", "CheckInputData", "The parameter: m_subbasin has not been set.");
        return false;
    }
    /*if (m_uhmaxCell == NULL)
    {
    throw ModelException("IUH_IF","CheckInputData","The parameter: m_uhmaxCell has not been set.");
    return false;
    }
    if (m_uhminCell == NULL)
    {
    throw ModelException("IUH_IF","CheckInputData","The parameter: m_uhminCell has not been set.");
    return false;
    }*/
    if (m_iuhCell == NULL)
    {
        throw ModelException("IUH_IF", "CheckInputData", "The parameter: m_iuhCell has not been set.");
        return false;
    }
    if (m_ssru == NULL)
    {
        throw ModelException("IUH_IF", "CheckInputData", "The parameter: m_rs has not been set.");
        return false;
    }
    if (m_date < 0)
    {
        throw ModelException("IUH_IF", "CheckInputData", "The parameter: m_date has not been set.");
        return false;
    }

    return true;
}

void IUH_IF::initialOutputs()
{
    if (this->m_nCells <= 0 || this->m_subbasin == NULL)
        throw ModelException("IUH_IF", "CheckInputData", "The dimension of the input data can not be less than zero.");
    // allocate the output variables

    if (m_nsub <= 0)
    {
        map<int, int> subs;
        for (int i = 0; i < this->m_nCells; i++)
        {
            subs[int(this->m_subbasin[i])] += 1;
        }
        this->m_nsub = subs.size();
    }

    if (m_cellFlow == NULL)
    {
        m_Q_SBIF = new float[m_nsub + 1];
        for (int i = 0; i <= m_nsub; i++)
        {
            m_Q_SBIF[i] = 0.f;
        }
        m_cellFlow = new float *[this->m_nCells];

        for (int i = 0; i < this->m_nCells; i++)
            m_cellFlowCols = max(int(m_iuhCell[i][1] + 1), m_cellFlowCols);

        //get m_cellFlowCols, i.e. the maximum of second column of iuh add 1.
#pragma omp parallel for
        for (int i = 0; i < this->m_nCells; i++)
        {
            m_cellFlow[i] = new float[m_cellFlowCols];
            for (int j = 0; j < m_cellFlowCols; j++)
                m_cellFlow[i][j] = 0.0f;
        }
    }
}

int IUH_IF::Execute()
{
    this->CheckInputData();

    this->initialOutputs();

#pragma omp parallel for
    for (int n = 0; n < m_nsub + 1; n++)
    {
        m_Q_SBIF[n] = 0.0f;    // delete value of last time step
    }


    //int nt = 0;
    //float qs_cell = 0.0f;
    float area = m_CellWidth * m_CellWidth;

    //#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++)
    {
        //forward one time step
        for (int j = 0; j < m_cellFlowCols; j++)
        {
            if (j != m_cellFlowCols - 1)
                m_cellFlow[i][j] = m_cellFlow[i][j + 1];
            else
                m_cellFlow[i][j] = 0.0f;
        }

        //add today's flow
        int subi = (int) m_subbasin[i];

        if (m_nsub == 1)
        {
            subi = 1;
        }
        else if (subi >= m_nsub + 1)
        {
            throw ModelException("IUH_IF", "Execute", "The subbasin " + ValueToString(subi) + " is invalid.");
        }

        float v_rs = m_ssru[i];
        if (v_rs > 0.f)
        {
            int min = int(this->m_iuhCell[i][0]);
            int max = int(this->m_iuhCell[i][1]);
            int col = 2;
            for (int k = min; k <= max; k++)
            {
                this->m_cellFlow[i][k] += v_rs / 1000.0f * m_iuhCell[i][col] * area / m_TimeStep;
                col++;
            }
        }
        //#pragma omp critical
        {
            m_Q_SBIF[subi] += this->m_cellFlow[i][0];    //get new value
        }
    }

    float tmp = 0.f;
    //#pragma omp parallel for reduction(+:tmp)
    for (int n = 1; n < m_nsub + 1; n++)
    {
        tmp += m_Q_SBIF[n];        //get overland flow routing for entire watershed.
    }
    m_Q_SBIF[0] = tmp;

    return 0;
}

bool IUH_IF::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        throw ModelException("IUH_IF", "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (this->m_nCells != n)
    {
        if (this->m_nCells <= 0) this->m_nCells = n;
        else
        {
            throw ModelException("IUH_IF", "CheckInputSize", "Input data for " + string(key) +
                                                             " is invalid. All the input data should have same size.");
            return false;
        }
    }

    return true;
}

void IUH_IF::SetValue(const char *key, float value)
{
    string sk(key);

    if (StringMatch(sk, Tag_TimeStep))
    {
        m_TimeStep = (int) value;
    }
    else if (StringMatch(sk, Tag_CellWidth))
    {
        m_CellWidth = value;
    }
    else if (StringMatch(sk, Tag_CellSize))
    {
        m_nCells = (int) value;
    }
    else if (StringMatch(sk, VAR_OMP_THREADNUM))
    {
        omp_set_num_threads((int) value);
    }
    else
        throw ModelException("IUH_IF", "SetValue", "Parameter " + sk
                                                   +
                                                   " does not exist in IUH_IF method. Please contact the module developer.");

}

void IUH_IF::Set1DData(const char *key, int n, float *data)
{

    this->CheckInputSize(key, n);

    //set the value
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSN))
    {
        m_subbasin = data;
    }
    else if (StringMatch(sk, VAR_SSRU))
    {
        m_ssru = data;
    }
    else
        throw ModelException("IUH_IF", "SetValue", "Parameter " + sk +
                                                   " does not exist in IUH_IF method. Please contact the module developer.");

}

void IUH_IF::Set2DData(const char *key, int nRows, int nCols, float **data)
{

    string sk(key);
    if (StringMatch(sk, VAR_OL_IUH))
    {
        this->CheckInputSize(VAR_OL_IUH, nRows);

        m_iuhCell = data;
        m_iuhCols = nCols;
    }
    else
        throw ModelException("IUH_IF", "SetValue", "Parameter " + sk +
                                                   " does not exist in IUH_IF method. Please contact the module developer.");
}

void IUH_IF::Get1DData(const char *key, int *n, float **data)
{
    string sk(key);
    if (StringMatch(sk, VAR_SBIF))
    {
        *data = this->m_Q_SBIF;
    }
    else
        throw ModelException("IUH_IF", "getResult", "Result " + sk +
                                                    " does not exist in IUH_IF method. Please contact the module developer.");

    *n = this->m_nsub + 1;
}


