/*!
 * \file PrintInfo.h
 * \brief Class to store and manage the PRINT information
 * From the file.out file or FILE_OUT collection in MongoDB
 *
 * \author Junzhi Liu, LiangJun Zhu
 * \version 1.1
 * \date June 2010
 */
#ifndef SEIMS_PRINTINFO_H
#define SEIMS_PRINTINFO_H

#include "basic.h"

#include "seims.h"
#include "ParamInfo.h"

using namespace ccgl;

/*!
 * \enum AggregationType
 * \ingroup module_setting
 * \brief Aggregation type for OUTPUT
 */
enum AggregationType {
    AT_Unknown = 0,      ///< unknown
    AT_Sum = 1,          ///< sum
    AT_Average = 2,      ///< average
    AT_Minimum = 3,      ///< minimum
    AT_Maximum = 4,      ///< maximum
    AT_SpecificCells = 5 ///< specific cells
};

/*!
 * \ingroup module_setting
 * \class PrintInfoItem
 *
 * \brief Class stores a single output item of an OuputID
 *
 */
class PrintInfoItem {
public:
    //! Constructor
    PrintInfoItem(int scenario_id = 0, int calibration_id = -1);

    //! Destructor
    ~PrintInfoItem();

    //! Aggregated data, the second dimension contains: row, col, value
    float** m_1DDataWithRowCol;
    //! rows number, i.e., number of valid cells
    int m_nRows;
    //! For 1D raster/array data
    float* m_1DData;
    //! number of layers of raster data, greater or equal than 1
    int m_nLayers;
    //! For 2D raster/array data
    float** m_2DData;

    //! For time series data of a single subbasin, DT_Single
    map<time_t, float> TimeSeriesData;
    //! For time series data of a single subbasin, DT_Raster1D or DT_Array1D
    map<time_t, float *> TimeSeriesDataForSubbasin;
    //! Count of #TimeSeriesDataForSubbasin
    int TimeSeriesDataForSubbasinCount;

    //! Add 1D time series data result to #TimeSeriesDataForSubbasin
    void add1DTimeSeriesResult(time_t, int n, const float* data);

    //! used only by PET_TS???
    ///< The site id
    int SiteID;
    ///< The site index in output array1D variable
    int SiteIndex;

    ///< The subbasin id
    int SubbasinID;
    ///< The subbasin index
    int SubbasinIndex;

    ////! Start time string
    //string StartTime;
    //! Start time \a time_t
    time_t m_startTime;

    //! get start time \a time_t
    time_t getStartTime() { return m_startTime; };

    //! set start time \a time_t
    void setStartTime(time_t& st) { m_startTime = st; }

    ////! End time string
    //string EndTime;
    //! End time  \a time_t
    time_t m_endTime;

    //! Get end time  \a time_t
    time_t getEndTime() { return m_endTime; };

    //! set end time \a time_t
    void setEndTime(time_t& st) { m_endTime = st; }

    //! file suffix, e.g., txt, tif, asc, etc.
    string Suffix;
    //! output filename without suffix, core name without subbasin ID
    string Corename;
    //! output filename without suffix, and contain subbasin ID as prefix for MPI version
    string Filename;
    //! Aggregation type string
    string AggType;

    //! create "output" folder to store all results
    void Flush(const string& projectPath, MongoGridFs* gfs, FloatRaster* templateRaster, const string& header);

    //! Determine if the given date is within the date range for this item
    bool IsDateInRange(time_t dt);

    //! Aggregate the 2D data from the given data parameter using the given method type.
    //! However this **data restrict to 3 layers, i.e., Row, Col, Value
    //! NO NEED TO USE?
    void AggregateData(int numrows, float** data, AggregationType type, float NoDataValue);

    //! Aggregate the 1D data from the given data parameter using the given method type
    void AggregateData(time_t time, int numrows, float* data);

    //! Aggregate the 2D raster data from the given data parameter using the given method type
    void AggregateData2D(time_t time, int nRows, int nCols, float** data);

    //! Set the Aggregation type
    void setAggregationType(AggregationType type) { m_AggregationType = type; };

    //! Get the Aggregation type
    AggregationType getAggregationType() { return m_AggregationType; };

    //! convert the given string into a matching Aggregation type
    static AggregationType MatchAggregationType(string type);

private:
    //! Scenario ID
    int m_scenarioID;
    //! Calibration ID
    int m_calibrationID;
    //! Counter of time series data, i.e., how many data has been aggregated.
    int m_Counter;
    //! Aggregation type of current print item
    AggregationType m_AggregationType;
};

/*!
 * \ingroup module_setting
 * \class PrintInfo
 * \brief Outputs of one variable, which may contain one or more `PrintInfoItem`
 * \sa PrintInfoItem
 */
class PrintInfo {
public:
    //! Scenario ID
    int m_scenarioID;
    //! Calibration ID
    int m_calibrationID;
    //! Time interval of output
    int m_Interval;
    //! Unit of time interval, which can only be DAY, HR, SEC.
    string m_IntervalUnits;
    //! Module index of the OutputID
    int m_moduleIndex;
    //! Unique Output ID, which should be one of "VAR_" defined in text.h and Output of any modules.
    string m_OutputID;
    //! The calibration parameters corresponding to the output id, if stated.
    ParamInfo* m_param;
    //! For one OutputID, there may be several output items, e.g., different time period, different subbasin ID. etc.
    vector<PrintInfoItem *> m_PrintItems;

private:
    //! Selected subbasin IDs for time series data, vector container
    vector<int> m_subbasinSeleted;
    //! Selected subbasin IDs for time series data, float array
    float* m_subbasinSelectedArray;
public:
    //! Constructor, initialize an empty instance
    PrintInfo(int scenario_id = 0, int calibration_id = -1);

    //! Destructor
    ~PrintInfo();

    //! Get the number of output items
    int ItemCount() const { return CVT_INT(m_PrintItems.size()); };

    //! Get all the subbasin IDs (in float array) selected for this outputID
    void getSubbasinSelected(int* count, float** subbasins);

    //! Set the OutputID for this object
    void setOutputID(string id) { m_OutputID = id; };

    //! Get the OutputId for this object
    string getOutputID() const { return m_OutputID; };

    //! Get Header string (all field names) for current OutputID. TODO, how to make it more flexible? By LJ.
    string getOutputTimeSeriesHeader();

    //! Set the interval
    void setInterval(int interval) { m_Interval = interval; };

    //! Get the interval
    int getInterval() { return m_Interval; };

    //! Set the interval units
    void setIntervalUnits(string& units) { m_IntervalUnits = units; };

    //! Get the interval units
    string getIntervalUnits() const { return m_IntervalUnits; };

    //! Add an output item with the given start time, end time and file name
    void AddPrintItem(time_t start, time_t end, string& file, string& sufi);

    //! Add an output item with the given aggregate type, start time, end time, file name and subbasin ID
    void AddPrintItem(string& type, time_t start, time_t end, string& file, string& sufi, int subbasinID = 0);

    //! Add an output item with the given start time (string), end time (string) and file name, Overloaded method
    void AddPrintItem(time_t start, time_t end, string& file, string sitename, string& sufi, bool isSubbasin);

    //! Get a reference to the output item located at the given index position
    PrintInfoItem* getPrintInfoItem(int index);
};
#endif /* SEIMS_PRINTINFO_H */
