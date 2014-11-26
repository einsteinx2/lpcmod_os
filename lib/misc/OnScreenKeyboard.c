#include "boot.h"
#include "video.h"
#include "memory_layout.h"
#include "lib/LPCMod/BootLCD.h"

#define FULL_KEYBOARD   0
#define IP_KEYPAD   1

char keymap[4][10] = {{'1','2','3','4','5','6','7','8','9','0'},
                      {'A','B','C','D','E','F','G','H','I','J'},
                      {'K','L','M','N','O','P','Q','R','S','T'},
                      {'U','V','W','X','Y','Z',',','.','\'',';'}};

char shiftkeymap[4][10] = {{'!','@','#','$','%','?','&','*','(',')'},
                           {'a','b','c','d','e','f','g','h','i','j'},
                           {'k','l','m','n','o','p','q','r','s','t'},
                           {'u','v','w','x','y','z','-','+','\"',':'}};

char ipKeypad[4][3] = {{'1','2','3'},
                       {'4','5','6'},
                       {'7','8','9'},
                       {' ','0','.'}};

void OnScreenKeyboard(char * string, u8 maxLength, u8 line, u8 kbType) {
    bool exit = false;
//    bool result = false;        //Start assuming user will not change string.
    u8 cursorposX = 0;
    u8 cursorposY = 0;
    u8 textpos = 0;
    u8 x,y;
    bool shift = false;
    bool refresh = true;
    u8 rowLength;
    //Array of function pointers to let "line" value decide which function needs to be called.
    void (*Printline[4])(bool centered, char *lineText) = {(xLCD.PrintLine0), (xLCD.PrintLine1), (xLCD.PrintLine2), (xLCD.PrintLine3)};
    char oldString[20];
    sprintf(oldString, "%s", string);	//Copy input string in case user cancels.
    textpos = strlen(string);           //Place cursor at end of entering string.
    
    if(kbType == IP_KEYPAD)
        rowLength = 3;
    else
        rowLength = 10; //Full keyboard by default;

    while(1){
        if(refresh){
            BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
            VIDEO_CURSOR_POSX=50;
            VIDEO_CURSOR_POSY=40;
            VIDEO_ATTR=0xffffffff;                        //White characters.
            if(kbType == IP_KEYPAD)
                printk("\n\1                       Back=Cancel   Start=Confirm   B=Backspace");
            else
                printk("\n\1             Back=Cancel   Start=Confirm   B=Backspace   X=Space   Y=Shift");
            VIDEO_ATTR=0xffff9f00;                	  //Orangeish
            VIDEO_CURSOR_POSX=75;
            VIDEO_CURSOR_POSY=50;
            printk("\n\n\n\n\2                 %s", string);
            VIDEO_ATTR=0xffffffff;
            if(kbType == IP_KEYPAD){
                printk("\n\n\n                                  ");
            }
            else{
                printk("\n\n\n           ");
            }
            for(y = 0; y < 4; y++){
                if(kbType == IP_KEYPAD){
                    printk("\n\n\n                                  ");
                }
                else{
                    printk("\n\n\n           ");
                }
                for(x = 0; x < rowLength; x++){
                    if(x == cursorposX && y == cursorposY)      //About to draw selected character
                        VIDEO_ATTR=0xffffef37;                  //In yellow
                    else
                        VIDEO_ATTR=0xffffffff;                  //the rest in white.
                    if(kbType == IP_KEYPAD){
                        printk("\2%c    ",ipKeypad[y][x]);
                    }
                    else{
                        if(shift){
                            printk("\2%c    ",shiftkeymap[y][x]);
                        }
                        else {
                            printk("\2%c    ",keymap[y][x]);
                        }
                    }
                }
            }
            if(xLCD.enable == 1){
                (*Printline[line])(JUSTIFYLEFT, string);
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

        if(kbType != IP_KEYPAD){
            if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_Y) == 1){   //Shift toggle
                shift = !shift;
                refresh = true;
            }
        }

        if(kbType != IP_KEYPAD){
            if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_X) == 1){   //Space
                if(textpos < maxLength){
                    string[textpos] = ' ';                  //Add space character
                    textpos += 1;                           //Move cursor one position to the right
                    refresh = true;
                }
            }
        }

        if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_B) == 1){   //Backspace
            if(textpos > 0){
                textpos -= 1;                               //Move cursor one position to the left
                string[textpos] = '\0';                     //Erase character
                refresh = true;
            }
        }

        if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) == 1){   //Select character
            if(textpos < maxLength){
                if(shift){
                    string[textpos] = shiftkeymap[cursorposY][cursorposX];
                }
                else {
                    string[textpos] = keymap[cursorposY][cursorposX];
                }
                                           //Move cursor one position to the right
                refresh = true;
            }
            if(textpos < maxLength - 1)
            	textpos += 1;
        }

        if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_UP) == 1){
            if(cursorposY == 0)         //Already at the top line
                cursorposY = 3;         //Roll to last
            else
                cursorposY -= 1;
            refresh = true;
        }

        if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_DOWN) == 1){
            if(cursorposY == 3)         //Already at the last line
                cursorposY = 0;         //Roll to top
            else
                cursorposY += 1;
            refresh = true;
        }

        if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_LEFT) == 1){
            if(cursorposX == 0)         //Already at the first column
                cursorposX = 9;         //Roll to last
            else
                cursorposX -= 1;
            refresh = true;
        }

        if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_RIGHT) == 1){
            if(cursorposX == 9)         //Already at the last column
                cursorposX = 0;         //Roll to first
            else
                cursorposX += 1;
            refresh = true;
        }
    
        if(exit){  
            BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
            VIDEO_CURSOR_POSX=0;
            VIDEO_CURSOR_POSY=0;
            return;
        }
        wait_ms(10);
    }
    return;    //Keep compiler happy.
}
