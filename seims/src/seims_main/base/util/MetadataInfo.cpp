/*!
 * \brief methods for MetadataInfo class
 *
 * \author Junzhi Liu
 * \version 1.1
 * \date Jun. 2010
 *
 * 
 */
#include "MetadataInfo.h"
#include "text.h"

// private methods
void MetadataInfo::OpenTag(string name, string attributes, int indent, string *sb)
{
    int sz = (int)(sb->size() + (name.size() + attributes.size() + (indent * 3) + 3));
    if (sz > (int) sb->capacity())
    {
        sb->reserve(sz);
    }
    for (int i = 0; i < indent; i++) sb->append("   ");
    sb->append("<");
    sb->append(name);
    if (attributes.size() > 0)
    {
        sb->append(" ");
        sb->append(attributes);
    }
    sb->append(">");
}

void MetadataInfo::CloseTag(string name, int indent, string *sb)
{
    int sz = (int)(sb->size() + (name.size() + (indent * 3) + 2));
    if (sz > (int) sb->capacity())
    {
        sb->reserve(sz);
    }
    for (int i = 0; i < indent; i++) sb->append("   ");
    sb->append("</");
    sb->append(name);
    sb->append(">");
}

void MetadataInfo::FullTag(string name, int indent, string content, string *sb)
{
    int sz = (int)(sb->size() + (name.size() + content.size() + (indent * 3) + 5));
    if (sz > (int) sb->capacity())
    {
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

void MetadataInfo::WriteClass(int indent, string *sb)
{
    OpenTag(TagClass, "", indent, sb);

    FullTag(TagClassName, indent + 1, m_oClass.Name, sb);
    FullTag(TagClassDescription, indent + 1, m_oClass.Description, sb);

    CloseTag(TagClass, indent, sb);
}

void MetadataInfo::WriteInformation(int indent, string *sb)
{
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

void MetadataInfo::DimensionTag(string tag, int indent, dimensionTypes dimType, string *sb)
{
    string strTmp = "";

    switch (dimType)
    {
        case DT_Array1D:
            strTmp = Type_Array1D;
            break;
        case DT_Array1DDateValue:
            strTmp = Type_Array1DDateValue;
            break;
        case DT_Array2D:
            strTmp = Type_Array2D;
            break;
        case DT_Array3D:
            strTmp = Type_Array3D;
            break;
        case DT_Raster1D:
            strTmp = Type_Raster1D;
            break;
        case DT_Raster2D:
            strTmp = Type_Raster2D;
            break;
        case DT_Single:
            strTmp = Type_Single;
            break;
        case DT_Scenario:
            strTmp = Type_Scenario;
            break;
        case DT_Reach:
            strTmp = Type_Reach;
            break;
		case DT_Subbasin:
			strTmp = Type_Subbasin;
			break;
        //case DT_SiteInformation:
        //    strTmp = Type_SiteInformation;
        //    break;
        //case DT_LapseRateArray:
        //    strTmp = Type_LapseRateArray;
        //    break;
        //case DT_LookupTable:
        //    strTmp = Type_LookupTable;
        //    break;
        default:
            break;
    }

    FullTag(tag, indent, strTmp, sb);
}

void MetadataInfo::WriteInputs(int indent, string *sb)
{
    if (m_vInputs.size() > 0)
    {
        OpenTag(TagInputs, "", indent, sb);
        for (size_t idx = 0; idx < m_vInputs.size(); idx++)
        {
            OpenTag(TagInputVariable, "", indent + 1, sb);
            InputVariable it = m_vInputs[idx];
            FullTag(TagInputVariableName, indent + 2, it.Name, sb);
            FullTag(TagInputVariableUnits, indent + 2, it.Units, sb);
            FullTag(TagInputVariableDescription, indent + 2, it.Description, sb);
            FullTag(TagInputVariableSource, indent + 2, it.Source, sb);
            DimensionTag(TagInputVariableDimension, indent + 2, it.Dimension, sb);

            CloseTag(TagInputVariable, indent + 1, sb);
        }
        CloseTag(TagInputs, indent, sb);
    }

}

void MetadataInfo::WriteOutputs(int indent, string *sb)
{
    if (m_vOutputs.size() > 0)
    {
        OpenTag(TagOutputs, "", indent, sb);
        for (size_t idx = 0; idx < m_vOutputs.size(); idx++)
        {
            OpenTag(TagOutputVariable, "", indent + 1, sb);
            OutputVariable it = m_vOutputs[idx];
            FullTag(TagOutputVariableName, indent + 2, it.Name, sb);
            FullTag(TagOutputVariableUnits, indent + 2, it.Units, sb);
            FullTag(TagOutputVariableDescription, indent + 2, it.Description, sb);
            DimensionTag(TagOutputVariableDimension, indent + 2, it.Dimension, sb);
            CloseTag(TagOutputVariable, indent + 1, sb);
        }
        CloseTag(TagOutputs, indent, sb);
    }
}

void MetadataInfo::WriteParameters(int indent, string *sb)
{
    if (m_vParameters.size() > 0)
    {
        OpenTag(TagParameters, "", indent, sb);
        for (size_t idx = 0; idx < m_vParameters.size(); idx++)
        {
            OpenTag(TagParameter, "", indent + 1, sb);

            Parameter it = m_vParameters[idx];
            FullTag(TagParameterName, indent + 2, it.Name, sb);
            FullTag(TagParameterUnits, indent + 2, it.Units, sb);
            FullTag(TagParameterDescription, indent + 2, it.Description, sb);
            FullTag(TagParameterSource, indent + 2, it.Source, sb);
            DimensionTag(TagParameterDimension, indent + 2, it.Dimension, sb);

            CloseTag(TagParameter, indent + 1, sb);
        }
        CloseTag(TagParameters, indent, sb);
    }
}

void MetadataInfo::WriteDependencies(int indent, string *sb)
{
    if (m_vDependencies.size() > 0)
    {
        OpenTag(TagDependencies, "", indent, sb);
        for (size_t idx = 0; idx < m_vDependencies.size(); idx++)
        {
            OpenTag(TagClass, "", indent + 1, sb);
            ModelClass it = m_vDependencies[idx];
            FullTag(TagClassName, indent + 2, it.Name, sb);
            FullTag(TagClassDescription, indent + 2, it.Description, sb);

            CloseTag(TagClass, indent + 1, sb);
        }
        CloseTag(TagDependencies, indent, sb);
    }
}

void MetadataInfo::WriteXMLHeader(string *sb)
{
    sb->append(XMLHeader);
    sb->append(XMLComment);
}

// public methods
MetadataInfo::MetadataInfo()
{
    m_strSchemaVersion = "0.4";
}

MetadataInfo::~MetadataInfo()
{
    m_vInputs.clear();
    m_vOutputs.clear();
    m_vParameters.clear();
    m_vDependencies.clear();
}

string MetadataInfo::SchemaVersion()
{
    return m_strSchemaVersion;
}

void MetadataInfo::SetClass(string name, string description)
{
    m_oClass.Name = name;
    m_oClass.Description = description;
}

string MetadataInfo::GetClassName()
{
    return m_oClass.Name;
}

string MetadataInfo::GetClassDescription()
{
    return m_oClass.Description;
}

void MetadataInfo::SetID(string ID)
{
    m_Info.Id = ID;
}

string MetadataInfo::GetID()
{
    return m_Info.Id;
}

void MetadataInfo::SetName(string name)
{
    m_Info.Name = name;
}

string MetadataInfo::GetName()
{
    return m_Info.Name;
}

void MetadataInfo::SetDescription(string description)
{
    m_Info.Description = description;
}

string MetadataInfo::GetDescription()
{
    return m_Info.Description;
}

void MetadataInfo::SetVersion(string version)
{
    m_Info.Version = version;
}

string MetadataInfo::GetVersion()
{
    return m_Info.Version;
}

void MetadataInfo::SetAuthor(string author)
{
    m_Info.Author = author;
}

string MetadataInfo::GetAuthor()
{
    return m_Info.Author;
}

void MetadataInfo::SetEmail(string email)
{
    m_Info.EMail = email;
}

string MetadataInfo::GetEmail()
{
    return m_Info.EMail;
}

void MetadataInfo::SetWebsite(string site)
{
    m_Info.Website = site;
}

string MetadataInfo::GetWebsite()
{
    return m_Info.Website;
}

void MetadataInfo::SetHelpfile(string file)
{
    m_Info.Helpfile = file;
}

string MetadataInfo::GetHelpfile()
{
    return m_Info.Helpfile;
}

int MetadataInfo::GetInputCount()
{
    return m_vInputs.size();
}

int MetadataInfo::AddInput(string name, string units, string desc, string source, dimensionTypes dimType)
{
    InputVariable param;
    param.Name = name;
    param.Units = units;
    param.Description = desc;
    param.Source = source;
    param.Dimension = dimType;

    m_vInputs.push_back(param);
    return m_vInputs.size();
}

string MetadataInfo::GetInputName(int index)
{
    string res;
    InputVariable it;

    res = "";        // default return value is empty string
    if (index >= 0 && index < (int) m_vInputs.size())
    {
        it = m_vInputs[index];
        res = it.Name;
    }

    return res;
}

string MetadataInfo::GetInputUnits(int index)
{
    string res;
    InputVariable it;

    res = "";        // default return value is empty string
    if (index >= 0 && index < (int) m_vInputs.size())
    {
        it = m_vInputs[index];
        res = it.Units;
    }

    return res;
}

string MetadataInfo::GetInputDescription(int index)
{
    string res;
    InputVariable it;

    res = "";        // default return value is empty string
    if (index >= 0 && index < (int) m_vInputs.size())
    {
        it = m_vInputs[index];
        res = it.Description;
    }

    return res;
}

string MetadataInfo::GetInputSource(int index)
{
    string res;
    InputVariable it;

    res = "";        // default return value is empty string
    if (index >= 0 && index < (int) m_vInputs.size())
    {
        it = m_vInputs[index];
        res = it.Source;
    }

    return res;
}

InputVariable MetadataInfo::GetInput(int index)
{
    InputVariable res;

    if (index >= 0 && index < (int) m_vInputs.size())
    {
        res = m_vInputs[index];
    }

    return res;
}

int MetadataInfo::GetOutputCount()
{
    return m_vOutputs.size();
}

int MetadataInfo::AddOutput(string name, string units, string desc, dimensionTypes dimType)
{
    OutputVariable param;
    param.Name = name;
    param.Units = units;
    param.Description = desc;
    param.Dimension = dimType;

    m_vOutputs.push_back(param);
    return m_vOutputs.size();
}

string MetadataInfo::GetOutputName(int index)
{
    string res;
    OutputVariable it;

    res = "";        // default return value is empty string
    if (index >= 0 && index < (int) m_vOutputs.size())
    {
        it = m_vOutputs[index];
        res = it.Name;
    }

    return res;
}

string MetadataInfo::GetOutputUnits(int index)
{
    string res;
    OutputVariable it;

    res = "";        // default return value is empty string
    if (index >= 0 && index < (int) m_vOutputs.size())
    {
        it = m_vOutputs[index];
        res = it.Units;
    }

    return res;
}

string MetadataInfo::GetOutputDescription(int index)
{
    string res;
    OutputVariable it;

    res = "";        // default return value is empty string
    if (index >= 0 && index < (int) m_vOutputs.size())
    {
        it = m_vOutputs[index];
        res = it.Description;
    }

    return res;
}

OutputVariable MetadataInfo::GetOutput(int index)
{
    OutputVariable res;

    if (index >= 0 && index < (int) m_vOutputs.size())
    {
        res = m_vOutputs[index];
    }

    return res;
}

int MetadataInfo::GetParameterCount()
{
    return m_vParameters.size();
}

int MetadataInfo::AddParameter(string name, string units, string desc, string source, dimensionTypes dimType)
{
    Parameter param;
    param.Name = name;
    param.Units = units;
    param.Description = desc;
    param.Source = source;
    param.Dimension = dimType;

    m_vParameters.push_back(param);
    return m_vParameters.size();
}

string MetadataInfo::GetParameterName(int index)
{
    string res;
    Parameter it;

    res = "";        // default return value is empty string
    if (index >= 0 && index < (int) m_vParameters.size())
    {
        it = m_vParameters[index];
        res = it.Name;
    }

    return res;
}

string MetadataInfo::GetParameterUnits(int index)
{
    string res;
    Parameter it;

    res = "";        // default return value is empty string
    if (index >= 0 && index < (int) m_vParameters.size())
    {
        it = m_vParameters[index];
        res = it.Units;
    }

    return res;
}

string MetadataInfo::GetParameterDescription(int index)
{
    string res;
    Parameter it;

    res = "";        // default return value is empty string
    if (index >= 0 && index < (int) m_vParameters.size())
    {
        it = m_vParameters[index];
        res = it.Description;
    }

    return res;
}

string MetadataInfo::GetParameterSource(int index)
{
    string res;
    Parameter it;

    res = "";        // default return value is empty string
    if (index >= 0 && index < (int) m_vParameters.size())
    {
        it = m_vParameters[index];
        res = it.Source;
    }

    return res;
}

Parameter MetadataInfo::GetParameter(int index)
{
    Parameter res;

    if (index >= 0 && index < (int) m_vParameters.size())
    {
        res = m_vParameters[index];
    }

    return res;
}

int MetadataInfo::GetDependencyCount()
{
    return m_vDependencies.size();
}

int MetadataInfo::AddDependency(string name, string description)
{
    ModelClass cl;
    cl.Name = name;
    cl.Description = description;

    m_vDependencies.push_back(cl);
    return m_vDependencies.size();
}

string MetadataInfo::GetDependencyName(int index)
{
    string res;
    ModelClass it;

    res = "";        // default return value is empty string
    if (index >= 0 && index < (int) m_vDependencies.size())
    {
        it = m_vDependencies[index];
        res = it.Name;
    }

    return res;
}

string MetadataInfo::GetDependencyDescription(int index)
{
    string res;
    ModelClass it;

    res = "";        // default return value is empty string
    if (index >= 0 && index < (int) m_vDependencies.size())
    {
        it = m_vDependencies[index];
        res = it.Description;
    }

    return res;
}

ModelClass MetadataInfo::GetDependency(int index)
{
    ModelClass res;

    if (index >= 0 && index < (int) m_vDependencies.size())
    {
        res = m_vDependencies[index];
    }

    return res;
}

string MetadataInfo::GetXMLDocument()
{
    string sb;

    WriteXMLHeader(&sb);
    OpenTag(TagMetadata, TagMetadataAttributes, 0, &sb);
    WriteClass(1, &sb);
    WriteInformation(1, &sb);
    WriteInputs(1, &sb);
    WriteParameters(1, &sb);
    WriteOutputs(1, &sb);
    WriteDependencies(1, &sb);
    CloseTag(TagMetadata, 0, &sb);

    return sb;
}