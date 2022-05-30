/* MFD-md (Qin et al., IJGIS, 2007)
 *
 * Original implementation by Cheng-Zhi Qin
 * SimDTA - https://github.com/lreis2415/SimDTA
 * https://github.com/lreis2415/SimDTA/blob/559905442a4ca8df501df2e3e98fbd4280480b6f/src/modMFD.bas#L457
 *
 * Input: DEM
 * Output: Compound flow direction and flow fractions of each cell to downslope cells
     
  Liangjun Zhu
  Lreis, CAS  
  Oct 19, 2020 
  
  changelog:
    - 1. 2021-04-01 - lj - Output flow fractions of 8 directions.
*/
#include "commonLib.h"
#include "MultiFlowDirMaxDown.h"

int main(int argc, char** argv) {
    char indemfile[MAXLN], outflowmfdfile[MAXLN], outflowportionfile[MAXLN];
    double dp0 = 1.1;
    double dp_range = 8.9;
    double tanb_lb = 0.;
    double tanb_ub = 1.;
    double min_portion = 0.05;

    char* err = nullptr;
    int scount = 2; // argument count of simple usage including the executable itself
    int i = 1;
    if (argc < scount) {
        printf("Error: To run the program, use either the Simple Usage option or\n");
        printf("the Usage with Specific file names option\n");
        goto errexit;
    }
    i = argc == scount ? scount : 1;
    while (argc > i) {
        if (strcmp(argv[i], "-dem") == 0) {
            i++;
            if (argc > i) {
                strcpy(indemfile, argv[i]);
                i++;
            }
            else goto errexit;
        }
        else if (strcmp(argv[i], "-mfd") == 0) {
            i++;
            if (argc > i) {
                strcpy(outflowmfdfile, argv[i]);
                i++;
            }
            else goto errexit;
        }
        else if (strcmp(argv[i], "-portion") == 0) {
            i++;
            if (argc > i) {
                strcpy(outflowportionfile, argv[i]);
                i++;
            }
            else goto errexit;
        }
        else if (strcmp(argv[i], "-p0") == 0) {
            i++;
            if (argc > i) {
                dp0 = strtod(argv[i], &err);
                i++;
            }
            else goto errexit;
        }
        else if (strcmp(argv[i], "-range") == 0) {
            i++;
            if (argc > i) {
                dp_range = strtod(argv[i], &err);
                i++;
            }
            else goto errexit;
        }
        else if (strcmp(argv[i], "-tanb_lb") == 0) {
            i++;
            if (argc > i) {
                tanb_lb = strtod(argv[i], &err);
                i++;
            }
            else goto errexit;
        }
        else if (strcmp(argv[i], "-tanb_ub") == 0) {
            i++;
            if (argc > i) {
                tanb_ub = strtod(argv[i], &err);
                i++;
            }
            else goto errexit;
        }
        else if (strcmp(argv[i], "-min_portion") == 0) {
            i++;
            if (argc > i) {
                min_portion = strtod(argv[i], &err);
                i++;
            }
            else goto errexit;
        }
        else goto errexit;
    }
    if (i == scount) {
        strcpy(indemfile, argv[1]);
        nameadd(outflowmfdfile, argv[1], "_mfdmd");
        nameadd(outflowportionfile, argv[1], "_flowfraction");
    }
    if (abs(tanb_ub - tanb_lb) <= ZERO) {
        printf("tanb_ub MUST NOT equals to tanb_lb!");
        goto errexit;
    }

    flowdirection_mfd_md(indemfile, outflowmfdfile, outflowportionfile, 
                         dp0, dp_range, tanb_lb, tanb_ub, min_portion);

    return 0;
errexit:
    printf("Simple Usage:\n %s <DEMfilename>\n", argv[0]);
    printf("Usage with specific file names:\n %s -dem <inDEM> -mfd <outFlowDir> "
           "-portion <outFlowPortion> [-p0 <P0> -range <P_range> "
           "-tanb_lb <tanb_LB> -tanb_ub <tanb_UB> -min_portion <min_portion>]\n", argv[0]);
    printf("<inDEM> is the full path of the input DEM file.\n");
    printf("<outFlowDir> is the output compound flow direction file.\n");
    printf("<outFlowPortion> is the output flow fraction files of each directions.\n");
    exit(0);
}
