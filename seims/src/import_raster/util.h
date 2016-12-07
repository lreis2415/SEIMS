/*----------------------------------------------------------------------
*	Purpose: 	Utility functions
*
*	Created:	Junzhi Liu
*	Date:		29-July-2010
*
*	Revision:
*   Date:
*---------------------------------------------------------------------*/

#ifndef SEIMS_UTIL_INCLUDE
#define SEIMS_UTIL_INCLUDE

#define NODATA -99.0f

#include <string>
#include <vector>
#include <stdio.h>

using namespace std;

enum LayeringMethod
{
    UP_DOWN,
    DOWN_UP
};


extern bool DoubleEqual(double d1, double d2);

extern bool FloatEqual(float d1, float d2);

extern string GetPathFromFullName(string fullFileName);

extern bool StringMatch(string text1, string text2);

extern string GetUpper(string);

extern void StatusMessage(const char *msg);

extern void Read2DArray(const char *filename, int &nRows, float **&data);

extern void Read1DArray(const char *filename, int &nRows, float *&data);

extern void Output1DArray(int n, float *data, const char *filename);

extern void Output2DArray(int nRows, int nCols, float **data, const char *filename);

extern double Max(double *a, int n);

vector<string> SplitString(string item, char delimiter);

extern double Sum(double *a, int n);

extern string &trim(string &s);

extern int FindFiles(const char *lpPath, const char *expression, vector<string> &vecFiles);

extern string GetCoreFileName(const string &fullFileName);

extern void Read2DArrayFromString(const char *s, int &nRows, float **&data);

string GetAppPath();

#ifndef linux
#define strprintf sprintf_s
#define StringTok strtok_s
#else
#define strprintf snprintf
#define StringTok strtok_r
extern int copyfile_linux(const char* srcfile, const char* dstfile);
#endif

#endif
