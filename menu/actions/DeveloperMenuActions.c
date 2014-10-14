/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "DeveloperMenuActions.h"
#include "ToolsMenuActions.h"
#include "boot.h"
#include "video.h"

void LPCIOWrite(void * ignored){
    u16 address = 0x00FF;
    u8 data = 0x00;
    u8 line = 0;
    bool refresh = true;
    while((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_B) != 1) &&
          (risefall_xpad_STATE(XPAD_STATE_BACK) != 1)){
        if(refresh){
            BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
            VIDEO_ATTR=0xffffffff;
            ToolHeader("Write LPC I/O");
            if(line == 0)
       	        VIDEO_ATTR=0xffffef37;    //In yellow
       	    else
       	        VIDEO_ATTR=0xffffffff;
            printk("\n\n\2           Address : 0x%04X", address);
            if(line == 1)
       	        VIDEO_ATTR=0xffffef37;    //In yellow
       	    else
       	        VIDEO_ATTR=0xffffffff;
            printk("\n\n\2           Data    : 0x%02X", data);
            if(line == 2)
       	        VIDEO_ATTR=0xffffef37;    //In yellow
       	    else
       	        VIDEO_ATTR=0xffffffff;
            printk("\n\n\2           Send");
            
            refresh = false;
        }
        if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) == 1 || risefall_xpad_STATE(XPAD_STATE_START) == 1){
            if(line == 0)
                address += 16;
            else if(line == 1)
                data += 16;
            else if(line == 2)
                WriteToIO(address, data);
                
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_RIGHT) == 1 ||
                 risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_RIGHT) == 1) {
            if(line == 0)
                address += 1;
            else if(line == 1 || line == 2)
                data += 1;
                
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_LEFT) == 1 ||
                 risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_LEFT) == 1) {
            if(line == 0)
                address -= 1;
            else if(line == 1 || line == 2)
                data -= 1;
                
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_DOWN) == 1) {
            if(line < 2)
                line += 1;
                
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_UP) == 1) {
            if(line > 0)
            	line -= 1;
            	
            refresh = true;
        }
    }
    return;
}

void LPCIORead(void * ignored){
    u16 address = 0x00FF;
    u8 data = 0x00;
    u8 line = 0;
    bool refresh = true;
    while((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_B) != 1) &&
          (risefall_xpad_STATE(XPAD_STATE_BACK) != 1)){
        if(refresh){
            BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
            VIDEO_ATTR=0xffffffff;
            ToolHeader("Read LPC I/O");
            if(line == 0)
       	        VIDEO_ATTR=0xffffef37;    //In yellow
       	    else
       	        VIDEO_ATTR=0xffffffff;
            printk("\n\n\2           Address : 0x%04X", address);
            if(line == 1)
       	        VIDEO_ATTR=0xffffef37;    //In yellow
       	    else
       	        VIDEO_ATTR=0xffffffff;
            printk("\n\n\2           Send");
        

 	    VIDEO_ATTR=0xffffffff;
            printk("\n\n\n\n\n\2           Data    : 0x%02X", data);
            refresh = false;
        }
        if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) == 1 || risefall_xpad_STATE(XPAD_STATE_START) == 1){
            if(line == 0)
                address += 16;
            else if(line == 1)
                data = ReadFromIO(address);
                
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_RIGHT) == 1 ||
                 risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_RIGHT) == 1) {
            address += 1;
            
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_LEFT) == 1 ||
                 risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_LEFT) == 1) {
            address -= 1;
            
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_DOWN) == 1) {
            if(line < 1)
                line += 1;
                
            refresh = true;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_UP) == 1) {
            if(line > 0)
            	line -= 1;
            	
            refresh = true;
        }
    }
    return;
}
