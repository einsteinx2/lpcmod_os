#ifndef _Boot_H_
#define _Boot_H_

#include "config.h"

/***************************************************************************
      Includes used by XBox boot code
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/////////////////////////////////
// configuration

#include "consts.h"
#include "stdint.h"
#include "cromwell_types.h"


unsigned int cromwell_config;
unsigned int cromwell_retryload;
unsigned int cromwell_loadbank;
unsigned int cromwell_Biostype;

unsigned int xbox_ram;

#define XROMWELL    0
#define CROMWELL    1

#define ICON_WIDTH 64
#define ICON_HEIGHT 64

static inline double min (double a, double b)
{
    if (a < b) return a; else return b;
}

static inline double max (double a, double b)
{
    if (a > b) return a; else return b;
}

#include "iso_fs.h"
#include "BootVideo.h"

#define ASSERT(exp) { if(!(exp)) { bprintf("Assert failed file " __FILE__ " line %d\n", __LINE__); } }

extern volatile CURRENT_VIDEO_MODE_DETAILS vmode;
unsigned int video_encoder;

volatile u32 VIDEO_CURSOR_POSX;
volatile u32 VIDEO_CURSOR_POSY;
volatile u32 VIDEO_ATTR;
volatile u32 VIDEO_LUMASCALING;
volatile u32 VIDEO_RSCALING;
volatile u32 VIDEO_BSCALING;
volatile u32 BIOS_TICK_COUNT;
volatile u32 VIDEO_VSYNC_POSITION;
volatile u32 VIDEO_VSYNC_DIR;
volatile u32 DVD_TRAY_STATE;

u8 VIDEO_AV_MODE ;

#define DVD_CLOSED         0
#define DVD_CLOSING         1
#define DVD_OPEN           2
#define DVD_OPENING           3

/////////////////////////////////
// Superfunky i386 internal structures

typedef struct gdt_t {
        unsigned short       m_wSize __attribute__ ((packed));
        unsigned long m_dwBase32 __attribute__ ((packed));
        unsigned short       m_wDummy __attribute__ ((packed));
} ts_descriptor_pointer;

typedef struct {  // inside an 8-byte protected mode interrupt vector
    u16 m_wHandlerHighAddressLow16;
    u16 m_wSelector;
    u16 m_wType;
    u16 m_wHandlerLinearAddressHigh16;
} ts_pm_interrupt;

typedef enum {
    EDT_UNKNOWN= 0,
    EDT_XBOXFS
} enumDriveType;

typedef struct tsHarddiskInfo {  // this is the retained knowledge about an IDE device after init
    unsigned short m_fwPortBase;
    unsigned short m_wCountHeads;
    unsigned short m_wCountCylinders;
    unsigned short m_wCountSectorsPerTrack;
    unsigned long m_dwCountSectorsTotal; /* total */
    unsigned char m_bLbaMode;    /* am i lba (0x40) or chs (0x00) */
    unsigned char m_szIdentityModelNumber[40];
    unsigned char term_space_1[2];
    unsigned char m_szSerial[20];
    unsigned char term_space_2[2];
    char m_szFirmware[8];
    unsigned char term_space_3[2];
    unsigned char m_fDriveExists;
    unsigned char m_fAtapi;  // true if a CDROM, etc
    enumDriveType m_enumDriveType;
    unsigned char m_bCableConductors;  // valid for device 0 if present
    unsigned short m_wAtaRevisionSupported;
    unsigned char s_length;
    unsigned char m_length;
    char m_fHasMbr : 2;
    unsigned char m_bIORDY : 2;
    unsigned char m_fDMAInit : 4;
    unsigned short m_securitySettings; //This contains the contents of the ATA security regs
    unsigned char m_maxBlockTransfer;  //Max number of blocks allowed in a single transfer.
    unsigned short m_minPIOcycle;
} tsHarddiskInfo;

/////////////////////////////////
// LED-flashing codes
// or these together as argument to I2cSetFrontpanelLed

enum {
    I2C_LED_RED0 = 0x80,
    I2C_LED_RED1 = 0x40,
    I2C_LED_RED2 = 0x20,
    I2C_LED_RED3 = 0x10,
    I2C_LED_GREEN0 = 0x08,
    I2C_LED_GREEN1 = 0x04,
    I2C_LED_GREEN2 = 0x02,
    I2C_LED_GREEN3 = 0x01
};

///////////////////////////////
/* BIOS-wide error codes        all have b31 set  */

enum {
    ERR_SUCCESS = 0,  // completed without error

    ERR_I2C_ERROR_TIMEOUT = 0x80000001,  // I2C action failed because it did not complete in a reasonable time
    ERR_I2C_ERROR_BUS = 0x80000002, // I2C action failed due to non retryable bus error

    ERR_BOOT_PIC_ALG_BROKEN = 0x80000101 // PIC algorithm did not pass its self-test
};

/////////////////////////////////
// some Boot API prototypes

//////// BootPerformPicChallengeResponseAction.c

/* ----------------------------  IO primitives -----------------------------------------------------------
*/

static __inline void IoOutputByte(u16 wAds, u8 bValue) {
//    __asm__  ("                 out    %%al,%%dx" : : "edx" (dwAds), "al" (bValue)  );
    __asm__ __volatile__ ("outb %b0,%w1": :"a" (bValue), "Nd" (wAds));
}

static __inline void IoOutputWord(u16 wAds, u16 wValue) {
//    __asm__  ("     out    %%ax,%%dx    " : : "edx" (dwAds), "ax" (wValue)  );
    __asm__ __volatile__ ("outw %0,%w1": :"a" (wValue), "Nd" (wAds));
    }

static __inline void IoOutputDword(u16 wAds, u32 dwValue) {
//    __asm__  ("     out    %%eax,%%dx    " : : "edx" (dwAds), "ax" (wValue)  );
    __asm__ __volatile__ ("outl %0,%w1": :"a" (dwValue), "Nd" (wAds));
}


static __inline u8 IoInputByte(u16 wAds) {
  unsigned char _v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (_v):"Nd" (wAds));
  return _v;
}

static __inline u16 IoInputWord(u16 wAds) {
  u16 _v;

  __asm__ __volatile__ ("inw %w1,%0":"=a" (_v):"Nd" (wAds));
  return _v;
}

static __inline u32 IoInputDword(u16 wAds) {
  u32 _v;

  __asm__ __volatile__ ("inl %w1,%0":"=a" (_v):"Nd" (wAds));
  return _v;
}

#define rdmsr(msr,val1,val2) \
       __asm__ __volatile__("rdmsr" \
                : "=a" (val1), "=d" (val2) \
                : "c" (msr))

#define wrmsr(msr,val1,val2) \
     __asm__ __volatile__("wrmsr" \
              : /* no outputs */ \
              : "c" (msr), "a" (val1), "d" (val2))


void BootPciInterruptEnable(void);

    // boot process
int BootPerformPicChallengeResponseAction(void);
    // LED control (see associated enum above)
int I2cSetFrontpanelLed(u8 b);

#define bprintf(...)

#if PRINT_TRACE
#define TRACE bprintf(__FILE__ " :%d\n\r",__LINE__);
#else
#define TRACE
#endif

typedef struct _LIST_ENTRY {
    struct _LIST_ENTRY *m_plistentryNext;
    struct _LIST_ENTRY *m_plistentryPrevious;
} LIST_ENTRY;

void ListEntryInsertAfterCurrent(LIST_ENTRY *plistentryCurrent, LIST_ENTRY *plistentryNew);
void ListEntryRemove(LIST_ENTRY *plistentryCurrent);

////////// BootPerformXCodeActions.c

int BootPerformXCodeActions(void);

#include "BootEEPROM.h"
#include "BootParser.h"

////////// BootStartBios.c

void StartBios(CONFIGENTRY *config,int nActivePartition, int nFATXPresent,int bootfrom);
int BootMenu(CONFIGENTRY *config,int nDrive,int nActivePartition, int nFATXPresent);

////////// BootResetActions.c
void ClearIDT (void);
void BootResetAction(void);
void BootCpuCache(bool fEnable) ;
int printk(const char *szFormat, ...);
void BiosCmosWrite(u8 bAds, u8 bData);
u8 BiosCmosRead(u8 bAds);


///////// BootPciPeripheralInitialization.c
void BootPciPeripheralInitialization(void);
void BootAGPBUSInitialization(void);
void BootDetectMemorySize(void);
extern void    ReadPCIByte(unsigned int bus, unsigned int dev, unsigned intfunc,     unsigned int reg_off, unsigned char *pbyteval);
extern void    WritePCIByte(unsigned int bus, unsigned int dev, unsigned int func,    unsigned int reg_off, unsigned char byteval);
extern void    ReadPCIDword(unsigned int bus, unsigned int dev, unsigned int func,    unsigned int reg_off, unsigned int *pdwordval);
extern void    WritePCIDword(unsigned int bus, unsigned int dev, unsigned int func,        unsigned int reg_off, unsigned int dwordval);
extern void    ReadPCIBlock(unsigned int bus, unsigned int dev, unsigned int func,        unsigned int reg_off, unsigned char *buf,    unsigned int nbytes);
extern void    WritePCIBlock(unsigned int bus, unsigned int dev, unsigned int func,     unsigned int reg_off, unsigned char *buf, unsigned int nbytes);

void PciWriteByte (unsigned int bus, unsigned int dev, unsigned int func,
        unsigned int reg_off, unsigned char byteval);
u8 PciReadByte(unsigned int bus, unsigned int dev, unsigned int func, unsigned int reg_off);
u32 PciWriteDword(unsigned int bus, unsigned int dev, unsigned int func, unsigned int reg_off, u32 dw);
u32 PciReadDword(unsigned int bus, unsigned int dev, unsigned int func, unsigned int reg_off);

///////// BootPerformPicChallengeResponseAction.c

int I2CTransmitWord(u8 bPicAddressI2cFormat, u16 wDataToWrite);
int I2CTransmitByteGetReturn(u8 bPicAddressI2cFormat, u8 bDataToWrite);
bool I2CGetTemperature(int *, int *);
void I2CModifyBits(u8 bAds, u8 bReg, u8 bData, u8 bMask);

///////// BootIde.c

extern tsHarddiskInfo tsaHarddiskInfo[];  // static struct stores data about attached drives
int BootIdeInit(void);
int BootIdeReadSector(int nDriveIndex, void * pbBuffer, unsigned int block, int byte_offset, int n_bytes);
int BootIdeBootSectorHddOrElTorito(int nDriveIndex, u8 * pbaResult);
int BootIdeAtapiAdditionalSenseCode(int nDrive, u8 * pba, int nLengthMaxReturn);
int BootIdeSetTransferMode(int nIndexDrive, int nMode);
int BootIdeSetMultimodeSectors(u8 nIndexDrive, u8 nbSectors);
//int BootIdeSetPIOMode(u8 nIndexDrive, u16 cycleTime);
int BootIdeWaitNotBusy(unsigned uIoBase);
bool BootIdeAtapiReportFriendlyError(int nDriveIndex, char * szErrorReturn, int nMaxLengthError);
void BootIdeAtapiPrintkFriendlyError(int nDriveIndex);

///////// BootUSB.c

void BootStopUSB(void);
void BootStartUSB(void);
void USBGetEvents(void);

#include "xpad.h"

extern struct xpad_data XPAD_current[4];
extern struct xpad_data XPAD_last[4];

extern void wait_ms(u32 ticks);
extern void wait_us(u32 ticks);
extern void wait_smalldelay(void);


void * memcpy(void *dest, const void *src,  size_t size);
void * memset(void *dest, int data,  size_t size);
int memcmp(const void *buffer1, const void *buffer2, size_t num);
int _strncmp(const char *sz1, const char *sz2, int nMax);
int _strncasecmp(const char *sz1, const char *sz2, int nMax);
char * strcpy(char *sz, const char *szc);
char * _strncpy (char * dest, const char * src, size_t n);
void chrreplace(char *string, char search, char ch);

#define printf printk
#define sleep wait_ms
int tolower(int ch);
int isspace (int c);

void MemoryManagementInitialization(void * pvStartAddress, u32 dwTotalMemoryAllocLength);
void * malloc(size_t size);
void free(void *);

extern volatile int nCountI2cinterrupts, nCountUnusedInterrupts, nCountUnusedInterruptsPic2, nCountInterruptsSmc, nCountInterruptsIde;
extern volatile bool fSeenPowerdown;
typedef enum {
    ETS_OPEN_OR_OPENING=0,
    ETS_CLOSING,
    ETS_CLOSED
} TRAY_STATE;
extern volatile TRAY_STATE traystate;


extern void BootInterruptsWriteIdt(void);

int copy_swap_trim(unsigned char *dst, unsigned char *src, int len);
void HMAC_SHA1( unsigned char *result,
                unsigned char *key, int key_length,
                unsigned char *text1, int text1_length,
                unsigned char *text2, int text2_length );

char *HelpGetLine(char *ptr);
void HelpGetParm(char *szBuffer, char *szOrig);
char *strrchr0(char *string, char ch);
void setLED(char *pattern);
int strlen(const char * s);
int sprintf(char * buf, const char *fmt, ...);
char * strstr(const char * s1,const char * s2);
int strlen(const char * s);
int sprintf(char * buf, const char *fmt, ...);
char * strstr(const char * s1,const char * s2);



//Configuration parameters saved in flash
typedef struct _OSsettings {
    u8    migrateSetttings;    //Flag to indicate if settings in this struct should be carried over a OS update.
    u8    reserved[15];
    u8    activeBank;        //Default Flash bank to load BIOS from.
    u8    altBank;          //Alternative BIOS bank to boot holding black button
    u8    Quickboot;        //Bypass OS and load selected bank in "activeBank"
    u8    selectedMenuItem;    //Default selected item in OS menu when booting into it.
    u8    fanSpeed;        //Why not
    u8    bootTimeout;
    u8    LEDColor;
    u8    TSOPcontrol;        //variable contains the following settings: bit0=active?
                    //                                            bit1=Split4ways?
                    //                                            bit2=A19 value
                    //                                            bit3=A18 value
                    //Make sure to mask properly when using this variable.
    u8    reserved1[12];
    char    biosName0[21];        //512KB bank name. 20 characters max to properly display on LCD.
    char    biosName1[21];        //256KB bank name
    char    biosName2[21];        //Reserved for future use.
    char    biosName3[21];        //Reserved for future use.
    char    biosName4[21];        //Reserved for future use.
    char    biosName5[21];        //Reserved for future use.
    char    biosName6[21];        //Reserved for future use.
    char    biosName7[21];        //Reserved for future use.
    u8    reserved2[34];
    u8    enableNetwork;        //Future use. For now, network is enabled only by NetFlash or WebUpdate
    u8    useDHCP;        //Self Explanatory
    u8    staticIP[4];        //Only useful when useDHCP is set to false.
    u8    staticGateway[4];    //Only useful when useDHCP is set to false.
    u8    staticDNS1[4];        //Only useful when useDHCP is set to false.
    u8    staticDNS2[4];        //Only useful when useDHCP is set to false.
}__attribute__((packed))_OSsettings;                //For a total of 256 bytes

typedef struct _LCDsettings {
    u8 migrateLCD;            //Flag to indicate if settings in this struct should be carried over a OS update.
    u8 enable5V;            //Flag to indicate if +5V rail should be enabled(for LCD power)
    u8 lcdType;            //HD44780 only for now
    u8 nbLines;            //User puts 4, meens 2 lines from HD44780 POV
    u8 lineLength;            //Should be 16 or 20 most of the time.
    u8 backlight;            //7-bit value
    u8 contrast;            //7-bit value
    u8 displayMsgBoot;        //Display text on LCD while booting
    u8 customTextBoot;        //Display custom text instead of default text.
    u8 displayBIOSNameBoot;        //Display BIOS name of active bank when booting
    u8 reserved0[5];
    char customString0[21];        //1 of 4 strings to be displayed either when in OS or while booting.
    char customString1[21];        //20 characters max to properly display on LCD.
    char customString2[21];
    char customString3[21];
    u8 reserved1[157];
}__attribute__((packed))_LCDsettings;                //For a total of 256 bytes

typedef struct _LPCmodSettings {
    _OSsettings OSsettings;
    _LCDsettings LCDsettings;
    EEPROMDATA bakeeprom;
}__attribute__((packed)) _LPCmodSettings;	//For a total size of 0x300.


_LPCmodSettings LPCmodSettings;



typedef struct _xLCD {
    int DisplayType;
    int enable;
    int    LineSize;
    int    TimingCMD;
    int    TimingData;

    u8    Line1Start;
    u8    Line2Start;
    u8    Line3Start;
    u8    Line4Start;

    void    (*Init)(void);
    void    (*Command)(u8 value);
    void    (*Data)(u8 value);

    void    (*WriteIO)(u8 data, bool RS, u16 wait);

    void    (*PrintLine0)(bool centered, char *text);
    void    (*PrintLine1)(bool centered, char *text);
    void    (*PrintLine2)(bool centered, char *text);
    void    (*PrintLine3)(bool centered, char *text);

    void    (*ClearLine)(u8 line);
}__attribute__((packed)) _xLCD;    //Will be know as xLCD from now on.

_xLCD xLCD;

//Xbox motherboard revision enum.
typedef enum {
    DEVKIT = 0x00,    //Includes a bunch of revisions
    DEBUGKIT = 0x01,        //2 known version ID
    REV1_0 = 0x02,        //1.0
    REV1_1 = 0x03,        //1.1
    REV1_2 = 0x04,        //1.2/1.3
    REV1_4 = 0x05,        //1.4/1.5
    REV1_6 = 0x06,        //1.6/1.6b
    REVUNKNOWN = 0x07    //dafuk?
} XBOX_REVISION;


//Put here to make it global (yeah yeah... I don't care. There are far worst things done in the VHDL code of the modchip trust me!)
u8 mbVersion;

//Global to hide code when running in XBE without modchip detected.
u16 fHasHardware;

u8 currentFlashBank;

u8 xblastControlRegister;
u8 xodusControlRegister;
#endif // _Boot_H_
