#include "MetadataInfo.h"
#include "text.h"

// private methods
void MetadataInfo::OpenTag(string name, string attributes, int indent, string* sb) {
    size_t sz = (sb->size() + (name.size() + attributes.size() + (indent * 3) + 3));
    if (sz > sb->capacity()) {
        sb->reserve(sz);
    }
    for (int i = 0; i < indent; i++) sb->append("   ");
    sb->append("<");
    sb->append(name);
    if (!attributes.empty()) {
        sb->append(" ");
        sb->append(attributes);
    }
    sb->append(">");
}

void MetadataInfo::CloseTag(string name, int indent, string* sb) {
    size_t sz = (sb->size() + (name.size() + (indent * 3) + 2));
    if (sz > sb->capacity()) {
        sb->reserve(sz);
    }
    for (int i = 0; i < indent; i++) sb->append("   ");
    sb->append("</");
    sb->append(name);
    sb->append(">");
}

void MetadataInfo::FullTag(const string& name, int indent, string& content, string* sb) {
    size_t sz = (sb->size() + (name.size() + content.size() + (indent * 3) + 5));
    if (sz > sb->capacity()) {
        sb->reserve(sz);
    }
    for (int i = 0; i < indent; i++) sb->append("   ");
    sb->append("<");
    sb->append(name);
    sb->append(">");
    sb->append(content);
    sb->append("</");
    sb->append(name);
    sb->append(">");
}

void MetadataInfo::WriteClass(int indent, string* sb) {
    OpenTag(TagClass, "", indent, sb);

    FullTag(TagClassName, indent + 1, m_oClass.Name, sb);
    FullTag(TagClassDescription, indent + 1, m_oClass.Description, sb);

    CloseTag(TagClass, indent, sb);
}

void MetadataInfo::WriteInformation(int indent, string* sb) {
    OpenTag(TagInformation, "", indent, sb);

    FullTag(TagInfoId, indent + 1, m_Info.Id, sb);
    FullTag(TagInfoName, indent + 1, m_Info.Name, sb);
    FullTag(TagInfoDescription, indent + 1, m_Info.Description, sb);
    FullTag(TagInfoVersion, indent + 1, m_Info.Version, sb);
    FullTag(TagInfoAuthor, indent + 1, m_Info.Author, sb);
    FullTag(TagInfoEmail, indent + 1, m_Info.EMail, sb);
    FullTag(TagInfoWebsite, indent + 1, m_Info.Website, sb);
    FullTag(TagInfoHelpfile, indent + 1, m_Info.Helpfile, sb);

    CloseTag(TagInformation, indent, sb);
}

void MetadataInfo::DimensionTag(string tag, int indent, dimensionTypes dimType, string* sb) {
    string strTmp = "";

    switch (dimType) {
        case DT_Array1D: strTmp = Type_Array1D;
            break;
        case DT_Array1DDateValue: strTmp = Type_Array1DDateValue;
            break;
        case DT_Array2D: strTmp = Type_Array2D;
            break;
        case DT_Raster1D: strTmp = Type_Raster1D;
            break;
        case DT_Raster2D: strTmp = Type_Raster2D;
            break;
        case DT_Single: strTmp = Type_Single;
            break;
        case DT_Scenario: strTmp = Type_Scenario;
            break;
        case DT_Reach: strTmp = Type_Reach;
            break;
        case DT_Subbasin: strTmp = Type_Subbasin;
            break;
        default: break;
    }

    FullTag(tag, indent, strTmp, sb);
}

void MetadataInfo::TransferTypeTag(string tag, int indent, transferTypes tf_type, string* sb) {
    string str_tmp;

    switch (tf_type) {
        case TF_None: str_tmp = TFType_Whole;
            break;
        case TF_SingleValue: str_tmp = TFType_Single;
            break;
        case TF_OneArray1D: str_tmp = TFType_Array1D;
            break;
        default: break;
    }

    FullTag(tag, indent, str_tmp, sb);
}

void MetadataInfo::WriteInputs(int indent, string* sb) {
    if (!m_vInputs.empty()) {
        OpenTag(TagInputs, "", indent, sb);
        for (size_t idx = 0; idx < m_vInputs.size(); idx++) {
            OpenTag(TagInputVariable, "", indent + 1, sb);
            InputVariable it = m_vInputs[idx];
            FullTag(TagVariableName, indent + 2, it.Name, sb);
            FullTag(TagVariableUnits, indent + 2, it.Units, sb);
            FullTag(TagVariableDescription, indent + 2, it.Description, sb);
            FullTag(TagVariableSource, indent + 2, it.Source, sb);
            DimensionTag(TagVariableDimension, indent + 2, it.Dimension, sb);
            TransferTypeTag(TagVariableTransfer, indent + 2, it.tfType, sb);
            CloseTag(TagInputVariable, indent + 1, sb);
        }
        CloseTag(TagInputs, indent, sb);
    }

}

void MetadataInfo::WriteOutputs(int indent, string* sb) {
    if (!m_vOutputs.empty()) {
        OpenTag(TagOutputs, "", indent, sb);
        for (size_t idx = 0; idx < m_vOutputs.size(); idx++) {
            OpenTag(TagOutputVariable, "", indent + 1, sb);
            OutputVariable it = m_vOutputs[idx];
            FullTag(TagVariableName, indent + 2, it.Name, sb);
            FullTag(TagVariableUnits, indent + 2, it.Units, sb);
            FullTag(TagVariableDescription, indent + 2, it.Description, sb);
            DimensionTag(TagVariableDimension, indent + 2, it.Dimension, sb);
            TransferTypeTag(TagVariableTransfer, indent + 2, it.tfType, sb);
            CloseTag(TagOutputVariable, indent + 1, sb);
        }
        CloseTag(TagOutputs, indent, sb);
    }
}

void MetadataInfo::WriteInOutputs(int indent, string* sb) {
    if (!m_vInOutputs.empty()) {
        OpenTag(TagInOutputs, "", indent, sb);
        for (size_t idx = 0; idx < m_vInOutputs.size(); idx++) {
            OpenTag(TagInOutputVariable, "", indent + 1, sb);
            InOutputVariable it = m_vInOutputs[idx];
            FullTag(TagVariableName, indent + 2, it.Name, sb);
            FullTag(TagVariableUnits, indent + 2, it.Units, sb);
            FullTag(TagVariableDescription, indent + 2, it.Description, sb);
            DimensionTag(TagVariableDimension, indent + 2, it.Dimension, sb);
            TransferTypeTag(TagVariableTransfer, indent + 2, it.tfType, sb);
            CloseTag(TagInOutputVariable, indent + 1, sb);
        }
        CloseTag(TagInOutputs, indent, sb);
    }
}

void MetadataInfo::WriteParameters(int indent, string* sb) {
    if (!m_vParameters.empty()) {
        OpenTag(TagParameters, "", indent, sb);
        for (size_t idx = 0; idx < m_vParameters.size(); idx++) {
            OpenTag(TagParameter, "", indent + 1, sb);

            Parameter it = m_vParameters[idx];
            FullTag(TagVariableName, indent + 2, it.Name, sb);
            FullTag(TagVariableUnits, indent + 2, it.Units, sb);
            FullTag(TagVariableDescription, indent + 2, it.Description, sb);
            FullTag(TagVariableSource, indent + 2, it.Source, sb);
            DimensionTag(TagVariableDimension, indent + 2, it.Dimension, sb);

            CloseTag(TagParameter, indent + 1, sb);
        }
        CloseTag(TagParameters, indent, sb);
    }
}

void MetadataInfo::WriteDependencies(int indent, string* sb) {
    if (!m_vDependencies.empty()) {
        OpenTag(TagDependencies, "", indent, sb);
        for (size_t idx = 0; idx < m_vDependencies.size(); idx++) {
            OpenTag(TagClass, "", indent + 1, sb);
            ModelClass it = m_vDependencies[idx];
            FullTag(TagClassName, indent + 2, it.Name, sb);
            FullTag(TagClassDescription, indent + 2, it.Description, sb);

            CloseTag(TagClass, indent + 1, sb);
        }
        CloseTag(TagDependencies, indent, sb);
    }
}

void MetadataInfo::WriteXMLHeader(string* sb) {
    sb->append(XMLHeader);
    sb->append(XMLComment);
}

// public methods
MetadataInfo::~MetadataInfo() {
    m_vInputs.clear();
    m_vOutputs.clear();
    m_vParameters.clear();
    m_vDependencies.clear();
}

void MetadataInfo::SetClass(const char* name, const char* description) {
    m_oClass.Name = name;
    m_oClass.Description = description;
}

int MetadataInfo::AddInput(const char* name, const char* units, const char* desc, const char* source,
                           dimensionTypes dimType, transferTypes tfType /* = TF_Whole */) {
    InputVariable param;
    param.Name = name;
    param.Units = units;
    param.Description = desc;
    param.Source = source;
    param.Dimension = dimType;
    param.tfType = tfType;

    m_vInputs.emplace_back(param);
    return CVT_INT(m_vInputs.size());
}

int MetadataInfo::AddOutput(const char* name, const char* units, const char* desc,
                            dimensionTypes dimType, transferTypes tfType /* = TF_Whole */) {
    OutputVariable param;
    param.Name = name;
    param.Units = units;
    param.Description = desc;
    param.Dimension = dimType;
    param.tfType = tfType;

    m_vOutputs.emplace_back(param);
    return CVT_INT(m_vOutputs.size());
}

int MetadataInfo::AddInOutput(const char* name, const char* units, const char* desc,
                              dimensionTypes dimType, transferTypes tfType /* = TF_Whole */) {
    InOutputVariable param;
    param.Name = name;
    param.Units = units;
    param.Description = desc;
    param.Source = Source_Module;
    param.Dimension = dimType;
    param.tfType = tfType;

    m_vInOutputs.emplace_back(param);

    AddOutput(name, units, desc, dimType, tfType);

    return CVT_INT(m_vInOutputs.size());
}

int MetadataInfo::AddParameter(const char* name, const char* units, const char* desc, const char* source,
                               dimensionTypes dimType) {
    Parameter param;
    param.Name = name;
    param.Units = units;
    param.Description = desc;
    param.Source = source;
    param.Dimension = dimType;

    m_vParameters.emplace_back(param);
    return CVT_INT(m_vParameters.size());
}

int MetadataInfo::AddDependency(const char* name, const char* description) {
    ModelClass cl;
    cl.Name = name;
    cl.Description = description;

    m_vDependencies.emplace_back(cl);
    return CVT_INT(m_vDependencies.size());
}

string MetadataInfo::GetXMLDocument() {
    string sb;

    WriteXMLHeader(&sb);
    OpenTag(TagMetadata, TagMetadataAttributes, 0, &sb);
    WriteClass(1, &sb);
    WriteInformation(1, &sb);
    WriteParameters(1, &sb);
    WriteInputs(1, &sb);
    WriteOutputs(1, &sb);
    WriteInOutputs(1, &sb);
    WriteDependencies(1, &sb);
    CloseTag(TagMetadata, 0, &sb);

    return sb;
}
