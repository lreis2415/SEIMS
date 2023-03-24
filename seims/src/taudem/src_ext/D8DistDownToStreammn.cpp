/*  D8DistDownToStream

  This function computes the distance from each grid cell moving downstream until a stream 
  grid cell as defined by the Stream Raster grid is encountered.  The optional threshold 
  input is to specify a threshold to be applied to the Stream Raster grid (src).  
  Stream grid cells are defined as having src value >= the threshold, or >=1 if a 
  threshold is not specified.
  Distance method used to calculate the distance down to the stream include: the total straight line path (Pythagoras), 
  the horizontal component of the straight line path, the vertical component of the straight line path, 
  or the total surface flow path.

  Liangjun Zhu
  Lreis, CAS  
  Apr 2, 2015 
  
*/

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "commonLib.h"

int distgrid(char *pfile, char *felfile, char *srcfile, char *distfile, int typemethod, int thresh);

int main(int argc, char **argv) {
    char pfile[MAXLN], felfile[MAXLN], srcfile[MAXLN], distfile[MAXLN];
    int err, thresh = 1, i;
    int typemethod = 0;
    //for (i=0;i<argc;i++)
    //{
    // printf(argv[i]);
    // printf("\n");
    //}
    //printf("%d\n",argc);


    if (argc < 2) {
        printf("Error: To run this program, use either the Simple Usage option or\n");
        printf("the Usage with Specific file names option\n");
        goto errexit;
    } else if (argc > 2) {
        i = 1;
    } else {
        i = 2;
    }

    while (argc > i) {
        if (strcmp(argv[i], "-p") == 0) {
            i++;
            if (argc > i) {
                strcpy(pfile, argv[i]);
                //printf(pfile);
                i++;
            } else { goto errexit; }
        } else if (strcmp(argv[i], "-fel") == 0) {
            i++;
            if (argc > i) {
                strcpy(felfile, argv[i]);
                //printf(felfile);
                i++;
            } else { goto errexit; }
        } else if (strcmp(argv[i], "-src") == 0) {
            i++;
            if (argc > i) {
                strcpy(srcfile, argv[i]);
                //printf(srcfile);
                i++;
            } else { goto errexit; }
        } else if (strcmp(argv[i], "-dist") == 0) {
            i++;
            if (argc > i) {
                strcpy(distfile, argv[i]);
                //printf(distfile);
                i++;
            } else { goto errexit; }
        } else if (strcmp(argv[i], "-m") == 0)     //argv[13] == "-m"
        {
            i++;
            if (argc > i) {
                if (strcmp(argv[i], "h") == 0)       //argv[14] == "h" or "v" or "p" or "s"
                {
                    typemethod = 0;
                } else if (strcmp(argv[i], "v") == 0) {
                    typemethod = 1;
                } else if (strcmp(argv[i], "p") == 0) {
                    typemethod = 2;
                } else if (strcmp(argv[i], "s") == 0) {
                    typemethod = 3;
                }
                i++;
            } else { goto errexit; }
        } else if (strcmp(argv[i], "-thresh") == 0) {
            i++;
            if (argc > i) {
                sscanf(argv[i], "%d", &thresh);
                i++;
            } else { goto errexit; }
        } else {
            //printf("error i:%d\n",i);
            goto errexit;
        }
    }

    if (argc == 2) {
        nameadd(pfile, argv[1], "p");
        nameadd(felfile, argv[1], "fel");
        nameadd(srcfile, argv[1], "src");
        nameadd(distfile, argv[1], "dist");
    }

    if (err = distgrid(pfile, felfile, srcfile, distfile, typemethod, thresh) != 0) {
        printf("D8 distance error %d\n", err);
    }

    return 0;

    errexit:
    printf("Simple Usage:\n %s <basefilename>\n", argv[0]);
    printf("Usage with specific file names:\n %s -p <pfile>\n", argv[0]);
    printf("-fel <felfile> -src <srcfile> -dist <distfile> [-m distmethod] [-thresh <thresh>]\n");
    printf("<basefilename> is the name of the base digital elevation model\n");
    printf("<pfile> is the d8 flow direction input file.\n");
    printf("<felfile> is the pit filled or carved elevation input file.\n");
    printf("<srcfile> is the stream raster input file.\n");
    printf("<distfile> is the distance to stream output file.\n");
    printf("[-m distmethod] is the optional method flag.\n");
    printf("    dist  can be h, v, p and s, which means Horizontal, ");
    printf("Vertical, Pythagoras and Surface respectively, the default is h.\n");
    printf("The optional <thresh> is the user input threshold number.\n");
    printf("The following are appended to the file names\n");
    printf("before the files are opened:\n");
    printf("p      D8 flow directions (input)\n");
    printf("src    stream raster file (Input)\n");
    printf("dist   distance to stream file(output)\n");
    exit(0);
} 
