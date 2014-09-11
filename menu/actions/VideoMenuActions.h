#ifndef _VIDEOMENUACTIONS_H_
#define _VIDEOMENUACTIONS_H_
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

void SetWidescreen(void *);
//void SetVideoStandard(void *);
void incrementVideoStandard(void * itemStr);
void decrementVideoStandard(void * itemStr);

void incrementVideoformat(void * itemStr);
void decrementVideoformat(void * itemStr);

void toggle480p(void * itemStr);
void toggle720p(void * itemStr);
void toggle1080i(void * itemStr);

#endif
