/*!
 * \file FuzzySlpPosInferencemn.cpp
 * \date 2015/04/09 14:12
 *
 * \author Liangjun Zhu
 * Contact: zlj@lreis.ac.cn
 *
 * \brief FuzzySlpPosInference is used to calculate similarity to a kind of slope position
    according to several parameters grid, typical location, and fuzzy inference function.
	Reference:
 *
 *
 * \note References
	[1] 秦承志, 朱阿兴, 施迅, 等. 坡位渐变信息的模糊推理[J]. 地理研究, 2007, 26(6): 1165-1174.
	[2] Qin C, Zhu A, Shi X, et al. Quantification of spatial gradation of slope positions[J]. Geomorphology, 2009, 110(3): 152-161.
	[3] 秦承志, 卢岩君, 包黎莉, 等. 简化数字地形分析软件 (SimDTA) 及其应用——以嫩江流域鹤山农场区的坡位模糊分类为例[J]. 地球信息科学, 2009, (6): 737-743.
	[4] 秦承志, 朱阿兴, 李宝林, 等. 坡位的分类及其空间分布信息的定量化[J]. 武汉大学学报: 信息科学版, 2009, 34(3): 374-377.
	[5] 秦承志, 卢岩君, 邱维理, 等. 模糊坡位信息在精细土壤属性空间推测中的应用[J]. 地理研究, 2010, 29(9): 1706-1714.
	[6] Qin C, Zhu A, Qiu W, et al. Mapping soil organic matter in small low-relief catchments using fuzzy slope position information[J]. Geoderma, 2012, 171: 64-74.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include "commonLib.h"
#include "FuzzySlpPosInference.h"

// using namespace std; // Avoid to using the entire namespace of std. Comment by Liangjun, 01/23/19

void split(char *src, const char *separator, char **dest, int *num)
{
    char *pNext;
    int count = 0;
    if (src == NULL || strlen(src) == 0) return;
    if (separator == NULL || strlen(separator) == 0) return;
    pNext = strtok(src, separator);
    while (pNext != NULL)
    {
        *dest++ = pNext;
        ++count;
        pNext = strtok(NULL, separator);
    }
    *num = count;
}

int main(int argc, char **argv)
{
    char protogrd[MAXLN], configfile[MAXLN], simfile[MAXLN], valuefile[MAXLN];
    int prototag = 1; // by default, the tag of prototype GRID is 1, it can also be assigned by user.
    int paramsNum = 0, lineNum = 0, i = 0, err = -1;
    //float **AllCellValues;
    //int *AllCellValuesNum;
    float exponent = 1.f;
    paramInfGRID *paramsgrd;
    char cfglines[20][MAXLN];
    int paramidx;
    if (argc < 3)
    {
        printf("Error: To run this program, use either the Simple Usage option or\n");
        printf("the Usage with Specific file names option\n");
        goto errexit;
    }
    else
    {
        paramidx = 1;
        while (argc > paramidx)
        {
            if ((argc > paramidx) && strcmp(argv[paramidx], "-in") == 0)
            {
                paramidx++;
                if (argc > paramidx)
                {
                    strcpy(configfile, argv[paramidx]);
                    paramidx++;
                }
                else goto errexit;
            }
        }
        //printf("%s\n",configfile);
        ifstream cfg(configfile, ios::in);
        while (!cfg.eof())
        {
            cfg.getline(cfglines[lineNum], MAXLN, '\n');
            lineNum++;
        }
        cfg.close();
        char *dest[MAXLN];
        int num = 0, paramline = 0, row = 0;
        while (lineNum > row)
        {
            split(cfglines[row], "\t", dest, &num);
            if (strcmp(dest[0], "PrototypeGRID") == 0 && num == 2)
            {
                strcpy(protogrd, dest[1]);
                row++;
            }
            else if (strcmp(dest[0], "ProtoTag") == 0 && num == 2)
            {
                sscanf(dest[1], "%d", &prototag);
                row++;
            }
            else if (strcmp(dest[0], "ParametersNUM") == 0 && num == 2)
            {
                sscanf(dest[1], "%d", &paramsNum);
                paramline = row + 1;
                row = row + paramsNum + 1;
            }
            else if (strcmp(dest[0], "DistanceExponentForIDW") == 0 && num == 2)
            {
                sscanf(dest[1], "%f", &exponent);
                row++;
            }
            else if (strcmp(dest[0], "OUTPUT") == 0 && num == 2)
            {
                strcpy(simfile, dest[1]);
                row++;
            }
            else row++;
        }
        paramsgrd = new paramInfGRID[paramsNum];
        i = 0;
        for (row = paramline; row < paramline + paramsNum; row++)
        {
            split(cfglines[row], "\t", dest, &num);
            strcpy(paramsgrd[i].path, dest[2]);
            strcpy(paramsgrd[i].shape, dest[3]);
            sscanf(dest[4], "%f", &paramsgrd[i].w1);
            sscanf(dest[5], "%f", &paramsgrd[i].r1);
            sscanf(dest[6], "%f", &paramsgrd[i].k1);
            sscanf(dest[7], "%f", &paramsgrd[i].w2);
            sscanf(dest[8], "%f", &paramsgrd[i].r2);
            sscanf(dest[9], "%f", &paramsgrd[i].k2);
            if (strcmp(paramsgrd[i].shape, "S") == 0) if (!(paramsgrd[i].k1 != 1.0 && paramsgrd[i].k2 == 1.0))
            {
                printf("Please check the parameters of S-shaped function!\n");
                goto errexit;
            }
            else if (strcmp(paramsgrd[i].shape, "Z") == 0) if (!(paramsgrd[i].k1 == 1.0 && paramsgrd[i].k2 != 1.0))
            {
                printf("Please check the parameters of Z-shaped function!\n");
                goto errexit;
            }
            else
                goto errexit;
            i++;
        }
    }

    if ((err = FuzzySlpPosInf(protogrd, prototag, paramsNum, paramsgrd, exponent, simfile)) != 0)
        printf("Error %d\n", err);
    //system("pause");
    return 0;
    errexit:
    printf("Usage with specific config file names:\n %s <configfile>\n", argv[0]);
    printf("The config file should contains context as below:\n");
    printf("PrototypeGRID	path of prototype grid\n");
    printf("ProtoTag	tag of prototype grid\n");
    printf("ParametersNUM	number of parameters grid\n");
    printf("Parameters	ParameterName path of parameters grid	similarity function type	w1	r1	k1	w2	r2	k2\n");
    printf("DistanceExponentForIDW	float number\n");
    printf("OUTPUT	path of output similarity grid\n");
    exit(0);
}
