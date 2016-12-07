/*!
 * \ingroup util
 * \brief Utilities class to handle string, date time and file
 *
 *
 * \author Junzhi Liu
 * \version 1.1
 * \date Jul. 2010
 *
 * 
 */

#pragma once

#include <vector>
#include <string>
#include <sstream> 
#include <algorithm> 
#include <iterator> 
#include <iostream>
#include <time.h>
#include <fstream>
#include <ctime>
#ifndef linux
#include <io.h>
#define _WINSOCKAPI_    // stops windows.h including winsock.h
#include <windows.h>
#include <winsock2.h>
#else
#include <unistd.h>
#include <fcntl.h>
#endif
using namespace std;

/*!
 * \class utils
 * \ingroup util
 * \brief utils class to handle string, date time and file
 *
 *
 *
 */
class utils
{

public:
    //! Constructor (void)
    utils(void);

    //! Destructor (void)
    ~utils(void);

    /*!
     * \brief Convert date time to string as the format of "YYYY-MM-DD"
     *
     *
     * \param[in] date \a time_t data type
     * \return Date time \a string
     */
    static string ConvertToString(const time_t *date);

    /*!
     * \brief  Convert date time to string as the format of "YYYY-MM-DD HH"
     *
     *
     * \param[in] date \a time_t data type
     * \return Date time \a string
     */
    static string ConvertToString2(const time_t *date);

    /*!
     * \brief Convert string to date time, string format could be %4d%2d%2d or %d-%d-%d
     *
     * e.g.: strDate => 20000323, format=> %4d%2d%2d
     *       strDate => 2000-03-23, format => %d-%d-%d
     *
     *
     * \param[in] strDate \a string date
     * \param[in] format \a string format
     * \param[in] includeHour \a bool Include Hour?
     * \return Date time \a time_t
     */
    static time_t ConvertToTime(string strDate, string format, bool includeHour);

    /*!
     * \brief Convert string to date time, string format could be "%4d-%2d-%2d %2d:%2d:%2d"
     *
     * e.g.: strDate => 2000-03-23 10:30:00, format=> %4d-%2d-%2d %2d:%2d:%2d
     *
     *
     * \param[in] strDate \a string date
     * \param[in] format \a string format
     * \param[in] includeHour \a bool Include Hour?
     * \return Date time \a time_t
     */
    static time_t ConvertToTime2(const string &strDate, const char *format, bool includeHour);

    /*!
     * \brief Convert integer year, month, and day to date time
     *
     * \param[in] strDate \a string date
     * \param[in] format \a string format
     * \param[in] includeHour \a bool Include Hour?
     * \return Date time \a time_t
     */
    static time_t ConvertYMDToTime(int &year, int &month, int &day);

    /*!
     * \brief Return a flag indicating if the given file exists
     *
     * \param[in] FileName String path of file
     * \return True if Exists, and false if not.
     */
    static bool FileExists(string FileName);

    /*!
     * \brief Get date information from \a time_t variable
     *
     * \param[in] t \a time_t date
     * \param[out] year, month, day \a int value
     */
    static int GetDateInfoFromTimet(time_t *t, int *year, int *month, int *day);

    /*!
     * \brief Writes an entry to the log file. Normally only used for debug
     *
     * \param[in] msg \a string log message
     */
    static void Log(string msg);

    /*!
     * \brief Splits the given string based on the given delimiter
     *
     * \param[in] item \a string information
     * \param[in] delimiter \a char
     * \return The split strings vector
     */
    static vector<string> SplitString(string item, char delimiter);

    /*
     * \brief Get numeric values by spliting the given string based on the given delimiter
     */
    template<typename T>
    vector<T> SplitStringForValues(string item, char delimiter)
    {
        vector<string> valueStrs = utils::SplitString(item, delimiter);
        vector<T> values;
        for (vector<string>::iterator it = valueStrs.begin(); it != valueStrs.end(); it++)
            values.push_back(atoi((*it).c_str()));
        vector<T>(values).swap(values);
        return values;
    }

    /*
     * \brief Get int values by spliting the given string based on the given delimiter
     */
    static vector<int> SplitStringForInt(string item, char delimiter);

    /*
     * \brief Get float values by spliting the given string based on the given delimiter
     */
    static vector<float> SplitStringForFloat(string item, char delimiter);

    /*!
     * \brief Splits the given string by spaces
     *
     * \param[in] item \a string information
     * \return The split strings vector
     */
    static vector<string> SplitString(string item);

    /*!
     * \brief Trim Both leading and trailing spaces
     *
     * \param[in] str \a string information (reference)
     * \return The trimmed string
     */
    static void TrimSpaces(string &str);
	
};

