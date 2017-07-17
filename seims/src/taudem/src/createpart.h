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
//#include "partition.h"
//#include "linearpart.h"

tdpartition *CreateNewPartition(DATA_TYPE datatype, long totalx, long totaly, double dxA, double dyA, void *nodata) {
    //Currently, this just creates a new linear partition
    //In the future, this could create any kind of partition, possibly using
    //a "partition_type" flag as an argument
    //Also note that any data types that can be used must be listed here

    tdpartition *ptr = NULL;
    if (datatype == SHORT_TYPE) {
        ptr = new linearpart<short>;
        ptr->init(totalx, totaly, dxA, dyA, MPI_SHORT, *((short *) nodata));
    } else if (datatype == LONG_TYPE) {
#ifdef MPI_INT32_T
        ptr = new linearpart<int32_t>;
        ptr->init(totalx, totaly, dxA, dyA, MPI_INT32_T, *((int32_t *) nodata));
#else
        // uncomment because when I tried to build TauDEM by Intel C++ Compiler with Intel MPI 4.0.3
        // Error occurred that the identifier "MPI_INT32_T" is undefined!
        // Since int32_t is actually int in VS 2013 (from my own computer's view).
        // So, I decided to update as follows. lj 07-11-17 
		ptr = new linearpart<int>;
		ptr->init(totalx, totaly, dxA, dyA, MPI_LONG, *((int*)nodata));
#endif
    } else if (datatype == FLOAT_TYPE) {
        ptr = new linearpart<float>;
        ptr->init(totalx, totaly, dxA, dyA, MPI_FLOAT, *((float *) nodata));
    }
    return ptr;
}

template<class type>
tdpartition *CreateNewPartition(DATA_TYPE datatype, long totalx, long totaly, double dxA, double dyA, type nodata) {
    //Overloaded template version of the function
    //Takes a constant as the nodata parameter, rather than a void pointer
    tdpartition *ptr = NULL;
    if (datatype == SHORT_TYPE) {
        ptr = new linearpart<short>;
        ptr->init(totalx, totaly, dxA, dyA, MPI_SHORT, (short) nodata);
    } else if (datatype == LONG_TYPE) {
#ifdef MPI_INT32_T
        ptr = new linearpart<int32_t>;
        ptr->init(totalx, totaly, dxA, dyA, MPI_INT32_T, (int32_t) nodata);
#else
        ptr = new linearpart<int>;
        ptr->init(totalx, totaly, dxA, dyA, MPI_LONG, (int) nodata);
#endif
    } else if (datatype == FLOAT_TYPE) {
        ptr = new linearpart<float>;
        ptr->init(totalx, totaly, dxA, dyA, MPI_FLOAT, (float) nodata);
    }
    return ptr;
}
#endif
