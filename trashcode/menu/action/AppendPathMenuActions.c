/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "AppendPathMenuActions.h"
#include "TextMenu.h"

static int position = -1;
char appendPath[200];
char *finalAppendPath = appendPath;
extern int ipA, ipB, ipC, ipD, ipP;

static void fixPosition(int len) {
	if(position < 0) {
		position = len-1;
	}
}

void enableHttpcAppendPath(void *whatever) {
	extern unsigned char *videosavepage;
	memcpy((void*)FB_START,videosavepage,FB_SIZE);
	VIDEO_ATTR=0xffef37;
	printk("\n\n\n\n\n\n");
	VIDEO_ATTR=0xffc8c8c8;
	initialiseNetwork();
	webBoot(ipA, ipB, ipC, ipD, ipP);
}

void incrementAlphabetAppendPath(void *chr) {
	memset(appendPath, 0, 200);
	strcpy(appendPath, chr);

	fixPosition(strlen(appendPath));

	if(appendPath[position] < 126) {
		appendPath[position]++;
	}

   sprintf(chr, "%s", appendPath);
}

void decrementAlphabetAppendPath(void *chr) {
	memset(appendPath, 0, 200);

	strcpy(appendPath, chr);
	fixPosition(strlen(appendPath));

	if(appendPath[position] > 33) {
		appendPath[position]--;
	}

   sprintf(chr, "%s", appendPath);
}

void nextLetterAppendPath(void *chr) {
	memset(appendPath, 0, 200);
	strcpy(appendPath, chr);

	fixPosition(strlen(appendPath));

	if(position < 119) {
		position++;
	
		if((appendPath[position] < 32) || (appendPath[position] > 126)) {
			appendPath[position] = appendPath[position-1];
		}
	
	   sprintf(chr, "%s", appendPath);
	}
}

void deleteLetterAppendPath(void *chr) {
	memset(appendPath, 0, 200);
	strcpy(appendPath, chr);
	fixPosition(strlen(appendPath));

	// Position > 1 so that we can never delete the leading slash!
	if(position > 1) {
		appendPath[position] = 0;
		position--;
	   sprintf(chr, "%s", appendPath);
	}
}

void setNumAppendPath(void *chr) {
	memset(appendPath, 0, 200);
	strcpy(appendPath, chr);
	fixPosition(strlen(appendPath));

	appendPath[position] = '0';
   sprintf(chr, "%s", appendPath);
}

void setLCAppendPath(void *chr) {
	memset(appendPath, 0, 200);
	strcpy(appendPath, chr);
	fixPosition(strlen(appendPath));

	if(appendPath[position] >= 65 && appendPath[position] <= 90) {
		appendPath[position] += 32;
	} else if(appendPath[position] >= 97 && appendPath[position] <= 122) {
		// Do nothing.
	} else {	
		appendPath[position] = 'a';
	}
   sprintf(chr, "%s", appendPath);
}

void setUCAppendPath(void *chr) {
	memset(appendPath, 0, 200);
	strcpy(appendPath, chr);
	fixPosition(strlen(appendPath));

	if(appendPath[position] >= 97 && appendPath[position] <= 122) {
		appendPath[position] -= 32;
	} else if(appendPath[position] >= 65 && appendPath[position] <= 90) {
		// Do nothing.
	} else {	
		appendPath[position] = 'A';
	}
   sprintf(chr, "%s", appendPath);
}

void setFullStopAppendPath(void *chr) {
	memset(appendPath, 0, 200);
	strcpy(appendPath, chr);
	fixPosition(strlen(appendPath));

	appendPath[position] = '.';
   sprintf(chr, "%s", appendPath);
}

void setFSlashAppendPath(void *chr) {
	memset(appendPath, 0, 200);
	strcpy(appendPath, chr);
	fixPosition(strlen(appendPath));

	appendPath[position] = '/';
   sprintf(chr, "%s", appendPath);
}

void setDashAppendPath(void *chr) {
	memset(appendPath, 0, 200);
	strcpy(appendPath, chr);
	fixPosition(strlen(appendPath));

	appendPath[position] = '-';
   sprintf(chr, "%s", appendPath);
}

void setUScoreAppendPath(void *chr) {
	memset(appendPath, 0, 200);
	strcpy(appendPath, chr);
	fixPosition(strlen(appendPath));

	appendPath[position] = '_';
   sprintf(chr, "%s", appendPath);
}


