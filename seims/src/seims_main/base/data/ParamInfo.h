/*!
 * \file ParamInfo.h
 * \brief Class to store parameter item information
 *
 * Changelog:
 *   - 1. 2018-05-18 - lj - Code review and reformat.
 *
 * \author Junzhi Liu, Liangjun Zhu
 * \version 1.1
 */
#ifndef SEIMS_PARAMETER_INFO_H
#define SEIMS_PARAMETER_INFO_H

#include <vector>

#include "MetadataInfoConst.h"
#include "basic.h"

using namespace ccgl;
using std::vector;
using std::map;


/*!
* \ingroup data
 * \class ParamInfo
 *
 * \brief Class to store and manage parameter information from the parameter database
 */
class ParamInfo {
public:
    //! Construct an empty instance.
    ParamInfo();

    //! Copy constructor
    ParamInfo(const ParamInfo& another);

    //! Destructor
    ~ParamInfo();

    /*!
     * \brief Return the adjusted value for this parameter
     * \param[in] pre_value Default is NODATA_VALUE which will be treated
     *                      as ParamInfo.Value, otherwise adjust the given value.
     * \return adjusted float value
     */
    float GetAdjustedValue(float pre_value = NODATA_VALUE);

    //! 
    float GetAdjustedValueWithChangeFunction(float pre_value, int opYrIndex);

    //! Adjust 1D array
    void Adjust1DArray(int n, float* data);

    //! Adjust 1D Raster, \sa Adjust1DArray()
    void Adjust1DRaster(int n, float* data);

    //! Adjust 1D Raster on selected area
    int Adjust1DRaster(int n, float* data, const float* units, const vector<int>& selunits,
                       const float* lu, const vector<int>& sellu);

    //! Adjust 2D array
    void Adjust2DArray(int n, float** data);

    //! Adjust 2D Raster
    void Adjust2DRaster(int n, int lyrs, float** data);

    //! Adjust 1D Raster on selected area
    int Adjust2DRaster(int n, int lyrs, float** data, float* units, const vector<int>& selunits,
                       float* lu, const vector<int>& sellu);


    //!
    //!
    int Adjust1DRasterWithOperatingYearIndexes(const int n, float* data, const float* units,
        const vector<int>& selunits, const int yearIdx, const float* lu, const vector<int>& sellu);
    int Adjust2DRasterWithOperatingYearIndexes(const int n, const int lyrs, float** data, float* units,
        const vector<int>& selunits, const int yearIdx, float* lu, const vector<int>& sellu);


    //! Name
    string Name;
    //! Units
    string Units;
    //! Description
    string Description;
    //! Used by Module Ids
    string ModuleID;
    //! Data dimension type
    dimensionTypes Dimension;
    //! Data transfer type
    transferTypes Transfer;
    //! Source, to identify which the parameters can be derived
    string Source;
    //! Value
    float Value;
    //! Impact value
    float Impact;
    //! Change type
    string Change;
    //! Absolute maximum value
    float Maximum;
    //! Absolute minimum value
    float Minimun;
    //! Dependence parameters
    ParamInfo* DependPara;
    //! Climate type
    string ClimateType;
    //! Is constant or not
    bool IsConstant;
    //! Is output or not
    bool IsOutput;
    //! Is output to other modules or not
    bool OutputToOthers;
    //! Basic name
    string BasicName;
    //! whether is initialized
    bool initialized;

    //!
    //! store changefunction and operating year index
    string changeFunctionType;  //LIN, LOG or EXP
    float changeFunctionPara1;
    float changeFunctionPara2;  //should be updated future for param list not only 2 parameters

};

#endif /* SEIMS_PARAMETER_INFO_H */
