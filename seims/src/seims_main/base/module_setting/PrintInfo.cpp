/*!
 * \brief Class to store and manage the PRINT information 
 * From the file.out file or FILE_OUT collection in MongoDB
 *
 * \author Junzhi Liu, LiangJun Zhu
 * \version 1.1
 * \date June 2010
 */
#include "ModelException.h"
#include "PrintInfo.h"
#include "clsRasterData.cpp"

//#ifndef linux
////#include "WinSock2i.h"
//#include <WinSock2.h>
//#include <windows.h>
//#else
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#endif
//////////////////////////////////////////////
///////////PrintInfoItem Class//////////////// 
//////////////////////////////////////////////

PrintInfoItem::PrintInfoItem(void) : m_Counter(-1), m_nRows(-1), m_nLayers(-1), 
	SubbasinID(-1), SubbasinIndex(-1), SiteID(-1), SiteIndex(-1),
	m_1DData(NULL), m_2DData(NULL), m_1DDataWithRowCol(NULL), 
	TimeSeriesDataForSubbasinCount(-1), m_AggregationType(AT_Unknown),
	StartTime(""), EndTime(""), m_startTime(0), m_endTime(0),
	Filename(""), Suffix("") 
{
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

PrintInfoItem::~PrintInfoItem(void)
{
    StatusMessage(("Start to release PrintInfoItem for " + Filename + " ...").c_str());

    if (m_1DDataWithRowCol != NULL)
		Release2DArray(m_Counter, m_1DDataWithRowCol);
    if (m_1DData != NULL)
		Release1DArray(m_1DData);
    if (m_2DData != NULL)
		Release2DArray(m_nRows, m_2DData);
	TimeSeriesData.clear();

    for (map<time_t, float *>::iterator it = TimeSeriesDataForSubbasin.begin(); it != TimeSeriesDataForSubbasin.end();)
    {
        if (it->second != NULL)
		{
			delete[] it->second;
			it->second = NULL;
		}
		it = TimeSeriesDataForSubbasin.erase(it);
    }
	TimeSeriesDataForSubbasin.clear();

    StatusMessage(("End to release PrintInfoItem for " + Filename + " ...").c_str());
}

bool PrintInfoItem::IsDateInRange(time_t dt)
{
    bool bStatus = false;
    if (dt >= m_startTime && dt <= m_endTime)
    {
        bStatus = true;
    }
    return bStatus;
}

void PrintInfoItem::add1DTimeSeriesResult(time_t t, int n, float *data)
{
    float *temp = new float[n];
    for (int i = 0; i < n; i++)
    {
        temp[i] = data[i];
    }
    TimeSeriesDataForSubbasin[t] = temp;
    TimeSeriesDataForSubbasinCount = n;
}

void PrintInfoItem::Flush(string projectPath, clsRasterData<float> *templateRaster, string header)
{
	bool outToMongoDB = false; /// added by LJ. 
	projectPath = projectPath;
    /// Get filenames existed in GridFS, i.e., "OUTPUT.files"
    //vector<string> outputExisted = GetGridFsFileNames(gfs);// No need to obtain the existing GridFS names.
	/// Filename should appended by AggregateType to avoiding the same names. By LJ, 2016-7-12
	if(!StringMatch(AggType,""))
		Filename = Filename + "_" + AggType;
    StatusMessage(("Creating output file " + Filename + "...").c_str());
	if (!outToMongoDB)
	{
		bson_error_t *err = NULL;
		/// delete the chunks
		mongoc_collection_t *chunk = mongoc_gridfs_get_chunks(gfs);
		mongoc_collection_drop(chunk, err);
	}
    // Don't forget add appropriate suffix to Filename... ZhuLJ, 2015/6/16
    if (m_AggregationType == AT_SpecificCells)
    {
        /*if(m_specificOutput != NULL)
        {
            m_specificOutput->dump(projectPath + Filename + ".txt");
            StatusMessage(("Create " + projectPath + Filename + " successfully!").c_str());
        }
            */
        return;
    }
    if (TimeSeriesData.size() > 0 && (SiteID != -1 || SubbasinID != -1))    //time series data
    {
        ofstream fs;
        utils util;
        string filename = projectPath + Filename + TextExtension;
        fs.open(filename.c_str(), ios::out|ios::app); /// append if more than one print item. By LJ
        if (fs.is_open())
        {
			if(SubbasinID == 0)
				fs << "Watershed: "<< endl;
			else
				fs << "Subbasin: "<<SubbasinID<<endl;
			// Write header
			fs << header << endl;
            map<time_t, float>::iterator it;
            for (it = TimeSeriesData.begin(); it != TimeSeriesData.end(); it++)
            {
                fs << util.ConvertToString2(&(it->first)) << " " << right << fixed << setw(15) << setfill(' ') <<
                setprecision(8) << it->second << endl;
            }
            fs.close();
            StatusMessage(("Create " + filename + " successfully!").c_str());
        }
        return;
    }
    if (TimeSeriesDataForSubbasin.size() > 0 && SubbasinID != -1)    //time series data for subbasin
    {
        ofstream fs;
        utils util;
        string filename = projectPath + Filename + TextExtension;
        fs.open(filename.c_str(), ios::out|ios::app);
        if (fs.is_open())
        {
			fs << endl;
			if(SubbasinID == 0)
				fs << "Watershed: "<< endl;
			else
				fs << "Subbasin: "<<SubbasinID<<endl;
            fs << header << endl;
            map<time_t, float *>::iterator it;
            for (it = TimeSeriesDataForSubbasin.begin(); it != TimeSeriesDataForSubbasin.end(); it++)
            {
                fs << util.ConvertToString2(&(it->first));
                for (int i = 0; i < TimeSeriesDataForSubbasinCount; i++)
                {
                    fs << " " << right << fixed << setw(15) << setfill(' ') << setprecision(8) << (it->second)[i];
                }
                fs << endl;
            }
            fs.close();
            StatusMessage(("Create " + filename + " successfully!").c_str());
        }
        return;
    }
    if (m_1DData != NULL && m_nRows > -1)    // ASC or GeoTIFF file
    {
        if (templateRaster == NULL)
            throw ModelException("PrintInfoItem", "Flush", "The templateRaster is NULL.");
        //cout << projectPath << Filename << endl;
		if (outToMongoDB)
		{
			bson_error_t *err = NULL;
			mongoc_gridfs_remove_by_filename(gfs, Filename.c_str(), err);
			clsRasterData<float>::outputToMongoDB(templateRaster, m_1DData, Filename, gfs);
		}
       
        string ascii(ASCIIExtension);
        if (ascii.find(Suffix) != ascii.npos)
            clsRasterData<float>::outputASCFile(templateRaster, m_1DData, projectPath + Filename + ASCIIExtension);
        else
            clsRasterData<float>::outputGTiff(templateRaster, m_1DData, projectPath + Filename + GTiffExtension);
        return;
    }

    if (m_2DData != NULL && m_nRows > -1 && m_nLayers > 0) /// Multi-Layers raster data
    {
        if (templateRaster == NULL)
            throw ModelException("PrintInfoItem", "Flush", "The templateRaster is NULL.");

        float *tmpData = new float[m_nRows];
        ostringstream oss;
        for (int j = 0; j < m_nLayers; j++)
        {
            for (int i = 0; i < m_nRows; i++)
                tmpData[i] = m_2DData[i][j];
            oss.str("");
            oss << projectPath << Filename << "_" << (j + 1);  // Filename_1.tif means layer 1
            string ascii(ASCIIExtension);
            if (ascii.find(Suffix) != ascii.npos)
                clsRasterData<float>::outputASCFile(templateRaster, tmpData, oss.str() + ASCIIExtension);
            else
                clsRasterData<float>::outputGTiff(templateRaster, tmpData, oss.str() + GTiffExtension);
        }
        delete[] tmpData;
		if (outToMongoDB)
		{
			bson_error_t *err = NULL;
			mongoc_gridfs_remove_by_filename(gfs, Filename.c_str(), err);
			clsRasterData<float>::outputToMongoDB(templateRaster, m_2DData, m_nLayers, Filename, gfs);
		}	
		return;
    }

    if (TimeSeriesData.size() > 0)    /// time series data
    {
        ofstream fs;
        utils util;
        string filename = projectPath + Filename + TextExtension;
        fs.open(filename.c_str(), ios::out|ios::app);
        if (fs.is_open())
        {
            map<time_t, float>::iterator it;
            for (it = TimeSeriesData.begin(); it != TimeSeriesData.end(); it++)
            {
                fs << util.ConvertToString2(&(it->first)) << " " << right << fixed << setw(15) << setfill(' ') <<
                setprecision(8) << it->second << endl;
            }
            fs.close();
            StatusMessage(("Create " + filename + " successfully!").c_str());
        }
        return;
    }

    throw ModelException("PrintInfoItem", "Flush", "Creating " + Filename +
                                                   " is failed. There is not result data for this file. Please check output variables of modules.");
}

void PrintInfoItem::AggregateData2D(time_t time, int nRows, int nCols, float **data)
{
    if (m_AggregationType == AT_SpecificCells)
    {
/*		if(m_specificOutput != NULL)
		{
			m_specificOutput->setData(time,data);
		}	*/
    }
    else
    {
        // check to see if there is an aggregate array to add data to
        if (m_2DData == NULL)
        {
            // create the aggregate array
            m_nRows = nRows;
            m_nLayers = nCols;
			Initialize2DArray(m_nRows, m_nLayers, m_2DData, NODATA_VALUE);
            m_Counter = 0;
        }

        switch (m_AggregationType)
        {
            case AT_Average:
				#pragma omp parallel for
                for (int i = 0; i < m_nRows; i++){
                    for (int j = 0; j < m_nLayers; j++){
						if(!FloatEqual(data[i][j], NODATA_VALUE)){
							if(FloatEqual(m_2DData[i][j], NODATA_VALUE))
								m_2DData[i][j] = 0.f;
							m_2DData[i][j] = (m_2DData[i][j] * m_Counter + data[i][j]) / (m_Counter + 1.f);
						}
					}
				}
                break;
            case AT_Sum:
				#pragma omp parallel for
                for (int i = 0; i < m_nRows; i++){
					for (int j = 0; j < m_nLayers; j++){
						if(!FloatEqual(data[i][j], NODATA_VALUE)){
							if(FloatEqual(m_2DData[i][j], NODATA_VALUE))
								m_2DData[i][j] = 0.f;
							m_2DData[i][j] += data[i][j];
						}
					}
				}
                break;
            case AT_Minimum:
				#pragma omp parallel for
                for (int i = 0; i < m_nRows; i++){
                    for (int j = 0; j < m_nLayers; j++)
                    {
                        if (!FloatEqual(data[i][j], NODATA_VALUE)){
							if(FloatEqual(m_2DData[i][j], NODATA_VALUE))
								m_2DData[i][j] = MAXFLOAT;
							if(data[i][j] <= m_2DData[i][j])
								m_2DData[i][j] = data[i][j];
						}
					}
				}
                break;
            case AT_Maximum:
				#pragma omp parallel for
                for (int i = 0; i < m_nRows; i++){
                    for (int j = 0; j < m_nLayers; j++)
                    {
                        if (!FloatEqual(data[i][j], NODATA_VALUE)){
							if(FloatEqual(m_2DData[i][j], NODATA_VALUE))
								m_2DData[i][j] = MISSINGFLOAT;
							if(data[i][j] >= m_2DData[i][j])
								m_2DData[i][j] = data[i][j];
						}
					}
				}
                break;
            default:
                break;
        }
        m_Counter++;
    }
}


void PrintInfoItem::AggregateData(time_t time, int numrows, float *data)
{
    if (m_AggregationType == AT_SpecificCells)
    {
        /*if(m_specificOutput != NULL)
        {
            m_specificOutput->setData(time,data);
        }		*/
    }
    else
    {
        // check to see if there is an aggregate array to add data to
        if (m_1DData == NULL)
        {
            // create the aggregate array
            m_nRows = numrows;
			Initialize1DArray(m_nRows, m_1DData, NODATA_VALUE);
            m_Counter = 0;
        }

        // depending on the type of aggregation
		#pragma omp parallel for
        for (int rw = 0; rw < m_nRows; rw++)
        {
            switch (m_AggregationType)
            {
                case AT_Average:
					if(!FloatEqual(data[rw], NODATA_VALUE))
					{
						if(FloatEqual(m_1DData[rw], NODATA_VALUE))
							m_1DData[rw] = 0.f;
						m_1DData[rw] = (m_1DData[rw] * m_Counter + data[rw]) / (m_Counter + 1.f);
					}
                    break;
                case AT_Sum:
					if(!FloatEqual(data[rw], NODATA_VALUE)){
						if(FloatEqual(m_1DData[rw], NODATA_VALUE))
							m_1DData[rw] = 0.f;
						m_1DData[rw] += data[rw];
					}
                    break;
                case AT_Minimum:
					if(!FloatEqual(data[rw], NODATA_VALUE)){
						if(FloatEqual(m_1DData[rw], NODATA_VALUE))
							m_1DData[rw] = MAXFLOAT;
						if (m_1DData[rw] >= data[rw])
							m_1DData[rw] = data[rw];
					}
                    break;
                case AT_Maximum:
					if(!FloatEqual(data[rw], NODATA_VALUE)){
						if(FloatEqual(m_1DData[rw], NODATA_VALUE))
							m_1DData[rw] = MISSINGFLOAT;
						if (m_1DData[rw] <= data[rw]) 
							m_1DData[rw] = data[rw];
					}
                    break;
                default:
                    break;
            }
        }
        m_Counter++;
    }
}


void PrintInfoItem::AggregateData(int numrows, float **data, AggregationType type, float NoDataValue)
{
    // check to see if there is an aggregate array to add data to
    if (m_1DDataWithRowCol == NULL)
    {
        // create the aggregate array
        m_nRows = numrows;
        m_1DDataWithRowCol = new float *[m_nRows];
        for (int i = 0; i < m_nRows; i++)
        {
            m_1DDataWithRowCol[i] = new float[3];
            m_1DDataWithRowCol[i][0] = NoDataValue;
            m_1DDataWithRowCol[i][1] = NoDataValue;
            m_1DDataWithRowCol[i][2] = NoDataValue;
        }
        m_Counter = 0;
    }

    // depending on the type of aggregation
    switch (type)
    {
        case AT_Average:
			#pragma omp parallel for
            for (int rw = 0; rw < m_nRows; rw++)
            {
                if (!FloatEqual(data[rw][2], NoDataValue))
                {
                    // initialize value to 0.0 if this is the first time
                    if (FloatEqual(m_1DDataWithRowCol[rw][2], NoDataValue)) m_1DDataWithRowCol[rw][2] = 0.0f;
                    m_1DDataWithRowCol[rw][0] = data[rw][0]; // store the row number
                    m_1DDataWithRowCol[rw][1] = data[rw][1]; // store the column number
                    if (m_Counter == 0)
                    {
                        // first value so average = value
                        m_1DDataWithRowCol[rw][2] = data[rw][2];    // store the value
                    }
                    else
                    {
                        // calculate the incremental average
                        m_1DDataWithRowCol[rw][2] = ((m_1DDataWithRowCol[rw][2] * m_Counter) + data[rw][2]) / (m_Counter + 1);
                    }
                }
            }
            m_Counter++;
            break;
        case AT_Sum:
			#pragma omp parallel for
            for (int rw = 0; rw < m_nRows; rw++)
            {
                if (!FloatEqual(data[rw][2], NoDataValue))
                {
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
            for (int rw = 0; rw < m_nRows; rw++)
            {
                if (!FloatEqual(data[rw][2], NoDataValue))
                {
                    // initialize value to 0.0 if this is the first time
                    if (FloatEqual(m_1DDataWithRowCol[rw][2], NoDataValue)) m_1DDataWithRowCol[rw][2] = 0.0f;
                    m_1DDataWithRowCol[rw][0] = data[rw][0];
                    m_1DDataWithRowCol[rw][1] = data[rw][1];
                    // if the next value is smaller than the current value
                    if (data[rw][2] <= m_1DDataWithRowCol[rw][2])
                    {
                        // set the current value to the next (smaller) value
                        m_1DDataWithRowCol[rw][2] = data[rw][2];
                    }
                }
            }
            break;
        case AT_Maximum:
			#pragma omp parallel for
            for (int rw = 0; rw < m_nRows; rw++)
            {
                if (!FloatEqual(data[rw][2], NoDataValue))
                {
                    // initialize value to 0.0 if this is the first time
                    if (FloatEqual(m_1DDataWithRowCol[rw][2], NoDataValue)) m_1DDataWithRowCol[rw][2] = 0.0f;
                    m_1DDataWithRowCol[rw][0] = data[rw][0];
                    m_1DDataWithRowCol[rw][1] = data[rw][1];
                    // if the next value is larger than the current value
                    if (data[rw][2] >= m_1DDataWithRowCol[rw][2])
                    {
                        // set the current value to the next (larger) value
                        m_1DDataWithRowCol[rw][2] = data[rw][2];
                    }
                }
            }
            break;
        default:
            break;
    }
}

AggregationType PrintInfoItem::MatchAggregationType(string type)
{
    AggregationType res = AT_Unknown;
    if (StringMatch(type, Tag_Unknown))
    {
        res = AT_Unknown;
    }
    if (StringMatch(type, Tag_Sum))
    {
        res = AT_Sum;
    }
    if (StringMatch(type, Tag_Average) || StringMatch(type, Tag_Average2) || StringMatch(type, Tag_Average3))
    {
        res = AT_Average;
    }
    if (StringMatch(type, Tag_Minimum))
    {
        res = AT_Minimum;
    }
    if (StringMatch(type, Tag_Maximum))
    {
        res = AT_Maximum;
    }
    if (StringMatch(type, Tag_SpecificCells))
    {
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


PrintInfo::PrintInfo(void)
{
    m_Interval = 0;
    m_IntervalUnits = "";
    m_OutputID = "";
    m_PrintItems.clear();
    m_param = NULL;
    m_moduleIndex = -1;
    m_subbasinSelectedArray = NULL;
}

PrintInfo::~PrintInfo(void)
{
	// no need to reset the basic data type. LJ
    //m_Interval = 0;
    //m_IntervalUnits = "";
    //m_OutputID = "";
	for(vector<PrintInfoItem *>::iterator it = m_PrintItems.begin(); it != m_PrintItems.end();)
	{
		if(*it != NULL)
		{
			delete *it;
			*it = NULL;
		}
		it = m_PrintItems.erase(it);
	}
	m_PrintItems.clear();
    if (m_subbasinSelectedArray != NULL)
        Release1DArray(m_subbasinSelectedArray);
}

string PrintInfo::getOutputTimeSeriesHeader()
{
    vector<string> headers;
    if (StringMatch(m_OutputID, VAR_SNWB))
    {
        headers.push_back("Time");
        headers.push_back("P");
        headers.push_back("P_net");
        headers.push_back("P_blow");
        headers.push_back("T");
        headers.push_back("Wind");
        headers.push_back("SR");
        headers.push_back("SE");
        headers.push_back("SM");
        headers.push_back("SA");
    }
    else if (StringMatch(m_OutputID, VAR_SOWB))
    {
        headers.push_back("Time");
        headers.push_back("PCP (mm)");
        headers.push_back("meanTmp (deg C)");
        headers.push_back("soilTmp (deg C)");
        headers.push_back("netPcp (mm)");
        headers.push_back("InterceptionET (mm)");
        headers.push_back("DepressionET (mm)");
        headers.push_back("Infiltration (mm)");
        headers.push_back("Total_ET (mm)");
        headers.push_back("Soil_ET (mm)");
        headers.push_back("NetPercolation (mm)");
        headers.push_back("Revaporization (mm)");
        headers.push_back("SurfaceRunoff (mm)");
        headers.push_back("SubsurfaceRunoff (mm)");
        headers.push_back("Baseflow (mm)");
        headers.push_back("AllRunoff (mm)");
        headers.push_back("SoilMoisture (mm)");
        //headers.push_back("MoistureDepth");
    }
    else if (StringMatch(m_OutputID, VAR_GWWB))
    {
        headers.push_back("Time");
        headers.push_back("Percolation (mm)");
        headers.push_back("Revaporization (mm)");
        headers.push_back("Deep Percolation (mm)");
        headers.push_back("Baseflow (mm)");
        headers.push_back("Groundwater storage (mm)");
        headers.push_back("Baseflow discharge (m3/s)");
    }
    else if (StringMatch(m_OutputID, "T_RECH"))
    {
        headers.push_back("Time");
        headers.push_back("QS");
        headers.push_back("QI");
        headers.push_back("QG");
        headers.push_back("Q(m^3/s)");
        headers.push_back("Q(mm)");
    }
    else if (StringMatch(m_OutputID, "T_WABA"))
    {
        headers.push_back("Time");
        headers.push_back("Vin");
        headers.push_back("Vside");
        headers.push_back("Vdiv");
        headers.push_back("Vpoint");
        headers.push_back("Vseep");
        headers.push_back("Vnb");
        headers.push_back("Ech");
        headers.push_back("Lbank");
        headers.push_back("Vbank");
        headers.push_back("Vout");
        headers.push_back("Vch");
        headers.push_back("Depth");
        headers.push_back("Velocity");
    }
    else if (StringMatch(m_OutputID, "T_RSWB"))
    {
        headers.push_back("Time");
        headers.push_back("Qin(m^3/s)");
        headers.push_back("Area(ha)");
        headers.push_back("Storage(10^4*m^3)");
        headers.push_back("QSout(m^3/s)");
        headers.push_back("QIout(m^3/s)");
        headers.push_back("QGout(m^3/s)");
        headers.push_back("Qout(m^3/s)");
        headers.push_back("Qout(mm)");
    }
    else if (StringMatch(m_OutputID, "T_CHSB"))
    {
        headers.push_back("Time");
        headers.push_back("SedInUpStream");
        headers.push_back("SedInSubbasin");
        headers.push_back("SedDeposition");
        headers.push_back("SedDegradation");
        headers.push_back("SedStorage");
        headers.push_back("SedOut");
    }
    else if (StringMatch(m_OutputID, "T_RESB"))
    {
        headers.push_back("Time");
        headers.push_back("ResSedIn");
        headers.push_back("ResSedSettling");
        headers.push_back("ResSedStorage");
        headers.push_back("ResSedOut");
    }
    ostringstream oss;
    vector<string>::iterator it;
    for (it = headers.begin(); it < headers.end(); it++)
    {
        if (StringMatch(*it, "Time"))
            oss << *it;
        else
            oss << " " << setw(15) << right << setfill(' ') << *it;
    }
    return oss.str();
}

void PrintInfo::AddPrintItem(string start, string end, string file, string sufi)
{
    // create a new object instance
    PrintInfoItem *itm = new PrintInfoItem();

    // set its properties
    itm->SiteID = -1;
    itm->StartTime = start;
    itm->EndTime = end;
    itm->Filename = file;
    itm->Suffix = sufi;
    /// Be default, date time format has hour info.
    itm->m_startTime = utils::ConvertToTime2(start, "%d-%d-%d %d:%d:%d", true);
    itm->m_endTime = utils::ConvertToTime2(end, "%d-%d-%d %d:%d:%d", true);
    // add it to the list
    m_PrintItems.push_back(itm);
}


void PrintInfo::AddPrintItem(string type, string start, string end, string file, string sufi, mongoc_client_t *conn,
                             mongoc_gridfs_t *gfs)
{
    // create a new object instance
    PrintInfoItem *itm = new PrintInfoItem();

    // set its properties
    itm->SiteID = -1;
    itm->StartTime = start;
    itm->EndTime = end;
    itm->Filename = file;
    itm->Suffix = sufi;
    itm->conn = conn;
    itm->gfs = gfs;

    itm->m_startTime = utils::ConvertToTime2(start, "%d-%d-%d %d:%d:%d", true);
    itm->m_endTime = utils::ConvertToTime2(end, "%d-%d-%d %d:%d:%d", true);


    type = trim(type);
	itm->AggType = type;
    AggregationType enumType = PrintInfoItem::MatchAggregationType(type);
    if (enumType == AT_Unknown)
        throw ModelException("PrintInfo", "AddPrintItem", "The type of output " + m_OutputID +
                                                          " can't be unknown. Please check file.out. The type should be MIN, MAX, SUM or AVERAGE (AVE or MEAN).");

    itm->setAggregationType(enumType);

    // add it to the list
    m_PrintItems.push_back(itm);
}


void PrintInfo::AddPrintItem(string start, string end, string file, string sitename, string sufi,mongoc_client_t *m_conn,mongoc_gridfs_t *m_outputGfs, bool isSubbasin)
{
    PrintInfoItem *itm = new PrintInfoItem();

    if (!isSubbasin) itm->SiteID = atoi(sitename.c_str());
    else
    {
        itm->SubbasinID = atoi(sitename.c_str());
        itm->SubbasinIndex = m_subbasinSeleted.size();
        m_subbasinSeleted.push_back(itm->SubbasinID);
    }
    itm->StartTime = start;
    itm->EndTime = end;
	itm->conn = m_conn;
	itm->gfs = m_outputGfs;
    itm->Filename = file;
    itm->Suffix = sufi;
    itm->m_startTime = utils::ConvertToTime2(start, "%d-%d-%d %d:%d:%d", true);
    itm->m_endTime = utils::ConvertToTime2(end, "%d-%d-%d %d:%d:%d", true);

    m_PrintItems.push_back(itm);
}

void PrintInfo::getSubbasinSelected(int *count, float **subbasins)
{
    *count = m_subbasinSeleted.size();
    if (m_subbasinSelectedArray == NULL && m_subbasinSeleted.size() > 0)
    {
        m_subbasinSelectedArray = new float[m_subbasinSeleted.size()];
        int index = 0;
        vector<int>::iterator it;
        for (it = m_subbasinSeleted.begin(); it < m_subbasinSeleted.end(); it++)
        {
            m_subbasinSelectedArray[index] = float(*it);
            index++;
        }
    }
    *subbasins = m_subbasinSelectedArray;
}


PrintInfoItem *PrintInfo::getPrintInfoItem(int index)
{
    // default is NULL
    PrintInfoItem *res = NULL;

    // is the index in the valid range
    if (index >= 0 && index < (int) m_PrintItems.size())
    {
        // assign the reference to the given item
        res = m_PrintItems.at(index);
    }
    // return the reference
    return res;
}
