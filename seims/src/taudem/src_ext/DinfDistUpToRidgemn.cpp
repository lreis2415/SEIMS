/*  DinfDistUp function to compute distance to ridge in DEM
    based on D-infinity flow direction model, ridge is
	assigned by user.

  Liangjun Zhu
  Lreis, CAS
  Apr 1, 2015

*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "commonLib.h"
#include "DinfDistUpToRidge.h"

int main(int argc, char **argv)
{
    char angfile[MAXLN], felfile[MAXLN], slpfile[MAXLN], wfile[MAXLN], rtrfile[MAXLN];
    char rdgfile[MAXLN]; // Ridge source grid assigned by user
    int err, i, statmethod = 0, typemethod = 0, usew = 0, concheck = 1;
    int userdg = 0; // Use ridge source grid or not, default is no use.
    float thresh = 0.0;

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
    while (argc > i) // argc >= 3, currently i = 1, argv[0] == DinfDistUpToRidge
    {
        if (strcmp(argv[i], "-ang") == 0)        //argv[1] == "-ang"
        {
            i++;
            if (argc > i)
            {
                strcpy(angfile, argv[i]);     //argv[2] == angfile
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
        else if (strcmp(argv[i], "-slp") == 0)   //argv[5] == "-slp"
        {
            i++;
            if (argc > i)
            {
                strcpy(slpfile, argv[i]);    //argv[6] == slpfile
                i++;
            }
            else goto errexit;
        }
        else if (strcmp(argv[i], "-rdg") == 0)  //argv[7] == "-rdg"
        {
            i++;
            if (argc > i)
            {
                strcpy(rdgfile, argv[i]);    //argv[8] == rdgfile
                userdg = 1;
                i++;
            }
            else goto errexit;
        }
        else if (strcmp(argv[i], "-wg") == 0)    //argv[9] == "-wg"
        {
            i++;
            if (argc > i)
            {
                strcpy(wfile, argv[i]);       //argv[10] == wfile
                usew = 1;
                i++;
            }
            else goto errexit;
        }
        else if (strcmp(argv[i], "-du") == 0)    //argv[11] == "-du"
        {
            i++;
            if (argc > i)
            {
                strcpy(rtrfile, argv[i]);     //argv[12] == rtrfile
                i++;
            }
            else goto errexit;
        }
        else if (strcmp(argv[i], "-m") == 0)     //argv[13] == "-m"
        {
            i++;
            if (argc > i)
            {
                if (strcmp(argv[i], "h") == 0)       //argv[14] == "h" or "v" or "p" or "s" or "ave" or "max" or "min"
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
                else if (strcmp(argv[i], "ave") == 0)
                {
                    statmethod = 0;
                }
                else if (strcmp(argv[i], "max") == 0)
                {
                    statmethod = 1;
                }
                else if (strcmp(argv[i], "min") == 0)
                {
                    statmethod = 2;
                }
                i++;
                if (strcmp(argv[i], "h") == 0)        //argv[15] == "h" or "v" or "p" or "s"
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
                else if (strcmp(argv[i], "ave") == 0)
                {
                    statmethod = 0;
                }
                else if (strcmp(argv[i], "max") == 0)
                {
                    statmethod = 1;
                }
                else if (strcmp(argv[i], "min") == 0)
                {
                    statmethod = 2;
                }
                i++;
            }
            else goto errexit;
        }
        else if (strcmp(argv[i], "-nc") == 0)       //argv[16] == "-nc"
        {
            i++;
            concheck = 0;
        }
        else if (strcmp(argv[i], "-thresh") == 0)   //argv[17] == "-thresh"
        {
            i++;
            if (argc > i)
            {
                sscanf(argv[i], "%f", &thresh);
                i++;
            }
            else goto errexit;
        }
        else if (strcmp(argv[i], "-nc") == 0)
        {
            i++;
            concheck = 0;
        }
        else
        {
            goto errexit;
        }
    }
    if (argc == 2)
    {
        nameadd(angfile, argv[1], "ang");
        nameadd(felfile, argv[1], "fel");
        nameadd(slpfile, argv[1], "slp");
        nameadd(wfile, argv[1], "wg");
        nameadd(rtrfile, argv[1], "du");
    }

    if ((err = dinfdistup(angfile, felfile, slpfile, rdgfile, wfile, rtrfile, statmethod,
                          typemethod, userdg, usew, concheck, thresh)) != 0)
        printf("area error %d\n", err);
    return 0;

    errexit:
    printf("Usage with specific file names:\n %s -ang <angfile>\n", argv[0]);
    printf("-fel <felfile> -slp <slpfile> [-rdg <rdgfile> -wg <wfile>] -du <rtrfile>\n");
    printf("[-m stats dist] [-thresh] [-nc]\n");
    printf("<angfile> is the D-infinity flow direction input file.\n");
    printf("<felfile> is the pit filled or carved elevation input file.\n");
    printf("<slpfile> is the D-infinity slope input file.\n");
    printf("<rdgfile> is the ridge source grid input file.\n");
    printf("<wgfile> is the wighted grid input file.\n");
    printf("<rtrfile> is the D-infinity distance up to ridges output file.\n");
    printf("[-m dist] is the optional method flag.\n");
    printf("    dist  can be h, v, p and s, which means horizontal, vertical, Pythagoras and Surface respectively, the default is h\n");
    printf("[-thresh] is the proportion threshold, the default is 0\n");
    printf("The flag -nc overrides edge contamination checking\n");
    printf("The following are appended to the file names\n");
    printf("before the files are opened:\n");
    printf("    ang   D-infinity contributing area file (output)\n");
    printf("    fel   Pit filled or carved elevation file\n");
    printf("    slp   D-infinity slope input file\n");
    printf("    rdg   Ridge source input file\n");
    printf("    wg    weight input file\n");
    printf("    du    distance to ridge output file\n");
    exit(0);
}
