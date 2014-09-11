#include "boot.h"
#include "video.h"
#include "BootFlash.h"
#include "memory_layout.h"

#define KEY_BCKSPC    0x3
#define KEY_OK        0x1
#define KEY_CANCEL    0x2
#define KEY_SHIFT    0x4
#define KEY_CAPS    0x5

char keymap[4][10] = {{'1','2','3','4','5','6','7','8','9','0'},
                          {'A','B','C','D','E','F','G','H','I','J'},
                          {'K','L','M','N','O','P','Q','R','S','T'},
                          {'U','V','W','X','Y','Z', 0,  0,  0,  0 }};

char shiftkeymap[4][10] = {{'!','"','#','$','%','?','&','*','(',')'},
                           {'a','b','c','d','e','f','g','h','i','j'},
                           {'k','l','m','n','o','p','q','r','s','t'},
                           {'u','v','w','x','y','z', 0,  0,  0,  0 }};

void OnScreenKeyboard(char * string) {
    bool exit = false;
    u8 cursorposX = 0;
    u8 cursorposY = 0;
    u8 x,y;
    bool shift = false;
    bool refresh = true;
    char * oldString, selectedKeymap;
    sprintf(oldString, "%s", string);	//Copy input string in case user cancels.
    
    while(1){
        if(refresh){
            BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
            VIDEO_CURSOR_POSX=50;
            VIDEO_CURSOR_POSY=40;
            VIDEO_ATTR=0xffffffff;                        //White characters.
            printk("\n\1             Back=Cancel   Start=Confirm   B=Backspace   X=Space   Y=Shift");
            VIDEO_ATTR=0xffffef15;                	  //Orangeish
            VIDEO_CURSOR_POSX=75;
            VIDEO_CURSOR_POSY=50;
            printk("\n\n\n\n\2                    %s", string);
            VIDEO_ATTR=0xffffffff;
            printk("\2\n\n\n\n\n           ");
            for(y = 0; y < 4; y++){
                for(x = 0; x < 10; x++){
                	if(shift){
            	            printk("\2%c    ",shiftkeymap[y][x]);
                        }
                        else {
                            printk("\2%c    ",keymap[y][x]);
                        }
                		
                }
                printk("\n\n\n           ");
            }
            refresh = false;
        }
       
    
        if(risefall_xpad_STATE(XPAD_STATE_BACK)){
    	    sprintf(string, "%s", oldString);
    	    exit = true;
        }
        if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_Y)){       
            shift = !shift;
            refresh = true;
        }
    
        if(exit){  
            BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
            VIDEO_CURSOR_POSX=0;
            VIDEO_CURSOR_POSY=0;
            return;
        }
    }
    return;    //Keep compiler happy.
}
