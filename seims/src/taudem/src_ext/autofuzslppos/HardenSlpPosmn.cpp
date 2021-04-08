/*  HardenSlpPos is used to generate hard class of slope positions.
	Reference:
    [1] 秦承志, 朱阿兴, 施迅, 等. 坡位渐变信息的模糊推理[J]. 地理研究, 2007, 26(6): 1165-1174.
	[2] Qin C, Zhu A, Shi X, et al. Quantification of spatial gradation of slope positions[J]. Geomorphology, 2009, 110(3): 152-161.
	[3] 秦承志, 卢岩君, 包黎莉, 等. 简化数字地形分析软件 (SimDTA) 及其应用——以嫩江流域鹤山农场区的坡位模糊分类为例[J]. 地球信息科学, 2009, (6): 737-743.
	[4] 秦承志, 朱阿兴, 李宝林, 等. 坡位的分类及其空间分布信息的定量化[J]. 武汉大学学报: 信息科学版, 2009, 34(3): 374-377.
	[5] 秦承志, 卢岩君, 邱维理, 等. 模糊坡位信息在精细土壤属性空间推测中的应用[J]. 地理研究, 2010, 29(9): 1706-1714.
	[6] Qin C, Zhu A, Qiu W, et al. Mapping soil organic matter in small low-relief catchments using fuzzy slope position information[J]. Geoderma, 2012, 171: 64-74.

  Liangjun Zhu
  Lreis, CAS
  Apr 13, 2015

  changelog: 17-08-02 lj - chang the input parameters from fixed slope position types to one tag '-inf'

*/
#if (defined _DEBUG) && (defined MSVC) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include "commonLib.h"
#include "HardenSlpPos.h"

// using namespace std; // Avoid to using the entire namespace of std. Comment by Liangjun, 01/23/19

int main(int argc, char **argv)
{
    vector<string> infiles; // input
    vector<int> tags; // input
    char hardfile[MAXLN], maxsimifile[MAXLN], sechardfile[MAXLN], secsimifile[MAXLN]; // output
    bool calSPSI = false;
    bool calSec = false;
    int SPSImodel = 1;
    char spsifile[MAXLN];
    int i, err;
    if (argc == 1)
    {
        printf("Error: To run this program, use either the Simple Usage option or\n");
        printf("the Usage with Specific file names option\n");
        goto errexit;
    }
    else if (argc > 8)  // at least, there should be 9 input parameters
    {
        i = 1;
        while (argc > i)
        {
            if (strcmp(argv[i], "-inf") == 0)
            {
                i++;
                if (argc > i) {
                    int infnum = atoi(argv[i]);
                    infiles.resize(infnum);
                    tags.resize(infnum);
                    for (int j = 0; j < infnum; j++) {
                        i++;
                        if (argc > i + 1) {
                            tags[j] = atoi(argv[i]);
                            infiles[j] = argv[++i];
                        }
                        else goto errexit;
                    }
                    i++;
                }
            }
            else if (strcmp(argv[i], "-maxS") == 0)
            {
                i++;
                if (argc > i)
                {
                    strcpy(hardfile, argv[i]);
                    i++;
                }
                if (argc > i)
                {
                    strcpy(maxsimifile, argv[i]);
                    i++;
                }
                else goto errexit;
            }
            else if (strcmp(argv[i], "-secS") == 0)
            {
                i++;
                if (argc > i)
                {
                    strcpy(sechardfile, argv[i]);
                    i++;
                }
                if (argc > i)
                {
                    strcpy(secsimifile, argv[i]);
                    calSec = true;
                    i++;
                }
                else goto errexit;
            }
            else if (strcmp(argv[i], "-m") == 0)
            {
                i++;
                if (argc > i)
                {
                    sscanf(argv[i], "%d", &SPSImodel);
                    i++;
                }
                if (argc > i)
                {
                    strcpy(spsifile, argv[i]);
                    calSPSI = true;
                    i++;
                }
                else goto errexit;
            }
            else goto errexit;
        }
    }
    else
    {
        printf("No simple use option for this function because slope position similarity files are needed.\n", argv[0]);
        goto errexit;
    }

    if ((err = HardenSlpPos(infiles, tags, hardfile, maxsimifile, calSec, sechardfile, secsimifile, calSPSI,
                            SPSImodel, spsifile)) != 0)
        printf("Error %d\n", err);
    //system("pause");
    return 0;
    errexit:
    printf("Usage with specific config file names:\n %s <configfile>\n", argv[0]);
    printf("-inf <num> <similarity of each slope position, seperated by SPACE.>\n");
    printf("-maxS <hardfile> <maxsimifile> [-secS <sechardfile> <secsimifile>]\n");
    printf("[-m SPSImodel <SPSIfile>]\n");
    printf("<hardfile> is the hard slope position\n");
    printf("<maxsimifile> is the maximum similarity\n");
    printf("<sechardfile> is the second hard slope position\n");
    printf("<secsimifile> is the second maximum similarity\n");
    printf("SPSImodel can be 1, 2 and 3, which means \n");
    printf("    Model 1: [HardCls] + sgn([2ndHardCls]-[HardCls]) * (1-[MaxSim])/2\n");
    printf("    Model 2: [HardCls] + sgn([2ndHardCls]-[HardCls]) * (1-([MaxSim]-[2ndMaxSim]))/2\n");
    printf("    Model 3: [HardCls] + sgn([2ndHardCls]-[HardCls]) * ([2ndMaxSim]/[MaxSim])/2\n");
    printf("<sechardfile> is the second hard slope position\n");
    exit(0);
}
