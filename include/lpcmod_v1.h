#ifndef lpcmod_v1_h
#define lpcmod_v1_h

#define OSBNKCTRLBIT    0x40    //Bit that must be sent when selecting a flash bank other than BNKOS

#define BNK512  0x40
#define BNK256  0x60
#define BNKOS  0x70
#define BNKTSOP 0x44            //When TSOP is not split
#define BNKTSOPSPLIT0 0x46
#define BNKTSOPSPLIT1 0x47
#define NOBNKID  0xFF
#define LPCMOD_TRUE 0x01
#define LPCMOD_FALSE    0x00

//Bit to send on DISABLE_MOD "register" to control OnBoard TSOP.
//Will only be used if Xbox rev. is 1.0 or 1.1.
#define TSOP_BOOT        0x01	//Just release D0
#define TSOP_CONTROL    0x02	//Allow modchip to control TSOP A19/A18 lines
#define TSOP_4_BANKS    0x04	//Tell modchip "1" = 4-way split, "0" = 2-way split
#define TSOP_512_SWITCH    0x10 //Actual A19 desired value
#define TSOP_256_SWITCH    0x20 //Actual A18 desired value
#define TSOP_DISABLE_MOD   0x80 //Totally mute modchip. no comm possible until power cycle


//XBlast Mod and SmartXX LPC registers to drive LCD

/*XBLAST_CONTROL register:
 * bit7: Not used
 * bit6: OS banks control (disable SW1 and SW2 physical input pins)
 * bit5: SW1 logical input (control A19 on flash chip)
 * bit4: SW2 logical input (control A18 on flash chip)
 * bit3: Enable 5V rail (LCD power)
 * bit2: Not Used
 * bit1: Write to GPO_1 physical output
 * bit0: Write to GPO_0 physical output
 */
#define XBLAST_CONTROL    0xF70F
#define XODUS_CONTROL    0x00FF
#define XODUS_ID        0x00FE
#define LCD_DATA    0xF700
#define LCD_BL        0xF701
#define LCD_CT        0xF703


//Xecuter 3 LPC registers to drive LCD
#define X3_DISP_O_DAT      0xF504
#define X3_DISP_O_CMD      0xF505
#define X3_DISP_O_DIR_DAT  0xF506
#define X3_DISP_O_DIR_CMD  0xF507
#define X3_DISP_O_LIGHT    0xF503



#define SYSCON_REG    0xF701
//For XBlast Lite
#define SYSCON_ID_V1    0x15    //Spoof SmartXX OPX using lower nibble. Highest nibble is what's different from SmartXX.
//Other revision will have to be identified here.

//SmartXX chip id
#define SYSCON_ID_XX1   0xF1
#define SYSCON_ID_XX2   0xF2
#define SYSCON_ID_XXOPX 0xF5
#define SYSCON_ID_XX3   0xF8

#define SYSCON_ID_X3    0x2A

//Other modchips ID by flash type.
#define FLASH_ID_XECUTER3       0x01AD
#define FLASH_ID_XECUTER2       0x01D5
#define FLASH_ID_XENIUM         0x01C4


#define HD44780        0x0

/*
#define ADR_ACTIVEBANK 0xFF010
#define ADR_QUICKBOOT 0xFF011
#define ADR_ACTIVEITEM 0xFF012
#define ADR_FANSPEED 0xFF013
#define ADR_BOOTTIMEOUT 0xFF014

#define ADR_BIOSNAME0 0xFF023
#define ADR_BIOSNAME1 0xFF037
#define ADR_BIOSNAME2 0xFF04B
#define ADR_BIOSNAME3 0xFF05F
#define ADR_BIOSNAME4 0xFF073
#define ADR_BIOSNAME5 0xFF087
#define ADR_BIOSNAME6 0xFF09B
#define ADR_BIOSNAME7 0xFF0AF
#define ADR_BIOSNAME0 0xFF0C3

#define ADR_ENABLENET 0xFF0ED
#define ADR_USEDHCP 0xFF0EE
#define ADR_IP 0xFF0EF
#define ADR_GATEWAY 0xFF0F3
#define ADR_DNS1 0xFF0F7
#define ADR_DNS2 0xFF0FB

#define ADR_EN5V 0xFF101
#define ADR_LCDTYPE 0xFF102
#define ADR_NBLINES 0xFF103
#define ADR_LINELENGTH 0xFF104
#define ADR_BACKLIGHT 0xFF105
#define ADR_CONTRAST 0xFF106
#define ADR_DISPMSGBOOT 0xFF107
#define ADR_CUSTOMTEXT 0xFF108
#define ADR_BIOSNAMEDISP 0xFF109
#define ADR_LCDSTRING0 0xFF110
#define ADR_LCDSTRING0 0xFF124
#define ADR_LCDSTRING0 0xFF138
#define ADR_LCDSTRING0 0xFF14C
*/


#endif
