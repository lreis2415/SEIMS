/*!
 * \brief
 * \author Junzhi Liu
 * \date May 2011
 */
#include "HS_WB.h"
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

HS_WB::HS_WB(void) : m_dtHs(-1.f), m_dtCh(-1.f), m_nReaches(-1), m_nCells(-1), m_qs(NULL), m_qi(NULL), m_subbasin(NULL),
                     m_streamLink(NULL),
                     m_qsInput(NULL), m_qiInput(NULL), m_qiTemp(NULL), m_qsTemp(NULL)
{
}

HS_WB::~HS_WB(void)
{
    if (m_qsInput != NULL)
        delete[] m_qsInput;
    if (m_qiInput != NULL)
        delete[] m_qiInput;
    if (m_qsTemp != NULL)
        delete[] m_qsTemp;
    if (m_qiTemp != NULL)
        delete[] m_qiTemp;
}

void  HS_WB::initialOutputs()
{
    if (m_qsInput == NULL)
    {
        CheckInputData();
        //cout << "Number of reaches: " << m_nReaches << endl;

        m_qsInput = new float[m_nReaches + 1];
        m_qiInput = new float[m_nReaches + 1];
        m_qsTemp = new float[m_nReaches + 1];
        m_qiTemp = new float[m_nReaches + 1];

#pragma omp parallel for
        for (int i = 0; i <= m_nReaches; i++)
        {
            m_qsInput[i] = 0.f;
            m_qiInput[i] = 0.f;
            m_qsTemp[i] = 0.f;
            m_qiTemp[i] = 0.f;
        }
    }
}

//Execute module
int HS_WB::Execute()
{

    initialOutputs();

    /// get the input of each subbasin in current step
#pragma omp parallel for
    for (int i = 1; i <= m_nReaches; i++)
    {
        m_qsTemp[i] = 0.f;
        m_qiTemp[i] = 0.f;
    }

    int reachId;
    for (int i = 0; i < m_nCells; i++)
    {
        if (m_streamLink[i] > 0)
        {
#ifdef MULTIPLY_REACHES
            reachId = int(m_streamLink[i]);
#else
            reachId = 1;
#endif
            m_qsTemp[reachId] += m_qs[i];
            if (m_qi != NULL)
                m_qiTemp[reachId] += m_qi[i];
        }
    }

    m_qsInput[0] = 0.f;
    m_qiInput[0] = 0.f;
    for (int i = 1; i <= m_nReaches; i++)
    {
        m_qsInput[i] = (m_qsInput[i] * (m_tsCounter - 1) + m_qsTemp[i]) / m_tsCounter;
        m_qiInput[i] = (m_qiInput[i] * (m_tsCounter - 1) + m_qiTemp[i]) / m_tsCounter;

        m_qsInput[0] += m_qsInput[i];
        m_qiInput[0] += m_qiInput[i];
    }

    m_tsCounter++;

    return 0;
}

void HS_WB::SetValue(const char *key, float data)
{
    string s(key);
    if (StringMatch(s, VAR_OMP_THREADNUM))
    {
        omp_set_num_threads((int) data);
    }
    else
        throw ModelException(MID_HS_WB, "SetValue", "Parameter " + s
                                                    +
                                                    " does not exist in current module. Please contact the module developer.");
}


void HS_WB::Set1DData(const char *key, int nRows, float *data)
{
    string s(key);

    this->CheckInputSize(key, nRows);

    if (StringMatch(s, VAR_QOVERLAND)) this->m_qs = data;
    else if (StringMatch(s, VAR_QSOIL)) this->m_qi = data;
    else if (StringMatch(s, VAR_SUBBSN)) this->m_subbasin = data;
    else if (StringMatch(s, VAR_STREAM_LINK)) this->m_streamLink = data;

        /*else if(StringMatch(s,"D_INLO"))				this->m_interception = data;
        else if(StringMatch(s,"D_P"))			this->m_precipitation = data;
        else if(StringMatch(s,"D_INET"))		this->m_ei = data;
        else if(StringMatch(s,"D_DPST"))		this->m_depression = data;
        else if(StringMatch(s,"D_DEET"))		this->m_ed = data;
        else if(StringMatch(s,"D_INFIL"))		this->m_infil = data;
        else if(StringMatch(s,"D_SOET"))		this->m_es = data;
        else if(StringMatch(s,"D_GRRE"))		this->m_percolation = data;
        else if(StringMatch(s,"D_Revap"))		this->m_revap = data;
        else if(StringMatch(s,"D_SSRU"))		this->m_ri = data;
        else if(StringMatch(s,"D_SNSB"))		this->m_se = data;
        else if(StringMatch(s,"D_TMIN"))		this->m_tMin = data;
        else if(StringMatch(s,"D_TMAX"))		this->m_tMax = data;
        else if(StringMatch(s,"D_SOTE"))		this->m_soilT = data;*/

    else if (StringMatch(s, VAR_SOILDEPTH)) this->m_rootdepth = data;
        //else if(StringMatch(s,VAR_SOMO))		this->m_soilMoisture = data;

    else if (StringMatch(s, VAR_POROST)) this->m_porosity = data;
    else if (StringMatch(s, VAR_FIELDCAP)) this->m_fieldCapacity = data;
    else if (StringMatch(s, VAR_NEPR)) m_pNet = data;
    else
        throw ModelException(MID_HS_WB, "Set1DData", "Parameter " + s +
                                                     " does not exist in current module. Please contact the module developer.");
}

void HS_WB::Get1DData(const char *key, int *nRows, float **data)
{
    *nRows = m_nReaches + 1;
    string s(key);
    if (StringMatch(s, VAR_SBOF))
        *data = m_qsInput;
    else if (StringMatch(s, VAR_SBIF))
        *data = m_qiInput;
    else
        throw ModelException(MID_HS_WB, "Get1DData", "Result " + string(key) +
                                                     " does not exist in current module. Please contact the module developer.");
}

void HS_WB::Get2DData(const char *key, int *nRows, int *nCols, float ***data)
{
    string s(key);
    if (StringMatch(s, VAR_SOWB))
    {
        //setValueToSubbasin();
        //*nRows = m_subbasinSelectedCount;
        *nCols = 17;
        *data = m_soilWaterBalance;
    }
    else
        throw ModelException(MID_HS_WB, "Get2DData", "Result " + s +
                                                     " does not exist in current module. Please contact the module developer.");
}

void HS_WB::Set2DData(const char *key, int nrows, int ncols, float **data)
{
    string sk(key);

    if (StringMatch(sk, Tag_RchParam))
    {
        m_nReaches = ncols - 1;
    }
    else
        throw ModelException(MID_HS_WB, "Set2DData", "Parameter " + sk
                                                     + " does not exist. Please contact the module developer.");
}

bool HS_WB::CheckInputData()
{
    //if(this->m_date <=0)				throw ModelException(MID_HS_WB,"CheckInputData","You have not set the time.");
    if (m_nCells <= 0)
        throw ModelException(MID_HS_WB, "CheckInputData", "The dimension of the input data can not be less than zero.");
    //if(this->m_qi  == NULL)				throw ModelException(MID_HS_WB,"CheckInputData","The interflow  can not be NULL.");
    if (this->m_qs == NULL) throw ModelException(MID_HS_WB, "CheckInputData", "The overland flow can not be NULL.");
    if (this->m_subbasin == NULL) throw ModelException(MID_HS_WB, "CheckInputData", "The subbasion can not be NULL.");
    if (this->m_streamLink == NULL)
        throw ModelException(MID_HS_WB, "CheckInputData", "The StreamLink can not be NULL.");


    //if(this->m_precipitation == NULL)	throw ModelException(MID_HS_WB,"CheckInputData","The precipitation data can not be NULL.");
    //if(this->m_depression  == NULL)		throw ModelException(MID_HS_WB,"CheckInputData","The depression data can not be NULL.");
    //if(this->m_ed == NULL)				throw ModelException(MID_HS_WB,"CheckInputData","The evaporation of depression can not be NULL.");
    //if(this->m_ei  == NULL)				throw ModelException(MID_HS_WB,"CheckInputData","The evaporation of interception can not be NULL.");
    //if(this->m_es  == NULL)				throw ModelException(MID_HS_WB,"CheckInputData","The evaporation of soil can not be NULL.");
    //if(this->m_infil  == NULL)			throw ModelException(MID_HS_WB,"CheckInputData","The infiltration can not be NULL.");
    //if(this->m_interception  == NULL)	throw ModelException(MID_HS_WB,"CheckInputData","The interception can not be NULL.");
    //
    //if(this->m_percolation  == NULL)	throw ModelException(MID_HS_WB,"CheckInputData","The percolation data can not be NULL.");
    //if(this->m_revap  == NULL)			throw ModelException(MID_HS_WB,"CheckInputData","The revap can not be NULL.");
    //
    //if(this->m_ri  == NULL)				throw ModelException(MID_HS_WB,"CheckInputData","The runoff of subsurface can not be NULL.");
    //if(this->m_rootdepth == NULL)		throw ModelException(MID_HS_WB,"CheckInputData","The root depth can not be NULL.");
    //
    //
    //if(this->m_tMin == NULL)			throw ModelException(MID_HS_WB,"CheckInputData","The min temperature can not be NULL.");
    //if(this->m_tMax == NULL)			throw ModelException(MID_HS_WB,"CheckInputData","The max temperature can not be NULL.");
    //if(this->m_soilT == NULL)			throw ModelException(MID_HS_WB,"CheckInputData","The soil temperature can not be NULL.");
    //if(this->m_porosity == NULL)		throw ModelException(MID_HS_WB,"CheckInputData","The porocity can not be NULL.");
    //if(this->m_fieldCapacity == NULL)		throw ModelException(MID_HS_WB,"CheckInputData","The field capacity can not be NULL.");

    return true;
}

bool HS_WB::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
    {
        throw ModelException(MID_HS_WB, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (this->m_nCells != n)
    {
        if (this->m_nCells <= 0)
            this->m_nCells = n;
        else
        {
            cout << key << "\t" << n << "\t" << m_nCells << endl;
            throw ModelException(MID_HS_WB, "CheckInputSize", "Input data for " + string(key) +
                                                              " is invalid. All the input data should have same size.");
            return false;
        }
    }

    return true;
}
