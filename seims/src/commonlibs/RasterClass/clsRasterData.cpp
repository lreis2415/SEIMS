/*!
 * \brief Implementation of clsRasterData class
 *
 * 1. Using GDAL and MongoDB (currently, mongo-c-driver 1.5.0)
 * 2. Array1D and Array2D raster data are supported
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date Apr. 2011
 * \revised May. 2016
 *
 */
#ifndef CLS_RASTER_DATA
#include "clsRasterData.h"

/************* Construct functions ***************/

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::_initialize_raster_class(){
	m_filePathName = "";
	m_coreFileName = "";
	m_nCells = -1;
	m_noDataValue = (T) NODATA_VALUE;
	m_rasterData = NULL;
	m_rasterPositionData = NULL;
	m_mask = NULL;
	m_nLyrs = 1;
	m_is2DRaster = false;
	m_raster2DData = NULL;
	m_calcPositions = false;
	m_useMaskExtent = false;
	m_statisticsCalculated = false;
	const char* RASTER_HEADERS[8] = {HEADER_RS_NCOLS, HEADER_RS_NROWS, HEADER_RS_XLL, HEADER_RS_YLL, HEADER_RS_CELLSIZE,
		HEADER_RS_NODATA, HEADER_RS_LAYERS, HEADER_RS_CELLSNUM};
	for(int i = 0; i < 6; i++)
		m_headers[RASTER_HEADERS[i]] = NODATA_VALUE;
	m_headers[HEADER_RS_LAYERS] = 1.;
	m_headers[HEADER_RS_CELLSNUM] = -1.;
	string statsnames[6] = {STATS_RS_VALIDNUM, STATS_RS_MIN, STATS_RS_MAX, STATS_RS_MEAN
							STATS_RS_STD, STATS_RS_RANGE};
	for (int i = 0; i < 6; i++)
	{
		m_statsMap[statsnames[i]] = NODATA_VALUE;
		m_statsMap2D[statsnames[i]] = NULL;
	}
	m_initialized = true;
}
template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::_initialize_read_function(string filename, bool calcPositions /* = true */, clsRasterData<MaskT> *mask /* = NULL */, bool useMaskExtent /* = true */){
	if (!m_initialized) this->_initialize_raster_class();
	if (mask != NULL) m_mask = mask;
	m_filePathName = filename; // full path
	m_coreFileName = GetCoreFileName(m_filePathName);
	m_calcPositions = calcPositions;
	m_useMaskExtent = useMaskExtent;
}
template<typename T, typename MaskT>
clsRasterData<T, MaskT>::clsRasterData(void) {
	this->_initialize_raster_class();
}

template<typename T, typename MaskT>
clsRasterData<T, MaskT>::clsRasterData(string filename, bool calcPositions /* = true */, clsRasterData<MaskT> *mask /* = NULL */, bool useMaskExtent /* = true */){
    this->ReadFromFile(filename, calcPositions, mask, useMaskExtent);
}
template<typename T, typename MaskT>
clsRasterData<T, MaskT>::clsRasterData(vector<string> filenames, bool calcPositions /* = true */, clsRasterData<MaskT> *mask /* = NULL */, bool useMaskExtent /* = true */){
	this->_check_raster_file_exists(filenames);
	this->_initialize_raster_class();
	/// if filenames is empty, throw an exception
	if (filenames.empty()) 
		throw ModelException("clsRasterData", "Constructor", "filenames must have at least one raster file path!\n");
	/// if filenames has only one file
	if (filenames.size() == 1){
		this->_construct_from_single_file(filenames[0], calcPositions, mask, useMaskExtent);
	}
	else{  /// construct from multi-layers file
		m_nLyrs = filenames.size();
		/// 1. firstly, take the first layer as the main input, to calculate position index or 
		///    extract by mask if stated.
		this->_construct_from_single_file(filenames[0], calcPositions, mask, useMaskExtent);
		/// 2. then, change the core file name and file path template which format is: <file dir>/CoreName_%d.<suffix>
		m_coreFileName = SplitString(m_coreFileName, '_')[0];
		m_filePathName = GetPathFromFullName(filenames[0]) + 
			SEP + m_coreFileName + "_%d." + GetSuffix(filenames[0]);  
		/// So, to get a given layer's filepath, please use the following code. Definitely, maximum 99 layers is supported now.
		///    string layerFilepath = m_filePathName.replace(m_filePathName.find_last_of("%d") - 1, 2, ValueToString(1));
		/// 3. initialize m_raster2DData and read the other layers according to position data if stated,
		///     or just read by row and col
		Initialize2DArray(m_nCells, m_nLyrs, m_raster2DData, m_noDataValue);
#pragma omp parallel for
		for (int i = 0; i < m_nCells; i++){
			m_raster2DData[i][0] = m_rasterData[i];
		}
		Release1DArray(m_rasterData);
		/// take the first layer as mask, and useMaskExtent is true, and no need to calculate position data
        //for (vector<string>::iterator iter = filenames.begin(); iter != filenames.end(); iter++){
		for (int fileidx = 1; fileidx < filenames.size(); fileidx++){
			map<string, double> tmpheader;
			T *tmplyrdata = NULL;
            string curfilename = filenames.at(fileidx);
			if (StringMatch(GetUpper(GetSuffix(curfilename)), string(ASCIIExtension)))
				this->_read_asc_file(curfilename, &tmpheader, &tmplyrdata);
			else
				this->_read_raster_file_by_gdal(curfilename, &tmpheader, &tmplyrdata);
			if (m_calcPositions){
				for (int i = 0; i < m_nCells; ++i) {
					int tmpRow = m_rasterPositionData[i][0];
					int tmpCol = m_rasterPositionData[i][1];
					this->_add_other_layer_raster_data(tmpRow, tmpCol, i, fileidx, tmpheader, tmplyrdata);
				}
			}
			else{
				for (int i = 0; i < m_headers[HEADER_RS_NROWS]; ++i) {
					for (int j = 0; j < m_headers[HEADER_RS_NCOLS]; ++j) {
						this->_add_other_layer_raster_data(i, j, i * (int) m_headers[HEADER_RS_NCOLS] + j, fileidx, tmpheader, tmplyrdata);
					}
				}
			}
			Release1DArray(tmplyrdata);
		}
		m_is2DRaster = true;
	}
}

template<typename T, typename MaskT>
clsRasterData<T, MaskT>::clsRasterData(clsRasterData<MaskT> *mask, T*& values){
	this->_initialize_raster_class();
	m_mask = mask;
	m_nCells = m_mask->getCellNumber();
	Initialize1DArray(m_nCells, m_rasterData, values);
	// m_rasterData = values; // DO NOT ASSIGN ARRAY DIRECTLY, IN CASE OF MEMORY ERROR!
	this->copyHeader(m_mask->getRasterHeader());
	m_calcPositions = false;
	m_useMaskExtent = true;
}

template<typename T, typename MaskT>
clsRasterData<T, MaskT>::clsRasterData(clsRasterData<MaskT> *mask, T**& values, int lyrs){
	this->_initialize_raster_class();
	m_mask = mask;
	m_nLyrs = lyrs;
	this->copyHeader(m_mask->getRasterHeader());
	m_nCells = m_mask->getCellNumber();
	Initialize2DArray(m_nCells, m_nLyrs, m_raster2DData, values);
	// m_raster2DData = values; // DO NOT ASSIGN ARRAY DIRECTLY, IN CASE OF MEMORY ERROR!
	m_useMaskExtent = true;
	m_is2DRaster = true;
}

#ifdef USE_MONGODB
template<typename T, typename MaskT>
clsRasterData<T, MaskT>::clsRasterData(mongoc_gridfs_t *gfs, const char *remoteFilename, bool calcPositions /* = true */, clsRasterData<MaskT> *mask /* = NULL */, bool useMaskExtent /* = true */){
	this->_initialize_raster_class();
	this->ReadFromMongoDB(gfs, remoteFilename, calcPositions, mask, useMaskExtent);
}
#endif

template<typename T, typename MaskT>
bool clsRasterData<T, MaskT>::_check_raster_file_exists(string filename){
	if (!FileExists(filename))
		throw ModelException("clsRasterData", "Constructor",
		"The file " + filename + " does not exist or has not read permission.");
	return true;
}
template<typename T, typename MaskT>
bool clsRasterData<T, MaskT>::_check_raster_file_exists(vector<string>&filenames){
	for (vector<string>::iterator it = filenames.begin(); it != filenames.end(); it++){
		if (!this->_check_raster_file_exists(*it))
			return false;
	}
}
template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::_construct_from_single_file(string filename, bool calcPositions /* = true */, clsRasterData<MaskT> *mask /* = NULL */, bool useMaskExtent /* = true */){
	if (mask != NULL) m_mask = mask;
	m_filePathName = filename; // full path
	m_coreFileName = GetCoreFileName(m_filePathName);
	m_calcPositions = calcPositions;
	m_useMaskExtent = useMaskExtent;

	if (StringMatch(GetUpper(GetSuffix(filename)), ASCIIExtension))
		_read_asc_file(m_filePathName, &m_headers, &m_rasterData);
	else
		_read_raster_file_by_gdal(m_filePathName, &m_headers, &m_rasterData);
	/******** Mask and calculate valid positions ********/
	this->_mask_and_calculate_valid_positions();
}
template<typename T, typename MaskT>
clsRasterData<T, MaskT>::~clsRasterData(void) {
	StatusMessage(("Release raster: " + m_coreFileName).c_str());
    if (m_rasterData != NULL) Release1DArray(m_rasterData);
    if (m_rasterPositionData != NULL && m_calcPositions) Release2DArray(m_nCells, m_rasterPositionData);
    if (m_raster2DData != NULL && m_is2DRaster) Release2DArray(m_nCells, m_raster2DData);
	if (m_is2DRaster && m_statisticsCalculated) this->releaseStatsMap2D();
}

/************* Get information functions ***************/

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::calculateStatistics(){
	if (this->m_statisticsCalculated) return;
	if (m_is2DRaster && m_raster2DData != NULL){
		double** derivedvs;
		basicStatistics(m_raster2DData, m_nCells, m_nLyrs, &derivedvs, m_noDataValue);
		m_statsMap2D[STATS_RS_VALIDNUM] = derivedvs[0];
		m_statsMap2D[STATS_RS_MEAN] = derivedvs[1];
		m_statsMap2D[STATS_RS_MAX] = derivedvs[2];
		m_statsMap2D[STATS_RS_MIN] = derivedvs[3];
		m_statsMap2D[STATS_RS_STD] = derivedvs[4];
		m_statsMap2D[STATS_RS_RANGE] = derivedvs[5];
		/// derivedvs will be released on the destructor function by releaseStatsMap2D().
	}
	else{
		double* derivedv = NULL;
		basicStatistics(m_rasterData, m_nCells, &derivedv, m_noDataValue);
		m_statsMap[STATS_RS_VALIDNUM] = derivedv[0];
		m_statsMap[STATS_RS_MEAN] = derivedv[1];
		m_statsMap[STATS_RS_MAX] = derivedv[2];
		m_statsMap[STATS_RS_MIN] = derivedv[3];
		m_statsMap[STATS_RS_STD] = derivedv[4];
		m_statsMap[STATS_RS_RANGE] = derivedv[5];
		Release1DArray(derivedv);
	}
	this->m_statisticsCalculated = true;
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::releaseStatsMap2D(){
	for (map<string, double*>::iterator it = m_statsMap2D.begin(); it != m_statsMap2D.end(); ){
		if (it->second != NULL)
			Release1DArray(it->second);
		it = m_statsMap2D.erase(it);
	}
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::updateStatistics(){
	if (m_is2DRaster && this->m_statisticsCalculated) this->releaseStatsMap2D();
	this->m_statisticsCalculated = false;
	this->calculateStatistics();
}

template<typename T, typename MaskT>
double clsRasterData<T, MaskT>::getStatistics(string  sindex, int lyr) {
	sindex = GetUpper(sindex);
	if (this->m_is2DRaster && m_raster2DData != NULL)  // for 2D raster data
	{
		map<string, double*>::iterator it = m_statsMap2D.find(sindex);
		if (it != m_statsMap2D.end())
		{
			if (it->second == NULL || !this->m_statisticsCalculated)
				this->calculateStatistics();
			return m_statsMap2D.at(sindex)[lyr - 1];
		}
		else
			throw ModelException("clsRasterData", "getStatistics", 
			sindex + " is not supported currently, please contact the developers.");
	} 
	else  // for 1D raster data
	{
		map<string, double>::iterator it = m_statsMap.find(sindex);
		if (it != m_statsMap.end())
		{
			if (FloatEqual(it->second, (double) NODATA_VALUE) || !this->m_statisticsCalculated)
				this->calculateStatistics();
			return m_statsMap.at(sindex);
		}
		else
			throw ModelException("clsRasterData", "getStatistics", 
			sindex + " is not supported currently, please contact the developers.");
	}
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::getStatistics(string  sindex, int *lyrnum, double **values){
	if (!m_is2DRaster || m_raster2DData == NULL)
		throw ModelException("claRasterData", "getValue", "Please initialize the raster object first.");
	sindex = GetUpper(sindex);
	*lyrnum = m_nLyrs;
	map<string, double*>::iterator it = m_statsMap2D.find(sindex);
	if (!this->is2DRaster()){
		*values = NULL;
		return;
	}
	if (it == m_statsMap2D.end())
	{
		*values = NULL;
		throw ModelException("clsRasterData", "getStatistics", 
			sindex + " is not supported currently, please contact the developers.");
	}
	if (it->second == NULL || !this->m_statisticsCalculated)
		this->calculateStatistics();
	*values = it->second;
	return;
}

template<typename T, typename MaskT>
int clsRasterData<T, MaskT>::getPosition(int row, int col) {
    if (m_calcPositions || m_rasterPositionData == NULL) return -1;
    for (int i = 0; i < m_nCells; i++) {
        if (row == m_rasterPositionData[i][0] && col == m_rasterPositionData[i][1])
			return i;
    }
    return -1;
}

template<typename T, typename MaskT>
int clsRasterData<T, MaskT>::getPosition(float x, float y) {
    return getPosition((double) x, (double) y);
}

template<typename T, typename MaskT>
int clsRasterData<T, MaskT>::getPosition(double x, double y) {
    double xllCenter = this->getXllCenter();
    double yllCenter = this->getYllCenter();
    float dx = this->getCellWidth();
    float dy = this->getCellWidth();
    int nRows = this->getRows();
    int nCols = this->getCols();

    double xmin = xllCenter - dx / 2.;
    double xMax = xmin + dx * nCols;
    if (x > xMax || x < xllCenter){
		return -1;
		// throw ModelException("claRasterData", "getPosition", "The x coordinate is beyond the scale!");
	}

    double ymin = yllCenter - dy / 2.;
    double yMax = ymin + dy * nRows;
    if (y > yMax || y < yllCenter){
		return -1;
		// throw ModelException("claRasterData", "getPosition", "The y coordinate is beyond the scale!");
	}

    int nRow = (int) ((yMax - y) / dy); //calculate from ymax
    int nCol = (int) ((x - xmin) / dx); //calculate from xmin

    return getPosition(nRow, nCol);
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::getRasterData(int *nRows, T **data) {
	if (m_rasterData != NULL){
		*nRows = m_nCells;
		*data = m_rasterData;
	}
	else{
		throw ModelException("claRasterData", "getRasterData", "Please initialize the raster object first.");
	}
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::get2DRasterData(int *nRows, int *nCols, T ***data) {
	if (m_is2DRaster && m_raster2DData != NULL){
		*nRows = m_nCells;
		*nCols = m_nLyrs;
		*data = m_raster2DData;
	}
	else{
		throw ModelException("claRasterData", "get2DRasterData", "Please initialize the raster object first.");
	}    
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::getRasterPositionData(int &nRows, int **&data) {
    if (m_mask != NULL && m_mask->PositionsCalculated() && m_useMaskExtent)
		m_mask->getRasterPositionData(nRows, data);
	else if (m_calcPositions){
		nRows = m_nCells;
		data = m_rasterPositionData;
	}
	else{// reCalculate positions data
		_calculate_valid_positions_from_grid_data();
		nRows = m_nCells;
		data = m_rasterPositionData;
    }
}

template<typename T, typename MaskT>
T clsRasterData<T, MaskT>::getValue(int validCellIndex, int lyr /* = 1 */) {
	if (m_rasterData == NULL || (m_is2DRaster && m_raster2DData == NULL))
		throw ModelException("claRasterData", "getValue", "Please initialize the raster object first.");
	if (m_nCells <= validCellIndex || lyr > m_nLyrs)
		throw ModelException("claRasterData", "getValue",
		"The index is too big! There are not so many valid cell in the raster.");
	if (m_is2DRaster && m_raster2DData != NULL)
		return m_raster2DData[validCellIndex][lyr - 1];
	else if (m_rasterData != NULL)
		return m_rasterData[validCellIndex];
	else  // this would not happen
		return m_noDataValue;
}
template<typename T, typename MaskT>
T clsRasterData<T, MaskT>::getValue(RowColCoor pos, int lyr /* = 1 */) {
	int row = pos.row;
	int col = pos.col;
	if (m_rasterData == NULL || (m_is2DRaster && m_raster2DData == NULL))
		throw ModelException("claRasterData", "getValue", "Please initialize the raster object first.");
	// return NODATA if row, col, or lyr exceeds the extent 
	if ((row < 0 || row > this->getRows()) || (col < 0 || col > this->getCols()) || lyr > m_nLyrs)
		return m_noDataValue;
	/// get index according to position data if possible
	if (m_calcPositions && m_rasterPositionData != NULL){
		int validCellIndex = this->getPosition(row, col);
		if (validCellIndex == -1)
			return m_noDataValue;
		else
			return this->getValue(validCellIndex, lyr);
	} else  // get data directly from row and col
	{
		if (m_is2DRaster && m_raster2DData != NULL)
			return m_raster2DData[row * this->getCols() + col][lyr - 1];
		else
			return m_rasterData[row * this->getCols() + col];
	}
}
template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::setValue(RowColCoor pos, T value, int lyr /* = 1 */){
	int idx = this->getPosition(pos.row, pos.col);
	if (idx == -1){
		if (m_is2DRaster)
			m_raster2DData[pos.row * this->getCols() + pos.col][lyr] = value;
		else
			m_rasterData[pos.row * this->getCols() + pos.col] = value;
	}
	else{
		if (m_is2DRaster)
			m_raster2DData[idx][lyr] = value;
		else
			m_rasterData[idx] = value;
	}
}
template<typename T, typename MaskT>
T clsRasterData<T, MaskT>::isNoData(RowColCoor pos, int lyr /* = 1 */){
	return FloatEqual(this->getValue(pos, lyr), m_noDataValue);
}
template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::getValue(int validCellIndex, int *nLyrs, T** values) {
	if (m_rasterData == NULL || (m_is2DRaster && m_raster2DData == NULL))
		throw ModelException("claRasterData", "getValue", "Please first initialize the raster object.");
	// return NODATA if row, col, or lyr exceeds the extent
    if (validCellIndex < 0 || validCellIndex > m_nCells || *nLyrs > m_nLyrs)
		return m_noDataValue;
	/// get index according to position data if possible
	if (m_calcPositions && m_rasterPositionData != NULL){
		if (m_nCells < validCellIndex)
			throw ModelException("clsRasterData", "getValue",
			"The index is too big! There are not so many valid cell in the raster.");
	}
	else
		throw ModelException("clsRasterData", "getValue", "The position data is not calculated!");
    if (m_is2DRaster && m_raster2DData == NULL) {
		T *cellValues = new T[m_nLyrs];
        for (int i = 0; i < m_nLyrs; i++)
            cellValues[i] = m_raster2DData[validCellIndex][i];
        *nLyrs = m_nLyrs;
        *values = cellValues;
    } else {
		*nLyrs = 1;
        T *cellValues = new T[1];
        cellValues[0] = m_rasterData[validCellIndex];
        *values = cellValues;
    }
}
template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::getValue(RowColCoor pos, int *nLyrs, T** values) {
	int row = pos.row;
	int col = pos.col;
    int validCellIndex = this->getPosition(row, col);
    if (validCellIndex == -1) {
        *nLyrs = -1;
        *values = NULL;
    } else {
        this->getValue(validCellIndex, nLyrs, values);
    }
}
/************* Output to file functions ***************/

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::outputToFile(string  filename) {
	string filetype = GetUpper(GetSuffix(filename));
	if (StringMatch(filetype, ASCIIExtension))
		outputASCFile(filename);
	else if (StringMatch(filetype, GTiffExtension))
		outputFileByGDAL(filename);
	else
		outputFileByGDAL(ReplaceSuffix(filename, string(GTiffExtension)));
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::_write_ASC_headers(string  filename, map<string, double>& header)
{
	DeleteExistedFile(filename);
	ofstream rasterFile(filename.c_str(), ios::app|ios::out);
	//write file
	int rows = int(header[HEADER_RS_NROWS]);
	int cols = int(header[HEADER_RS_NCOLS]);
	/// write header
	rasterFile << HEADER_RS_NCOLS << " " << cols << endl;
	rasterFile << HEADER_RS_NROWS << " " << rows << endl;
	rasterFile << HEADER_RS_XLL << " " << header[HEADER_RS_XLL] << endl;
	rasterFile << HEADER_RS_YLL << " " << header[HEADER_RS_YLL] << endl;
	rasterFile << HEADER_RS_CELLSIZE << " " << (float) header[HEADER_RS_CELLSIZE] << endl;
	rasterFile << HEADER_RS_NODATA << " " << setprecision(6) << header[HEADER_RS_NODATA] << endl;
	rasterFile.close();
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::outputASCFile(string  filename) {
	/// 1. Is there need to calculate valid position index?
	int count;
	int** position;
	bool outputdirectly = true;
	if (m_calcPositions && m_rasterPositionData != NULL){
		this->getRasterPositionData(count, position);
		outputdirectly = false;
	}
	else if (m_useMaskExtent && m_mask != NULL){
		m_mask->getRasterPositionData(count, position);
		outputdirectly = false;
	}
	/// 2. Write ASC raster headers first (for 1D raster data only)
	if (!m_is2DRaster) this->_write_ASC_headers(filename, m_headers);
	/// 3. Begin to write raster data
	int rows = int(m_headers[HEADER_RS_NROWS]);
	int cols = int(m_headers[HEADER_RS_NCOLS]);
	/// 3.1 2D raster data
	if (m_is2DRaster)
	{
		string prePath = GetPathFromFullName(filename);
		string coreName = GetCoreFileName(filename);
		for (int lyr = 0; lyr < m_nLyrs; lyr++) {
			stringstream oss;
			oss << prePath << coreName << "_" << (lyr + 1) << "." << ASCIIExtension;
			string tmpfilename = oss.str();
			this->_write_ASC_headers(tmpfilename, m_headers);
			// write data
			ofstream rasterFile(tmpfilename.c_str(), ios::app|ios::out);
			int index = 0;
			for (int i = 0; i < rows; ++i) {
				for (int j = 0; j < cols; ++j) {
					if (outputdirectly)
					{
						index = i * cols + j;
						rasterFile << setprecision(6) << m_raster2DData[index][lyr] << " ";
						continue;
					}
					if (index < m_nCells && (position[index][0] == i && position[index][1] == j)) {
						rasterFile << setprecision(6) << m_raster2DData[index][lyr] << " ";
						index++;
					} else rasterFile << setprecision(6) << NODATA_VALUE << " ";
				}
				rasterFile << endl;
			}
			rasterFile.close();
		}
	}
	else{  /// 3.2 1D raster data
		ofstream rasterFile(filename.c_str(), ios::app|ios::out);
		int index = 0;
		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < cols; ++j) {
				if (outputdirectly)
				{
					index = i * cols + j;
					rasterFile << setprecision(6) << m_rasterData[index] << " ";
					continue;
				}
				if (index < m_nCells) {
					if (position[index][0] == i && position[index][1] == j) {
						rasterFile << setprecision(6) << m_rasterData[index] << " ";
						index++;
					} else rasterFile << setprecision(6) << NODATA_VALUE << " ";
				} else rasterFile << setprecision(6) << NODATA_VALUE << " ";
			}
			rasterFile << endl;
		}
		rasterFile.close();
	}
	position = NULL;
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::_write_single_geotiff(string  filename, map<string, double>& header,string  srs, float *values){
	/// 1. Create GeoTiff file driver
	GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	char **papszOptions = poDriver->GetMetadata();
	int nRows = int(header[HEADER_RS_NROWS]);
	int nCols = int(header[HEADER_RS_NCOLS]);
	GDALDataset *poDstDS = poDriver->Create(filename.c_str(), nCols, nRows, 1, GDT_Float32, papszOptions);
	/// 2. Write raster data
	GDALRasterBand *poDstBand = poDstDS->GetRasterBand(1);
	poDstBand->RasterIO(GF_Write, 0, 0, nCols, nRows, values, nCols, nRows, GDT_Float32, 0, 0);
	poDstBand->SetNoDataValue(header[HEADER_RS_NODATA]);
	/// 3. Writer header information
	double geoTrans[6];
	geoTrans[0] = header[HEADER_RS_XLL] - 0.5 * header[HEADER_RS_CELLSIZE];
	geoTrans[1] = header[HEADER_RS_CELLSIZE];
	geoTrans[2] = 0.;
	geoTrans[3] = header[HEADER_RS_YLL] + (nRows - 0.5) * header[HEADER_RS_CELLSIZE];
	geoTrans[4] = 0.;
	geoTrans[5] = -header[HEADER_RS_CELLSIZE];
	poDstDS->SetGeoTransform(geoTrans);
	poDstDS->SetProjection(srs.c_str());
	GDALClose(poDstDS);
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::outputFileByGDAL(string  filename){
	/// 1. Is there need to calculate valid position index?
	int count;
	int** position;
	bool outputdirectly = true;
	if (m_calcPositions && m_rasterPositionData != NULL){
		this->getRasterPositionData(count, position);
		outputdirectly = false;
	}
	else if (m_useMaskExtent && m_mask != NULL){
		m_mask->getRasterPositionData(count, position);
		outputdirectly = false;
	}
	/// 2. Get raster data
	/// 2.1 2D raster data
	T noDataValue = (T) m_headers[HEADER_RS_NODATA];
	int nRows = int(m_headers[HEADER_RS_NROWS]);
	int nCols = int(m_headers[HEADER_RS_NCOLS]);
	if (m_is2DRaster) {
		string prePath = GetPathFromFullName(filename);
		string coreName = GetCoreFileName(filename);
		for (int lyr = 0; lyr < m_nLyrs; lyr++) {
			stringstream oss;
			oss << prePath << coreName << "_" << (lyr + 1) << "." << GTiffExtension;
			string tmpfilename = oss.str();
			float *rasterdata1D = NULL;
			Initialize1DArray(nRows * nCols, rasterdata1D, (float) noDataValue);
			int validnum = 0;
			for (int i = 0; i < nRows; ++i) {
				for (int j = 0; j < nCols; ++j) {
					int index = i * nCols + j;
					if (outputdirectly)
					{
						rasterdata1D[index] = m_raster2DData[index][lyr];
						continue;
					}
					if (validnum < m_nCells && (position[validnum][0] == i && position[validnum][1] == j)) {
						rasterdata1D[index] = m_raster2DData[validnum][lyr];
						validnum++;
					}
				}
			}
			this->_write_single_geotiff(tmpfilename, m_headers, m_srs, rasterdata1D);
			Release1DArray(rasterdata1D);
		}
	}
	else{  /// 3.2 1D raster data
		float *rasterdata1D = NULL;
		bool newbuilddata = true;
		if (outputdirectly){
			if (typeid(T) != typeid(float)){
				/// copyArray() should be an common used function
				rasterdata1D = new float[m_nCells];
				for (int i = 0; i < m_nCells; i++)
					rasterdata1D[i] = (float) m_rasterData[i];
			}
			else{
				rasterdata1D = (float* ) m_rasterData;
				newbuilddata = false;
			}
		}
		else
			Initialize1DArray(nRows * nCols, rasterdata1D, (float) noDataValue);
		int validnum = 0;
		if (!outputdirectly) {
			for (int i = 0; i < nRows; ++i) {
				for (int j = 0; j < nCols; ++j) {
					int index = i * nCols + j;
					if (validnum < m_nCells && (position[validnum][0] == i && position[validnum][1] == j)) {
						rasterdata1D[index] = m_rasterData[validnum];
						validnum++;
					}
				}
			}
		}
		this->_write_single_geotiff(filename, m_headers, m_srs, rasterdata1D);
		if (!newbuilddata) rasterdata1D = NULL;
		else Release1DArray(rasterdata1D);
	}
	position = NULL;
}
#ifdef USE_MONGODB
template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::outputToMongoDB(string  filename, mongoc_gridfs_t *gfs){
	/// 1. Is there need to calculate valid position index?
	int count;
	int** position;
	bool outputdirectly = true;
	if (m_calcPositions && m_rasterPositionData != NULL){
		this->getRasterPositionData(count, position);
		outputdirectly = false;
	}
	else if (m_useMaskExtent && m_mask != NULL){
		m_mask->getRasterPositionData(count, position);
		outputdirectly = false;
	}
	/// 2. Get raster data
	/// 2.1 2D raster data
	T noDataValue = (T) m_headers[HEADER_RS_NODATA];
	int nRows = int(m_headers[HEADER_RS_NROWS]);
	int nCols = int(m_headers[HEADER_RS_NCOLS]);
	int datalength;
	if (m_is2DRaster){
		string coreName = GetCoreFileName(filename);
		T *rasterdata1D = NULL;
		datalength = nRows * nCols * m_nLyrs;
		Initialize1DArray(datalength, rasterdata1D, noDataValue);
		int countindex = 0;
		int rowcolindex = 0;
		int dataIndex = 0;
		for (int i = 0; i < nRows; ++i)
		{
			for (int j = 0; j < nCols; ++j)
			{
				rowcolindex = i * nCols + j;
				for (int k = 0; k < m_nLyrs; k++)
				{
					dataIndex = i * nCols * m_nLyrs + j * m_nLyrs + k;
					if (outputdirectly){
						rasterdata1D[dataIndex] = m_raster2DData[rowcolindex][k];
						continue;
					}
					if (countindex < m_nCells && (position[countindex][0] == i && position[countindex][1] == j))
					{
						rasterdata1D[dataIndex] = m_raster2DData[countindex][k];
						countindex++;
					}
				}
			}
		}
		this->_write_stream_data_as_gridfs(gfs, filename, m_headers, m_srs, rasterdata1D, datalength);
		Release1DArray(rasterdata1D);
	}
	else{  /// 3.2 1D raster data
		float *rasterdata1D = NULL;
		datalength = nRows * nCols;
		if (outputdirectly)
			rasterdata1D = m_rasterData;
		else
			Initialize1DArray(datalength, rasterdata1D, (T) noDataValue);
		int validnum = 0;
		if (!outputdirectly) {
			for (int i = 0; i < nRows; ++i) {
				for (int j = 0; j < nCols; ++j) {
					int index = i * nCols + j;

					if (validnum < m_nCells && (position[validnum][0] == i && position[validnum][1] == j)) {
						rasterdata1D[index] = m_rasterData[validnum];
						validnum++;
					}
				}
			}
		}
		this->_write_stream_data_as_gridfs(gfs, filename, m_headers, m_srs, rasterdata1D, datalength);
		if (outputdirectly) rasterdata1D = NULL;
		else Release1DArray(rasterdata1D);
	}
}
template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::_write_stream_data_as_gridfs(mongoc_gridfs_t* gfs,string  filename, map<string, double>& header,string  srs, T *values, int datalength) {
	bson_t p = BSON_INITIALIZER;
	for (map<string, double>::iterator iter = header.begin(); iter != header.end(); iter++){
		BSON_APPEND_DOUBLE(&p, iter->first.c_str(), iter->second);
	}
	BSON_APPEND_UTF8(&p, HEADER_RS_SRS, srs.c_str());
	char* buf = (char* )values;
	int buflength = datalength * sizeof(T);
	MongoGridFS().writeStreamData(filename, buf, buflength, &p, gfs);

	//mongoc_gridfs_file_t *gfile = NULL;
	//mongoc_gridfs_file_opt_t gopt = {0};
	//gopt.filename = filename.c_str();
	//gopt.content_type = "float"; // TODO, Is the content_type can be any STRING?
	//gopt.metadata = &p;
	//gfile = mongoc_gridfs_create_file(gfs, &gopt);
	//mongoc_iovec_t ovec;
	//ovec.iov_base = buf;
	//ovec.iov_len = buflength;
	//mongoc_gridfs_file_writev(gfile, &ovec, 1, 0);
	//mongoc_gridfs_file_save(gfile);
	//mongoc_gridfs_file_destroy(gfile);

	bson_destroy(&p);
}
#endif
/************* Read functions ***************/
template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::ReadFromFile(string  filename, bool calcPositions /* = true */, clsRasterData<MaskT> *mask /* = NULL */, bool useMaskExtent /* = true */) {
    this->_check_raster_file_exists(filename);
    this->_initialize_raster_class();
    this->_construct_from_single_file(filename, calcPositions, mask, useMaskExtent);
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::ReadASCFile(string  filename, bool calcPositions /* = true */, clsRasterData<MaskT> *mask /* = NULL */, bool useMaskExtent /* = true */) {
	this->_initialize_read_function(filename, calcPositions, mask, useMaskExtent);
	this->_read_asc_file(m_filePathName, &m_headers, &m_rasterData);
	m_srs = "";
	this->_mask_and_calculate_valid_positions();
}
template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::ReadByGDAL(string  filename, bool calcPositions /* = true */, clsRasterData<MaskT> *mask /* = NULL */, bool useMaskExtent /* = true */){
	this->_initialize_read_function(filename, calcPositions, mask, useMaskExtent);
	this->_read_raster_file_by_gdal(m_filePathName, &m_headers, &m_rasterData, &m_srs);
	this->_mask_and_calculate_valid_positions();
}
#ifdef USE_MONGODB
template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::ReadFromMongoDB(mongoc_gridfs_t *gfs,string  filename, bool calcPositions /* = true */, clsRasterData<MaskT> *mask /* = NULL */, bool useMaskExtent /* = true */){
	this->_initialize_read_function(filename, calcPositions, mask, useMaskExtent);
	/// 1. Get stream data and metadata by file name
	MongoGridFS mgfs = MongoGridFS();
	char* buf;
	int length;
	mgfs.getStreamData(filename, buf, length, gfs);
	bson_t *bmeta = mgfs.getFileMetadata(filename, gfs);
	/// 2. Retrieve raster header values
	const char* RASTER_HEADERS[8] = {HEADER_RS_NCOLS, HEADER_RS_NROWS, HEADER_RS_XLL, HEADER_RS_YLL, HEADER_RS_CELLSIZE,
		HEADER_RS_NODATA, HEADER_RS_LAYERS, HEADER_RS_CELLSNUM};
	for (int i = 0; i < 8; i++){
		GetNumericFromBson(bmeta, RASTER_HEADERS[i], m_headers[RASTER_HEADERS[i]]);
	}
	m_srs = GetStringFromBson(bmeta, HEADER_RS_SRS);

	int nRows = (int) m_headers[HEADER_RS_NROWS];
	int nCols = (int) m_headers[HEADER_RS_NCOLS];
	T nodatavalue = (T)m_headers[HEADER_RS_NODATA];
	m_nLyrs = (int) m_headers[HEADER_RS_LAYERS];
	if (m_headers.find(HEADER_RS_CELLSNUM) != m_headers.end())
		m_nCells = (int) m_headers[HEADER_RS_CELLSNUM];
	if (m_nCells < 0)
		m_nCells = length / sizeof(T) / m_nLyrs;

	/// 3. Store data.
	try{
		bool reBuildData = false;
		/// check data length
		if (m_nCells != nRows * nCols){
			if (m_mask == NULL)
				throw ModelException("clsRasterData", "ReadFromMongoDB", 
				"When raster stored in MongoDB is not full-sized, mask data must be provided!\n");
			int nValidMaskNumber = m_mask->getCellNumber();
			if (nValidMaskNumber != m_nCells)
				throw ModelException("clsRasterData", "ReadFromMongoDB", 
				"The cell number must the same between mask and the current raster data.\n");
		}
		else reBuildData = true;
		/// read data directly
		if (m_nLyrs == 1){
			m_rasterData = (T* ) buf;
			m_is2DRaster = false;
		}
		else{
			T *tmpdata = (T *) buf;
			m_raster2DData = new T *[m_nCells];
			for (int i = 0; i < m_nCells; i++){
				m_raster2DData[i] = new T [m_nLyrs];
				for (int j = 0; j < m_nLyrs; j++){
					int idx = i * m_nLyrs + j;
					m_raster2DData[i][j] = tmpdata[idx];
				}
			}
			m_is2DRaster = true;
			tmpdata = NULL;
		}
		buf = NULL;
		if (reBuildData) this->_mask_and_calculate_valid_positions();
	}
	catch (ModelException e){
		cout << e.toString() << endl;
		exit(EXIT_FAILURE);
	}
}
#endif

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::_read_asc_file(string  ascFileName, map<string, double> *header, T**values) {
    StatusMessage(("Read " + ascFileName + "...").c_str());
    ifstream rasterFile(ascFileName.c_str());
    string tmp, xlls, ylls;
    T noData;
    double tempFloat;
    int rows, cols;
	map<string, double> tmpheader;
    /// read header
    rasterFile >> tmp >> cols;
    tmpheader[HEADER_RS_NCOLS] = double(cols);
    rasterFile >> tmp >> rows;
    tmpheader[HEADER_RS_NROWS] = double(rows);
    rasterFile >> xlls >> tempFloat;
    tmpheader[HEADER_RS_XLL] = tempFloat;
    rasterFile >> ylls >> tempFloat;
    tmpheader[HEADER_RS_YLL] = tempFloat;
    rasterFile >> tmp >> tempFloat;
    tmpheader[HEADER_RS_CELLSIZE] = tempFloat;
    rasterFile >> tmp >> noData;
    tmpheader[HEADER_RS_NODATA] = noData;
	m_noDataValue = (T) noData;
    /// default is center, if corner, then:
    if (StringMatch(xlls, "XLLCORNER")) tmpheader[HEADER_RS_XLL] += 0.5 * tmpheader[HEADER_RS_CELLSIZE];
    if (StringMatch(ylls, "YLLCORNER")) tmpheader[HEADER_RS_YLL] += 0.5 * tmpheader[HEADER_RS_CELLSIZE];

    /// get all raster values (i.e., include NODATA_VALUE, m_excludeNODATA = False)
	T *tmprasterdata = new T[rows * cols];
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            rasterFile >> tempFloat;
            tmprasterdata[i * cols + j] = (T) tempFloat;
        }
    }
    rasterFile.close();
	/// returned parameters
	*header = tmpheader;
	*values = tmprasterdata;
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::_read_raster_file_by_gdal(string  filename, map<string, double> *header, T**values, string* srs /* = NULL */){
	StatusMessage(("Read " + filename + "...").c_str());
	GDALDataset *poDataset = (GDALDataset *) GDALOpen(filename.c_str(), GA_ReadOnly);
	if (poDataset == NULL) {
		throw ModelException("clsRasterData", "ReadRasterByGDAL", "Open file " + filename + " failed.\n");
	}
	//cout<<poDataset->GetRasterCount()<<endl;
	GDALRasterBand *poBand = poDataset->GetRasterBand(1);
	map<string, double> tmpheader;
	int nRows = poBand->GetYSize();
	int nCols = poBand->GetXSize();
	tmpheader[HEADER_RS_NCOLS] = (double) nCols;
	tmpheader[HEADER_RS_NROWS] = (double) nRows;
	tmpheader[HEADER_RS_NODATA] = (double) poBand->GetNoDataValue();
	double adfGeoTransform[6];
	poDataset->GetGeoTransform(adfGeoTransform);
	tmpheader[HEADER_RS_CELLSIZE] = adfGeoTransform[1];
	tmpheader[HEADER_RS_XLL] = adfGeoTransform[0] + 0.5 * tmpheader[HEADER_RS_CELLSIZE];
	tmpheader[HEADER_RS_YLL] = adfGeoTransform[3] + (tmpheader[HEADER_RS_NROWS] - 0.5) * adfGeoTransform[5];
	string tmpsrs = string(poDataset->GetProjectionRef());
	/// get all raster values (i.e., include NODATA_VALUE)
	T *tmprasterdata = new T[nRows * nCols];

	GDALDataType dataType = poBand->GetRasterDataType();
	if (dataType == GDT_Float32) {
		float *pData = (float *) CPLMalloc(sizeof(float) * nCols * nRows);
		poBand->RasterIO(GF_Read, 0, 0, nCols, nRows, pData, nCols, nRows, GDT_Float32, 0, 0);
		for (int i = 0; i < nRows; ++i) {
			for (int j = 0; j < nCols; ++j) {
				int index = i * nCols + j;
				tmprasterdata[index] = (T) pData[index];
			}
		}
		CPLFree(pData);
	} else if (dataType == GDT_Int32) {
		int *pData = (int *) CPLMalloc(sizeof(int) * nCols * nRows);
		poBand->RasterIO(GF_Read, 0, 0, nCols, nRows, pData, nCols, nRows, GDT_Int32, 0, 0);
		for (int i = 0; i < nRows; ++i) {
			for (int j = 0; j < nCols; ++j) {
				int index = i * nCols + j;
				tmprasterdata[index] = (T) pData[index];
			}
		}
		CPLFree(pData);
	} else {
		throw ModelException("clsRasterData", "ReadRasterByGDAL",
			"The data type of " + filename + " is neither GDT_Float32 nor GDT_Int32.");
	}
	GDALClose(poDataset);

	/// returned parameters
	*header = tmpheader;
	*values = tmprasterdata;
	srs = &tmpsrs;
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::_add_other_layer_raster_data(int row, int col, int cellidx, int lyr, map<string, double> lyrheader, T *lyrdata){
	int tmpcols = (int) lyrheader[HEADER_RS_NCOLS];
	double *tmpXY = this->getCoordinateByRowCol(row, col);
	/// get current raster layer's value by XY
	int* tmpPosition = this->getPositionByCoordinate(tmpXY[0], tmpXY[1], &lyrheader);
	if (tmpPosition[0] == -1 || tmpPosition[1] == -1)
		m_raster2DData[cellidx][lyr] = m_noDataValue;
	else
		m_raster2DData[cellidx][lyr] = lyrdata[tmpPosition[0] * tmpcols + tmpPosition[1]];
}
template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::Copy(clsRasterData &orgraster){
	m_filePathName = orgraster.getFilePath();
	m_coreFileName = orgraster.getCoreName();
	m_nCells = orgraster.getCellNumber();
	m_noDataValue = (T) orgraster.getNoDataValue();
	if (orgraster.is2DRaster()){
		m_nLyrs = orgraster.getLayers();
		Initialize2DArray(m_nCells, m_nLyrs, m_raster2DData, orgraster.get2DRasterDataPointer());
	}
	else {
		m_rasterData = NULL;
		Initialize1DArray(m_nCells, m_rasterData, orgraster.getRasterDataPointer());
	}
	m_mask = orgraster.getMask();
	m_calcPositions = orgraster.PositionsCalculated();
	if (m_calcPositions)
		Initialize2DArray(m_nCells, 2, m_rasterPositionData, orgraster.getRasterPositionDataPointer());
	m_useMaskExtent = orgraster.MaskExtented();
	m_statisticsCalculated = orgraster.StatisticsCalculated();
	if (m_statisticsCalculated){
		map<string, double> *stats = orgraster.getStatistics();
		for (map<string, double>::iterator iter = (*stats).begin(); iter != (*stats).end(); iter++) {
			m_statsMap[iter->first] = iter->second;
		}
	}
	this->copyHeader(orgraster.getRasterHeader());
}
template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::replaceNoData(T replacedv){
	if (m_is2DRaster && m_raster2DData != NULL) {
#pragma omp parallel for
		for (int i = 0; i < m_nCells; i++){
			for (int lyr = 0; lyr < m_nLyrs; lyr++){
				if (FloatEqual(m_raster2DData[i][lyr], m_noDataValue))
					m_raster2DData[i][lyr] = replacedv;
			}
		}
	} 
	else if (m_rasterData != NULL) {
#pragma omp parallel for
		for (int i = 0; i < m_nCells; i++){
			if (FloatEqual(m_rasterData[i], m_noDataValue))
				m_rasterData[i] = replacedv;
		}
	}
}
/************* Utility functions ***************/

template<typename T, typename MaskT>
double* clsRasterData<T, MaskT>::getCoordinateByRowCol(int row, int col) {
    double xllCenter = this->getXllCenter();
    double yllCenter = this->getYllCenter();
    double cs = this->getCellWidth();
    double nrows = this->getRows();
    double* xy = new double[2];
    xy[0] = xllCenter + col * cs;
    xy[1] = yllCenter + (nrows - row - 1) * cs;
    return xy;
}

template<typename T, typename MaskT>
int* clsRasterData<T, MaskT>::getPositionByCoordinate(double x, double y, map<string, double> *header /* = NULL */){
	if (header == NULL)
		header = &m_headers;
	double xllCenter = (*header)[HEADER_RS_XLL];
    double yllCenter = (*header)[HEADER_RS_YLL];
    float dx = (float)(*header)[HEADER_RS_CELLSIZE];
    float dy = dx;
    int nRows = (int)(*header)[HEADER_RS_NROWS];
    int nCols = (int)(*header)[HEADER_RS_NCOLS];

    int *position = new int[2];

    double xmin = xllCenter - dx / 2.;
    double xMax = xmin + dx * nCols;

    double ymin = yllCenter - dy / 2.;
    double yMax = ymin + dy * nRows;
    if ((x > xMax || x < xllCenter) || (y > yMax || y < yllCenter)) {
        position[0] = -1;
        position[1] = -1;
    } else{
        position[0] = (int) ((yMax - y) / dy); //calculate from ymax
        position[1] = (int) ((x - xmin) / dx); //calculate from xmin
    }
    return position;
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::copyHeader(map<string, double> *maskHeader) {
	for (map<string, double>::iterator iter = (*maskHeader).begin(); iter != (*maskHeader).end(); iter++) {
		m_headers[iter->first] = iter->second;
	}
    //m_headers[HEADER_RS_NCOLS] = (*maskHeader)[HEADER_RS_NCOLS];
    //m_headers[HEADER_RS_NROWS] = (*maskHeader)[HEADER_RS_NROWS];
    //m_headers[HEADER_RS_NODATA] = (*maskHeader)[HEADER_RS_NODATA];
    //m_headers[HEADER_RS_CELLSIZE] = (*maskHeader)[HEADER_RS_CELLSIZE];
    //m_headers[HEADER_RS_XLL] = (*maskHeader)[HEADER_RS_XLL];
    //m_headers[HEADER_RS_YLL] = (*maskHeader)[HEADER_RS_YLL];
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::_calculate_valid_positions_from_grid_data()
{
	int oldcellnumber = m_nCells;
	/// initial vectors
	vector<T> values;
	vector<vector<T> > values2D; /// store layer 2~n
	vector<int> positionRows;
	vector<int> positionCols;
	int nrows = (int) m_headers[HEADER_RS_NROWS];
	int ncols = (int) m_headers[HEADER_RS_NCOLS];
	/// get all valid values (i.e., exclude NODATA_VALUE)
	for (int i = 0; i < nrows; ++i) {
		for (int j = 0; j < ncols; ++j) {
			int idx = i * ncols + j;
			T tempFloat;
			if (m_is2DRaster)
				tempFloat = m_raster2DData[idx][0];
			else
				tempFloat = m_rasterData[idx];
			if (FloatEqual(double(tempFloat), double(m_noDataValue))) continue;
			values.push_back(tempFloat);
			if (m_is2DRaster && m_nLyrs > 1){
				vector<T> tmpv(m_nLyrs - 1);
				for (int lyr = 1; lyr < m_nLyrs; lyr++)
					tmpv[lyr - 1] = m_raster2DData[idx][lyr];
				values2D.push_back(tmpv);
			}
			positionRows.push_back(i);
			positionCols.push_back(j);
		}
	}
	/// swap vector to save memory
	vector<T>(values).swap(values);
	if (m_is2DRaster && m_nLyrs > 1)
		vector<vector<T> >(values2D).swap(values2D);
	vector<int>(positionRows).swap(positionRows);
	vector<int>(positionCols).swap(positionCols);
	/// reCreate raster data array
	m_nCells = (int) values.size();
	m_headers[HEADER_RS_CELLSNUM] = m_nCells;
	if (m_is2DRaster){
		Release2DArray(oldcellnumber, m_raster2DData);
		Initialize2DArray(m_nCells, m_nLyrs, m_raster2DData, m_noDataValue);
	}
	else{
		Release1DArray(m_rasterData);
		Initialize1DArray(m_nCells, m_rasterData, m_noDataValue);
	}
	
	/// m_rasterPositionData is NULL till now.
	m_rasterPositionData = new int *[m_nCells];
	for (int i = 0; i < m_nCells; ++i) {
		if (m_is2DRaster){
			m_raster2DData[i][0] = values.at(i);
			if (m_nLyrs > 1){
				for (int lyr = 1; lyr < m_nLyrs; lyr++)
					m_raster2DData[i][lyr] = values2D[i][lyr - 1];
			}
		}
		else
			m_rasterData[i] = values.at(i);
		m_rasterPositionData[i] = new int[2];
		m_rasterPositionData[i][0] = positionRows.at(i);
		m_rasterPositionData[i][1] = positionCols.at(i);
	}
	m_calcPositions = true;
}

template<typename T, typename MaskT>
void clsRasterData<T, MaskT>::_mask_and_calculate_valid_positions(){
	int oldcellnumber = m_nCells;
	if (m_mask != NULL) {
		/// 1. Get new values and positions according to Mask's position data
		/// initial vectors
		vector<T> values;
		vector<vector<T> > values2D; /// store layer 2~n data (excluding the first layerS)
		vector<int> positionRows;
		vector<int> positionCols;
		int cols = (int) m_headers[HEADER_RS_NCOLS];
		if (m_mask->PositionsCalculated())
		{
			/// Get the position data from mask
			int nValidMaskNumber;
			int **validPosition = NULL;
			m_mask->getRasterPositionData(nValidMaskNumber, validPosition);
			/// Get the valid data according to coordinate
			for (int i = 0; i < nValidMaskNumber; ++i) {
				int tmpRow = validPosition[i][0];
				int tmpCol = validPosition[i][1];
				double *tmpXY = m_mask->getCoordinateByRowCol(tmpRow, tmpCol);
				/// get current raster value by XY
				int* tmpPosition = this->getPositionByCoordinate(tmpXY[0], tmpXY[1]);
				if (tmpPosition[0] == -1 || tmpPosition[1] == -1) continue;

				T tmpValue;
				if (m_is2DRaster){
					tmpValue = m_raster2DData[tmpPosition[0] * cols + tmpPosition[1]][0];
					if (m_nLyrs > 1){
						vector<T> tmpValues(m_nLyrs - 1);
						for (int lyr = 1; lyr < m_nLyrs; lyr++)
							tmpValues[lyr - 1] = m_raster2DData[tmpPosition[0] * cols + tmpPosition[1]][lyr];
						values2D.push_back(tmpValues);
					}
				}
				else{
					tmpValue = m_rasterData[tmpPosition[0] * cols + tmpPosition[1]];
				}
				values.push_back(tmpValue);
				positionRows.push_back(tmpRow);
				positionCols.push_back(tmpCol);
				/// release temporary array
				Release1DArray(tmpXY);
				Release1DArray(tmpPosition);
			}
		} 
		else
		{
			int maskRows = m_mask->getRows();
			int maskCols = m_mask->getCols();
			for (int i = 0; i < maskRows; ++i) {
				for (int j = 0; j < maskCols; ++j) {
					/// check mask data
					RowColCoor pos = {i, j};
					if (FloatEqual(m_mask->getValue(pos), m_mask->getNoDataValue())) continue;
					double *tmpXY = m_mask->getCoordinateByRowCol(i, j);
					/// get current raster value by XY
					int* tmpPosition = this->getPositionByCoordinate(tmpXY[0], tmpXY[1]);
					if (tmpPosition[0] == -1 || tmpPosition[1] == -1) continue;
					T tmpValue;
					if (m_is2DRaster){
						tmpValue = m_raster2DData[tmpPosition[0] * cols + tmpPosition[1]][0];
						if (m_nLyrs > 1){
							vector<T> tmpValues(m_nLyrs - 1);
							for (int lyr = 1; lyr < m_nLyrs; lyr++)
								tmpValues[lyr - 1] = m_raster2DData[tmpPosition[0] * cols + tmpPosition[1]][lyr];
							values2D.push_back(tmpValues);
						}
					}
					else{
						tmpValue = m_rasterData[tmpPosition[0] * cols + tmpPosition[1]];
					}
					positionRows.push_back(i);
					positionCols.push_back(j);
					/// release temporary array
					Release1DArray(tmpXY);
					Release1DArray(tmpPosition);
				}
			}
		}
		/// swap vector to save memory
		if (m_is2DRaster && m_nLyrs > 1)
			vector<vector<T> >(values2D).swap(values2D);
		vector<T>(values).swap(values);
		vector<int>(positionRows).swap(positionRows);
		vector<int>(positionCols).swap(positionCols);
		/// 2. Use the extent of Mask data or not.
		/// overwrite header information by Mask's
		this->copyHeader(m_mask->getRasterHeader());
		/// avoid to assign the Mask's NODATA
		m_headers[HEADER_RS_NODATA] = m_noDataValue; 
		m_srs = string(m_mask->getSRS());
		/// 2.1 Is the valid grid extent same as the mask data?
		bool sameExtentWithMask = true;
		/// reCalculate headers
		vector<int>::iterator maxRow = max_element(positionRows.begin(), positionRows.end());
		vector<int>::iterator minRow = min_element(positionRows.begin(), positionRows.end());
		vector<int>::iterator maxCol = max_element(positionCols.begin(), positionCols.end());
		vector<int>::iterator minCol = min_element(positionCols.begin(), positionCols.end());
		int max_row = *maxRow;
		int min_row = *minRow;
		int max_col = *maxCol;
		int min_col = *minCol;
		int newRows = max_row - min_row + 1;
		int newCols = max_col - min_col + 1;
		if (newRows != m_mask->getRows() || newCols != m_mask->getCols())
			sameExtentWithMask = false;
		/// 2.2 If do not use the extent of mask, the headers will be recalculated.
		if(!m_useMaskExtent && !sameExtentWithMask){ 
			m_headers[HEADER_RS_NCOLS] = double(newCols);
			m_headers[HEADER_RS_NROWS] = double(newRows);
			m_headers[HEADER_RS_XLL] += min_col * m_headers[HEADER_RS_CELLSIZE];
			m_headers[HEADER_RS_YLL] += (m_mask->getRows() - 1 - max_row) * m_headers[HEADER_RS_CELLSIZE];
			/// clean redundant values (i.e., NODATA)
			vector<int>::iterator rit = positionRows.begin();
			vector<int>::iterator cit = positionCols.begin();
			typename vector<T>::iterator vit = values.begin();
			typename vector<vector<T> >::iterator data2dit = values2D.begin();
			for (typename vector<T>::iterator it = values.begin(); it != values.end();)
			{
				int idx = distance(vit, it);
				int tmpr = positionRows.at(idx);
				int tmpc = positionCols.at(idx);
				
				if (tmpr > max_row || tmpr < min_row || tmpc > max_col || tmpc < min_col
					|| FloatEqual((T)*it, m_noDataValue))
				{
					it = values.erase(it);
					if (m_is2DRaster && m_nLyrs > 1){
						values2D.erase(data2dit + idx);
						data2dit = values2D.begin();
					}
					positionCols.erase(cit + idx);
					positionRows.erase(rit + idx);
					/// reset the iterators
					rit = positionRows.begin();
					cit = positionCols.begin();
					vit = values.begin();
				}
				else{
					/// get new column and row number
					positionRows[idx] -= min_row;
					positionCols[idx] -= min_col;
					++it;
				}
			}
			/// swap vector to save memory
			vector<T>(values).swap(values);
			if (m_is2DRaster && m_nLyrs > 1) vector<vector<T> >(values2D).swap(values2D);
			vector<int>(positionRows).swap(positionRows);
			vector<int>(positionCols).swap(positionCols);
		}
		/// 3. Create new raster data, considering valid data only or not.
		bool tmpStorePositions = true;
		if (m_mask != NULL && m_mask->PositionsCalculated() && m_useMaskExtent && sameExtentWithMask){
			m_mask->getRasterPositionData(m_nCells, m_rasterPositionData);
			tmpStorePositions = false;
		}
		if (m_calcPositions)
		{
			m_nCells = (int) values.size();
			m_headers[HEADER_RS_CELLSNUM] = m_nCells;
			if (m_is2DRaster && m_raster2DData != NULL) {
				Release2DArray(oldcellnumber, m_raster2DData);
				Initialize2DArray(m_nCells, m_nLyrs, m_raster2DData, m_noDataValue);
			}
			else if (m_rasterData != NULL) {
				Release1DArray(m_rasterData);
				Initialize1DArray(m_nCells, m_rasterData, m_noDataValue);
			}
			if (tmpStorePositions) m_rasterPositionData = new int *[m_nCells];
			//SetDefaultOpenMPThread();
#pragma omp parallel for
			for (int i = 0; i < m_nCells; ++i) {
				if (tmpStorePositions) {
					m_rasterPositionData[i] = new int[2];
					m_rasterPositionData[i][0] = positionRows.at(i);
					m_rasterPositionData[i][1] = positionCols.at(i);
				}
				if (m_is2DRaster) {
					m_raster2DData[i][0] = values.at(i);
					if (m_nLyrs > 1){
						for (int lyr = 1; lyr < m_nLyrs; lyr++){
							m_raster2DData[i][lyr] = values2D[i][lyr - 1];
						}
					}
				}
				else {
					m_rasterData[i] = values.at(i);
				}
			}
		}
		else{ // reStore all cell values to m_rasterData, and the position data in case of later usage.
			int ncols = (int) m_headers[HEADER_RS_NCOLS];
			int nrows = (int) m_headers[HEADER_RS_NROWS];
			m_nCells = ncols * nrows;
			m_headers[HEADER_RS_CELLSNUM] = m_nCells;
			if (m_rasterData != NULL) Release1DArray(m_rasterData);
			Initialize1DArray(m_nCells, m_rasterData, m_noDataValue);
			if (m_is2DRaster && m_raster2DData != NULL) {
				Release2DArray(oldcellnumber, m_raster2DData);
				Initialize2DArray(m_nCells, m_nLyrs, m_raster2DData, m_noDataValue);
			}
			else if (m_rasterData != NULL) {
				Release1DArray(m_rasterData);
				Initialize1DArray(m_nCells, m_rasterData, m_noDataValue);
			}
			//SetDefaultOpenMPThread();
#pragma omp parallel for
			for (int k = 0; k < positionRows.size(); ++k) {
				int newIdx = positionRows.at(k) * ncols + positionCols.at(k);
				if (m_is2DRaster){
					m_raster2DData[newIdx][0] = values.at(k);
					if (m_nLyrs > 1){
						for (int lyr = 1; lyr < m_nLyrs; lyr++){
							m_raster2DData[newIdx][lyr] = values2D[k][lyr - 1];
						}
					}
				}
				else
					m_rasterData[newIdx] = values.at(k);
			}
		}
	}
	else{ // No mask data is provided.
		if (m_calcPositions)
		{
			_calculate_valid_positions_from_grid_data();
		}
		else{
			m_nCells = this->getRows()* this->getCols();
			m_headers[HEADER_RS_CELLSNUM] = m_nCells;
			// do nothing
		}
	}
}
#endif
