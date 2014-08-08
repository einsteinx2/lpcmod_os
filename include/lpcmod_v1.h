#ifndef lpcmod_v1_h
#define lpcmod_v1_h

#define BNK512  0x00
#define BNK256  0x02
#define BNKOS  0x03
#define BNKTSOP 0x04
#define LPCMOD_TRUE 0x01
#define LPCMOD_FALSE	0x00


#define BNK_CONTROL	0xF710
#define DISABLE_MOD	0xF711
#define ENABLE_5V	0xF713
#define LCD_DATA	0xF701
#define LCD_BL		0xF700
#define LCD_CT		0xF703

#define SYSCON_REG	0xF701
#define SYSCON_ID	0x15	//Spoof SmartXX OPX using lower nibble. Highest nibble is what's different from SmartXX.

#endif
