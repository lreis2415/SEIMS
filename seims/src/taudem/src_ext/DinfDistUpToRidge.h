#ifndef DINFDISTUP_TO_RIDGE_H
#define DINFDISTUP_TO_RIDGE_H

int nameadd(char *full, char *arg, char *suff);

int dinfdistup(char *angfile, char *felfile, char *slpfile, char *rdgfile, char *wfile,
               char *dtsfile, int statmethod, int typemethod, int userdg,
               int usew, int concheck, float thresh);

int hdisttoridgegrd(char *angfile, char *rdgfile, char *wfile, char *rtrfile, int statmethod,
                    int concheck, float thresh, int userdg, int useweight);

int vrisetoridgegrd(char *angfile, char *felfile, char *rdgfile, char *rtrfile, int statmethod,
                    int concheck, float thresh, int userdg);

int pdisttoridgegrd(char *angfile, char *felfile, char *rdgfile, char *wfile, char *rtrfile,
                    int statmethod, int userdg, int useweight, int concheck, float thresh);

int sdisttoridgegrd(char *angfile, char *slpfile, char *rdgfile, char *wfile, char *rtrfile,
                    int statmethod, int userdg, int useweight, int concheck, float thresh);

#define MAXLN 4096

#endif /* DINFDISTUP_TO_RIDGE_H */
