////////////////////// compile-time options ////////////////////////////////

#define PROG_NAME "XBlast OS"

//XBlast OS version number
#define VERSION "0.41 (beta)"

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


// display a line like Composite 480 detected if uncommented
#undef REPORT_VIDEO_MODE

// show the MBR table if the MBR is valid
#undef DISPLAY_MBR_INFO

#undef DEBUG_MODE
// enable/ disable Etherboot
#undef ETHERBOOT
// enable/ disable LWIP
#define LWIP

