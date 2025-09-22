#ifndef TAUDEM_D8_H
#define TAUDEM_D8_H
#include "linearpart.h"

extern double **fact; // Declaration the global variable that may be used by d8.cpp and mfdmd.cpp

// SET2 is from dinf.h, which will be invoked in resolveflats
void SET2(int I, int J, double *DXX, double DD, tdpartition *elevDEM, tdpartition *elev2, tdpartition *flowDir,
          tdpartition *dn);

// for compatible with d8 and dinf
int dontCross(int k, int i, int j, tdpartition *flowDir, int alg=0);

//Write the slope information
void writeSlope(tdpartition *flowDir, tdpartition *elevDEM, tdpartition *slopefile);

//Open files, initialize grid memory....
int setdird8(char *demfile, char *pointfile, char *slopefile, char *flowfile, int useflowfile);

long setPosDir(tdpartition *elevDEM, tdpartition *flowDir, tdpartition *flow, int useflowfile);

// Add an argument alg to indicate d8 or dinf is used. by lj 09/22/2025
long resolveflats(tdpartition *elevDEM, tdpartition *flowDir, queue <node> *que, int alg, bool &first);
//int resolveflats( tdpartition *elevDEM, tdpartition *flowDir);

#endif // TAUDEM_D8_H