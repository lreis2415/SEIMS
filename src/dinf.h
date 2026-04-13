#ifndef TAUDEM_DINF_H
#define TAUDEM_DINF_H
#include "linearpart.h"

/// These functions are from d8.h, which are compatible with d8 and dinf

long resolveflats(tdpartition *elevDEM, tdpartition *flowDir, queue <node> *que, int alg, bool &first);

/// end functions from d8.n

int setdir(char *demfile, char *angfile, char *slopefile, char *flowfile, int useflowfile);

void VSLOPE(float E0, float E1, float E2,
            double D1, double D2, double DD,
            float *S, float *A);

void SET2(int I, int J, double *DXX, double DD, tdpartition *elevDEM, tdpartition *flowDir, tdpartition *slope);

void SET2(int I, int J, double *DXX, double DD, tdpartition *elevDEM, tdpartition *elev2, tdpartition *flowDir,
          tdpartition *dn);

long setPosDirDinf(tdpartition *elevDEM, tdpartition *flowDir, tdpartition *slope, int useflowfile);

// No need to duplicate the implementation of resolveflats in d8.cpp and dinf.cpp
// long resolveflats(tdpartition *elevDEM, tdpartition *flowDir, queue <node> *que, bool &first);

#endif // TAUDEM_DINF_H
