#ifndef lpcmod_v1_h
#define lpcmod_v1_h

//0x00FE always read 0xAA if no physical tampering with SW1-2 port.
#define READPSW1 0x80
#define READPSw2 0x20

//0x00FF register bits configuration
#define KILL_MOD 0x20   //Completely mute modchip until a power cycle
#define GROUNDA15 0x08 // Enable bit to ground TSOP signal
#define GROUNDD0  0x04 // Enable bit to ground TSOP signal
#define RELEASED0 0x00 // Does not interfere with A19control and A19value
//Values used in Evolution-X dash to toggle banks.
#define XODUSFULLTSOP 0x40	//Make sure to drop D0, A19control and A19value(while we're at it)
#define XODUSBNK0TSOP 0x45      //Values on XODUS_CONTROL register to switch banks on TSOP.
#define XODUSBNK1TSOP 0x47      //D0 signal must be set to '1' when TSOP is split
#define XODUSBNK0MOD 0x44       //Values on XODUS_CONTROL register to switch user banks on XBlast Mod.
#define XODUSBNK1MOD 0x46       //D0 signal must be set to '1'
#define BOOTFROMTSOP 0x50 //Delimiter in logic to differentiate booting from TSOP and from on board flash.

//0x00FF read register bits
#define READISW1VALUE   0x10
#define READA19CTRL     0x20
#define READISW2VALUE   0x40
#define READA19VALUE	0x01
#define READA15CONTROL	0x02
#define READCHAMMODE1   0x04
#define READCHAMMODE0   0x08
#define READCHAMMODE    0x0C

//0xF70D read
#define READGPO  0xF0
#define READGPO3 0x80
#define READGPO2 0x40
#define READGPO1 0x20
#define READGPO0 0x10
#define READGPI  0x0C
#define READGPI1 0x08
#define READGPI0 0x04
#define READ5V   0x02
#define READOSTSOPCTRL 0x01

//0xF70D write register bits configuration
#define GPO3_ON 0x80u
#define GPO2_ON 0x40u
#define GPO1_ON 0x20u
#define GPO0_ON 0x10u
#define ENABLE_5V 0x01u

//0xF70F write register bits configuration
#define OSBNKCTRLBIT    0x80u    //Bit that must be sent when selecting a flash bank other than BNKOS
#define OSKILLMOD       0x20u    //Completely mute modchip until a power cycle
#define TSOPA19CTRLBIT  0x10u    //Bit to enable manual drive of the TSOP's A19 pin.
#define OSGROUNDD0      0x04u    //Won't be used much here.
#define BNKFULLTSOP    0x00u
#define BNKTSOPSPLIT0 0x10u
#define BNKTSOPSPLIT1 0x18u
#define NOBNKID  0xFF
//XBlast Mod bank toggle values
#define BNK512  0x84u
#define BNK256  0x86u
#define BNKOS  0x87u


typedef enum
{
    FlashBank_OSBank = BNKOS,
    FlashBank_512Bank = BNK512,
    FlashBank_256Bank = BNK256,
    FlashBank_FullTSOPBank = BNKFULLTSOP,
    FlashBank_SplitTSOP0Bank = BNKTSOPSPLIT0,
    FlashBank_SplitTSOP1Bank = BNKTSOPSPLIT1,
    FlashBank_NoBank = NOBNKID
} FlashBank;

#define LPCMOD_TRUE 0x01u
#define LPCMOD_FALSE    0x00u


//XBlast Mod and SmartXX LPC registers to drive LCD
#define XBLAST_IO    0xF70Du
#define XBLAST_CONTROL    0xF70Fu
#define XODUS_CONTROL    0x00FFu
#define LCD_DATA    0xF700u
#define LCD_BL        0xF701u
#define LCD_CT        0xF703u
#define XODUS_ID      0x00FEu

#define SMARTXX_FLASH_WRITEPROTECT 0xF704

//Xecuter 3 LPC registers to drive LCD
#define X3_DISP_O_DAT      0xF504u
#define X3_DISP_O_CMD      0xF505u
#define X3_DISP_O_DIR_DAT  0xF506u
#define X3_DISP_O_DIR_CMD  0xF507u
#define X3_DISP_O_LIGHT    0xF503u



#define SYSCON_REG    0xF701u
//For XBlast Lite
#define SYSCON_ID_V1    0x15u    //Spoof SmartXX OPX using lower nibble. Highest nibble is what's different from SmartXX.
#define SYSCON_ID_V1_PRE_EDITION 0x55u//Special version of SYSCON for the special "Pre-Edition" of the XBlast Lite V1.
//Other revision will have to be identified here.
#define SYSCON_ID_V1_TSOP    0x1015u     //Arbitrary value, constant used to specify to program that XBlast mod was
                                        //detected but active flash device detected is not SST49LF080A which suggest a TSOP boot
#define SYSCON_ID_XT    0x11u		//Aladdin XT Diamond mod.
#define SYSCON_ID_XT_TSOP 0x1011u

//SmartXX chip id
#define SYSCON_ID_XX1   0xF1u
#define SYSCON_ID_XX2   0xF2u
#define SYSCON_ID_XXOPX 0xF5u
#define SYSCON_ID_XX3   0xF8u

#define SYSCON_ID_X3    0x2Au

#define SYSCON_ID_CHAM 0xAAu     //Xodus Chameleon ID at 0x00FE

#define SYSCON_ID_UNKNOWN 0x00u
#define SYSCON_ID_UNKNOWN_TSOP 0x1000u

//Other modchips ID by flash type.
#define FLASH_ID_XECUTER3       0x01ADu
#define FLASH_ID_XECUTER2       0x01D5u
#define FLASH_ID_XENIUM         0x01C4u


#define LCDTYPE_HD44780        0x0u
#define LCDTYPE_KS0073         0x1u


#endif
