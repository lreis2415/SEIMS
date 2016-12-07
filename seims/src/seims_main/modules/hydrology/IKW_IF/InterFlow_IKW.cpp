/*!
 * \brief Interflow routing using implicit finite difference method
 * kinematic wave method in LISEM model
 * \author Junzhi Liu
 * \date Feb. 2011 
 */

//#include "vld.h"
#include "InterFlow_IKW.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "util.h"
#include <omp.h>
#include <cmath>
#include <iostream>

using namespace std;

InterFlow_IKW::InterFlow_IKW(void) : m_nCells(-1), m_dt(-1.0f), m_CellWidth(-1.0f), m_chWidth(NULL),
                                     m_s0(NULL), m_rootDepth(NULL), m_ks(NULL), m_landuseFactor(1.f),
                                     m_soilMoistrue(NULL), m_porosity(NULL), m_poreIndex(NULL), m_fieldCapacity(NULL),
                                     m_flowInIndex(NULL), m_routingLayers(NULL), m_nLayers(-1),
                                     m_q(NULL), m_h(NULL), m_sr(NULL), m_streamLink(NULL), m_hReturnFlow(NULL),
                                     m_soilDepth(NULL)
{
    SQ2 = sqrt(2.f);
}


InterFlow_IKW::~InterFlow_IKW(void)
{
    if (m_h != NULL)
        delete[] m_h;
    if (m_q != NULL)
        delete[] m_q;
    if (m_hReturnFlow != NULL)
        delete[] m_hReturnFlow;
}

bool InterFlow_IKW::CheckInputData(void)
{
    if (this->m_date <= 0)
    {
        throw ModelException(MID_IKW_IF, "CheckInputData", "You have not set the Date variable.");
        return false;
    }

    if (this->m_nCells <= 0)
    {
        throw ModelException(MID_IKW_IF, "CheckInputData", "The cell number of the input can not be less than zero.");
        return false;
    }

    if (this->m_dt <= 0)
    {
        throw ModelException(MID_IKW_IF, "CheckInputData", "You have not set the TimeStep variable.");
        return false;
    }

    if (this->m_CellWidth <= 0)
    {
        throw ModelException(MID_IKW_IF, "CheckInputData", "You have not set the CellWidth variable.");
        return false;
    }

    if (m_chWidth == NULL)
        throw ModelException(MID_IKW_CH, "CheckInputData", "The parameter: CHWIDTH has not been set.");

    if (m_flowInIndex == NULL)
        throw ModelException(MID_IKW_IF, "CheckInputData", "The parameter: flow in index has not been set.");
    if (m_routingLayers == NULL)
        throw ModelException(MID_IKW_IF, "CheckInputData", "The parameter: routingLayers has not been set.");

    if (m_s0 == NULL)
        throw ModelException(MID_IKW_IF, "CheckInputData", "The parameter: slope has not been set.");
    if (m_rootDepth == NULL)
        throw ModelException(MID_IKW_IF, "CheckInputData", "The parameter: soil depth has not been set.");
    if (m_ks == NULL)
        throw ModelException(MID_IKW_IF, "CheckInputData", "The parameter: Conductivity has not been set.");

    if (this->m_porosity == NULL)
        throw ModelException(MID_IKW_IF, "CheckInputData", "The porosity can not be NULL.");
    if (this->m_poreIndex == NULL)
        throw ModelException(MID_IKW_IF, "CheckInputData", "The pore index can not be NULL.");
    if (this->m_fieldCapacity == NULL)
        throw ModelException(MID_IKW_IF, "CheckInputData", "The field capacity can not be NULL.");
    if (this->m_soilMoistrue == NULL)
        throw ModelException(MID_IKW_IF, "CheckInputData", "The soil moistrue can not be NULL.");
    if (this->m_streamLink == NULL)
        throw ModelException(MID_IKW_IF, "CheckInputData", "The STREAM_LINK can not be NULL.");

    if (this->m_sr == NULL)
        throw ModelException(MID_IKW_IF, "CheckInputData", "The parameter D_SURU is not set.");

    return true;
}

void  InterFlow_IKW::initialOutputs()
{
    if (this->m_nCells <= 0)
        throw ModelException(MID_IKW_IF, "initialOutputs", "The cell number of the input can not be less than zero.");

    if (m_q == NULL)
    {
        CheckInputData();

        m_q = new float[m_nCells];
        m_h = new float[m_nCells];
        m_hReturnFlow = new float[m_nCells];
        for (int i = 0; i < m_nCells; ++i)
        {
            m_q[i] = 0.0f;
            m_h[i] = 0.f;
            m_hReturnFlow[i] = 0.f;
        }
    }
}

void InterFlow_IKW::FlowInSoil(int id)
{
    //sum the upstream overland flow
    float qUp = 0.0f;
    for (int k = 1; k <= (int) m_flowInIndex[id][0]; ++k)
    {
        int flowInID = (int) m_flowInIndex[id][k];
        if (m_streamLink[id] > 0)
            qUp += m_q[flowInID];
    }

    float flowWidth = m_CellWidth;
    // there is no land in this cell
    if (m_streamLink[id] > 0)
    {
        flowWidth -= m_chWidth[id];
        if (flowWidth <= 0)
        {
            m_q[id] = qUp;
            m_h[id] = 0.f;
        }
        return;
    }

    // adjust soil moisture
    float s0 = m_s0[id];
    float soilVolumn = m_rootDepth[id] / 1000 * m_CellWidth * flowWidth / cos(atan(s0)); //m3
    m_soilMoistrue[id] += qUp * m_dt / soilVolumn;

    // the water exceeds the porosity is added to storage (return flow)
    if (m_soilMoistrue[id] > m_porosity[id])
    {
        m_hReturnFlow[id] = (m_soilMoistrue[id] - m_porosity[id]) * m_rootDepth[id];
        m_sr[id] += m_hReturnFlow[id];
        m_soilMoistrue[id] = m_porosity[id];
    }

    // if soil moisture is below the field capacity, no interflow will be generated
    if (m_soilMoistrue[id] < m_fieldCapacity[id])
    {
        m_q[id] = 0.f;
        m_h[id] = 0.f;
        return;
    }

    // calculate effective hydraulic conductivity (mm/h -> m/s)
    //float k = m_ks[id]/1000/3600 * pow((m_soilMoistrue[id] - m_residual[id])/(m_porosity[id] - m_residual[id]), m_poreIndex[id]);
    float k = m_ks[id] / 1000 / 3600 * pow(m_soilMoistrue[id] / m_porosity[id], m_poreIndex[id]);
    // calculate interflow (m3/s)
    m_q[id] = m_landuseFactor * m_rootDepth[id] / 1000 * s0 * k * m_CellWidth;

    // available water
    float availableWater = (m_soilMoistrue[id] - m_fieldCapacity[id]) * soilVolumn;
    float interFlow = m_q[id] * m_dt; // m3
    if (interFlow > availableWater)
    {
        m_q[id] = availableWater / m_dt;
        interFlow = availableWater;
    }
    m_h[id] = 1000 * interFlow / (m_CellWidth * m_CellWidth);

    // adjust soil moisture
    m_soilMoistrue[id] -= interFlow / soilVolumn;
}

int InterFlow_IKW::Execute()
{

    initialOutputs();

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
            FlowInSoil(id);
        }
    }
    return 0;
}

bool InterFlow_IKW::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        //this->StatusMsg("Input data for "+string(key) +" is invalid. The size could not be less than zero.");
        return false;
    }
    if (this->m_nCells != n)
    {
        if (this->m_nCells <= 0) this->m_nCells = n;
        else
        {
            //this->StatusMsg("Input data for "+string(key) +" is invalid. All the input data should have same size.");
            ostringstream oss;
            oss << "Input data for " + string(key) << " is invalid with size: " << n << ". The origin size is " <<
            m_nCells << ".\n";
            throw ModelException(MID_IKW_IF, "CheckInputSize", oss.str());
        }
    }

    return true;
}

void InterFlow_IKW::SetValue(const char *key, float data)
{
    string sk(key);
    if (StringMatch(sk, Tag_HillSlopeTimeStep))
        m_dt = data;
    else if (StringMatch(sk, Tag_CellWidth))
        m_CellWidth = data;
    else if (StringMatch(sk, Tag_CellSize))
        m_nCells = (int) data;
    else if (StringMatch(sk, VAR_KI))
        m_landuseFactor = data;
    else if (StringMatch(sk, VAR_OMP_THREADNUM))
        omp_set_num_threads((int) data);
    else
        throw ModelException(MID_IKW_IF, "SetSingleData", "Parameter " + sk
                                                          + " does not exist. Please contact the module developer.");

}

void InterFlow_IKW::Set1DData(const char *key, int n, float *data)
{
    //check the input data
    CheckInputSize(key, n);
    string s(key);
    if (StringMatch(s, VAR_SLOPE))
        m_s0 = data;
    else if (StringMatch(s, VAR_SOILDEPTH))
        m_soilDepth = data;
    else if (StringMatch(s, VAR_FIELDCAP))
        this->m_fieldCapacity = data;
    else if (StringMatch(s, VAR_SOILDEPTH))
        this->m_rootDepth = data;
    else if (StringMatch(s, VAR_CONDUCT))
        this->m_ks = data;
    else if (StringMatch(s, VAR_POROST))
        this->m_porosity = data;
    else if (StringMatch(s, VAR_POREID))
        this->m_poreIndex = data;
    else if (StringMatch(s, VAR_SOL_ST))
        this->m_soilMoistrue = data;
    else if (StringMatch(s, VAR_CHWIDTH))
        m_chWidth = data;
    else if (StringMatch(s, VAR_SURU))
        m_sr = data;
    else if (StringMatch(s, VAR_STREAM_LINK))
        m_streamLink = data;
    else
        throw ModelException(MID_IKW_IF, "Set1DData", "Parameter " + s
                                                      + " does not exist. Please contact the module developer.");

}

void InterFlow_IKW::Get1DData(const char *key, int *n, float **data)
{
    initialOutputs();

    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_QSOIL))
        *data = m_q;
    else if (StringMatch(sk, VAR_RETURNFLOW))
        *data = m_hReturnFlow;
    else
        throw ModelException(MID_IKW_IF, "Get1DData", "Output " + sk
                                                      +
                                                      " does not exist in current module. Please contact the module developer.");
}

void InterFlow_IKW::Set2DData(const char *key, int nrows, int ncols, float **data)
{
    //check the input data

    string sk(key);
    if (StringMatch(sk, Tag_ROUTING_LAYERS))
    {
        m_nLayers = nrows;
        m_routingLayers = data;
    }
    else if (StringMatch(sk, Tag_FLOWIN_INDEX_D8))
        m_flowInIndex = data;
    else
        throw ModelException(MID_IKW_IF, "Set2DData", "Parameter " + sk
                                                      + " does not exist. Please contact the module developer.");
}