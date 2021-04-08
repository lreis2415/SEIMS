/*!
 * \brief main entrance for RPISkidmore
 * \author Liangjun Zhu
 * \version 1.0
 * \date June 2015
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include "commonLib.h"
#include "RPISkidmore.h"
// using namespace std; // Avoid to using the entire namespace of std. Comment by Liangjun, 01/23/19

int main(int argc, char **argv)
{
	char vlysrcfile[MAXLN],rdgsrcfile[MAXLN],dist2vlyfile[MAXLN],dist2rdgfile[MAXLN],rpifile[MAXLN];
	int vlytag = 1, rdgtag = 1;/// by default, the tag of ridge and valley GRID is 1, it can also be assigned by user.
	int i,err;
	bool dist2vlyExport=false,dist2rdgExport=false;
	if(argc < 7)
	{  
		printf("Error: To run this program, see the Usage option below\n");
		goto errexit;
	}
	else
	{
		i = 1;
	}
	while(argc > i)
	{
		if ((argc > i) && strcmp(argv[i],"-vly") == 0)
		{
			i++;
			if (argc > i)
			{
				strcpy(vlysrcfile,argv[i]);
				i++;
			}
			else goto errexit;
		}
		if ((argc > i) && strcmp(argv[i],"-rdg") == 0)
		{
			i++;
			if (argc > i)
			{
				strcpy(rdgsrcfile,argv[i]);
				i++;
			}
			else goto errexit;
		}
		if ((argc > i) && strcmp(argv[i],"-rpi") == 0)
		{
			i++;
			if (argc > i)
			{
				strcpy(rpifile,argv[i]);
				i++;
			}
			else goto errexit;
		}
		if ((argc > i) && strcmp(argv[i],"-vlytag") == 0)
		{
			i++;
			if (argc > i)
			{
				sscanf(argv[i],"%d",&vlytag);
				i++;
			}
			else goto errexit;
		}
		if ((argc > i) && strcmp(argv[i],"-rdgtag") == 0)
		{
			i++;
			if (argc > i)
			{
				sscanf(argv[i],"%d",&rdgtag);
				i++;
			}
			else goto errexit;
		}
		if ((argc > i) && strcmp(argv[i],"-dist2vly") == 0)
		{
			i++;
			if (argc > i)
			{
				strcpy(dist2vlyfile,argv[i]);
				dist2vlyExport=true;
				i++;
			}
			else goto errexit;
		}
		if ((argc > i) && strcmp(argv[i],"-dist2rdg") == 0)
		{
			i++;
			if (argc > i)
			{
				strcpy(dist2rdgfile,argv[i]);
				dist2rdgExport=true;
				i++;
			}
			else goto errexit;
		}
	}
	if((err=RPISkidmore(vlysrcfile,rdgsrcfile, vlytag, rdgtag, rpifile,dist2vlyfile,dist2rdgfile,dist2vlyExport,dist2rdgExport))!= 0)
		printf("Error %d\n",err); 
	//system("pause");
	return 0;
errexit:
	printf("Usage :\n %s -vly <vlyGRID> -rdg <rdgGRID> -rpi <rpiGRID> [-vlytag <number> -rdgtag <number> -dist2vly <dist2vlyGRID> -dist2rdg <dist2rdgGRID>] \n",argv[0]);
	exit(0);
}