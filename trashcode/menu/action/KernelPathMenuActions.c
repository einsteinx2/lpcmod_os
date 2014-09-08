/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "KernelPathMenuActions.h"
#include "TextMenu.h"

static int position = -1;
char kernelPath[200];
char *finalKernelPath = kernelPath;

static void fixPosition(int len) {
	if(position < 0) {
		position = len-1;
	}
}

void incrementAlphabetKernelPath(void *chr) {
	memset(kernelPath, 0, 200);
	strcpy(kernelPath, chr);

	fixPosition(strlen(kernelPath));

	if(kernelPath[position] < 126) {
		kernelPath[position]++;
	}

   sprintf(chr, "%s", kernelPath);
}

void decrementAlphabetKernelPath(void *chr) {
	memset(kernelPath, 0, 200);

	strcpy(kernelPath, chr);
	fixPosition(strlen(kernelPath));

	if(kernelPath[position] > 33) {
		kernelPath[position]--;
	}

   sprintf(chr, "%s", kernelPath);
}

void nextLetterKernelPath(void *chr) {
	memset(kernelPath, 0, 200);
	strcpy(kernelPath, chr);

	fixPosition(strlen(kernelPath));

	if(position < 119) {
		position++;
	
		if((kernelPath[position] < 32) || (kernelPath[position] > 126)) {
			kernelPath[position] = kernelPath[position-1];
		}
	
	   sprintf(chr, "%s", kernelPath);
	}
}

void deleteLetterKernelPath(void *chr) {
	memset(kernelPath, 0, 200);
	strcpy(kernelPath, chr);
	fixPosition(strlen(kernelPath));

	// Position > 1 so that we can never delete the leading slash!
	if(position > 1) {
		kernelPath[position] = 0;
		position--;
	   sprintf(chr, "%s", kernelPath);
	}
}

void setNumKernelPath(void *chr) {
	memset(kernelPath, 0, 200);
	strcpy(kernelPath, chr);
	fixPosition(strlen(kernelPath));

	kernelPath[position] = '0';
   sprintf(chr, "%s", kernelPath);
}

void setLCKernelPath(void *chr) {
	memset(kernelPath, 0, 200);
	strcpy(kernelPath, chr);
	fixPosition(strlen(kernelPath));

	if(kernelPath[position] >= 65 && kernelPath[position] <= 90) {
		kernelPath[position] += 32;
	} else if(kernelPath[position] >= 97 && kernelPath[position] <= 122) {
		// Do nothing.
	} else {	
		kernelPath[position] = 'a';
	}
   sprintf(chr, "%s", kernelPath);
}

void setUCKernelPath(void *chr) {
	memset(kernelPath, 0, 200);
	strcpy(kernelPath, chr);
	fixPosition(strlen(kernelPath));

	if(kernelPath[position] >= 97 && kernelPath[position] <= 122) {
		kernelPath[position] -= 32;
	} else if(kernelPath[position] >= 65 && kernelPath[position] <= 90) {
		// Do nothing.
	} else {	
		kernelPath[position] = 'A';
	}
   sprintf(chr, "%s", kernelPath);
}

void setFullStopKernelPath(void *chr) {
	memset(kernelPath, 0, 200);
	strcpy(kernelPath, chr);
	fixPosition(strlen(kernelPath));

	kernelPath[position] = '.';
   sprintf(chr, "%s", kernelPath);
}

void setFSlashKernelPath(void *chr) {
	memset(kernelPath, 0, 200);
	strcpy(kernelPath, chr);
	fixPosition(strlen(kernelPath));

	kernelPath[position] = '/';
   sprintf(chr, "%s", kernelPath);
}

void setDashKernelPath(void *chr) {
	memset(kernelPath, 0, 200);
	strcpy(kernelPath, chr);
	fixPosition(strlen(kernelPath));

	kernelPath[position] = '-';
   sprintf(chr, "%s", kernelPath);
}

void setUScoreKernelPath(void *chr) {
	memset(kernelPath, 0, 200);
	strcpy(kernelPath, chr);
	fixPosition(strlen(kernelPath));

	kernelPath[position] = '_';
   sprintf(chr, "%s", kernelPath);
}


