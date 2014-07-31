/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "IPMenuActions.h"
#include "TextMenu.h"

int ipA = WB_BLOCK_A, ipB = WB_BLOCK_B, ipC = WB_BLOCK_C, ipD = WB_BLOCK_D, ipP = WB_PORT;

void incrementNumberA(void *num) {
	char *text = (char *)num;
	int n = simple_strtol(num+4, NULL, 10);
	n++;
	if(n > 255) {
		n = 0;
	}
	ipA = n;
	sprintf(text, "%s %i", "A: ", n);
}

void decrementNumberA(void *num) {
	char *text = (char *)num;
	int n = simple_strtol(num+4, NULL, 10);
	n--;
	if(n < 0) {
		n = 255;
	}
	ipA = n;
	sprintf(text, "%s %i", "A: ", n);
}

void skipTenA(void *num) {
	char *text = (char *)num;
	int n = simple_strtol(num+4, NULL, 10);
	n += 10;
	if(n > 255) {
		n -= 256;
	}
	ipA = n;
	sprintf(text, "%s %i", "A: ", n);
}

void incrementNumberB(void *num) {
	char *text = (char *)num;
	int n = simple_strtol(num+4, NULL, 10);
	n++;
	if(n > 255) {
		n = 0;
	}
	ipB = n;
	sprintf(text, "%s %i", "B: ", n);
}

void decrementNumberB(void *num) {
	char *text = (char *)num;
	int n = simple_strtol(num+4, NULL, 10);
	n--;
	if(n < 0) {
		n = 255;
	}
	ipB = n;
	sprintf(text, "%s %i", "B: ", n);
}

void skipTenB(void *num) {
	char *text = (char *)num;
	int n = simple_strtol(num+4, NULL, 10);
	n += 10;
	if(n > 255) {
		n -= 256;
	}
	ipB = n;
	sprintf(text, "%s %i", "B: ", n);
}


void incrementNumberC(void *num) {
	char *text = (char *)num;
	int n = simple_strtol(num+4, NULL, 10);
	n++;
	if(n > 255) {
		n = 0;
	}
	ipC = n;
	sprintf(text, "%s %i", "C: ", n);
}

void decrementNumberC(void *num) {
	char *text = (char *)num;
	int n = simple_strtol(num+4, NULL, 10);
	n--;
	if(n < 0) {
		n = 255;
	}
	ipC = n;
	sprintf(text, "%s %i", "C: ", n);
}

void skipTenC(void *num) {
	char *text = (char *)num;
	int n = simple_strtol(num+4, NULL, 10);
	n += 10;
	if(n > 255) {
		n -= 256;
	}
	ipC = n;
	sprintf(text, "%s %i", "C: ", n);
}

void incrementNumberD(void *num) {
	char *text = (char *)num;
	int n = simple_strtol(num+4, NULL, 10);
	n++;
	if(n > 255) {
		n = 1;
	}
	ipD = n;
	sprintf(text, "%s %i", "D: ", n);
}

void decrementNumberD(void *num) {
	char *text = (char *)num;
	int n = simple_strtol(num+4, NULL, 10);
	n--;
	if(n < 0) {
		n = 255;
	}
	ipD = n;
	sprintf(text, "%s %i", "D: ", n);
}

void skipTenD(void *num) {
	char *text = (char *)num;
	int n = simple_strtol(num+4, NULL, 10);
	n += 10;
	if(n > 255) {
		n -= 255;
	}
	ipD = n;
	sprintf(text, "%s %i", "D: ", n);
}

void incrementNumberP(void *num) {
	char *text = (char *)num;
	int n = simple_strtol(num+4, NULL, 10);
	n++;
	if(n > 65535) {
		n = 1;
	}
	ipP = n;
	sprintf(text, "%s %i", "P: ", n);
}

void decrementNumberP(void *num) {
	char *text = (char *)num;
	int n = simple_strtol(num+4, NULL, 10);
	n--;
	if(n < 1) {
		n = 65535;
	}
	ipP = n;
	sprintf(text, "%s %i", "P: ", n);
}

void skipTenP(void *num) {
	char *text = (char *)num;
	int n = simple_strtol(num+4, NULL, 10);
	n += 10;
	if(n > 65535) {
		n -= 65535;
	}
	ipP = n;
	sprintf(text, "%s %i", "P: ", n);
}

