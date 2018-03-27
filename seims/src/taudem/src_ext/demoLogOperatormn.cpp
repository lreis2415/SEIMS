/* This demo is a preliminary example to develop an extension algorithm
   under the TauDEM parallelized framework.

   Read a tiff, calculate log, and write to another tiff file.

   Liangjun, Zhu
   zlj@lreis.ac.cn
   LREIS, IGSNRR, CAS
   Apr 4, 2015
*/
// include TauDEM header files
#include "commonLib.h"
// include algorithm header file
#include "demoLogOperator.h"

int main(int argc, char **argv) {
    char infile[MAXLN], outfile[MAXLN];
    int err, i;
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
        if (strcmp(argv[i], "-in") == 0) {
            i++;
            if (argc > i) {
                strcpy(infile, argv[i]);
                i++;
            } else { goto errexit; }
        } else if (strcmp(argv[i], "-out") == 0) {
            i++;
            if (argc > i) {
                strcpy(outfile, argv[i]);
                i++;
            } else { goto errexit; }
        } else { goto errexit; }
    }
    if (argc == 2) {
        strcpy(infile, argv[1]);
        nameadd(outfile, argv[1], "log");
    }
    if ((err = logOperator(infile, outfile)) != 0) {
        printf("area error %d\n", err);
    }
    return 0;
    errexit:
    printf("Simple Usage:\n %s <basefilename>\n", argv[0]);
    printf("Usage with specific file names:\n %s -in <infile> -out <outfile>\n", argv[0]);
    printf("<infile> is the full path of the input raster file.\n");
    printf("<outfile> is the output file.\n");
    exit(0);
}