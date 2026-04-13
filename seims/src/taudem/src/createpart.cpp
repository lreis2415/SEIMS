
#include "createpart.h"

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
#ifdef MPI_INT16_T
        ptr->init(totalx, totaly, dxA, dyA, MPI_INT16_T, ndinit);
#else
        ptr->init(totalx, totaly, dxA, dyA, MPI_SHORT, ndinit);
#endif
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
