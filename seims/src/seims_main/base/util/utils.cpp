/*!
 * \brief methods of utils class
 *
 * Implementation of the methods for the utils class
 *
 * \author Junzhi Liu
 * \version 1.1
 * \date Jul. 2010
 *
 * 
 */

#include "utils.h"

using namespace std;

utils::utils(void)
{
}

utils::~utils(void)
{
}

static int daysOfMonth[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/*!
 *\def LEAPYEAR(y)
 *\brief A macro that returns if \a y is a leap year.
 */
#define LEAPYEAR(y) ((y % 4) == 0 && ((y % 100) != 0 || (y % 400) == 0))

string utils::ConvertToString(const time_t *date)
{
    struct tm dateInfo;
#ifndef linux
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

string utils::ConvertToString2(const time_t *date)
{
    struct tm dateInfo;
#ifndef linux
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
                    if (LEAPYEAR(dateInfo.tm_year))
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

time_t utils::ConvertToTime(string strDate, string format, bool includeHour)
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
#ifdef MSVC
            sscanf_s(strDate.c_str(), format.c_str(), &yr, &mn, &dy, &hr);
#else
            sscanf(strDate.c_str(), format.c_str(), &yr, &mn, &dy, &hr);
#endif
        }
        else
        {
#ifdef MSVC
            sscanf_s(strDate.c_str(), format.c_str(), &yr, &mn, &dy);
#else
            sscanf(strDate.c_str(), format.c_str(), &yr, &mn, &dy);
#endif
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

time_t utils::ConvertToTime2(const string &strDate, const char *format, bool includeHour)
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
#ifdef MSVC
            sscanf_s(strDate.c_str(), format, &yr, &mn, &dy, &hr, &m, &s);
#else
            sscanf(strDate.c_str(), format, &yr, &mn, &dy, &hr, &m, &s);
#endif
        }
        else
        {
#ifdef MSVC
            sscanf_s(strDate.c_str(), format, &yr, &mn, &dy);
#else
            sscanf(strDate.c_str(), format, &yr, &mn, &dy);
#endif
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

time_t utils::ConvertYMDToTime(int &year, int &month, int &day)
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

bool utils::FileExists(string FileName)
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
}

int utils::GetDateInfoFromTimet(time_t *t, int *year, int *month, int *day)
{
    struct tm dateInfo;
#ifndef linux
    localtime_s(&dateInfo, t);
#else
    localtime_r(t, &dateInfo);
#endif
    if (dateInfo.tm_isdst > 0)
        dateInfo.tm_hour -= 1;

    char dateString[30];
    strftime(dateString, 30, "%Y-%m-%d %X", &dateInfo);
    int hour, min, sec;
#ifdef MSVC
    sscanf_s(dateString, "%4d-%2d-%2d %2d:%2d:%2d", year, month, day, &hour, &min, &sec);
#else
    sscanf(dateString, "%4d-%2d-%2d %2d:%2d:%2d", year, month, day, &hour, &min, &sec);
#endif
    return 0;
}

void utils::Log(string msg)
{
    struct tm timeptr;
    time_t now;
    char buffer[32];
    time(&now);
#ifndef linux
    localtime_s(&timeptr, &now);
    asctime_s(buffer, 32, &timeptr);
#else
    localtime_r(&now, &timeptr);
    asctime_r(&timeptr, buffer);
#endif
    string timestamp = buffer;
    timestamp = timestamp.substr(0, timestamp.length() - 1);
    fstream fs("SEIMSCore.log", ios::app);
    if (fs.is_open())
    {
        fs << timestamp;
        fs << ": ";
        fs << msg;
        fs << endl;
        fs.close();
    }
}

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

vector<int> utils::SplitStringForInt(string item, char delimiter)
{
    vector<string> valueStrs = utils::SplitString(item, delimiter);
    vector<int> values;
    for (vector<string>::iterator it = valueStrs.begin(); it != valueStrs.end(); it++)
        values.push_back(atoi((*it).c_str()));
    vector<int>(values).swap(values);
    return values;
}

vector<float> utils::SplitStringForFloat(string item, char delimiter)
{
    vector<string> valueStrs = utils::SplitString(item, delimiter);
    vector<float> values(valueStrs.size());
    for (vector<string>::iterator it = valueStrs.begin(); it != valueStrs.end(); it++)
        values.push_back((float)atof((*it).c_str()));
    return values;
}

vector<string> utils::SplitString(string item)
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

void utils::TrimSpaces(string &str)
{
    size_t startpos = str.find_first_not_of(
            " \t"); // Find the first character position after excluding leading blank spaces
    size_t endpos = str.find_last_not_of(" \t"); // Find the first character position from reverse af

    // if all spaces or empty return an empty string
    if ((string::npos == startpos) || (string::npos == endpos))
    {
        str = "";
    }
    else
    {
        str = str.substr(startpos, endpos - startpos + 1);
    }
}
