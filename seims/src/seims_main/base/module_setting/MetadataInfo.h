/*!
 * \file MetadataInfo.h
 * \brief Define MetadataInfo class used by modules
 *
 * Changelog:
 *   - 1. 2018-3-16 - lj - Simplify code, and add In/Output parameters related for MPI version.
 *   - 2. 2023-7-27 - lj - Add intervalTypes to indicate simulation time scale
 *
 * \author Junzhi Liu, Liangjun Zhu
 * \version 1.2
 */
#ifndef SEIMS_METADATA_INFO_H
#define SEIMS_METADATA_INFO_H

#include <vector>

#include "basic.h"
#include "MetadataInfoConst.h"

using namespace ccgl;
using std::vector;

/*!
 * \brief Convert dimension type of data to string
 */
string dimensionTypeToString(dimensionTypes dt);

/*!
 * \brief Match string to dimension type of data, e.g., 1D array
 */
dimensionTypes matchDimensionType(const string& type_str);

/*!
 * \brief Convert transfer type of data to string
 */
string transferTypeToString(transferTypes tt);

/*!
 * \brief Match data transfer type, e.g., TF_SingleValue
 */
transferTypes matchTransferType(const string& tf_type);

/*!
 * \brief Convert interval type of simulation to string
 */
string intervalTypeToString(intervalTypes it);

/*!
 * \brief Match time interval type, e.g., TI_Storm
 */
intervalTypes matchIntervalType(const string& ti_type);


/*!
 * \ingroup module_setting
 * \struct ModuleClass
 * \brief Module class, e.g., climate, hydrology, erosion
 */
struct ModuleClass {
    ModuleClass() { }

    string Name;
    string Description;
};

/*!
 * \ingroup module_setting
 * \struct ModuleInfo
 *
 * \brief Module identification information
 */
struct ModuleInfo {
    ModuleInfo() { }

    string Id;          ///< Module ID, without spaces and unique in module library
    string Name;        ///< Module Name, which can be more detail than ID
    string Description; ///< Module description
    string TimeScale;   ///< Time Scale of simulation
    string Version;     ///< Module Version
    string Author;      ///< Author
    string Email;       ///< Email
    string Website;     ///< Website
    string Helpfile;    ///< Helpfile
};

/*!
 * \ingroup module_setting
 * \struct baseParameter
 *
 * \brief Basic model parameter information
 */
struct baseParameter {
    baseParameter() : Dimension(DT_Unknown), timeType(TI_Unlimit) {
    }

    string Name;              ///< Name
    string Units;             ///< Units
    string Description;       ///< Description
    dimensionTypes Dimension; ///< Data dimension type
    intervalTypes timeType;   ///< Time interval type for simulation
};

/*!
 * \ingroup module_setting
 * \struct Parameter
 *
 * \brief Model parameter information class
 */
struct Parameter: baseParameter {
    Parameter() : baseParameter() { }

    string Source; ///< Source type
};

/*!
 * \ingroup module_setting
 * \struct InputVariable
 *
 * \brief Input variable information class
 */
struct InputVariable: Parameter {
    InputVariable() : Parameter(), tfType(TF_None) {
    }

    transferTypes tfType;
};

/*!
 * \ingroup module_setting
 * \struct OutputVariable
 * \brief Output variable information class
 */
struct OutputVariable: baseParameter {
    OutputVariable() : baseParameter(), tfType(TF_None) {
    }

    transferTypes tfType;
};

/*!
 * \ingroup module_setting
 * \struct InOutputVariable
 * \brief Input and output variable information class
 */
struct InOutputVariable: InputVariable {
    InOutputVariable(): InputVariable() { }
};

/*!
 * \ingroup module_setting
 * \class MetadataInfo
 * \brief Metadata information of module
 */
class MetadataInfo: Interface {
public:
    MetadataInfo() { m_strSchemaVersion = "0.4"; }

    ~MetadataInfo();

    string SchemaVersion() { return m_strSchemaVersion; }

    void SetClass(const char* name, const char* description);

    string GetClassName() { return m_oClass.Name; }

    string GetClassDescription() { return m_oClass.Description; }

    void SetID(const char* ID) { m_Info.Id = ID; }

    string GetID() { return m_Info.Id; }

    void SetName(const char* name) { m_Info.Name = name; }

    string GetName() { return m_Info.Name; }

    void SetTimeScale(intervalTypes ts) { m_Info.TimeScale = intervalTypeToString(ts); }

    string GetTimeScale() { return m_Info.TimeScale; }

    void SetDescription(const char* description) { m_Info.Description = description; }

    string GetDescription() { return m_Info.Description; }

    void SetVersion(const char* version) { m_Info.Version = version; }

    string GetVersion() { return m_Info.Version; }

    void SetAuthor(const char* author) { m_Info.Author = author; }

    string GetAuthor() { return m_Info.Author; }

    void SetEmail(const char* email) { m_Info.Email = email; }

    string GetEmail() { return m_Info.Email; }

    void SetWebsite(const char* site) { m_Info.Website = site; }

    string GetWebsite() { return m_Info.Website; }

    void SetHelpfile(const char* file) { m_Info.Helpfile = file; }

    string GetHelpfile() { return m_Info.Helpfile; }

    /************ INPUT PARAMETERS FROM OTHER MODULES ************/

    int GetInputCount() { return CVT_INT(m_vInputs.size()); }

    int AddInput(const char* name, const char* units, const char* desc, const char* source, dimensionTypes dimType,
                 transferTypes tfType = TF_None, intervalTypes tiType = TI_Unlimit);

    string GetInputName(int index) { return index >= 0 && index < m_vInputs.size() ? m_vInputs[index].Name : ""; }

    string GetInputUnits(int index) { return index >= 0 && index < m_vInputs.size() ? m_vInputs[index].Units : ""; }

    string GetInputDescription(int index) {
        return index >= 0 && index < m_vInputs.size() ? m_vInputs[index].Description : "";
    }

    string GetInputSource(int index) {
        return index >= 0 && index < m_vInputs.size() ? m_vInputs[index].Source : "";
    }

    dimensionTypes GetInputDimension(int index) {
        return index >= 0 && index < m_vInputs.size() ? m_vInputs[index].Dimension : DT_Unknown;
    }

    transferTypes GetInputTfType(int index) {
        return index >= 0 && index < m_vInputs.size() ? m_vInputs[index].tfType : TF_None;
    }

    intervalTypes GetInputTiType(int index) {
        return index >= 0 && index < m_vInputs.size() ? m_vInputs[index].timeType : TI_Unlimit;
    }

    InputVariable GetInput(int index) {
        return index >= 0 && index < m_vInputs.size() ? m_vInputs[index] : InputVariable();
    }

    /************ OUTPUT PARAMETERS ************/

    int GetOutputCount() { return CVT_INT(m_vOutputs.size()); }

    int AddOutput(const char* name, const char* units, const char* desc, dimensionTypes dimType,
                  transferTypes tfType = TF_None, intervalTypes tiType = TI_Unlimit);

    string GetOutputName(int index) { return index >= 0 && index < m_vOutputs.size() ? m_vOutputs[index].Name : ""; }

    string GetOutputUnits(int index) {
        return index >= 0 && index < m_vOutputs.size() ? m_vOutputs[index].Units : "";
    }

    string GetOutputDescription(int index) {
        return index >= 0 && index < m_vOutputs.size() ? m_vOutputs[index].Description : "";
    }

    dimensionTypes GetOutputDimension(int index) {
        return index >= 0 && index < m_vOutputs.size() ? m_vOutputs[index].Dimension : DT_Unknown;
    }

    transferTypes GetOutputTfType(int index) {
        return index >= 0 && index < m_vOutputs.size() ? m_vOutputs[index].tfType : TF_None;
    }

    intervalTypes GetOutputTiType(int index) {
        return index >= 0 && index < m_vOutputs.size() ? m_vOutputs[index].timeType : TI_Unlimit;
    }

    OutputVariable GetOutput(int index) {
        return index >= 0 && index < m_vOutputs.size() ? m_vOutputs[index] : OutputVariable();
    }

    /************ IN/OUTPUT PARAMETERS ************/

    int GetInOutputCount() { return CVT_INT(m_vInOutputs.size()); }

    int AddInOutput(const char* name, const char* units, const char* desc, dimensionTypes dimType,
                    transferTypes tfType = TF_None, intervalTypes tiType = TI_Unlimit);

    string GetInOutputName(int index) {
        return index >= 0 && index < m_vInOutputs.size() ? m_vInOutputs[index].Name : "";
    }

    string GetInOutputUnits(int index) {
        return index >= 0 && index < m_vInOutputs.size() ? m_vInOutputs[index].Units : "";
    }

    string GetInOutputDescription(int index) {
        return index >= 0 && index < m_vInOutputs.size() ? m_vInOutputs[index].Description : "";
    }

    dimensionTypes GetInOutputDimension(int index) {
        return index >= 0 && index < m_vInOutputs.size() ? m_vInOutputs[index].Dimension : DT_Unknown;
    }

    transferTypes GetInOutputTfType(int index) {
        return index >= 0 && index < m_vInOutputs.size() ? m_vInOutputs[index].tfType : TF_None;
    }

    intervalTypes GetInOutputTiType(int index) {
        return index >= 0 && index < m_vInOutputs.size() ? m_vInOutputs[index].timeType : TI_Unlimit;
    }

    InOutputVariable GetInOutput(int index) {
        return index >= 0 && index < m_vInOutputs.size() ? m_vInOutputs[index] : InOutputVariable();
    }

    /************ PARAMETERS FROM DATABASE ************/

    int GetParameterCount() { return CVT_INT(m_vParameters.size()); }

    int AddParameter(const char* name, const char* units, const char* desc, const char* source, dimensionTypes dimType,
                     intervalTypes tiType = TI_Unlimit);

    string GetParameterName(int index) {
        return index >= 0 && index < m_vParameters.size() ? m_vParameters[index].Name : "";
    }

    string GetParameterUnits(int index) {
        return index >= 0 && index < m_vParameters.size() ? m_vParameters[index].Units : "";
    }

    string GetParameterDescription(int index) {
        return index >= 0 && index < m_vParameters.size() ? m_vParameters[index].Description : "";
    }

    string GetParameterSource(int index) {
        return index >= 0 && index < m_vParameters.size() ? m_vParameters[index].Source : "";
    }

    dimensionTypes GetParameterDimension(int index) {
        return index >= 0 && index < m_vParameters.size() ? m_vParameters[index].Dimension : DT_Unknown;
    }

    intervalTypes GetParameterTiType(int index) {
        return index >= 0 && index < m_vParameters.size() ? m_vParameters[index].timeType : TI_Unlimit;
    }

    Parameter GetParameter(int index) {
        return index >= 0 && index < m_vParameters.size() ? m_vParameters[index] : Parameter();
    }

    /************ DEPENDENT MODULES ************/

    int GetDependencyCount() { return CVT_INT(m_vDependencies.size()); }

    int AddDependency(const char* id);

    int AddDependency(const char* id, const char* description); // todo: remove in future. lj

    string GetDependencyName(int index) { // todo: remove in future. lj
        return GetDependency(index);
    }

    string GetDependency(int index) {
        return index >= 0 && index < m_vDependencies.size() ? m_vDependencies[index] : "";
    }

    string GetXMLDocument();

    static void OpenTag(string name, string attributes, int indent, string* sb);

    static void CloseTag(string name, int indent, string* sb);

    static void FullTag(const string& name, int indent, string& content, string* sb);

    void WriteClass(int indent, string* sb);

    void WriteInformation(int indent, string* sb);

    void WriteInputs(int indent, string* sb);

    void WriteOutputs(int indent, string* sb);

    void WriteInOutputs(int indent, string* sb);

    void WriteParameters(int indent, string* sb);

    void WriteDependencies(int indent, string* sb);

    static void WriteXMLHeader(string* sb);

    static void DimensionTag(string tag, int indent, dimensionTypes dimType, string* sb);

    static void TransferTypeTag(string tag, int indent, transferTypes tfType, string* sb);

    static void TimeIntervalTypeTag(string tag, int indent, intervalTypes tiType, string* sb);

private:
    string m_strSchemaVersion;             ///< latest XML schema version supported by this class
    ModuleClass m_oClass;                  ///< class name for the module
    ModuleInfo m_Info;                     ///< the general information for the module
    vector<Parameter> m_vParameters;       ///< list of parameters for the module
    vector<InputVariable> m_vInputs;       ///< list of input parameters for the module
    vector<OutputVariable> m_vOutputs;     ///< list of output parameters for the module
    vector<InOutputVariable> m_vInOutputs; ///< list of In/Output parameters for the module for MPI version
    vector<string> m_vDependencies;        ///< list of dependency classes for the module
};

#endif /* SEIMS_METADATA_INFO_H */
