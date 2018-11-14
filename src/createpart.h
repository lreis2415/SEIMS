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
#include "linearpart.h"

// noDatarefactor 11/18/17  apparrently both functions are needed so that sometimes a no data pointer can be input and sometimes a nodata value
tdpartition *CreateNewPartition(DATA_TYPE datatype, long totalx, long totaly, double dxA, double dyA, double nodata){
    //Takes a double as the nodata parameter to accommodate double returns from GDAL through tiffIO
    tdpartition *ptr = NULL;
    int rank;
    MPI_Comm_rank(MCW, &rank);//returns the rank of the calling processes in a communicator
    if (datatype == SHORT_TYPE){
        ptr = new linearpart<int16_t>;
        int16_t ndinit = (int16_t)nodata;
        if (rank == 0) {
            printf("Nodata value input to create partition from file: %lf\n", nodata);
            printf("Nodata value recast to int16_t used in partition raster: %d\n", ndinit);
            fflush(stdout);
        }
        ptr->init(totalx, totaly, dxA, dyA, MPI_INT16_T, ndinit);
    }
    else if (datatype == LONG_TYPE){
        int32_t ndinit = (int32_t)nodata;
        if (rank == 0) {
            printf("Nodata value input to create partition from file: %lf\n", nodata);
            printf("Nodata value recast to int32_t used in partition raster: %d\n", ndinit);
            fflush(stdout);
        }
#ifdef MPI_INT32_T
        ptr = new linearpart<int32_t>;
        ptr->init(totalx, totaly, dxA, dyA, MPI_INT32_T, ((int32_t)nodata));
#else
        // uncomment because when I tried to build TauDEM by Intel C++ Compiler with Intel MPI 4.0.3
        // Error occurred that the identifier "MPI_INT32_T" is undefined!
        // Since int32_t is actually int in VS 2013 (from my own computer's view).
        // So, I decided to update as follows. lj 07-11-17 
        ptr = new linearpart<int>;
        ptr->init(totalx, totaly, dxA, dyA, MPI_LONG, ((int)nodata));
#endif
    }
    else if (datatype == FLOAT_TYPE){
        ptr = new linearpart<float>;
        float ndinit = (float)nodata;
        if (rank == 0) {
            printf("Nodata value input to create partition from file: %lf\n", nodata);
            printf("Nodata value recast to float used in partition raster: %f\n", ndinit);
            fflush(stdout);
        }
        ptr->init(totalx, totaly, dxA, dyA, MPI_FLOAT, ((float)nodata));
    }
    return ptr;
}

template<class type>
tdpartition *CreateNewPartition(DATA_TYPE datatype, long totalx, long totaly, double dxA, double dyA, type nodata) {
    //Overloaded template version of the function
    //Takes a constant as the nodata parameter, rather than a void pointer
    tdpartition *ptr = NULL;
    if (datatype == SHORT_TYPE) {
        ptr = new linearpart<int16_t>;
        ptr->init(totalx, totaly, dxA, dyA, MPI_INT16_T, nodata);
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
