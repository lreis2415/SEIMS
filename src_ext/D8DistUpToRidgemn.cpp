/*  D8DistUpToRidge is used to compute distance to ridge in DEM
    based on D8 flow direction model, ridge is assigned by user.
	Also, cells without any sources are belong to ridge.

  Liangjun, Zhu
  Lreis, CAS
  Apr 1, 2015

*/

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "commonLib.h"

//========================
// Header
int nameadd(char *full, char *arg, char *suff);

int d8distup(char *pfile, char *felfile, char *rdgfile, char *dtsfile, int typemethod, int statmethod, int userdg);

int hd8uptoridgegrd(char *pfile, char *rdgfile, char *dtsfile, int statmethod, int userdg);

int vrisetoridgegrd(char *pfile, char *felfile, char *rdgfile, char *dtsfile, int statmethod, int userdg);

int pdisttoridgegrd(char *pfile, char *felfile, char *rdgfile, char *dtsfile, int statmethod, int userdg);

int sdisttoridgegrd(char *pfile, char *felfile, char *rdgfile, char *dtsfile, int statmethod, int userdg);

#define MAXLN 4096

int main(int argc, char **argv)
{
    char pfile[MAXLN], felfile[MAXLN], dtsfile[MAXLN];
    char rdgfile[MAXLN]; // Ridge source grid assigned by user
    int userdg = 0;
    int statmethod = 0;
    int err, i, typemethod = 0;
    if (argc < 2)
    {
        printf("Error: To run this program, use either the Simple Usage option or\n");
        printf("the Usage with Specific file names option\n");
        goto errexit;
    }
    else if (argc > 2)
    {
        i = 1;
    }
    else
    {
        i = 2;
    }
    while (argc > i) // argc >= 3, currently i = 1, argv[0] == D8DistUpToRidge
    {
        if (strcmp(argv[i], "-p") == 0)        //argv[1] == "-p"
        {
            i++;
            if (argc > i)
            {
                strcpy(pfile, argv[i]);     //argv[2] == pfile
                i++;
            }
            else goto errexit;
        }
        else if (strcmp(argv[i], "-fel") == 0)   //argv[3] == "-fel"
        {
            i++;
            if (argc > i)
            {
                strcpy(felfile, argv[i]);     //argv[4] == felfile
                i++;
            }
            else goto errexit;
        }
        else if (strcmp(argv[i], "-rdg") == 0)  //argv[5] == "-rdg"
        {
            i++;
            if (argc > i)
            {
                strcpy(rdgfile, argv[i]);    //argv[6] == rdgfile
                userdg = 1;
                i++;
            }
            else goto errexit;
        }
        else if (strcmp(argv[i], "-du") == 0)    //argv[7] == "-du"
        {
            i++;
            if (argc > i)
            {
                strcpy(dtsfile, argv[i]);     //argv[8] == dtsfile
                i++;
            }
            else goto errexit;
        }
        else if (strcmp(argv[i], "-m") == 0)     //argv[9] == "-m"
        {
            i++;
            if (argc > i)
            {
                if (strcmp(argv[i], "h") == 0)       //argv[10] == "h" or "v" or "p" or "s"
                {
                    typemethod = 0;
                }
                else if (strcmp(argv[i], "v") == 0)
                {
                    typemethod = 1;
                }
                else if (strcmp(argv[i], "p") == 0)
                {
                    typemethod = 2;
                }
                else if (strcmp(argv[i], "s") == 0)
                {
                    typemethod = 3;
                }
                else
                    typemethod = 0;
                i++;
            }
            else goto errexit;
        }
        else
        {
            goto errexit;
        }
    }
    if (argc == 2)
    {
        nameadd(pfile, argv[1], "ang");
        nameadd(felfile, argv[1], "fel");
        nameadd(rdgfile, argv[1], "rdg");
        nameadd(dtsfile, argv[1], "du");
    }

    if ((err = d8distup(pfile, felfile, rdgfile, dtsfile, typemethod, statmethod, userdg)) != 0)
        printf("area error %d\n", err);
    return 0;

    errexit:
    printf("Usage with specific file names:\n %s -p <pfile>\n", argv[0]);
    printf("-fel <felfile> [-rdg <rdgfile>] -du <dtsfile>\n");
    printf("[-m dist stats]\n");
    printf("<pfile> is the D8 flow direction input file.\n");
    printf("<felfile> is the pit filled or carved elevation input file.\n");
    printf("<rdgfile> is the ridge source grid input file.\n");
    printf("<dtsfile> is the D8 distance up to ridges output file.\n");
    printf("[-m stats dist] is the optional method flag.\n");
    printf("    dist  can be h, v, p and s, which means horizontal, vertical, Pythagoras and Surface respectively, the default is h");
    printf("    stats can be ave,max and min, the default is ave\n");
    exit(0);
}
