#include "MetadataInfo.h"
#include "text.h"
#include "utils_string.h"

using ccgl::utils_string::StringMatch;

string dimensionTypeToString(dimensionTypes dt) {
    string str_tmp = "";

    switch (dt) {
        case DT_Array1D: str_tmp = Type_Array1D;
            break;
        case DT_Array1DInt: str_tmp = Type_Array1DInt;
            break;
        case DT_Array1DDateValue: str_tmp = Type_Array1DDateValue;
            break;
        case DT_Array2D: str_tmp = Type_Array2D;
            break;
        case DT_Array2DInt: str_tmp = Type_Array2DInt;
            break;
        case DT_Raster1D: str_tmp = Type_Raster1D;
            break;
        case DT_Raster1DInt: str_tmp = Type_Raster1DInt;
            break;
        case DT_Raster2D: str_tmp = Type_Raster2D;
            break;
        case DT_Raster2DInt: str_tmp = Type_Raster2DInt;
            break;
        case DT_Single: str_tmp = Type_Single;
            break;
        case DT_SingleInt: str_tmp = Type_SingleInt;
            break;
        case DT_Scenario: str_tmp = Type_Scenario;
            break;
        case DT_Reach: str_tmp = Type_Reach;
            break;
        case DT_Subbasin: str_tmp = Type_Subbasin;
            break;
        default: break;
    }
    return str_tmp;
}

dimensionTypes matchDimensionType(const string& type_str) {
    dimensionTypes typ = DT_Unknown;
    if (StringMatch(type_str, Type_Single)) typ = DT_Single;
    else if (StringMatch(type_str, Type_SingleInt)) typ = DT_SingleInt;
    else if (StringMatch(type_str, Type_Array1D)) typ = DT_Array1D;
    else if (StringMatch(type_str, Type_Array1DInt)) typ = DT_Array1DInt;
    else if (StringMatch(type_str, Type_Array2D)) typ = DT_Array2D;
    else if (StringMatch(type_str, Type_Array2DInt)) typ = DT_Array2DInt;
    else if (StringMatch(type_str, Type_Array1DDateValue)) typ = DT_Array1DDateValue;
    else if (StringMatch(type_str, Type_Raster1D)) typ = DT_Raster1D;
    else if (StringMatch(type_str, Type_Raster1DInt)) typ = DT_Raster1DInt;
    else if (StringMatch(type_str, Type_Raster2D)) typ = DT_Raster2D;
    else if (StringMatch(type_str, Type_Raster2DInt)) typ = DT_Raster2DInt;
    else if (StringMatch(type_str, Type_Scenario)) typ = DT_Scenario;
    else if (StringMatch(type_str, Type_Reach)) typ = DT_Reach;
    else if (StringMatch(type_str, Type_Subbasin)) typ = DT_Subbasin;
    else typ = DT_Unknown;

    return typ;
}


string transferTypeToString(transferTypes tt) {
    string str_tmp;

    switch (tt) {
        case TF_None: str_tmp = TFType_None;
            break;
        case TF_SingleValue: str_tmp = TFType_Single;
            break;
        case TF_OneArray1D: str_tmp = TFType_Array1D;
            break;
        default: break;
    }
    return str_tmp;
}

transferTypes matchTransferType(const string& tf_type) {
    transferTypes typ = TF_None;
    if (StringMatch(tf_type, TFType_None)) typ = TF_None;
    else if (StringMatch(tf_type, TFType_Single)) typ = TF_SingleValue;
    else if (StringMatch(tf_type, TFType_Array1D)) typ = TF_OneArray1D;
    else typ = TF_None;
    return typ;
}

string intervalTypeToString(intervalTypes it) {
    string str_tmp;

    switch (it) {
        case TI_Unlimit: str_tmp = TIType_Unlimit;
            break;
        case TI_Daily: str_tmp = TIType_Daily;
            break;
        case TI_Storm: str_tmp = TIType_Storm;
            break;
        default: break;
    }
    return str_tmp;
}

intervalTypes matchIntervalType(const string& ti_type) {
    intervalTypes typ = TI_Unlimit;
    if (StringMatch(ti_type, TIType_Unlimit)) typ = TI_Unlimit;
    else if (StringMatch(ti_type, TIType_Daily)) typ = TI_Daily;
    else if (StringMatch(ti_type, TIType_Storm)) typ = TI_Storm;
    else typ = TI_Unlimit;
    return typ;
}

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
    FullTag(TagInfoTimeScale, indent + 1, m_Info.TimeScale, sb);
    FullTag(TagInfoDescription, indent + 1, m_Info.Description, sb);
    FullTag(TagInfoVersion, indent + 1, m_Info.Version, sb);
    FullTag(TagInfoAuthor, indent + 1, m_Info.Author, sb);
    FullTag(TagInfoEmail, indent + 1, m_Info.Email, sb);
    FullTag(TagInfoWebsite, indent + 1, m_Info.Website, sb);
    FullTag(TagInfoHelpfile, indent + 1, m_Info.Helpfile, sb);

    CloseTag(TagInformation, indent, sb);
}

void MetadataInfo::DimensionTag(string tag, int indent, dimensionTypes dimType, string* sb) {
    string str_tmp = dimensionTypeToString(dimType);
    FullTag(tag, indent, str_tmp, sb);
}

void MetadataInfo::TransferTypeTag(string tag, int indent, transferTypes tfType, string* sb) {
    string str_tmp = transferTypeToString(tfType);
    FullTag(tag, indent, str_tmp, sb);
}

void MetadataInfo::TimeIntervalTypeTag(string tag, int indent, intervalTypes tiType, string* sb) {
    string str_tmp = intervalTypeToString(tiType);
    FullTag(tag, indent, str_tmp, sb);
}

void MetadataInfo::WriteInputs(int indent, string* sb) {
    if (m_vInputs.empty()) { return; }
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
        TimeIntervalTypeTag(TagVariableTimescale, indent + 2, it.timeType, sb);
        CloseTag(TagInputVariable, indent + 1, sb);
    }
    CloseTag(TagInputs, indent, sb);
}

void MetadataInfo::WriteOutputs(int indent, string* sb) {
    if (m_vOutputs.empty()) { return; }
    OpenTag(TagOutputs, "", indent, sb);
    for (size_t idx = 0; idx < m_vOutputs.size(); idx++) {
        OpenTag(TagOutputVariable, "", indent + 1, sb);
        OutputVariable it = m_vOutputs[idx];
        FullTag(TagVariableName, indent + 2, it.Name, sb);
        FullTag(TagVariableUnits, indent + 2, it.Units, sb);
        FullTag(TagVariableDescription, indent + 2, it.Description, sb);
        DimensionTag(TagVariableDimension, indent + 2, it.Dimension, sb);
        TransferTypeTag(TagVariableTransfer, indent + 2, it.tfType, sb);
        TimeIntervalTypeTag(TagVariableTimescale, indent + 2, it.timeType, sb);
        CloseTag(TagOutputVariable, indent + 1, sb);
    }
    CloseTag(TagOutputs, indent, sb);
}

void MetadataInfo::WriteInOutputs(int indent, string* sb) {
    if (m_vInOutputs.empty()) { return; }
    OpenTag(TagInOutputs, "", indent, sb);
    for (size_t idx = 0; idx < m_vInOutputs.size(); idx++) {
        OpenTag(TagInOutputVariable, "", indent + 1, sb);
        InOutputVariable it = m_vInOutputs[idx];
        FullTag(TagVariableName, indent + 2, it.Name, sb);
        FullTag(TagVariableUnits, indent + 2, it.Units, sb);
        FullTag(TagVariableDescription, indent + 2, it.Description, sb);
        DimensionTag(TagVariableDimension, indent + 2, it.Dimension, sb);
        TransferTypeTag(TagVariableTransfer, indent + 2, it.tfType, sb);
        TimeIntervalTypeTag(TagVariableTimescale, indent + 2, it.timeType, sb);
        CloseTag(TagInOutputVariable, indent + 1, sb);
    }
    CloseTag(TagInOutputs, indent, sb);
}

void MetadataInfo::WriteParameters(int indent, string* sb) {
    if (m_vParameters.empty()) { return; }
    OpenTag(TagParameters, "", indent, sb);
    for (size_t idx = 0; idx < m_vParameters.size(); idx++) {
        OpenTag(TagParameter, "", indent + 1, sb);

        Parameter it = m_vParameters[idx];
        FullTag(TagVariableName, indent + 2, it.Name, sb);
        FullTag(TagVariableUnits, indent + 2, it.Units, sb);
        FullTag(TagVariableDescription, indent + 2, it.Description, sb);
        FullTag(TagVariableSource, indent + 2, it.Source, sb);
        DimensionTag(TagVariableDimension, indent + 2, it.Dimension, sb);
        TimeIntervalTypeTag(TagVariableTimescale, indent + 2, it.timeType, sb);
        CloseTag(TagParameter, indent + 1, sb);
    }
    CloseTag(TagParameters, indent, sb);
}

void MetadataInfo::WriteDependencies(int indent, string* sb) {
    if (m_vDependencies.empty()) { return; }
    OpenTag(TagDependencies, "", indent, sb);
    for (size_t idx = 0; idx < m_vDependencies.size(); idx++) {
        OpenTag(TagClass, "", indent + 1, sb);
        string it = m_vDependencies[idx];
        FullTag(TagClass, indent + 2, it, sb);
        CloseTag(TagClass, indent + 1, sb);
    }
    CloseTag(TagDependencies, indent, sb);
}

void MetadataInfo::WriteXMLHeader(string* sb) {
    sb->append(XMLHeader);
    sb->append(XMLComment);
}

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
                           dimensionTypes dimType, transferTypes tfType /* = TF_None */,
                           intervalTypes tiType /* = TI_Unlimit */) {
    InputVariable param;
    param.Name = name;
    param.Units = units;
    param.Description = desc;
    param.Source = source;
    param.Dimension = dimType;
    param.tfType = tfType;
    param.timeType = tiType;

    m_vInputs.emplace_back(param);
    return CVT_INT(m_vInputs.size());
}

int MetadataInfo::AddOutput(const char* name, const char* units, const char* desc,
                            dimensionTypes dimType, transferTypes tfType /* = TF_None */,
                            intervalTypes tiType /* = TI_Unlimit */) {
    OutputVariable param;
    param.Name = name;
    param.Units = units;
    param.Description = desc;
    param.Dimension = dimType;
    param.tfType = tfType;
    param.timeType = tiType;

    m_vOutputs.emplace_back(param);
    return CVT_INT(m_vOutputs.size());
}

int MetadataInfo::AddInOutput(const char* name, const char* units, const char* desc,
                              dimensionTypes dimType, transferTypes tfType /* = TF_None */,
                              intervalTypes tiType /* = TI_Unlimit */) {
    InOutputVariable param;
    param.Name = name;
    param.Units = units;
    param.Description = desc;
    param.Source = Source_Module;
    param.Dimension = dimType;
    param.tfType = tfType;
    param.timeType = tiType;

    m_vInOutputs.emplace_back(param);

    AddOutput(name, units, desc, dimType, tfType, tiType);

    return CVT_INT(m_vInOutputs.size());
}

int MetadataInfo::AddParameter(const char* name, const char* units, const char* desc, const char* source,
                               dimensionTypes dimType, intervalTypes tiType /* = TI_Unlimit */) {
    Parameter param;
    param.Name = name;
    param.Units = units;
    param.Description = desc;
    param.Source = source;
    param.Dimension = dimType;
    param.timeType = tiType;

    m_vParameters.emplace_back(param);
    return CVT_INT(m_vParameters.size());
}

int MetadataInfo::AddDependency(const char* id) {
    m_vDependencies.emplace_back(id);
    return CVT_INT(m_vDependencies.size());
}

int MetadataInfo::AddDependency(const char* id, const char* description) {
    return AddDependency(id);
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
