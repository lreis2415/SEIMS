/*----------------------------------------------------------------------
*	Purpose: 	Utility functions
*
*	Created:	Junzhi Liu
*	Date:		29-July-2010
*
*	Revision:
*   Date:
*---------------------------------------------------------------------*/

#ifndef linux
#define SEP "\\"
#else
#define SEP "/"
#endif

#include <cmath>
#include "util.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <sstream>

#ifndef linux
#include <WinSock2.h>
#include <Windows.h>
#include <direct.h>

#else
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

using namespace std;

#define UTIL_ZERO 0.0000000001
#define PATH_MAX 255

int FindFiles(const char *lpPath, const char *expression, vector<string> &vecFiles)
{
#ifndef linux
    char szFind[MAX_PATH];
    strcpy(szFind, lpPath);
    strcat(szFind, "\\");
    strcat(szFind, expression);

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = ::FindFirstFile(szFind, &findFileData);
    if (INVALID_HANDLE_VALUE == hFind)
        return -1;
    do
    {
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        char fullpath[MAX_PATH];
        strcpy(fullpath, lpPath);
        strcat(fullpath, "\\");
        strcat(fullpath, findFileData.cFileName);

        vecFiles.push_back(fullpath);

    } while (::FindNextFile(hFind, &findFileData));
#else
    //char ch,infile[1024],outfile[1024];
    struct dirent *ptr;
    DIR *dir;
    dir = opendir(lpPath);
    while((ptr=readdir(dir)) != NULL)
    {
        if(ptr->d_name[0] == '.')
            continue;

        string filename(ptr->d_name);
		//cout << filename<<endl;
        int n = filename.length();
        string ext = filename.substr(n-4, 4);
        //cout << ext << "\t" << expression << endl;
		if(StringMatch(ext, expression) || StringMatch(expression, ".*") 
			|| StringMatch(expression, "*.*"))
        {
            ostringstream oss;
            oss << lpPath << "/" <<  filename;
			//cout<<oss.str()<<endl;
            vecFiles.push_back(oss.str());
        }
    }
    closedir(dir);
#endif

    return 0;
}

vector<string> SplitString(string item, char delimiter)
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

string GetCoreFileName(const string &fullFileName)
{
    string::size_type start = fullFileName.find_last_of("\\");
    if (start == string::npos)
    {
        start = fullFileName.find_last_of("/");
    }

    if (start == string::npos)
        start = -1;///< added by ZhuLJ,2015/6/19

    string::size_type end = fullFileName.find_last_of(".");

    if (end == string::npos)
        end = fullFileName.length();

    return fullFileName.substr(start + 1, end - start - 1);
}

string &trim(string &s)
{
    if (s.empty())
        return s;
    s.erase(0, s.find_first_not_of(" \n\r\t"));
    return s.erase(s.find_last_not_of(" \n\r\t") + 1);
}


string GetUpper(string str)
{
    string strTmp1 = string(str);
    for (int j = 0; j < (int) strTmp1.length(); j++) strTmp1[j] = toupper(strTmp1[j]);
    return strTmp1;
}

bool StringMatch(string text1, string text2)
{
    // convert the key to UPPERCASE for comparison
    string strTmp1 = string(text1);
    for (int j = 0; j < (int) strTmp1.length(); j++) strTmp1[j] = toupper(strTmp1[j]);

    string strTmp2 = string(text2);
    for (int j = 0; j < (int) strTmp2.length(); j++) strTmp2[j] = toupper(strTmp2[j]);

    return (strTmp1 == strTmp2);
}

bool DoubleEqual(double d1, double d2)
{
    if (abs(d1 - d2) < UTIL_ZERO)
        return true;
    else
        return false;
}

bool FloatEqual(float d1, float d2)
{
    if (abs(d1 - d2) < UTIL_ZERO)
        return true;
    else
        return false;
}

string GetPathFromFullName(string &fullFileName)
{
    string::size_type i = fullFileName.find_last_of("\\");
    if (i == string::npos)
    {
        i = fullFileName.find_last_of("/");
    }

    if (i == string::npos)
        return "";

    return fullFileName.substr(0, i + 1);
}

//! Output a status message
void StatusMessage(const char *msg)
{
    //cout << msg;
    //cout << endl;
}

void Read2DArray(const char *filename, int &nRows, float **&data)
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

void Read1DArray(const char *filename, int &nRows, float *&data)
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

void Read2DArrayFromString(const char *s, int &nRows, float **&data)
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

void Output1DArray(int n, float *data, const char *filename)
{
    ofstream ofs(filename);

    for (int i = 0; i < n; ++i)
        ofs << data[i] << "\n";

    ofs.close();
}

void Output2DArray(int nRows, int nCols, float **data, const char *filename)
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

double Sum(double *a, int n)
{
    double s = 0.0;
    for (int i = 0; i < n; i++)
    {
        s += a[i];
    }
    return s;
}

double Max(double *a, int n)
{
    double m = a[0];
    for (int i = 1; i < n; i++)
    {
        if (a[i] > m)
        {
            m = a[i];
        }
    }
    return m;
}

// gets the root path to the current executable.
string GetAppPath()
{
    string RootPath;
#ifndef linux
    TCHAR buffer[PATH_MAX];
    GetModuleFileName(NULL, buffer, PATH_MAX);
    RootPath = string((char *) buffer);
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

//bool CopyFile (const char* srcFileName, const char* destFileName)
//{
//    std::ifstream src; // the source file
//    std::ofstream dest; // the destination file
// 
//    src.open (srcFileName, std::ios::binary); // open in binary to prevent jargon at the end of the buffer
//    dest.open (destFileName, std::ios::binary); // same again, binary
//    if (!src.is_open() || !dest.is_open())
//        return false; // could not be copied
// 
//    dest << src.rdbuf (); // copy the content
//    dest.close (); // close destination file
//    src.close (); // close source file
// 
//    return true; // file copied successfully
//}

#ifdef linux
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
int copyfile_linux(const char* srcfile, const char* dstfile)
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

