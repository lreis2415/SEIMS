/*  SimpleCalculator is used to conduct some simple algorithms like add, minus etc.

  Liangjun Zhu
  Lreis, CAS
  Apr 13, 2015

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include "commonLib.h"
#include "SimpleCalculator.h"

// using namespace std; // Avoid to using the entire namespace of std. Comment by Liangjun, 01/23/19

int main(int argc, char **argv)
{
    char Afile[MAXLN], Bfile[MAXLN], outfile[MAXLN];
    int op = 0;
    int i, err;
    if (argc == 1)
    {
        printf("Error: To run this program, use either the Simple Usage option or\n");
        printf("the Usage with Specific file names option\n");
        goto errexit;
    }
    else if (argc == 8)
    {
        i = 1;
        while (argc > i)
        {
            if (strcmp(argv[i], "-in") == 0)
            {
                i++;
                if (argc > i)
                {
                    strcpy(Afile, argv[i]);
                    i++;
                }
                if (argc > i)
                {
                    strcpy(Bfile, argv[i]);
                    i++;
                }
                else goto errexit;
            }
            else if (strcmp(argv[i], "-out") == 0)
            {
                i++;
                if (argc > i)
                {
                    strcpy(outfile, argv[i]);
                    i++;
                }
                else goto errexit;
            }
            else if (strcmp(argv[i], "-op") == 0)
            {
                i++;
                if (argc > i)
                {
                    sscanf(argv[i], "%d", &op);
                    i++;
                }
                else goto errexit;
            }
            else goto errexit;
        }
    }
    else
    {
        printf("No simple use option for this function because both input and output files and operator string are needed.\n");
        goto errexit;
    }

    // test if the input is correct!
    /*printf("A:%s\n",Afile);
    printf("B:%s\n",Bfile);
    printf("operation:%d\n",op);
    printf("Out:%s\n",outfile);*/
    // end test
    if ((err = SimpleCalc(Afile, Bfile, outfile, op)) != 0)
        printf("Error %d\n", err);
    //system("pause");
    return 0;
    errexit:
    printf("Usage with specific config file names:\n %s <configfile>\n", argv[0]);
    printf("-in <Afile> <Bfile> -out <outfile> -op operation\n");
    printf("<Afile> and <Bfile> are input tiff files\n");
    printf("<outfile> is the output\n");
    printf("operation is a integer which indicate the command :\n");
    printf("    0: add\n");
    printf("    1: minus\n");
    printf("    2: multiply\n");
    printf("    3: divide\n");
    printf("    4: a/(a+b)\n");
    printf("    5: mask\n");
    printf("    to be continued...\n");
    exit(0);
}
