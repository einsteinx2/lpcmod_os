/*
 *
 *
 */

#ifndef _BootLCD_H_
#define _BootLCD_H_

//Various predefined values to write to LCD to init.
//bit0 = RS
//bit1 = E
//bit2 = D4
//bit3 = D5
//bit4 = D6
//bit5 = D7
#define DISPLAY_E 		0x04		//Mapped for LPCMod output format.
#define DISPLAY_RS		0x02		//Mapped for LPCMod output format.
#define DISPLAY_FUNCTION_SET 	0x20
#define DISPLAY_DL_FLAG 	0x10	//4 bit mode flag
#define DISP_CLEAR		0x01
#define DISP_HOME		0x02
#define DISP_ENTRY_MODE_SET	0x04
#define DISP_S_FLAG		0x01
#define DISP_ID_FLAG		0x02
#define DISP_CONTROL		0x08
#define DISP_D_FLAG		0x04
#define DISP_C_FLAG		0x02
#define DISP_B_FLAG		0x01
#define DISP_SHIFT		0x10
#define DISP_SC_FLAG		0x08
#define DISP_RL_FLAG		0x04
#define DISP_FUNCTION_SET	0x20
#define DISP_DL_FLAG		0x10
#define DISP_N_FLAG		0x08
#define DISP_F_FLAG		0x04
#define DISP_CGRAM_SET		0x40
#define DISP_DDRAM_SET		0x80
#define DISP_RE_FLAG  0x04
#define DISP_SEGRAM_SET  0x40
#define DISP_NW_FLAG  0x01

void BootLCDInit(void);

void toggleEN5V(u8 value);
void setLCDContrast(u8 value);
void setLCDBacklight(u8 value);

void assertInitLCD(void);

void WriteLCDInit(struct Disp_controller *xLCD);
void WriteLCDCommand(struct Disp_controller *xLCD, u8 value);
void WriteLCDData(struct Disp_controller *xLCD, u8 value);
void WriteLCDIO(struct Disp_controller *xLCD, u8 data, bool RS, u16 wait);
void WriteLCDPoll(struct Disp_controller *xLCD);
void WriteLCDLine1(struct Disp_controller *xLCD, char *lineText);
void WriteLCDLine2(struct Disp_controller *xLCD, char *lineText);
void WriteLCDLine3(struct Disp_controller *xLCD, char *lineText);
void WriteLCDLine4(struct Disp_controller *xLCD, char *lineText);
u8 WriteLCDNibbleGen(u8 value);
void WriteLCDIO_strobe(struct Disp_controller *xLCD, u8 data, u8 flag, u32 waitTime);
void WriteLCDTrimString(char * StringOut, char * stringIn);

#endif // _BootLCD_H_
