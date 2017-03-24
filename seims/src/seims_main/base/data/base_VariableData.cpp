#include "base_VariableData.h"

base_VariableData::base_VariableData(void) {
    m_Dimension = DT_Unknown;
}

base_VariableData::base_VariableData(const base_VariableData &obj) {
    m_Dimension = obj.m_Dimension;
}

base_VariableData::~base_VariableData(void) {
}

base_VariableData &base_VariableData::operator=(const base_VariableData &obj) {
    m_Dimension = obj.m_Dimension;
    return *this;
}

dimensionTypes base_VariableData::Dimension(void) {
    return DT_Unknown;
}

