/*!
 * \brief Define MetadataInfo class used by modules
 * \author Junzhi Liu, Liangjun Zhu
 * \version 1.0
 * \date June 2010
 */
#ifndef SEIMS_METADATA_INFO_H
#define SEIMS_METADATA_INFO_H

#include "utilities.h"
#include "MetadataInfoConst.h"

using namespace std;

/*!
 * \ingroup util
 * \struct ModelClass
 * \brief Module basic description
 */
struct ModelClass {
public:
    ModelClass():Name(""),Description(""){};
public:
    string Name;
    string Description;
};

/*!
 * \ingroup util
 * \struct Parameter
 *
 * \brief Model parameter information class
 */
struct Parameter {
public:
    Parameter() : Name(""), Units(""), Description(""), Source(""), Dimension(DT_Unknown) {};
public:
    string Name;                 ///< Name
    string Units;                ///< Units
    string Description;          ///< Description
    string Source;               ///< Source type
    dimensionTypes Dimension;    ///< Data dimension type
};

/*!
 * \ingroup util
 * \struct Information
 *
 * \brief Module development information class
 */
struct Information {
public:
    Information() : Id(""), Name(""), Description(""), Version(""), Author(""),
                    EMail(""), Website(""), Helpfile("") {};
public:
    string Id;             ///< Module ID
    string Name;           ///< Module Name
    string Description;    ///< Module Description
    string Version;        ///< Module Version
    string Author;         ///< Author
    string EMail;          ///< Email
    string Website;        ///< Website
    string Helpfile;       ///< Helpfile
};

/*!
 * \ingroup util
 * \struct InputVariable
 *
 * \brief Input variable information class
 */
struct InputVariable {
public:
    InputVariable() : Name(""), Units(""), Description(""), Source(""), Dimension(DT_Unknown) {};
public:
    string Name;                ///< Name
    string Units;               ///< Units
    string Description;         ///< Description
    string Source;              ///< Source
    dimensionTypes Dimension;   ///< Data dimension type
};

/*!
 * \ingroup Util
 * \struct OutputVariable
 * \brief Output variable information class
 */
struct OutputVariable {
public:
    OutputVariable() : Name(""), Units(""), Description(""), Dimension(DT_Unknown) {};
public:
    string Name;                 ///< Name
    string Units;                ///< Units
    string Description;          ///< Description
    dimensionTypes Dimension;    ///< Data dimension type
};

/*!
 * \ingroup Util
 * \class MetadataInfo
 * \brief Metadata information of module
 */
class MetadataInfo {
private:
    string m_strSchemaVersion;                ///< latest XML schema version supported by this class
    ModelClass m_oClass;                      ///< class name for the module
    Information m_Info;                       ///< the general information for the module
    vector<InputVariable> m_vInputs;         ///< list of input parameters for the module
    vector<OutputVariable> m_vOutputs;       ///<list of output parameters for the module
    vector<ModelClass> m_vDependencies;      ///< list of dependency classes for the module
    vector<Parameter> m_vParameters;         ///< list of parameters for the module

    void OpenTag(string name, string attributes, int indent, string *sb);

    void CloseTag(string name, int indent, string *sb);

    void FullTag(const string &name, int indent, string &content, string *sb);

    void WriteClass(int indent, string *sb);

    void WriteInformation(int indent, string *sb);

    void WriteInputs(int indent, string *sb);

    void WriteOutputs(int indent, string *sb);

    void WriteParameters(int indent, string *sb);

    void WriteDependencies(int indent, string *sb);

    void WriteXMLHeader(string *sb);

    void DimensionTag(string tag, int indent, dimensionTypes dimType, string *sb);

public:
    MetadataInfo();

    ~MetadataInfo();

    string SchemaVersion();

    void SetClass(const char* name, const char* description);

    string GetClassName();

    string GetClassDescription();

    void SetID(const char* ID);

    string GetID();

    void SetName(const char* name);

    string GetName();

    void SetDescription(const char* description);

    string GetDescription();

    void SetVersion(const char* version);

    string GetVersion();

    void SetAuthor(const char* author);

    string GetAuthor();

    void SetEmail(const char* email);

    string GetEmail();

    void SetWebsite(const char* site);

    string GetWebsite();

    void SetHelpfile(const char* file);

    string GetHelpfile();

    int GetInputCount();

    int AddInput(const char* name, const char* units, const char* desc, const char* source, dimensionTypes dimType);

    string GetInputName(int index);

    string GetInputUnits(int index);

    string GetInputDescription(int index);

    string GetInputSource(int index);

    // dimensionTypes GetInputDimension(int index); // Useless?

    InputVariable GetInput(int index);

    int GetOutputCount();

    int AddOutput(const char* name, const char* units, const char* desc, dimensionTypes dimType);

    string GetOutputName(int index);

    string GetOutputUnits(int index);

    string GetOutputDescription(int index);

    // dimensionTypes GetOutputDimension(int index); // Useless?

    OutputVariable GetOutput(int index);

    int GetParameterCount();

    int AddParameter(const char* name, const char* units, const char* desc, const char* source, dimensionTypes dimType);

    string GetParameterName(int index);

    string GetParameterUnits(int index);

    string GetParameterDescription(int index);

    string GetParameterSource(int index);

    // dimensionTypes GetParameterDimension(int index); // Useless?

    Parameter GetParameter(int index);

    int GetDependencyCount();

    int AddDependency(const char* name, const char* description);

    string GetDependencyName(int index);

    string GetDependencyDescription(int index);

    ModelClass GetDependency(int index);

    string GetXMLDocument();
};

#endif /* SEIMS_METADATA_INFO_H */