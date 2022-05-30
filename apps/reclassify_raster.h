#ifndef CCGL_APP_RECLASSIFY_RASTER_H
#define CCGL_APP_RECLASSIFY_RASTER_H


#include "data_raster.hpp"

using namespace ccgl;
using namespace data_raster;

/// Full usage information
void Usage(const string& appname, const string& error_msg = std::string());

class InputArgs : Interface {
public:
    /*!
     * \brief Constructor by detail parameters.
     * \param[in] in_raster Paths of input raster (could be 1D or 2D raster)
     * \param[in] recls Reclassification map
     * \param[in] defvalues Default values for locations that not covered classification
     * \param[in] out_rasters Paths of output rasters
     * \param[in] outtype Data types of output raster
     * \param[in] thread_num thread or processor number, which must be greater or equal than 1
     */
    InputArgs(string& in_raster, vector<map<vint, double> >& recls,
              vector<double>& defvalues, vector<string>& out_rasters, vector<string>& outtype,
              int thread_num = 1);

    /*!
     * \brief Initializer of input arguments.
     * \param[in] argc Number of arguments
     * \param[in] argv \a char* Arguments
     */
    static InputArgs* Init(int argc, const char** argv);

public:
    int thread_num;                     ///< thread number for OpenMP
    string rs_path;                     ///< Path of categorized raster layer
    vector<map<vint, double> > recls;   ///< Reclassification maps
    vector<string> out_paths;           ///< Path of output raster
    vector<double> def_values;          ///< Default value
    vector<string> out_types;           ///< Data type of output raster
};

/// Read reclassify key-value map from plain text file
bool read_reclassification(const string& filename, map<vint, double>& reclass_map);

/// Main function of reclassify raster according to multiple reclassification key-value map
bool reclassify_raster(string& infile, vector<map<vint, double> >& reclsmap,
                       vector<string>& outtypes, vector<double>& defvalues,
                       vector<string>& outfiles);

#endif /* CCGL_APP_RECLASSIFY_RASTER_H */
