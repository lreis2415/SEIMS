/*----------------------------------------------------------------------
*	Purpose: 	Utility functions
*
*	Created:	Junzhi Liu
*	Date:		29-July-2010
*
*	Revision:
*   Date:
*---------------------------------------------------------------------*/
#pragma once
#define NODATA -9999.0f
#define UTIL_ZERO 1.e-6
#include <string>
#include <vector>

using namespace std;

enum FlowDirectionMethod
{
	TauDEM = 0,
	ArcGIS = 1
};

enum LayeringMethod
{
	UP_DOWN,
	DOWN_UP
};
// define some macro for string related built-in functions, by Liangjun
#ifdef MSVC
#define stringcat strcat_s
#define stringcpy strcpy_s
#else
#define stringcat strcat
#define stringcpy strcpy
#endif
extern bool DoubleEqual(double d1, double d2);
extern bool FloatEqual(float d1, float d2);
extern string GetPathFromFullName(string& fullFileName);
extern bool StringMatch(string text1, string text2);
extern string GetUpper(string);
extern string GetLower(string);
extern void StatusMessage(const char* msg);
extern void Read2DArray(const char* filename, int& nRows, float**& data);
extern void Read1DArray(const char* filename, int& nRows, float*& data);
extern void Output1DArray(int n, float* data, const char* filename);
extern void Output2DArray(int nRows, int nCols, float** data, const char* filename);
extern double Max(double *a, int n);
extern double Sum(double *a, int n);
extern string& trim(string& s);
extern int FindFiles(const char *lpPath, const char *expression, vector<string>& vecFiles);
extern string GetCoreFileName(const string& fullFileName);
extern string GetSuffix(const string& fullFileName);
extern void Read2DArrayFromString(const char* s, int& nRows, float**& data);
#ifndef linux
    #define strprintf sprintf_s
#else
    #define strprintf snprintf
    extern int copyfile_linux(const char* srcfile, const char* dstfile);
#endif
