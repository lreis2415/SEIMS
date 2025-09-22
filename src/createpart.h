/*  Taudem create partition header

  David Tarboton, Dan Watson
  Utah State University  
  May 23, 2010
  
*/

/*  Copyright (C) 2009  David Tarboton, Utah State University

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License 
version 2, 1991 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

A copy of the full GNU General Public License is included in file 
gpl.html. This is also available at:
http://www.gnu.org/copyleft/gpl.html
or from:
The Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
Boston, MA  02111-1307, USA.

If you wish to use or incorporate this program (or parts of it) into 
other software that does not meet the GNU General Public License 
conditions contact the author to request permission.
David G. Tarboton  
Utah State University 
8200 Old Main Hill 
Logan, UT 84322-8200 
USA 
http://www.engineering.usu.edu/dtarb/ 
email:  dtarb@usu.edu 
*/

//  This software is distributed from http://hydrology.usu.edu/taudem/

#ifndef CREATEPART_H
#define CREATEPART_H

#include "commonLib.h"
#include "linearpart.h"

// noDatarefactor 11/18/17  apparrently both functions are needed so that sometimes a no data pointer can be input and sometimes a nodata value
tdpartition *CreateNewPartition(DATA_TYPE datatype, long totalx, long totaly, double dxA, double dyA, double nodata);

template<class type>
inline tdpartition *CreateNewPartition(DATA_TYPE datatype, long totalx, long totaly, double dxA, double dyA, type nodata) {
    //Overloaded template version of the function
    //Takes a constant as the nodata parameter, rather than a void pointer
    tdpartition *ptr = NULL;
    if (datatype == SHORT_TYPE) {
        ptr = new linearpart<int16_t>;
#ifdef MPI_INT16_T
        ptr->init(totalx, totaly, dxA, dyA, MPI_INT16_T, nodata);
#else
        ptr->init(totalx, totaly, dxA, dyA, MPI_SHORT, nodata);
#endif
    } else if (datatype == LONG_TYPE) {
#ifdef MPI_INT32_T
        ptr = new linearpart<int32_t>;
        ptr->init(totalx, totaly, dxA, dyA, MPI_INT32_T, nodata);
#else
        ptr = new linearpart<int>;
        ptr->init(totalx, totaly, dxA, dyA, MPI_LONG, nodata);
#endif
    } else if (datatype == FLOAT_TYPE) {
        ptr = new linearpart<float>;
        ptr->init(totalx, totaly, dxA, dyA, MPI_FLOAT, nodata);
    }
    return ptr;
}
#endif
