/*!
 * \brief Define macro name for utility functions
 * These functions are independent on a class instance, i.e. can be used separately.
 *
 * \author Liangjun Zhu
 * \version 1.0
 * \date Feb. 2017
 * 
 */
#include "utils.h"
#include "ModelException.h"

#ifdef CLS_UTILS
/// Class utilsTime
#define TimeCounting			utilsTime::TimeCounting
#define isLeapYear				utilsTime::isLeapYear
#define ConvertToString			utilsTime::ConvertToString
#define ConvertToString2		utilsTime::ConvertToString2
#define ConvertToTime			utilsTime::ConvertToTime
#define ConvertToTime2			utilsTime::ConvertToTime2
#define ConvertYMDToTime		utilsTime::ConvertYMDToTime
#define GetDateInfoFromTimet	utilsTime::GetDateInfoFromTimet
#define LocalTime				utilsTime::LocalTime

/// Class utilsString
#define GetUpper				utilsString::GetUpper
#define StringMatch				utilsString::StringMatch
#define TrimSpaces				utilsString::TrimSpaces
#define trim					utilsString::trim
#define SplitString				utilsString::SplitString
#define SplitStringForValues	utilsString::SplitStringForValues
#define SplitStringForInt		utilsString::SplitStringForInt
#define SplitStringForFloat		utilsString::SplitStringForFloat
#define ValueToString			utilsString::ValueToString

/// Class utilsArray
#define Initialize1DArray		utilsArray::Initialize1DArray
#define Initialize2DArray		utilsArray::Initialize2DArray
#define Release1DArray			utilsArray::Release1DArray
#define Release2DArray			utilsArray::Release2DArray
#define Output1DArrayToTxtFile	utilsArray::Output1DArrayToTxtFile
#define Output2DArrayToTxtFile	utilsArray::Output2DArrayToTxtFile
#define Read1DArrayFromTxtFile	utilsArray::Read1DArrayFromTxtFile
#define Read2DArrayFromTxtFile	utilsArray::Read2DArrayFromTxtFile
#define Read2DArrayFromString	utilsArray::Read2DArrayFromString
#define ValueInVector			utilsArray::ValueInVector
#define RemoveValueInVector		utilsArray::RemoveValueInVector

/// Class utilsMath
#define FloatEqual				utilsMath::FloatEqual
#define Expo					utilsMath::Expo
#define Power					utilsMath::Power
#define Max						utilsMath::Max
#define Sum						utilsMath::Sum
#define basicStatistics			utilsMath::basicStatistics

/// Class utilsFileIO
#ifndef windows
#define copyfile_unix			utilsFileIO::copyfile_unix
#endif
#define GetAppPath				utilsFileIO::GetAppPath
#define GetCoreFileName			utilsFileIO::GetCoreFileName
#define GetSuffix				utilsFileIO::GetSuffix
#define ReplaceSuffix			utilsFileIO::ReplaceSuffix
#define GetPathFromFullName		utilsFileIO::GetPathFromFullName
#define FileExists				utilsFileIO::FileExists
#define PathExists				utilsFileIO::PathExists
#define DeleteExistedFile		utilsFileIO::DeleteExistedFile
#define FindFiles				utilsFileIO::FindFiles

/// Class utils
#define isIPAddress				utils::isIPAddress
#define Log						utils::Log
#define GetAvailableThreadNum	utils::GetAvailableThreadNum
#define SetDefaultOpenMPThread	utils::SetDefaultOpenMPThread
#define StatusMessage			utils::StatusMessage
#endif