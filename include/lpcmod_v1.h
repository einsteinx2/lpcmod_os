#ifndef lpcmod_v1_h
#define lpcmod_v1_h

//0x00FF register bits configuration
#define KILL_MOD 0x20   //Completely mute modchip until a power cycle
#define GROUNDA15 0x08 // Enable bit to ground TSOP signal
#define GROUNDD0  0x04 // Enable bit to ground TSOP signal
#define RELEASED0 0x00 // Load a full TSOP only requires to release D0.
#define BNKFULLTSOP 0x03	//Don't do nothing to the register
#define BOOTFROMTSOP 0x70 //Delimiter in logic to differentiate booting from TSOP and from on board flash.

//0x00FF register read bits
#define READA19VALUE	0x01
#define READA19CONTROL	0x02
#define READD0CONTROL	0x04
#define READA15CONTROL	0x08
#define READSWVALUES	0x30
#define READGPI		0xC0

//0xF70E register bits configuration
#define OSBNKCTRLBIT    0x80    //Bit that must be sent when selecting a flash bank other than BNKOS
#define TSOPA19CTRLBIT  0x10    //Bit to enable manual drive of the TSOP's A19 pin.
#define BNK512  0x80
#define BNK256  0x82
#define BNKOS  0x83
#define BNKTSOPSPLIT0 0x10
#define BNKTSOPSPLIT1 0x18
#define NOBNKID  0xFF

//0xF70F register bits configuration
#define GPO3_ON 0x80
#define GPO2_ON 0x40
#define GPO1_ON 0x20
#define GPO0_ON 0x10
#define ENNABLE_5V 0x01



#define LPCMOD_TRUE 0x01
#define LPCMOD_FALSE    0x00


//XBlast Mod and SmartXX LPC registers to drive LCD
#define XBLAST_CONTROL    0xF70E
#define XODUS_CONTROL    0x00FF
#define PIN_CONTROL    0xF70F
#define LCD_DATA    0xF700
#define LCD_BL        0xF701
#define LCD_CT        0xF703
#define XODUS_ID      0x00FE

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
#define SYSCON_ID_V1_TSOP    0x1015     //Arbitrary value, constant used to specify to program that XBlast mod was
                                        //detected but active flash device detected is not SST49LF080A which suggest a TSOP boot

//SmartXX chip id
#define SYSCON_ID_XX1   0xF1
#define SYSCON_ID_XX2   0xF2
#define SYSCON_ID_XXOPX 0xF5
#define SYSCON_ID_XX3   0xF8

#define SYSCON_ID_X3    0x2A

#define SYSCON_ID_CHAM 0xAA     //Xodus Chameleon ID at 0x00FE

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
