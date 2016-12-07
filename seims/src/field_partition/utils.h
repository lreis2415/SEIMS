//! Utility class 
#pragma once

#include <vector>
#include <string>
#include <sstream> 
#include <algorithm> 
#include <iterator> 
#include <iostream>
#include <time.h>

using namespace std;

class utils
{
public:
	utils(void);
	~utils(void);

	void TrimSpaces( string& str);
	static vector<string> SplitString(string item, char delimiter);
	static vector<string> SplitString(string item);
	time_t ConvertToTime(string strDate, string format, bool includeHour);
	time_t ConvertToTime2(string strDate, string format, bool includeHour);
	bool FileExists( string FileName );

	void Log(string msg);

	string ConvertToString(const time_t *);
	string ConvertToString2(const time_t *date);
};
