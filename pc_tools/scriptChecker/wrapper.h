/*
 * wrapper.h
 *
 *  Created on: May 7, 2017
 *      Author: bennyboy
 */

#ifndef PC_TOOLS_SCRIPTCHECKER_WRAPPER_H_
#define PC_TOOLS_SCRIPTCHECKER_WRAPPER_H_

#define TRIGGER_XPAD_KEY_A           0
#define TRIGGER_XPAD_KEY_B            1
#define TRIGGER_XPAD_KEY_X            2
#define TRIGGER_XPAD_KEY_Y            3
#define TRIGGER_XPAD_KEY_BLACK        4
#define TRIGGER_XPAD_KEY_WHITE         5

unsigned char cromwellLoop() { return 1; }

struct xLCD
{
    unsigned char LineSize;
}xLCD;


#endif /* PC_TOOLS_SCRIPTCHECKER_WRAPPER_H_ */
