#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include "commonLib.h"
#include "RidgeExtraction.h"

// using namespace std; // Avoid to using the entire namespace of std. Comment by Liangjun, 01/23/19

int main(int argc, char **argv)
{
	char dirsfile[MAXLN], felfile[MAXLN], rdgsrcfile[MAXLN];
	int err, i;
	float elevt = 0.f;

	if (argc < 7)
	{
		printf("Error: To run this program, use the Usage with Specific file names option\n");
		goto errexit;
	}
	i = 1;
	while (argc > i)
	{
		if (strcmp(argv[i], "-dir") == 0)
		{
			i++;
			if (argc > i)
			{
				strcpy(dirsfile, argv[i]);
				i++;
			}
			else goto errexit;
		}
		else if (strcmp(argv[i], "-fel") == 0)
		{
			i++;
			if (argc > i)
			{
				strcpy(felfile, argv[i]);
				i++;
			}
			else goto errexit;
		}
		else if (strcmp(argv[i], "-src") == 0)
		{
			i++;
			if (argc > i)
			{
				strcpy(rdgsrcfile, argv[i]);
				i++;
			}
			else goto errexit;
		}
		else if (strcmp(argv[i], "-th") == 0)
		{
			i++;
			if (argc > i)
			{
				sscanf(argv[i], "%f", &elevt);
			}
			else goto errexit;
		}
		else
		{
			goto errexit;
		}
	}
	if ((err = ExtractRidges(dirsfile, felfile, elevt, rdgsrcfile)) != 0)
		printf("Error %d\n", err);
    //system("pause");
    return 0;
    errexit:
	printf("Usage with specific file names:\n %s -dir <dirfile>\n", argv[0]);
	printf("-fel <felfile> -src <srcfile> [-th <elethreshold>]\n");
	printf("<dirfile> is the flow direction input file.\n");
	printf("<felfile> is the pit filled or carved elevation input file.\n");
	printf("<srcfile> is the ridge sources raster output file.\n");
	printf("<elethreshold> is the minimal elevation to be ridges.\n");
    exit(0);
}
