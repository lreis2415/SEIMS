/*!
 * \brief Class to store parameter item information
 *
 * \author Junzhi Liu
 * \version 1.0
 * \date June 2010
 *
 */
#pragma once
#include "MetadataInfoConst.h"
#include "text.h"
#include "utilities.h"

using namespace std;

/*!
* \ingroup module_setting
 * \class ParamInfo
 *
 * \brief Class to store and manage parameter information from the parameter database
 *
 *
 *
 */
class ParamInfo {
public:
    //! Construct an empty instance.
    ParamInfo(void);

    //! Destructor
    ~ParamInfo(void);

    //! Reset the contents of the object to default values
    void Reset(void);

    /*!
     * \brief Return the adjusted value for this parameter
     * \param[in] pre_value Default is NODATA_VALUE which will be treated
     *                      as ParamInfo.Value, otherwise adjust the given value.
     * \return adjusted float value
     */
    float GetAdjustedValue(float pre_value = NODATA_VALUE);

    //! Adjust 1D array
    void Adjust1DArray(int n, float *data);

    //! Adjust 1D Raster, \sa Adjust1DArray()
    void Adjust1DRaster(int n, float *data);

    //! Adjust 2D array
    void Adjust2DArray(int n, float **data);

    //! Adjust 2D Raster
    void Adjust2DRaster(int n, int lyr, float **data);

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
    //! Use or not
    string Use;
    //! Dependence parameters
    ParamInfo *DependPara;
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
};
