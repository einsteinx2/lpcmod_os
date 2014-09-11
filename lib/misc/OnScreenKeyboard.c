#include "boot.h"
#include "video.h"
#include "BootFlash.h"
#include "memory_layout.h"

char keymap[4][10] = {{'1','2','3','4','5','6','7','8','9','0'},
                      {'A','B','C','D','E','F','G','H','I','J'},
                      {'K','L','M','N','O','P','Q','R','S','T'},
                      {'U','V','W','X','Y','Z',',','.','\'',';'}};

char shiftkeymap[4][10] = {{'!','"','#','$','%','?','&','*','(',')'},
                           {'a','b','c','d','e','f','g','h','i','j'},
                           {'k','l','m','n','o','p','q','r','s','t'},
                           {'u','v','w','x','y','z','<','>','\"',':'}};

void OnScreenKeyboard(char * string, u8 maxLength) {
    bool exit = false;
    u8 cursorposX = 0;
    u8 cursorposY = 0;
    u8 textpos = 0;
    u8 x,y;
    bool shift = false;
    bool refresh = true;
    char * oldString, selectedKeymap;
    sprintf(oldString, "%s", string);	//Copy input string in case user cancels.
    textpos = strlen(string);           //Place cursor at end of entering string.
    
    while(1){
        if(refresh){
            BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
            VIDEO_CURSOR_POSX=50;
            VIDEO_CURSOR_POSY=40;
            VIDEO_ATTR=0xffffffff;                        //White characters.
            printk("\n\1             Back=Cancel   Start=Confirm   B=Backspace   X=Space   Y=Shift");
            VIDEO_ATTR=0xffff00ff;                	  //Orangeish
            VIDEO_CURSOR_POSX=75;
            VIDEO_CURSOR_POSY=50;
            printk("\n\n\n\n\2                    %s", string);
            VIDEO_ATTR=0xffffffff;
            printk("\2\n\n           ");
            for(y = 0; y < 4; y++){
                printk("\n\n\n           ");
                for(x = 0; x < 10; x++){
                    if(x == cursorposX && y == cursorposY)      //About to draw selected character
                        VIDEO_ATTR=0xffffef37;                  //In yellow
                    else
                        VIDEO_ATTR=0xffffffff;                  //the rest in white.
                    if(shift){
            	        printk("\2%c    ",shiftkeymap[y][x]);
                    }
                    else {
                        printk("\2%c    ",keymap[y][x]);
                    }
                }
            }
            refresh = false;
        }
       
        if(risefall_xpad_STATE(XPAD_STATE_BACK)){       //Cancel
    	    sprintf(string, "%s", oldString);
    	    exit = true;
        }

        if(risefall_xpad_STATE(XPAD_STATE_START)){      //Accept
            if(textpos < maxLength)
                string[textpos] = '\0';
            exit = true;
        }

        if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_Y)){   //Shift toggle
            shift = !shift;
            refresh = true;
        }

        if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_X)){   //Space
            if(textpos < maxLength){
                string[textpos] = ' ';                  //Add space character
                textpos += 1;                           //Move cursor one position to the right
                refresh = true;
            }
        }

        if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_B)){   //Backspace
            if(textpos > 0){
                textpos -= 1;                               //Move cursor one position to the left
                string[textpos] = '\0';                     //Erase character
                refresh = true;
            }
        }

        if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A)){   //Select character
            if(textpos < maxLength){
                if(shift){
                    string[textpos] = shiftkeymap[cursorposY][cursorposX];
                }
                else {
                    string[textpos] = keymap[cursorposY][cursorposX];
                }
                textpos += 1;                           //Move cursor one position to the right
                refresh = true;
            }
        }

        if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_UP)){
            if(cursorposY == 0)         //Already at the top line
                cursorposY = 3;         //Roll to last
            else
                cursorposY -= 1;
            refresh = true;
        }

        if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_DOWN)){
            if(cursorposY == 3)         //Already at the last line
                cursorposY = 0;         //Roll to top
            else
                cursorposY += 1;
            refresh = true;
        }

        if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_LEFT)){
            if(cursorposX == 0)         //Already at the first column
                cursorposY = 9;         //Roll to last
            else
                cursorposY -= 1;
            refresh = true;
        }

        if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_RIGHT)){
            if(cursorposX == 9)         //Already at the last column
                cursorposY = 0;         //Roll to first
            else
                cursorposY += 1;
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
