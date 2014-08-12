/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
//#include "URLMenuActions.h"
#include "TextMenu.h"

static int position = -1;
char url[200];
char *finalURL = url;

static void fixPosition(int len) {
	if(position < 0) {
		position = len-1;
	}
}

void incrementAlphabetURL(void *chr) {
	memset(url, 0, 200);
	strcpy(url, chr);

	fixPosition(strlen(url));

	if(url[position] < 126) {
		url[position]++;
	}

   sprintf(chr, "%s", url);
}

void decrementAlphabetURL(void *chr) {
	memset(url, 0, 200);

	strcpy(url, chr);
	fixPosition(strlen(url));

	if(url[position] > 33) {
		url[position]--;
	}

   sprintf(chr, "%s", url);
}

void nextLetterURL(void *chr) {
	memset(url, 0, 200);
	strcpy(url, chr);

	fixPosition(strlen(url));

	if(position < 119) {
		position++;
	
		if((url[position] < 32) || (url[position] > 126)) {
			url[position] = url[position-1];
		}
	
	   sprintf(chr, "%s", url);
	}
}

void deleteLetterURL(void *chr) {
	memset(url, 0, 200);
	strcpy(url, chr);
	fixPosition(strlen(url));

	// Position > 1 so that we can never delete the leading slash!
	if(position > 1) {
		url[position] = 0;
		position--;
	   sprintf(chr, "%s", url);
	}
}

void setNumURL(void *chr) {
	memset(url, 0, 200);
	strcpy(url, chr);
	fixPosition(strlen(url));

	url[position] = '0';
   sprintf(chr, "%s", url);
}

void setLCURL(void *chr) {
	memset(url, 0, 200);
	strcpy(url, chr);
	fixPosition(strlen(url));

	if(url[position] >= 65 && url[position] <= 90) {
		url[position] += 32;
	} else if(url[position] >= 97 && url[position] <= 122) {
		// Do nothing.
	} else {	
		url[position] = 'a';
	}
   sprintf(chr, "%s", url);
}

void setUCURL(void *chr) {
	memset(url, 0, 200);
	strcpy(url, chr);
	fixPosition(strlen(url));

	if(url[position] >= 97 && url[position] <= 122) {
		url[position] -= 32;
	} else if(url[position] >= 65 && url[position] <= 90) {
		// Do nothing.
	} else {	
		url[position] = 'A';
	}
   sprintf(chr, "%s", url);
}

void setFullStopURL(void *chr) {
	memset(url, 0, 200);
	strcpy(url, chr);
	fixPosition(strlen(url));

	url[position] = '.';
   sprintf(chr, "%s", url);
}

void setFSlashURL(void *chr) {
	memset(url, 0, 200);
	strcpy(url, chr);
	fixPosition(strlen(url));

	url[position] = '/';
   sprintf(chr, "%s", url);
}

void setDashURL(void *chr) {
	memset(url, 0, 200);
	strcpy(url, chr);
	fixPosition(strlen(url));

	url[position] = '-';
   sprintf(chr, "%s", url);
}

void setUScoreURL(void *chr) {
	memset(url, 0, 200);
	strcpy(url, chr);
	fixPosition(strlen(url));

	url[position] = '_';
   sprintf(chr, "%s", url);
}


