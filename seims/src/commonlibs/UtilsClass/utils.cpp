#ifndef CLS_UTILS
#include "utils.h"

/************ utilsTime ******************/
utilsTime::utilsTime(void) {}

utilsTime::~utilsTime(void) {}

double utilsTime::TimeCounting()
{
#ifdef windows
	LARGE_INTEGER li;
	if (QueryPerformanceFrequency(&li)) /// CPU supported
	{
		double PCFreq = 0.;
		PCFreq = double(li.QuadPart);
		QueryPerformanceCounter(&li);
		return (double)li.QuadPart / PCFreq; // seconds
	}
	else
		return (double)clock() / CLK_TCK;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL); 
	return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.;
#endif
}

string utilsTime::ConvertToString(const time_t *date)
{
	struct tm dateInfo;
#ifdef windows
	localtime_s(&dateInfo, date);
#else
	localtime_r(date, &dateInfo);
#endif
	if (dateInfo.tm_isdst > 0)
		dateInfo.tm_hour -= 1;

	char dateString[11];
	strftime(dateString, 11, "%Y-%m-%d", &dateInfo);

	return string(dateString);
}

string utilsTime::ConvertToString2(const time_t *date)
{
	struct tm dateInfo;
#ifdef windows
	localtime_s(&dateInfo, date);
#else
	localtime_r(date, &dateInfo);
#endif
	if (dateInfo.tm_isdst > 0)
	{
		if (dateInfo.tm_hour != 0)
			dateInfo.tm_hour -= 1;
		else
		{
			dateInfo.tm_hour = 23;
			dateInfo.tm_mday -= 1;
			if (dateInfo.tm_mday == 0)
			{
				dateInfo.tm_mon -= 1;

				if (dateInfo.tm_mon == 0)
				{
					dateInfo.tm_year -= 1;
					dateInfo.tm_mon = 12;
					dateInfo.tm_mday = 31;
				}
				else
				{
					if (utilsTime::isLeapYear(dateInfo.tm_year))
						dateInfo.tm_mday = daysOfMonth[dateInfo.tm_mon] + 1;
					else
						dateInfo.tm_mday = daysOfMonth[dateInfo.tm_mon];
				}

			}
		}
	}
	char dateString[30];
	strftime(dateString, 30, "%Y-%m-%d %X", &dateInfo);

	string s(dateString);
	return s;
}

time_t utilsTime::ConvertToTime(string strDate, string const& format, bool includeHour)
{
	struct tm *timeinfo;
	time_t t;
	int yr;
	int mn;
	int dy;
	int hr = 0;

	try
	{
		if (includeHour)
		{
			StringScanf(strDate.c_str(), format.c_str(), &yr, &mn, &dy, &hr);
//#ifdef MSVC
//			sscanf_s(strDate.c_str(), format.c_str(), &yr, &mn, &dy, &hr);
//#else
//			sscanf(strDate.c_str(), format.c_str(), &yr, &mn, &dy, &hr);
//#endif
		}
		else
		{
			StringScanf(strDate.c_str(), format.c_str(), &yr, &mn, &dy);
//#ifdef MSVC
//			sscanf_s(strDate.c_str(), format.c_str(), &yr, &mn, &dy);
//#else
//			sscanf(strDate.c_str(), format.c_str(), &yr, &mn, &dy);
//#endif
		}

		timeinfo = new struct tm;
		timeinfo->tm_year = yr - 1900;
		timeinfo->tm_mon = mn - 1;
		timeinfo->tm_mday = dy;
		timeinfo->tm_hour = hr;
		timeinfo->tm_min = 0;
		timeinfo->tm_sec = 0;
		timeinfo->tm_isdst = false;
		t = mktime(timeinfo);
	}
	catch (...)
	{
		throw;
	}

	return t;
}

time_t utilsTime::ConvertToTime2(string const& strDate, const char *format, bool includeHour)
{
	time_t t;
	int yr;
	int mn;
	int dy;
	int hr = 0;
	int m = 0;
	int s = 0;

	try
	{
		if (includeHour)
		{
			StringScanf(strDate.c_str(), format, &yr, &mn, &dy, &hr, &m, &s);
//#ifdef MSVC
//			sscanf_s(strDate.c_str(), format, &yr, &mn, &dy, &hr, &m, &s);
//#else
//			sscanf(strDate.c_str(), format, &yr, &mn, &dy, &hr, &m, &s);
//#endif
		}
		else
		{
			StringScanf(strDate.c_str(), format, &yr, &mn, &dy);
//#ifdef MSVC
//			sscanf_s(strDate.c_str(), format, &yr, &mn, &dy);
//#else
//			sscanf(strDate.c_str(), format, &yr, &mn, &dy);
//#endif
		}

		struct tm timeinfo;
		timeinfo.tm_year = yr - 1900;
		timeinfo.tm_mon = mn - 1;
		timeinfo.tm_mday = dy;
		timeinfo.tm_hour = hr;
		timeinfo.tm_min = m;
		timeinfo.tm_sec = s;
		timeinfo.tm_isdst = false;
		t = mktime(&timeinfo);
	}
	catch (...)
	{
		cout << "Error in ConvertToTime2.\n";
		throw;
	}

	return t;
}

time_t utilsTime::ConvertYMDToTime(int &year, int &month, int &day)
{
	time_t t;
	try
	{
		struct tm timeinfo;
		timeinfo.tm_year = year - 1900;
		timeinfo.tm_mon = month - 1;
		timeinfo.tm_mday = day;
		timeinfo.tm_isdst = false;
		t = mktime(&timeinfo);
	}
	catch (...)
	{
		cout << "Error in ConvertYMDToTime.\n";
		throw;
	}
	return t;
}

int utilsTime::GetDateInfoFromTimet(time_t *t, int *year, int *month, int *day)
{
	struct tm dateInfo;
#ifdef windows
	localtime_s(&dateInfo, t);
#else
	localtime_r(t, &dateInfo);
#endif
	if (dateInfo.tm_isdst > 0)
		dateInfo.tm_hour -= 1;

	char dateString[30];
	strftime(dateString, 30, "%Y-%m-%d %X", &dateInfo);
	int hour, min, sec;
	StringScanf(dateString, "%4d-%2d-%2d %2d:%2d:%2d", year, month, day, &hour, &min, &sec);
//#ifdef MSVC
//	sscanf_s(dateString, "%4d-%2d-%2d %2d:%2d:%2d", year, month, day, &hour, &min, &sec);
//#else
//	sscanf(dateString, "%4d-%2d-%2d %2d:%2d:%2d", year, month, day, &hour, &min, &sec);
//#endif
	return 0;
}

void utilsTime::LocalTime(time_t date, struct tm *t)
{
#ifdef windows
	localtime_s(t, &date);
#else
	localtime_r(&date, t);
#endif
}
/************ utilsString ****************/
utilsString::utilsString(void) {}

utilsString::~utilsString(void) {}

string utilsString::GetUpper(string const& str)
{
	string strTmp1 = string(str);
	for (int j = 0; j < (int) strTmp1.length(); j++) strTmp1[j] = (char) toupper(strTmp1[j]);
	return strTmp1;
}

void utilsString::TrimSpaces(string& str)
{
	// Find the first character position after excluding leading blank spaces
	size_t startpos = str.find_first_not_of(" \t"); 
	// Find the first character position from reverse
	size_t endpos = str.find_last_not_of(" \t");
	// if all spaces or empty return an empty string
	if ((string::npos == startpos) || (string::npos == endpos))
		str = "";
	else
		str = str.substr(startpos, endpos - startpos + 1);
}

vector<string> utilsString::SplitString(string const& item, char delimiter)
{
	istringstream iss(item);
	vector<string> tokens;

	std::string field;
	while (std::getline(iss, field, delimiter))
	{
		tokens.push_back(field);
	}
	return tokens;
}



vector<int> utilsString::SplitStringForInt(string const& item, char delimiter)
{
	vector<string> valueStrs = utilsString::SplitString(item, delimiter);
	vector<int> values;
	for (vector<string>::iterator it = valueStrs.begin(); it != valueStrs.end(); it++)
		values.push_back(atoi((*it).c_str()));
	vector<int>(values).swap(values);
	return values;
}

vector<float> utilsString::SplitStringForFloat(string const& item, char delimiter)
{
	vector<string> valueStrs = utilsString::SplitString(item, delimiter);
	vector<float> values(valueStrs.size());
	for (vector<string>::iterator it = valueStrs.begin(); it != valueStrs.end(); it++)
		values.push_back((float)atof((*it).c_str()));
	return values;
}

vector<string> utilsString::SplitString(string const& item)
{
	istringstream iss(item);
	vector<string> tokens;

	std::string field;
	iss >> field;
	while (!iss.eof())
	{
		tokens.push_back(field);
		iss >> field;
	}
	tokens.push_back(field);

	return tokens;
}

bool utilsString::StringMatch(const char *a, const char *b)
{
#ifdef windows
	return _stricmp(a, b) == 0;
#else
	return strcasecmp(a, b) == 0;
#endif
}

bool utilsString::StringMatch(string const& text1, string const& text2)
{
	// convert the key to UPPERCASE for comparison
	string strTmp1 = utilsString::GetUpper(text1);
	string strTmp2 = utilsString::GetUpper(text2);
	return (strTmp1 == strTmp2);
}

string& utilsString::trim(string &s)
{
	if (s.empty())
		return s;
	s.erase(0, s.find_first_not_of(" \n\r\t"));
	return s.erase(s.find_last_not_of(" \n\r\t") + 1);
}


/************ utilsArray *****************/
utilsArray::utilsArray(void){}

utilsArray::~utilsArray(void){}



void utilsArray::Output1DArrayToTxtFile(int n, float *data, const char *filename)
{
	ofstream ofs(filename);
	for (int i = 0; i < n; ++i)
		ofs << data[i] << "\n";
	ofs.close();
}

void utilsArray::Output2DArrayToTxtFile(int nRows, int nCols, float **data, const char *filename)
{
	ofstream ofs(filename);
	for (int i = 0; i < nRows; ++i)
	{
		for (int j = 0; j < nCols; ++j)
		{
			ofs << data[i][j] << "\t";
		}
		ofs << "\n";
	}
	ofs.close();
}

void utilsArray::Read1DArrayFromTxtFile(const char *filename, int &nRows, float *&data)
{
	ifstream ifs(filename);
	string tmp;
	ifs >> tmp >> nRows;
	data = new float[nRows];
	for (int i = 0; i < nRows; i++)
	{
		ifs >> data[i];
	}
	ifs.close();
}

void utilsArray::Read2DArrayFromTxtFile(const char *filename, int &nRows, float **&data)
{
	ifstream ifs(filename);
	string tmp;
	ifs >> tmp >> nRows;
	data = new float *[nRows];
	int n;
	for (int i = 0; i < nRows; i++)
	{
		ifs >> n;
		data[i] = new float[n + 1];
		data[i][0] = (float) n;
		for (int j = 1; j <= n; j++)
			ifs >> data[i][j];
	}
	ifs.close();
}

void utilsArray::Read2DArrayFromString(const char *s, int &nRows, float **&data)
{
	istringstream ifs(s);

	string tmp;
	ifs >> tmp >> nRows;
	data = new float *[nRows];
	int n;
	for (int i = 0; i < nRows; i++)
	{
		ifs >> n;
		data[i] = new float[n + 1];
		data[i][0] = (float) n;
		for (int j = 1; j <= n; j++)
			ifs >> data[i][j];
	}
}
/************ utilsMath ******************/
utilsMath::utilsMath(void){}

utilsMath::~utilsMath(void){}

float utilsMath::Expo(float xx, float upper /* = 20.f */, float lower /* = -20.f */){
	if (xx < lower) xx = lower;
	if (xx > upper) xx = upper;
	return exp(xx);
}

float utilsMath::Power(float a, float n)
{
	if (a >= 0.f)
		return pow(a, n);
	else
		return -pow(-a, n);
}


/************ utilsFileIO ******************/
utilsFileIO::utilsFileIO(void){}

utilsFileIO::~utilsFileIO(void){}

#ifndef windows
int utilsFileIO::copyfile_unix(const char* srcfile, const char* dstfile)
{
	struct stat file;
	if(stat(srcfile, &file) == -1)
	{
		printf("Can not get info of file %s in function: copyfile_linux.", srcfile);
		return -1;
	}

	int f1 = open(srcfile, O_RDONLY);
	if(f1 == -1)
	{
		printf("Can not read file %s in function: copyfile_linux.", srcfile);
		return -1;
	}

	int f2 = creat(dstfile, file.st_mode);
	if(f2 == -1)
	{
		printf("Can not create file %s in function: copyfile_linux.", srcfile);
		close(f1);
		return -1;
	}

	char buf[200] = "";
	int size = 0;
	while((size = read(f1, buf, 200)) != 0)
	{
		if(write(f2, buf, size) != size)
		{
			printf("Write error to file: %s function: copyfile_linux.", dstfile);
			close(f1);
			close(f2);
			return -1;
		}
	}

	close(f1);
	close(f2);

	return 0;
}
#endif

bool utilsFileIO::FileExists(string const& FileName)
{
#ifdef windows
	struct _finddata_t fdt;
	intptr_t ptr = _findfirst(FileName.c_str(), &fdt);
	bool found = (ptr != -1);
	_findclose(ptr);
	return found;
#else
	if(access(FileName.c_str(), F_OK) == 0)
		return true;
	else
		return false;
#endif
}
bool utilsFileIO::PathExists(string const& fullpath){
	const char *path = fullpath.c_str();
	bool isExists;
#ifdef windows
	struct _stat fileStat;
	isExists = (_stat(path, &fileStat) == 0) && (fileStat.st_mode & _S_IFDIR);
#else
	struct stat fileStat;
	isExists = (stat(path, &fileStat) == 0) && S_ISDIR(fileStat.st_mode);
#endif
	return isExists;
}
int utilsFileIO::DeleteExistedFile(string const& filepath)
{
	if (utilsFileIO::FileExists(filepath))
		return remove(filepath.c_str());
	else
		return -1;
}

int utilsFileIO::FindFiles(const char *lpPath, const char *expression, vector<string> &vecFiles)
{
#ifdef windows
	char szFind[MAX_PATH];
	stringcpy(szFind, lpPath);
	stringcat(szFind, "\\");
	stringcat(szFind, expression);

	WIN32_FIND_DATA findFileData;
	HANDLE hFind = ::FindFirstFile(szFind, &findFileData);
	if (INVALID_HANDLE_VALUE == hFind)
		return -1;
	do
	{
		if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		char fullpath[MAX_PATH];
		stringcpy(fullpath, lpPath);
		stringcat(fullpath, "\\");
		stringcat(fullpath, findFileData.cFileName);

		vecFiles.push_back(fullpath);

	} while (::FindNextFile(hFind, &findFileData));
#else
	DIR *dir = opendir(lpPath);
	//cout<<"Find existed files ..."<<endl;
    if (dir)
    {
        struct dirent* hFile;
        errno = 0;
        while ((hFile = readdir(dir)) != NULL )
        {
            if (!strcmp(hFile->d_name, ".")) continue;
            if (!strcmp(hFile->d_name, "..")) continue;
            
            // in linux hidden files all start with '.'
            if (hFile->d_name[0] == '.') continue;
            
            string filename(hFile->d_name);
            // cout << filename<<endl;
            string ext = utilsFileIO::GetSuffix(filename);
            // cout << ext << "\t" << expression << endl;
            string strexpression = string(expression);
            if(utilsString::StringMatch(ext.c_str(), expression) || strexpression.find(ext) != string::npos
               || utilsString::StringMatch(expression, ".*")
               || utilsString::StringMatch(expression, "*.*"))
            {
                ostringstream oss;
                oss << lpPath << "/" <<  filename;
                cout<<oss.str()<<endl;
                vecFiles.push_back(oss.str());
            }
        } 
        closedir(dir);
    }
#endif
	return 0;
}

string utilsFileIO::GetAppPath()
{
	string RootPath;
#ifdef windows
	TCHAR buffer[PATH_MAX];
	GetModuleFileName(NULL, buffer, PATH_MAX);
	RootPath = string((char *) buffer);
#elif (defined macos) || (defined macosold)
    /// http://stackoverflow.com/a/8149380/4837280
    int ret;
    pid_t pid;
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
    pid = getpid();
    ret = proc_pidpath (pid, pathbuf, sizeof(pathbuf));
    if ( ret <= 0 ) {
        fprintf(stderr, "PID %d: proc_pidpath ();\n", pid);
        fprintf(stderr, "    %s\n", strerror(errno));
    } else {
        printf("proc %d: %s\n", pid, pathbuf);
    }
    RootPath = pathbuf;
#else
	static char buf[PATH_MAX];
	int rslt = readlink("/proc/self/exe", buf, PATH_MAX);
	if(rslt < 0 || rslt >= PATH_MAX)
		buf[0] = '\0';
	else
		buf[rslt] = '\0';
	RootPath = buf;
#endif
	basic_string<char>::size_type idx = RootPath.find_last_of(SEP);
	RootPath = RootPath.substr(0, idx + 1);

	return RootPath;
}

string utilsFileIO::GetCoreFileName(string const& fullFileName)
{
	string::size_type start = fullFileName.find_last_of("\\");
	if (fullFileName.find_last_of("/") != string::npos)
	{
		start = fullFileName.find_last_of("/");
	}

	if (start == string::npos)
		start = -1; // old code: start = 0; Modified by ZhuLJ, 2015/6/16

	string::size_type end = fullFileName.find_last_of(".");

	if (end == string::npos)
		end = fullFileName.length();

	return fullFileName.substr(start + 1, end - start - 1);
}

string utilsFileIO::GetSuffix(string const& fullFileName)
{
	vector<string> tokens = utilsString::SplitString(fullFileName, '.');
	if (tokens.size() >= 2)
		return tokens[tokens.size() - 1];
	else
		return "";
}

string utilsFileIO::ReplaceSuffix(string const& fullFileName, string const& newSuffix){
	string filedir = utilsFileIO::GetPathFromFullName(fullFileName);
	string corename = utilsFileIO::GetCoreFileName(fullFileName);
	string oldSuffix = utilsFileIO::GetSuffix(fullFileName);
	if (filedir == "" || oldSuffix == "") return "";
	return filedir + corename + "." + newSuffix;
}

string utilsFileIO::GetPathFromFullName(string const& fullFileName)
{
	string::size_type i = fullFileName.find_last_of("\\");
	if (fullFileName.find_last_of("/") != string::npos)
	{
		i = fullFileName.find_last_of("/");
	}
	if (i == string::npos)
		return "";
	return fullFileName.substr(0, i + 1);
}

/************ utils ******************/
utils::utils(void) {}

utils::~utils(void) {}
bool utils::isIPAddress(const char *ip) {
	const char *pChar;
	bool rv = true;
	int tmp1, tmp2, tmp3, tmp4, i;

	while (1) {
		i = StringScanf(ip,"%d.%d.%d.%d",&tmp1, &tmp2, &tmp3, &tmp4);
		if (i != 4) {
			rv = false;
			//cout << "IP Address format is not correct!" << endl;
			break;
		}
		if ((tmp1 > 255) || (tmp2 > 255) || (tmp3 > 255) || (tmp4 > 255) || (tmp1 < 0) || (tmp2 < 0) || (tmp3 < 0) ||
			(tmp4 < 0)) {
				rv = false;
				//cout << "IP Address format is not correct!" << endl;
				break;
		}
		for (pChar = ip; *pChar != 0; pChar++) {
			if ((*pChar != '.') && ((*pChar < '0') || (*pChar > '9'))) {
				rv = false;
				//cout << "IP Address format is not correct!" << endl;
				break;
			}
		}
		break;
	}
	return rv;
}
void utils::Log(string msg, string logpath /* = "debugInfo.log" */)
{
    struct tm timeptr;
    time_t now;
    char buffer[32];
    time(&now);
#ifdef windows
    localtime_s(&timeptr, &now);
    asctime_s(buffer, 32, &timeptr);
#else
    localtime_r(&now, &timeptr);
    asctime_r(&timeptr, buffer);
#endif
    string timestamp = buffer;
    timestamp = timestamp.substr(0, timestamp.length() - 1);
    fstream fs(logpath.c_str(), ios::app);
    if (fs.is_open())
    {
        fs << timestamp;
        fs << ": ";
        fs << msg;
        fs << endl;
        fs.close();
    }
}

int utils::GetAvailableThreadNum(){
#ifdef windows
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
#endif
#ifdef linux
	return (int) sysconf(_SC_NPROCESSORS_ONLN);
#endif
#ifdef macos
	// macOS X 10.5 and later
	return (int) sysconf(_SC_NPROCESSORS_ONLN);
#endif
#ifdef macosold
	// macOS X 10.0 - 10.4
	int mib[4];
	int numCPU;
	std::size_t len = sizeof(numCPU); 

	/* set the mib for hw.ncpu */
	mib[0] = CTL_HW;
	mib[1] = HW_AVAILCPU;  // alternatively, try HW_NCPU;

	/* get the number of CPUs from the system */
	sysctl(mib, 2, &numCPU, &len, NULL, 0);

	if (numCPU < 1) 
	{
		mib[1] = HW_NCPU;
		sysctl(mib, 2, &numCPU, &len, NULL, 0);
		if (numCPU < 1)
			numCPU = 1;
	}
	return numCPU;
#endif
}

void utils::SetDefaultOpenMPThread(){
#ifdef SUPPORT_OMP
	// omp thread have not been set
	if (omp_get_num_threads() <= 1){
		// set one half of the available threads as default
		omp_set_num_threads(GetAvailableThreadNum() / 2);
	}
#endif
    /// do nothing if OMP is not supported
}

void utils::StatusMessage(const char *msg)
{
	/// Just for debugging ///
	// cout << msg << endl;
}
#endif
