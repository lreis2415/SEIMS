/*  Curvature algorithm calculate ProfileCurvature, PlanCurvature...
    algorithm is adpoted from Shary et al.(2002).

  Liangjun Zhu
  Lreis, CAS
  Apr 8, 2015

  changelog: 17-08-03  lj - update the command arguments
*/
// include TauDEM header files
#include "commonLib.h"
// include algorithm header file
#include "Curvature.h"

int main(int argc, char** argv) {
    //printf("%d",argc);
    char demfile[MAXLN], profcfile[MAXLN], plancfile[MAXLN], horizcfile[MAXLN];
    char unspherfile[MAXLN], meancfile[MAXLN], maxcfile[MAXLN], mincfile[MAXLN];
    int err, i;
    bool calprof = false, calplan = false, calhoriz = false, calunspher = false;
    bool calmeanc = false, calmaxc = false, calminc = false;
    if (argc < 2) {
        printf("Error: To run the program, use either the Simple Usage option or\n");
        printf("the Usage with Specific file names option\n");
        goto errexit;
    } else if (argc > 2) {
        i = 1;
    } else {
        i = 2;
    }
    while (argc > i) {
        if (strcmp(argv[i], "-fel") == 0) {
            i++;
            if (argc > i) {
                strcpy(demfile, argv[i]);
                //printf("dem:%s\n",demfile);
                i++;
            } else {
                goto errexit;
            }
        } else if (strcmp(argv[i], "-prof") == 0) {
            i++;
            if (argc > i) {
                //printf("%s\n",argv[i]);
                strcpy(profcfile, argv[i]);
                calprof = true;
                //printf("prof:%s\n",profcfile);
                i++;
            }
        } else if (strcmp(argv[i], "-plan") == 0) {
            i++;
            if (argc > i) {
                strcpy(plancfile, argv[i]);
                calplan = true;
                //printf("plan:%s\n",plancfile);
                i++;
            }
        } else if (strcmp(argv[i], "-horiz") == 0) {
            i++;
            if (argc > i) {
                strcpy(horizcfile, argv[i]);
                calhoriz = true;
                //printf("horiz:%s\n",horizcfile);
                i++;
            }
        } else if (strcmp(argv[i], "-unspher") == 0) {
            i++;
            if (argc > i) {
                strcpy(unspherfile, argv[i]);
                calunspher = true;
                //printf("unspher:%s\n",unspherfile);
                i++;
            }
        } else if (strcmp(argv[i], "-ave") == 0) {
            i++;
            if (argc > i) {
                strcpy(meancfile, argv[i]);
                calmeanc = true;
                //printf("ave:%s\n",meancfile);
                i++;
            }
        } else if (strcmp(argv[i], "-max") == 0) {
            i++;
            if (argc > i) {
                strcpy(maxcfile, argv[i]);
                calmaxc = true;
                //printf("max:%s\n",maxcfile);
                i++;
            }
        } else if (strcmp(argv[i], "-min") == 0) {
            i++;
            if (argc > i) {
                strcpy(mincfile, argv[i]);
                calminc = true;
                //printf("min:%s\n",mincfile);
                i++;
            }
        } else {
            goto errexit;
        }
    }
    if (argc == 2) {
        strcpy(demfile, argv[1]);
        nameadd(profcfile, argv[1], "profc");
        nameadd(plancfile, argv[1], "planc");
        nameadd(horizcfile, argv[1], "horizc");
        nameadd(unspherfile, argv[1], "unspher");
        nameadd(meancfile, argv[1], "avec");
        nameadd(maxcfile, argv[1], "maxc");
        nameadd(mincfile, argv[1], "minc");
        calprof = true;
        calplan = true;
        calhoriz = true;
        calunspher = true;
        calmeanc = true;
        calmaxc = true;
        calminc = true;
    }
    if ((err = Curvature(demfile, profcfile, plancfile, horizcfile, unspherfile, meancfile, maxcfile, mincfile, calprof,
                         calplan, calhoriz, calunspher, calmeanc, calmaxc, calminc)) != 0) {
        printf("area error %d\n", err);
    }
    return 0;
errexit:
    printf("Simple Usage:\n %s <basefilename>\n", argv[0]);
    printf("Usage with specific file names:\n %s -fel <felfile>\n", argv[0]);
    printf("[-prof <profcfile> -plan <plancfile> -horiz <horizcfile> -unspher <unspherfile>"
           " -ave <meancfile> -max <maxcfile> -min <mincfile>]\n");
    printf("<felfile> is the name of the filled digital elevation model\n");
    printf("<profcfile> is the profile curvature output file.\n");
    printf("<plancfile> is the plan curvature output file.\n");
    printf("<horizcfile> is the horizontal curvature output file.\n");
    printf("<unspherfile> is the unsphericity output file.\n");
    printf("<meancfile> is the mean curvature output file.\n");
    printf("<maxcfile> is the maximum curvature output file.\n");
    printf("<mincfile> is the minimum curvature output file.\n");
    printf("The following are appended to the file names\n");
    printf("before the files are opened:\n");
    printf("    profc      profile curvature output file\n");
    printf("    planc      plan curvature output file\n");
    printf("    horizc     horizontal curvature output file\n");
    printf("    unspher    unsphericity output file\n");
    printf("    avec       mean curvature output file\n");
    printf("    maxc       maximum curvature output file\n");
    printf("    minc       minimum curvature output file\n");
    exit(0);
}
