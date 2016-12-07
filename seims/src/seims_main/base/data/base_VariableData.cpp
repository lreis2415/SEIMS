/*!
 * \brief Base class of variable data
 *
 * Base class used to store various versions of the variable data
 *
 * \author Junzhi Liu
 * \version 1.1
 * \date Jun. 2010
 *
 * 
 */

#include "base_VariableData.h"


base_VariableData::base_VariableData(void)
{
    m_Dimension = DT_Unknown;
}


base_VariableData::base_VariableData(const base_VariableData &obj)
{
    m_Dimension = obj.m_Dimension;
}


base_VariableData::~base_VariableData(void)
{
}


base_VariableData &base_VariableData::operator=(const base_VariableData &obj)
{
    m_Dimension = obj.m_Dimension;
    return *this;
}


dimensionTypes base_VariableData::Dimension(void)
{
    return DT_Unknown;
}

