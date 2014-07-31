////////////////////// compile-time options ////////////////////////////////

//Gentoox Loader version number
#define VERSION "6.07"

// selects between the supported video modes, see boot.h for enum listing those available
//#define VIDEO_PREFERRED_MODE VIDEO_MODE_800x600
#define VIDEO_PREFERRED_MODE VIDEO_MODE_640x480

//Uncomment to include BIOS flashing support from CD
#define FLASH

//Uncomment to enable the 'advanced menu'
#define ADVANCED_MENU

//Time to wait in seconds before auto-selecting default boot item
#define BOOT_TIMEWAIT 5

//Time to wait in seconds before auto-selecting default menu item
#define MENU_TIMEWAIT 5

//Uncomment to make connected Xpads rumble briefly at init.
//#define XPAD_VIBRA_STARTUP

//Uncomment for ultra-quiet mode. Menu is still present, but not
//shown unless you move the xpad. Just backdrop->boot, otherwise
#define SILENT_MODE

//Obsolete


// display a line like Composite 480 detected if uncommented
#undef REPORT_VIDEO_MODE

// show the MBR table if the MBR is valid
#undef DISPLAY_MBR_INFO

#undef DEBUG_MODE
// enable logging to serial port.  You probably don't have this.
#define INCLUDE_SERIAL 0
// enable trace message printing for debugging - with serial only
#define PRINT_TRACE 0
// enable/ disable Etherboot
#undef ETHERBOOT
// enable/ disable LWIP
#define LWIP

// IP configuration for WebBoot
#define WB_BLOCK_A 192
#define WB_BLOCK_B 168
#define WB_BLOCK_C 1
#define WB_BLOCK_D 200
#define WB_PORT 80

// IP configuration for WebUpdate
#define WF_BLOCK_A 193
#define WF_BLOCK_B 1
#define WF_BLOCK_C 193
#define WF_BLOCK_D 66
#define WF_PORT 80
