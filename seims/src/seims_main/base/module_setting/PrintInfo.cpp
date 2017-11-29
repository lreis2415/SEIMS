#include "PrintInfo.h"


//////////////////////////////////////////////
///////////PrintInfoItem Class////////////////
//////////////////////////////////////////////

PrintInfoItem::PrintInfoItem() : m_Counter(-1), m_nRows(-1), m_nLayers(-1),
                                     SubbasinID(-1), SubbasinIndex(-1), SiteID(-1), SiteIndex(-1),
                                     m_1DData(nullptr), m_2DData(nullptr), m_1DDataWithRowCol(nullptr),
                                     TimeSeriesDataForSubbasinCount(-1), m_AggregationType(AT_Unknown),
                                     StartTime(""), EndTime(""), m_startTime(0), m_endTime(0),
                                     Filename(""), Suffix("") {
    TimeSeriesData.clear();
    TimeSeriesDataForSubbasin.clear();
}
//Deprecated
//void PrintInfoItem::setSpecificCellRasterOutput(string projectPath,string databasePath,
//	clsRasterData* templateRasterData,string outputID)
//{
//	if(m_AggregationType == AT_SpecificCells)
//		m_specificOutput = new clsSpecificOutput(projectPath,databasePath,templateRasterData,outputID);
//}

PrintInfoItem::~PrintInfoItem() {
    StatusMessage(("Start to release PrintInfoItem for " + Filename + " ...").c_str());
    Release2DArray(m_Counter, m_1DDataWithRowCol);
    Release1DArray(m_1DData);
    Release2DArray(m_nRows, m_2DData);

    for (auto it = TimeSeriesDataForSubbasin.begin();
         it != TimeSeriesDataForSubbasin.end();) {
        if (it->second != nullptr) {
            delete[] it->second;
            it->second = nullptr;
        }
        TimeSeriesDataForSubbasin.erase(it++);
    }
    TimeSeriesDataForSubbasin.clear();

    StatusMessage(("End to release PrintInfoItem for " + Filename + " ...").c_str());
}

bool PrintInfoItem::IsDateInRange(time_t dt) {
    bool bStatus = false;
    if (dt >= m_startTime && dt <= m_endTime) {
        bStatus = true;
    }
    return bStatus;
}

void PrintInfoItem::add1DTimeSeriesResult(time_t t, int n, const float *data) {
    float *temp = new float[n];
    for (int i = 0; i < n; i++) {
        temp[i] = data[i];
    }
    TimeSeriesDataForSubbasin[t] = temp;
    TimeSeriesDataForSubbasinCount = n;
}

void PrintInfoItem::Flush(string projectPath, clsRasterData<float> *templateRaster, string header) {
    //bool outToMongoDB = false; /// added by LJ.
    //projectPath = projectPath + SEP;
    /// Get filenames existed in GridFS, i.e., "OUTPUT.files"
    //vector<string> outputExisted = GetGridFsFileNames(gfs);// No need to obtain the existing GridFS names.
    /// Filename should appended by AggregateType to avoiding the same names. By LJ, 2016-7-12
    if (!StringMatch(AggType, "")) {
        Filename = Filename + "_" + AggType;
    }
    StatusMessage(("Creating output file " + Filename + "...").c_str());
    //if (!outToMongoDB) {
    //    bson_error_t *err = NULL;
    //    /// delete the chunks
    //    mongoc_collection_t *chunk = mongoc_gridfs_get_chunks(gfs);
    //    mongoc_collection_drop(chunk, err);
    //}
    // Don't forget add appropriate suffix to Filename... ZhuLJ, 2015/6/16
    if (m_AggregationType == AT_SpecificCells) {
        /*if(m_specificOutput != NULL)
        {
            m_specificOutput->dump(projectPath + Filename + ".txt");
            StatusMessage(("Create " + projectPath + Filename + " successfully!").c_str());
        }
            */
        return;
    }
    if (!TimeSeriesData.empty() && (SiteID != -1 || SubbasinID != -1))    //time series data
    {
        ofstream fs;
        string filename = projectPath + Filename + "." + TextExtension;
        fs.open(filename.c_str(), ios::out | ios::app); /// append if more than one print item. By LJ
        if (fs.is_open()) {
            if (SubbasinID == 0) {
                fs << "Watershed: " << endl;
            } else {
                fs << "Subbasin: " << SubbasinID << endl;
            }
            // Write header
            fs << header << endl;
            map<time_t, float>::iterator it;
            for (it = TimeSeriesData.begin(); it != TimeSeriesData.end(); it++) {
                fs << ConvertToString2(&(it->first)) << " " << right << fixed << setw(15) << setfill(' ') <<
                   setprecision(8) << it->second << endl;
            }
            fs.close();
            StatusMessage(("Create " + filename + " successfully!").c_str());
        }
        return;
    }
    if (!TimeSeriesDataForSubbasin.empty() && SubbasinID != -1)    //time series data for subbasin
    {
        ofstream fs;
        string filename = projectPath + Filename + "." + TextExtension;
        fs.open(filename.c_str(), ios::out | ios::app);
        if (fs.is_open()) {
            fs << endl;
            if (SubbasinID == 0) {
                fs << "Watershed: " << endl;
            } else {
                fs << "Subbasin: " << SubbasinID << endl;
            }
            fs << header << endl;
            map<time_t, float *>::iterator it;
            for (it = TimeSeriesDataForSubbasin.begin(); it != TimeSeriesDataForSubbasin.end(); it++) {
                fs << ConvertToString2(&(it->first));
                for (int i = 0; i < TimeSeriesDataForSubbasinCount; i++) {
                    fs << " " << right << fixed << setw(15) << setfill(' ') << setprecision(8) << (it->second)[i];
                }
                fs << endl;
            }
            fs.close();
            StatusMessage(("Create " + filename + " successfully!").c_str());
        }
        return;
    }
    if (nullptr != m_1DData && m_nRows > -1)    // ASC or GeoTIFF file
    {
        if (templateRaster == nullptr) {
            throw ModelException("PrintInfoItem", "Flush", "The templateRaster is NULL.");
        }
        //cout << projectPath << Filename << endl;
        //if (outToMongoDB) {
        //    MongoGridFS().removeFile(Filename, gfs);
        //    clsRasterData<float>(templateRaster, m_1DData).outputToMongoDB(Filename, gfs);
        //}
        clsRasterData<float>(templateRaster, m_1DData).outputToFile(projectPath + Filename + "." + Suffix);
        return;
    }

    if (nullptr != m_2DData && m_nRows > -1 && m_nLayers > 0) /// Multi-Layers raster data
    {
        if (templateRaster == nullptr) {
            throw ModelException("PrintInfoItem", "Flush", "The templateRaster is NULL.");
        }
        clsRasterData<float>(templateRaster, m_2DData, m_nLayers).outputToFile(projectPath + Filename + "." + Suffix);

        //if (outToMongoDB) {
        //    MongoGridFS().removeFile(Filename, gfs);
        //    clsRasterData<float>(templateRaster, m_2DData, m_nLayers).outputToMongoDB(Filename, gfs);
        //}
        return;
    }

    if (!TimeSeriesData.empty())    /// time series data
    {
        ofstream fs;
        string filename = projectPath + Filename + "." + TextExtension;
        fs.open(filename.c_str(), ios::out | ios::app);
        if (fs.is_open()) {
            map<time_t, float>::iterator it;
            for (it = TimeSeriesData.begin(); it != TimeSeriesData.end(); it++) {
                fs << ConvertToString2(&(it->first)) << " " << right << fixed << setw(15) << setfill(' ') <<
                   setprecision(8) << it->second << endl;
            }
            fs.close();
            StatusMessage(("Create " + filename + " successfully!").c_str());
        }
        return;
    }
    //Don't throw exception, just print the warning message. by lj 08/6/17
    //throw ModelException("PrintInfoItem", "Flush", "Creating " + Filename +
    //    " is failed. There is no result data for this file. Please check output variables of modules.");
    cout << "PrintInfoItem\n Flush\n Creating " << Filename <<
         " is failed. There is no result data for this file. Please check output variables of modules." << endl;
}

void PrintInfoItem::AggregateData2D(time_t time, int nRows, int nCols, float **data) {
    if (m_AggregationType == AT_SpecificCells) {
/*		if(m_specificOutput != NULL)
		{
			m_specificOutput->setData(time,data);
		}	*/
    } else {
        // check to see if there is an aggregate array to add data to
        if (m_2DData == nullptr) {
            // create the aggregate array
            m_nRows = nRows;
            m_nLayers = nCols;
            Initialize2DArray(m_nRows, m_nLayers, m_2DData, NODATA_VALUE);
            m_Counter = 0;
        }

        switch (m_AggregationType) {
            case AT_Average:
#pragma omp parallel for
                for (int i = 0; i < m_nRows; i++) {
                    for (int j = 0; j < m_nLayers; j++) {
                        if (!FloatEqual(data[i][j], NODATA_VALUE)) {
                            if (FloatEqual(m_2DData[i][j], NODATA_VALUE)) {
                                m_2DData[i][j] = 0.f;
                            }
                            m_2DData[i][j] = (m_2DData[i][j] * m_Counter + data[i][j]) / (m_Counter + 1.f);
                        }
                    }
                }
                break;
            case AT_Sum:
#pragma omp parallel for
                for (int i = 0; i < m_nRows; i++) {
                    for (int j = 0; j < m_nLayers; j++) {
                        if (!FloatEqual(data[i][j], NODATA_VALUE)) {
                            if (FloatEqual(m_2DData[i][j], NODATA_VALUE)) {
                                m_2DData[i][j] = 0.f;
                            }
                            m_2DData[i][j] += data[i][j];
                        }
                    }
                }
                break;
            case AT_Minimum:
#pragma omp parallel for
                for (int i = 0; i < m_nRows; i++) {
                    for (int j = 0; j < m_nLayers; j++) {
                        if (!FloatEqual(data[i][j], NODATA_VALUE)) {
                            if (FloatEqual(m_2DData[i][j], NODATA_VALUE)) {
                                m_2DData[i][j] = MAXIMUMFLOAT;
                            }
                            if (data[i][j] <= m_2DData[i][j]) {
                                m_2DData[i][j] = data[i][j];
                            }
                        }
                    }
                }
                break;
            case AT_Maximum:
#pragma omp parallel for
                for (int i = 0; i < m_nRows; i++) {
                    for (int j = 0; j < m_nLayers; j++) {
                        if (!FloatEqual(data[i][j], NODATA_VALUE)) {
                            if (FloatEqual(m_2DData[i][j], NODATA_VALUE)) {
                                m_2DData[i][j] = MISSINGFLOAT;
                            }
                            if (data[i][j] >= m_2DData[i][j]) {
                                m_2DData[i][j] = data[i][j];
                            }
                        }
                    }
                }
                break;
            default:break;
        }
        m_Counter++;
    }
}

void PrintInfoItem::AggregateData(time_t time, int numrows, float *data) {
    if (m_AggregationType == AT_SpecificCells) {
        /*if(m_specificOutput != NULL)
        {
            m_specificOutput->setData(time,data);
        }		*/
    } else {
        // check to see if there is an aggregate array to add data to
        if (m_1DData == nullptr) {
            // create the aggregate array
            m_nRows = numrows;
            Initialize1DArray(m_nRows, m_1DData, NODATA_VALUE);
            m_Counter = 0;
        }

        // depending on the type of aggregation
#pragma omp parallel for
        for (int rw = 0; rw < m_nRows; rw++) {
            switch (m_AggregationType) {
                case AT_Average:
                    if (!FloatEqual(data[rw], NODATA_VALUE)) {
                        if (FloatEqual(m_1DData[rw], NODATA_VALUE)) {
                            m_1DData[rw] = 0.f;
                        }
                        m_1DData[rw] = (m_1DData[rw] * m_Counter + data[rw]) / (m_Counter + 1.f);
                    }
                    break;
                case AT_Sum:
                    if (!FloatEqual(data[rw], NODATA_VALUE)) {
                        if (FloatEqual(m_1DData[rw], NODATA_VALUE)) {
                            m_1DData[rw] = 0.f;
                        }
                        m_1DData[rw] += data[rw];
                    }
                    break;
                case AT_Minimum:
                    if (!FloatEqual(data[rw], NODATA_VALUE)) {
                        if (FloatEqual(m_1DData[rw], NODATA_VALUE)) {
                            m_1DData[rw] = MAXIMUMFLOAT;
                        }
                        if (m_1DData[rw] >= data[rw]) {
                            m_1DData[rw] = data[rw];
                        }
                    }
                    break;
                case AT_Maximum:
                    if (!FloatEqual(data[rw], NODATA_VALUE)) {
                        if (FloatEqual(m_1DData[rw], NODATA_VALUE)) {
                            m_1DData[rw] = MISSINGFLOAT;
                        }
                        if (m_1DData[rw] <= data[rw]) {
                            m_1DData[rw] = data[rw];
                        }
                    }
                    break;
                default:break;
            }
        }
        m_Counter++;
    }
}

void PrintInfoItem::AggregateData(int numrows, float **data, AggregationType type, float NoDataValue) {
    // check to see if there is an aggregate array to add data to
    if (m_1DDataWithRowCol == nullptr) {
        // create the aggregate array
        m_nRows = numrows;
        m_1DDataWithRowCol = new float *[m_nRows];
        for (int i = 0; i < m_nRows; i++) {
            m_1DDataWithRowCol[i] = new float[3];
            m_1DDataWithRowCol[i][0] = NoDataValue;
            m_1DDataWithRowCol[i][1] = NoDataValue;
            m_1DDataWithRowCol[i][2] = NoDataValue;
        }
        m_Counter = 0;
    }

    // depending on the type of aggregation
    switch (type) {
        case AT_Average:
#pragma omp parallel for
            for (int rw = 0; rw < m_nRows; rw++) {
                if (!FloatEqual(data[rw][2], NoDataValue)) {
                    // initialize value to 0.0 if this is the first time
                    if (FloatEqual(m_1DDataWithRowCol[rw][2], NoDataValue)) m_1DDataWithRowCol[rw][2] = 0.0f;
                    m_1DDataWithRowCol[rw][0] = data[rw][0]; // store the row number
                    m_1DDataWithRowCol[rw][1] = data[rw][1]; // store the column number
                    if (m_Counter == 0) {
                        // first value so average = value
                        m_1DDataWithRowCol[rw][2] = data[rw][2];    // store the value
                    } else {
                        // calculate the incremental average
                        m_1DDataWithRowCol[rw][2] =
                            ((m_1DDataWithRowCol[rw][2] * m_Counter) + data[rw][2]) / (m_Counter + 1);
                    }
                }
            }
            m_Counter++;
            break;
        case AT_Sum:
#pragma omp parallel for
            for (int rw = 0; rw < m_nRows; rw++) {
                if (!FloatEqual(data[rw][2], NoDataValue)) {
                    // initialize value to 0.0 if this is the first time
                    if (FloatEqual(m_1DDataWithRowCol[rw][2], NoDataValue)) m_1DDataWithRowCol[rw][2] = 0.0f;
                    m_1DDataWithRowCol[rw][0] = data[rw][0];
                    m_1DDataWithRowCol[rw][1] = data[rw][1];
                    // add the next value to the current sum
                    m_1DDataWithRowCol[rw][2] += data[rw][2];
                }
            }
            break;
        case AT_Minimum:
#pragma omp parallel for
            for (int rw = 0; rw < m_nRows; rw++) {
                if (!FloatEqual(data[rw][2], NoDataValue)) {
                    // initialize value to 0.0 if this is the first time
                    if (FloatEqual(m_1DDataWithRowCol[rw][2], NoDataValue)) m_1DDataWithRowCol[rw][2] = 0.0f;
                    m_1DDataWithRowCol[rw][0] = data[rw][0];
                    m_1DDataWithRowCol[rw][1] = data[rw][1];
                    // if the next value is smaller than the current value
                    if (data[rw][2] <= m_1DDataWithRowCol[rw][2]) {
                        // set the current value to the next (smaller) value
                        m_1DDataWithRowCol[rw][2] = data[rw][2];
                    }
                }
            }
            break;
        case AT_Maximum:
#pragma omp parallel for
            for (int rw = 0; rw < m_nRows; rw++) {
                if (!FloatEqual(data[rw][2], NoDataValue)) {
                    // initialize value to 0.0 if this is the first time
                    if (FloatEqual(m_1DDataWithRowCol[rw][2], NoDataValue)) m_1DDataWithRowCol[rw][2] = 0.0f;
                    m_1DDataWithRowCol[rw][0] = data[rw][0];
                    m_1DDataWithRowCol[rw][1] = data[rw][1];
                    // if the next value is larger than the current value
                    if (data[rw][2] >= m_1DDataWithRowCol[rw][2]) {
                        // set the current value to the next (larger) value
                        m_1DDataWithRowCol[rw][2] = data[rw][2];
                    }
                }
            }
            break;
        default:break;
    }
}

AggregationType PrintInfoItem::MatchAggregationType(string type) {
    AggregationType res = AT_Unknown;
    if (StringMatch(type, Tag_Unknown)) {
        res = AT_Unknown;
    }
    if (StringMatch(type, Tag_Sum)) {
        res = AT_Sum;
    }
    if (StringMatch(type, Tag_Average) || StringMatch(type, Tag_Average2) || StringMatch(type, Tag_Average3)) {
        res = AT_Average;
    }
    if (StringMatch(type, Tag_Minimum)) {
        res = AT_Minimum;
    }
    if (StringMatch(type, Tag_Maximum)) {
        res = AT_Maximum;
    }
    if (StringMatch(type, Tag_SpecificCells)) {
        res = AT_SpecificCells;
    }
    return res;
}


//////////////////////////////////////////
///////////PrintInfo Class////////////////
//////////////////////////////////////////

//void PrintInfo::setSpecificCellRasterOutput(string projectPath, string databasePath,
//clsRasterData* templateRasterData)
//{
//	vector<PrintInfoItem*>::iterator it;
//	for(it= m_PrintItems.begin();it<m_PrintItems.end();it++)
//	{
//		(*it)->setSpecificCellRasterOutput(projectPath,databasePath,templateRasterData,m_OutputID);
//	}
//}


PrintInfo::PrintInfo()
    : m_Interval(0), m_IntervalUnits(""), m_OutputID(""), m_param(nullptr),
      m_moduleIndex(-1), m_subbasinSelectedArray(nullptr) {
    m_PrintItems.clear();
}

PrintInfo::~PrintInfo() {
    for (auto it = m_PrintItems.begin(); it != m_PrintItems.end();) {
        if (nullptr!=*it) {
            delete *it;
            *it = nullptr;
        }
        it = m_PrintItems.erase(it);
    }
    m_PrintItems.clear();
    vector<PrintInfoItem *>().swap(m_PrintItems);
    m_param = nullptr;  /// since m_param has not been malloc by new, just set it to nullptr
    if (nullptr!=m_subbasinSelectedArray) {
        Release1DArray(m_subbasinSelectedArray);
    }
}

string PrintInfo::getOutputTimeSeriesHeader() {
    vector<string> headers;
    if (StringMatch(m_OutputID, VAR_SNWB)) {
        headers.emplace_back("Time");
        headers.emplace_back("P");
        headers.emplace_back("P_net");
        headers.emplace_back("P_blow");
        headers.emplace_back("T");
        headers.emplace_back("Wind");
        headers.emplace_back("SR");
        headers.emplace_back("SE");
        headers.emplace_back("SM");
        headers.emplace_back("SA");
    } else if (StringMatch(m_OutputID, VAR_SOWB)) {
        headers.emplace_back("Time");
        headers.emplace_back("PCP (mm)");
        headers.emplace_back("meanTmp (deg C)");
        headers.emplace_back("soilTmp (deg C)");
        headers.emplace_back("netPcp (mm)");
        headers.emplace_back("InterceptionET (mm)");
        headers.emplace_back("DepressionET (mm)");
        headers.emplace_back("Infiltration (mm)");
        headers.emplace_back("Total_ET (mm)");
        headers.emplace_back("Soil_ET (mm)");
        headers.emplace_back("NetPercolation (mm)");
        headers.emplace_back("Revaporization (mm)");
        headers.emplace_back("SurfaceRunoff (mm)");
        headers.emplace_back("SubsurfaceRunoff (mm)");
        headers.emplace_back("Baseflow (mm)");
        headers.emplace_back("AllRunoff (mm)");
        headers.emplace_back("SoilMoisture (mm)");
        //headers.emplace_back("MoistureDepth");
    } else if (StringMatch(m_OutputID, VAR_GWWB)) {
        headers.emplace_back("Time");
        headers.emplace_back("Percolation (mm)");
        headers.emplace_back("Revaporization (mm)");
        headers.emplace_back("Deep Percolation (mm)");
        headers.emplace_back("Baseflow (mm)");
        headers.emplace_back("Groundwater storage (mm)");
        headers.emplace_back("Baseflow discharge (m3/s)");
    } else if (StringMatch(m_OutputID, "T_RECH")) {
        headers.emplace_back("Time");
        headers.emplace_back("QS");
        headers.emplace_back("QI");
        headers.emplace_back("QG");
        headers.emplace_back("Q(m^3/s)");
        headers.emplace_back("Q(mm)");
    } else if (StringMatch(m_OutputID, "T_WABA")) {
        headers.emplace_back("Time");
        headers.emplace_back("Vin");
        headers.emplace_back("Vside");
        headers.emplace_back("Vdiv");
        headers.emplace_back("Vpoint");
        headers.emplace_back("Vseep");
        headers.emplace_back("Vnb");
        headers.emplace_back("Ech");
        headers.emplace_back("Lbank");
        headers.emplace_back("Vbank");
        headers.emplace_back("Vout");
        headers.emplace_back("Vch");
        headers.emplace_back("Depth");
        headers.emplace_back("Velocity");
    } else if (StringMatch(m_OutputID, "T_RSWB")) {
        headers.emplace_back("Time");
        headers.emplace_back("Qin(m^3/s)");
        headers.emplace_back("Area(ha)");
        headers.emplace_back("Storage(10^4*m^3)");
        headers.emplace_back("QSout(m^3/s)");
        headers.emplace_back("QIout(m^3/s)");
        headers.emplace_back("QGout(m^3/s)");
        headers.emplace_back("Qout(m^3/s)");
        headers.emplace_back("Qout(mm)");
    } else if (StringMatch(m_OutputID, "T_CHSB")) {
        headers.emplace_back("Time");
        headers.emplace_back("SedInUpStream");
        headers.emplace_back("SedInSubbasin");
        headers.emplace_back("SedDeposition");
        headers.emplace_back("SedDegradation");
        headers.emplace_back("SedStorage");
        headers.emplace_back("SedOut");
    } else if (StringMatch(m_OutputID, "T_RESB")) {
        headers.emplace_back("Time");
        headers.emplace_back("ResSedIn");
        headers.emplace_back("ResSedSettling");
        headers.emplace_back("ResSedStorage");
        headers.emplace_back("ResSedOut");
    }
    ostringstream oss;
    vector<string>::iterator it;
    for (it = headers.begin(); it < headers.end(); it++) {
        if (StringMatch(*it, "Time")) {
            oss << *it;
        } else {
            oss << " " << setw(15) << right << setfill(' ') << *it;
        }
    }
    return oss.str();
}

void PrintInfo::AddPrintItem(string& start, string& end, string& file, string& sufi) {
    // create a new object instance
    PrintInfoItem *itm = new PrintInfoItem();

    // set its properties
    itm->SiteID = -1;
    itm->StartTime = start;
    itm->EndTime = end;
    itm->Filename = file;
    itm->Suffix = sufi;
    /// Be default, date time format has hour info.
    itm->m_startTime = ConvertToTime2(start, "%d-%d-%d %d:%d:%d", true);
    itm->m_endTime = ConvertToTime2(end, "%d-%d-%d %d:%d:%d", true);
    // add it to the list
    m_PrintItems.emplace_back(itm);
}

void PrintInfo::AddPrintItem(string& type, string& start, string& end, string& file, string& sufi) {
    // create a new object instance
    PrintInfoItem *itm = new PrintInfoItem();

    // set its properties
    itm->SiteID = -1;
    itm->StartTime = start;
    itm->EndTime = end;
    itm->Filename = file;
    itm->Suffix = sufi;

    itm->m_startTime = ConvertToTime2(start, "%d-%d-%d %d:%d:%d", true);
    itm->m_endTime = ConvertToTime2(end, "%d-%d-%d %d:%d:%d", true);

    type = trim(type);
    itm->AggType = type;
    AggregationType enumType = PrintInfoItem::MatchAggregationType(type);
    if (enumType == AT_Unknown) {
        throw ModelException("PrintInfo", "AddPrintItem", "The type of output " + m_OutputID +
            " can't be unknown. Please check file.out. The type should be MIN, MAX, SUM or AVERAGE (AVE or MEAN).");
    }

    itm->setAggregationType(enumType);

    // add it to the list
    m_PrintItems.emplace_back(itm);
}

void
PrintInfo::AddPrintItem(string& start, string& end, string& file, string sitename, string& sufi, bool isSubbasin) {
    PrintInfoItem *itm = new PrintInfoItem();

    if (!isSubbasin) { itm->SiteID = atoi(sitename.c_str()); }
    else {
        itm->SubbasinID = atoi(sitename.c_str());
        itm->SubbasinIndex = (int) m_subbasinSeleted.size();
        m_subbasinSeleted.emplace_back(itm->SubbasinID);
    }
    itm->StartTime = start;
    itm->EndTime = end;
    itm->Filename = file;
    itm->Suffix = sufi;
    itm->m_startTime = ConvertToTime2(start, "%d-%d-%d %d:%d:%d", true);
    itm->m_endTime = ConvertToTime2(end, "%d-%d-%d %d:%d:%d", true);

    m_PrintItems.emplace_back(itm);
}

void PrintInfo::getSubbasinSelected(int *count, float **subbasins) {
    *count = (int) m_subbasinSeleted.size();
    if (m_subbasinSelectedArray == nullptr && !m_subbasinSeleted.empty()) {
        m_subbasinSelectedArray = new float[m_subbasinSeleted.size()];
        int index = 0;
        vector<int>::iterator it;
        for (it = m_subbasinSeleted.begin(); it < m_subbasinSeleted.end(); it++) {
            m_subbasinSelectedArray[index] = float(*it);
            index++;
        }
    }
    *subbasins = m_subbasinSelectedArray;
}

PrintInfoItem *PrintInfo::getPrintInfoItem(int index) {
    // default is nullptr
    PrintInfoItem *res = nullptr;

    // is the index in the valid range
    if (index >= 0 && index < (int) m_PrintItems.size()) {
        // assign the reference to the given item
        res = m_PrintItems.at(index);
    }
    // return the reference
    return res;
}
