/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "InitrdPathMenuActions.h"
#include "TextMenu.h"

static int position = -1;
char initrdPath[200];
char *finalInitrdPath = initrdPath;

static void fixPosition(int len) {
	if(position < 0) {
		position = len-1;
	}
}

void incrementAlphabetInitrdPath(void *chr) {
	memset(initrdPath, 0, 200);
	strcpy(initrdPath, chr);

	fixPosition(strlen(initrdPath));

	if(initrdPath[position] < 126) {
		initrdPath[position]++;
	}

   sprintf(chr, "%s", initrdPath);
}

void decrementAlphabetInitrdPath(void *chr) {
	memset(initrdPath, 0, 200);

	strcpy(initrdPath, chr);
	fixPosition(strlen(initrdPath));

	if(initrdPath[position] > 33) {
		initrdPath[position]--;
	}

   sprintf(chr, "%s", initrdPath);
}

void nextLetterInitrdPath(void *chr) {
	memset(initrdPath, 0, 200);
	strcpy(initrdPath, chr);

	fixPosition(strlen(initrdPath));

	if(position < 119) {
		position++;
	
		if((initrdPath[position] < 32) || (initrdPath[position] > 126)) {
			initrdPath[position] = initrdPath[position-1];
		}
	
	   sprintf(chr, "%s", initrdPath);
	}
}

void deleteLetterInitrdPath(void *chr) {
	memset(initrdPath, 0, 200);
	strcpy(initrdPath, chr);
	fixPosition(strlen(initrdPath));

	// Position > 1 so that we can never delete the leading slash!
	if(position > 1) {
		initrdPath[position] = 0;
		position--;
	   sprintf(chr, "%s", initrdPath);
	}
}

void setNumInitrdPath(void *chr) {
	memset(initrdPath, 0, 200);
	strcpy(initrdPath, chr);
	fixPosition(strlen(initrdPath));

	initrdPath[position] = '0';
   sprintf(chr, "%s", initrdPath);
}

void setLCInitrdPath(void *chr) {
	memset(initrdPath, 0, 200);
	strcpy(initrdPath, chr);
	fixPosition(strlen(initrdPath));

	if(initrdPath[position] >= 65 && initrdPath[position] <= 90) {
		initrdPath[position] += 32;
	} else if(initrdPath[position] >= 97 && initrdPath[position] <= 122) {
		// Do nothing.
	} else {	
		initrdPath[position] = 'a';
	}
   sprintf(chr, "%s", initrdPath);
}

void setUCInitrdPath(void *chr) {
	memset(initrdPath, 0, 200);
	strcpy(initrdPath, chr);
	fixPosition(strlen(initrdPath));

	if(initrdPath[position] >= 97 && initrdPath[position] <= 122) {
		initrdPath[position] -= 32;
	} else if(initrdPath[position] >= 65 && initrdPath[position] <= 90) {
		// Do nothing.
	} else {	
		initrdPath[position] = 'A';
	}
   sprintf(chr, "%s", initrdPath);
}

void setFullStopInitrdPath(void *chr) {
	memset(initrdPath, 0, 200);
	strcpy(initrdPath, chr);
	fixPosition(strlen(initrdPath));

	initrdPath[position] = '.';
   sprintf(chr, "%s", initrdPath);
}

void setFSlashInitrdPath(void *chr) {
	memset(initrdPath, 0, 200);
	strcpy(initrdPath, chr);
	fixPosition(strlen(initrdPath));

	initrdPath[position] = '/';
   sprintf(chr, "%s", initrdPath);
}

void setDashInitrdPath(void *chr) {
	memset(initrdPath, 0, 200);
	strcpy(initrdPath, chr);
	fixPosition(strlen(initrdPath));

	initrdPath[position] = '-';
   sprintf(chr, "%s", initrdPath);
}

void setUScoreInitrdPath(void *chr) {
	memset(initrdPath, 0, 200);
	strcpy(initrdPath, chr);
	fixPosition(strlen(initrdPath));

	initrdPath[position] = '_';
   sprintf(chr, "%s", initrdPath);
}


