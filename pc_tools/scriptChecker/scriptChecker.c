/*
 * scriptChecker.c
 *
 *  Created on: Jan 16, 2015
 *      Author: bennydiamond
 */

#include "../../xblast/scriptEngine/xblastScriptEngine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


void showUsage(const char *progname);

int main (int argc, const char * argv[])
{
	FILE *pFile;
	unsigned int fileSize;
	unsigned char * buffer;
     printf("XBlast OS script syntax checker.\n");

    if( argc < 2 ) {
        showUsage(argv[0]);
        exit(1);
    }

    pFile = fopen(argv[1],"rb");
    if(pFile == NULL){
        printf("File open error.\n");
        exit(1);
    }

    fseek(pFile, 0, SEEK_END);
    fileSize = ftell(pFile);
    if(fileSize == -1)
    {
    	printf("File size error...");
    	exit(1);
    }
    fseek(pFile, 0, SEEK_SET);
    buffer = (unsigned char *)malloc(sizeof(unsigned char) * (fileSize + 1));

    fileSize = fread(buffer, sizeof(unsigned char), fileSize, pFile);

    fclose(pFile);
    fileSize = trimScript(&buffer, fileSize);
    runScript(buffer, fileSize, 0, NULL);

    free(buffer);

    return EXIT_SUCCESS;
}


void showUsage(const char *progname) {
    printf("Usage:   ");
    printf("%s 'input file'\n",progname);
}


void showUsage(const char *progname);
