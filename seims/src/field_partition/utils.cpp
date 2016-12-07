//! Implementation of the methods for the utils class
#include "utils.h"
#include <fstream>
#include <cstdio>
#include <ctime>
#ifndef linux
#include <io.h>
#else
#include <unistd.h>
#include <fcntl.h>
#endif

using namespace std;

#define UTIL_ZERO 0.000001

//! Returns a flag indicating if the given file exists
bool utils::FileExists( string FileName )
{
#ifndef linux
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
	//fstream fs;
	//fs.open(FileName.c_str(),ios::in);
	//if(!fs)
	//{
	//	return false;
	//}
	//else
	//{
	//	fs.close();
	//	return true;
	//}
 //   FILE* fp = NULL;
	//errno_t err;

 //   //will not work if you do not have read permissions
 //   //to the file, but if you don't have read, it
 //   //may as well not exist to begin with.

	//err = fopen_s(&fp, FileName.c_str(), "rb");
 //   if( fp != NULL )
 //   {
 //       fclose( fp );
 //       return true;
 //   }

//    return false;
}

//! Constructor
utils::utils(void)
{
}

//! Destructor
utils::~utils(void)
{
}

//! Writes an entry to the log file.
//! normally only used for debugging
void utils::Log(string msg)
{
	//struct tm timeptr;
	//time_t now;
	//char buffer[32];

	//time(&now);
	//localtime_s(&timeptr, &now);
	//asctime_s(buffer, 32, &timeptr);
	//string timestamp = buffer;
	//timestamp = timestamp.substr(0, timestamp.length() -1);

	//ofstream fs = ofstream("WetSpaCore.log", ios::app);
	//if (fs != NULL)
	//{
	//	if (fs.is_open())
	//	{
	//		fs << timestamp;
	//		fs << ": ";
	//		fs << msg;
	//		fs << endl;
	//		fs.close();
	//	}
	//}
}

// Trim Both leading and trailing spaces
void utils::TrimSpaces( string& str)
{
    size_t startpos = str.find_first_not_of(" \t"); // Find the first character position after excluding leading blank spaces
    size_t endpos = str.find_last_not_of(" \t"); // Find the first character position from reverse af
 
    // if all spaces or empty return an empty string
    if(( string::npos == startpos ) || ( string::npos == endpos))
    {
        str = "";
    }
    else
	{
        str = str.substr( startpos, endpos-startpos+1 );
	}
}

//! Splits the given string based on the given delimiter
vector<string> utils::SplitString(string item, char delimiter)
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

vector<string> utils::SplitString(string item)
{
	istringstream iss(item); 
	vector<string> tokens; 

	std::string field;
	iss >> field;
	while(!iss.eof())
	{
		tokens.push_back(field);
		iss >> field;
	}
	tokens.push_back(field);

	return tokens;
}

string utils::ConvertToString(const time_t *date)
{
	struct tm dateInfo;
#ifndef linux
	localtime_s(&dateInfo,date);
#else
    localtime_r(date, &dateInfo);
#endif
	char dateString[11];	
	strftime(dateString,11,"%Y-%m-%d",&dateInfo);
	
	string s(dateString);
	return s;
}

string utils::ConvertToString2(const time_t *date)
{
	struct tm dateInfo;
#ifndef linux
	localtime_s(&dateInfo,date);
#else
    localtime_r(date, &dateInfo);
#endif
	char dateString[30];	
	strftime(dateString,30,"%Y-%m-%d %X",&dateInfo);

	string s(dateString);
	return s;
}


// assumes the strDate contains only the year month and day
// format string should indicate positions of year, month and day
// Ex: strDate => 20000323, format=> %4d%2d%2d
// Ex: strDate => 2000-03-23, format => %d-%d-%d
time_t utils::ConvertToTime(string strDate, string format, bool includeHour)
{
	struct tm* timeinfo;
	time_t t;
	int yr;
	int mn;
	int dy;
	int hr = 0;

	try
	{
		if (includeHour)
		{
			sscanf(strDate.c_str(), format.c_str(), &yr, &mn, &dy, &hr);
		}
		else
		{
			sscanf(strDate.c_str(), format.c_str(), &yr, &mn, &dy);
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

time_t utils::ConvertToTime2(string strDate, string format, bool includeHour)
{
	struct tm* timeinfo;
	time_t t;
	int yr;
	int mn;
	int dy;
	int hr = 0;
	int m, s;

	try
	{
		if (includeHour)
		{
			sscanf(strDate.c_str(), format.c_str(), &yr, &mn, &dy, &hr, &m, &s);
		}
		else
		{
			sscanf(strDate.c_str(), format.c_str(), &yr, &mn, &dy);
		}

		timeinfo = new struct tm;
		timeinfo->tm_year = yr - 1900;
		timeinfo->tm_mon = mn - 1;
		timeinfo->tm_mday = dy;
		timeinfo->tm_hour = hr;
		timeinfo->tm_min = m;
		timeinfo->tm_sec = s;
		timeinfo->tm_isdst = false;
		t = mktime(timeinfo);
	}
	catch (...)
	{
		throw;
	}

	return t;
}
