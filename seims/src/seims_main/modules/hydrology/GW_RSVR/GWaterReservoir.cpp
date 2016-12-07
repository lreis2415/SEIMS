/*!
 * \brief Calculate groundwater using reservoir method
 * \author Junzhi Liu
 * \date Oct. 2011
 * \revesed LiangJun Zhu
 * \note Change the module name from GWATER_RESERVOIR to GW_RSVR
 */
#include "GWaterReservoir.h"
#include "MetadataInfo.h"
#include <cmath>
#include <ctime>
#include <iostream>
#include <fstream>
#include "ModelException.h"
#include "util.h"

#include <omp.h>

using namespace std;

GWaterReservoir::GWaterReservoir(void) : m_recharge(NULL), m_storage(NULL), m_recessionCoefficient(-1.f),
                                         m_recessionExponent(1.f), m_CellWidth(-1.f),
                                         m_deepCoefficient(0.f), m_nCells(-1), m_nReaches(-1), m_qg(NULL),
                                         m_percSubbasin(NULL), m_subbasin(NULL),
                                         m_nCellsSubbasin(NULL), m_initStorage(0.f)
{
}

GWaterReservoir::~GWaterReservoir(void)
{
    if (m_qg != NULL)
        delete[] m_qg;

    if (m_nCellsSubbasin != NULL)
        delete[] m_nCellsSubbasin;

    if (m_percSubbasin != NULL)
        delete[] m_percSubbasin;

    if (m_storage != NULL)
        delete[] m_storage;
}


bool GWaterReservoir::CheckInputData()
{
    if (this->m_date < 0)
    {
        throw ModelException(MID_GW_RSVR, "CheckInputData", "You have not set the time.");
        return false;
    }
    if (this->m_dt < 0)
    {
        throw ModelException(MID_GW_RSVR, "CheckInputData", "You have not set the time step.");
        return false;
    }
    if (this->m_nCells <= 0)
    {
        throw ModelException(MID_GW_RSVR, "CheckInputData", "The cell number is not set.");
        return false;
    }

    if (m_CellWidth <= 0)
    {
        throw ModelException(MID_GW_RSVR, "CheckInputData", "The cell width  is not set.");
        return false;
    }

    if (m_recessionCoefficient <= 0)
    {
        throw ModelException(MID_GW_RSVR, "CheckInputData", "The base flow recession coefficient is not set.");
        return false;
    }

    if (m_recharge == NULL)
    {
        throw ModelException(MID_GW_RSVR, "CheckInputData", "The percolation is not set.");
        return false;
    }

    return true;
}

bool GWaterReservoir::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        throw ModelException(MID_GW_RSVR, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (this->m_nCells != n)
    {
        if (this->m_nCells <= 0)
            this->m_nCells = n;
        else
        {
            //throw ModelException(MID_GW_RSVR,"CheckInputSize","Input data for "+string(key) +" is invalid. All the input data should have same size.");
            ostringstream oss;
            oss << "Input data for " + string(key) << " is invalid with size: " << n << ". The origin size is " <<
            m_nCells << ".\n";
            throw ModelException(MID_GW_RSVR, "CheckInputSize", oss.str());
        }
    }
    return true;
}


void GWaterReservoir::InitOutputs(void)
{
    if (m_qg == NULL)
    {
        m_qg = new float[m_nReaches + 1];
        m_percSubbasin = new float[m_nReaches + 1];
        m_nCellsSubbasin = new int[m_nReaches + 1];
        m_storage = new float[m_nReaches + 1];

#pragma omp parallel for
        for (int i = 0; i <= m_nReaches; i++)
        {
            m_qg[i] = 0.f;
            m_nCellsSubbasin[i] = 0;
            m_storage[i] = m_initStorage;
        }

#ifdef MULTIPLY_REACHES
        for (int i = 0; i < m_nCells; i++)
        {
            int subbasinId = (int) m_subbasin[i];
            m_nCellsSubbasin[subbasinId] += 1;
        }
#else
        m_nCellsSubbasin[1] = m_nCells;
#endif
    }
}

int GWaterReservoir::Execute(void)
{
    InitOutputs();
    CheckInputData();
#pragma omp parallel for
    for (int i = 0; i <= m_nReaches; i++)
        m_percSubbasin[i] = 0.f;

    // get percolation for each subbasin
    for (int i = 0; i < m_nCells; i++)
    {
#ifdef MULTIPLY_REACHES
        int subbasinId = (int) m_subbasin[i];
#else
        int subbasinId = 1;
#endif
        m_percSubbasin[subbasinId] += m_recharge[i];
    }

    //float sum = 0.f;
#pragma omp parallel for //reduction(+:sum)
    for (int i = 1; i <= m_nReaches; i++)
    {
        float percolation = m_percSubbasin[i] * (1 - m_deepCoefficient) / m_nCellsSubbasin[i];
        // depth of groundwater runoff(mm)
        float outFlowDepth = m_recessionCoefficient * pow(m_storage[i], m_recessionExponent);
        // groundwater flow out of the subbasin at time t (m3/s)
        m_qg[i] = outFlowDepth / 1000 * m_nCellsSubbasin[i] * m_CellWidth * m_CellWidth / m_dt;
        //sum = sum + m_qg[i];

        // water balance (mm)
        m_storage[i] += percolation - outFlowDepth;
    }
    //m_qg[0] = sum;

    return 0;
}


// set value
void GWaterReservoir::SetValue(const char *key, float value)
{
    string sk(key);
    if (StringMatch(sk, Tag_HillSlopeTimeStep))
    {
        m_dt = value;
    }
    else if (StringMatch(sk, Tag_CellWidth))
    {
        m_CellWidth = value;
    }
    else if (StringMatch(sk, VAR_OMP_THREADNUM))
    {
        omp_set_num_threads((int) value);
    }
    else if (StringMatch(sk, VAR_GW_KG))
    {
        m_recessionCoefficient = value;
    }
    else if (StringMatch(sk, VAR_Base_ex))
    {
        m_recessionExponent = value;
    }
    else if (StringMatch(sk, VAR_GW0))
    {
        m_initStorage = value;
    }
    else if (StringMatch(sk, VAR_GWMAX))
    {
        m_storageMax = value;
    }
    else
    {
        throw ModelException(MID_GW_RSVR, "SetValue", "Parameter " + sk + " does not exist in SetValue method.");
    }
}

void GWaterReservoir::Set1DData(const char *key, int n, float *data)
{
    //check the input data
    if (!this->CheckInputSize(key, n)) return;
    //set the value
    string sk(key);
    if (StringMatch(sk, VAR_PERCO))
    {
        m_recharge = data;
    }
    else if (StringMatch(sk, VAR_SUBBSN))
        this->m_subbasin = data;
    else
    {
        throw ModelException(MID_GW_RSVR, "Set1DData",
                             "Parameter " + sk + " does not exist. Please contact the module developer.");
    }
}

void GWaterReservoir::Set2DData(const char *key, int nrows, int ncols, float **data)
{
    string sk(key);

    if (StringMatch(sk, Tag_RchParam))
    {
        m_nReaches = ncols - 1;
    }
    else
        throw ModelException(MID_GW_RSVR, "Set2DData", "Parameter " + sk
                                                       + " does not exist. Please contact the module developer.");
}

void GWaterReservoir::Get1DData(const char *key, int *n, float **data)
{
    InitOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SBQG))
    {
        *data = m_qg;
    }
    else if (StringMatch(sk, VAR_SBGS))
    {
        *data = m_storage;
    }
    else
    {
        throw ModelException(MID_GW_RSVR, "Get1DData",
                             "Parameter " + sk + " does not exist. Please contact the module developer.");
    }
}
