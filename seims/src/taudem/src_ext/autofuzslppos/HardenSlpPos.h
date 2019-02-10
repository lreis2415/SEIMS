#ifndef HARDEN_SLPPOS_H
#define HARDEN_SLPPOS_H

#include <vector>
#include <string>

using std::string;
using std::vector;

int HardenSlpPos(vector<string> infiles, vector<int> tags, char *hardfile, char *maxsimifile, bool calsec,
                 char *sechardfile,char *secsimifile, bool calspsi, int spsimodel, char *spsifile);

#endif /* HARDEN_SLPPOS_H */
