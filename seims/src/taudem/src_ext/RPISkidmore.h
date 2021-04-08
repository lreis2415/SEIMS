/*!
 * \file RPISkidmore.h
 * \brief original algorithm for RPI after skidmore 1990
 *
 *
 *
 * \author Liangjun Zhu
 * \version 1.0
 * \date June 2015
 *
 *
 */
#ifndef RPI_SKIDMORE_H
#define RPI_SKIDMORE_H

#include "commonLib.h"

struct SourcePt
{
    int col;
    int row;
};
inline bool operator==(const SourcePt& n1, const SourcePt& n2)
{
    return (n1.col == n2.col) && (n1.row == n2.row);
}
template<typename T>
bool coorInList(T col, T row, T*  coors, int count)
{
	for (int i = 0; i < count; i++)
	{
		if (abs(col - coors[2*i]) < 1e-6 && abs(row-coors[2*i+1]) < 1e-6)
			return true;
	}
	return false;
};
int RPISkidmore(char *vlysrcfile,char *rdgsrcfile,int vlytag, int rdgtag, char *rpifile,char *dist2vlyfile,char *dist2rdgfile,bool dist2vlyExport,bool dist2rdgExport);

#endif /* RPI_SKIDMORE_H */
