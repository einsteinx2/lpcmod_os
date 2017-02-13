/*
 * LCDRingBuffer.h
 *
 *  Created on: Feb 8, 2017
 *      Author: cromwelldev
 */

#ifndef LIB_LPCMOD_LCDRINGBUFFER_H_
#define LIB_LPCMOD_LCDRINGBUFFER_H_

void putInLCDRingBuffer(unsigned char data, unsigned char RS, unsigned short delay);

void LCDRingBufferInit(void);
void updateLCDRingBuffer(void);

#endif /* LIB_LPCMOD_LCDRINGBUFFER_H_ */
