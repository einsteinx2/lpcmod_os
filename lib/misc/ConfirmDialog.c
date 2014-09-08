#include "boot.h"
#include "video.h"
#include "BootFlash.h"
#include "memory_layout.h"

char bypassConfirmDialog[100];		//Arbitrary length

bool ConfirmDialog(char * string, bool critical) {

	bool result = true;
		
	if(!strncmp(string, bypassConfirmDialog, strlen(string)) && !critical) 
		return false;

	BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
	VIDEO_CURSOR_POSX=75;
	VIDEO_CURSOR_POSY=125;
	
	printk("\2%s\2\n", string);
	printk("\n\n\n\1                                  Hold RT, LT, Start and White to confirm\n");
	printk("\1                                               Press Back to cancel");	
	
	while(!risefall_xpad_STATE(XPAD_STATE_BACK)){
		if(risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_RIGHT) &&
		   risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_LEFT) &&
		   risefall_xpad_STATE(XPAD_STATE_START) /*&&
		   risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_WHITE)*/
		   && XPAD_current[0].keys[5]){		//white button
		   sprintf(bypassConfirmDialog,"%s", string);
		   result = false;
		   break;
		}
		   
	
	}
	BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
	VIDEO_CURSOR_POSX=0;
	VIDEO_CURSOR_POSY=0;
	return result;
}
