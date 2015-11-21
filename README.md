# README #

This is the code repository for the XBlast's Xbox Legal "OS".
This code is based on the excellent Gentoox Loader software which is based on Cromwell.
#### This software contains no copyrighted code ####
#### This software cannot circumvent security mechanism of an Xbox console ####

### Required setup to build ###

For now, I was only able to build a working binary under Linux.
Here's my working setup:

* Lubuntu x86 14.04
* Make 3.81
* gcc-3.3 "3.3.6 (Ubuntu 1:3.3.6-15ubuntu6)" TARGET=i486-linux-gnu
* ld 2.24


### Important information ###

This program is in it's infancy stage. Expect lots of updates.

There is no code size protection inside the 256KB range. When building the image, it is important that the imagebld tool report at least 4KB of unused space. Settings for the OS are dynamically written in the last 4 KB of flash of this image. I will think of something to protect this space.

### Who do I talk to? ###

* bennydiamond on AssemblerGames forums
* psyko_chewbacca on XBMC4Xbox forums


### Implemented features ###

This project is available in both "BIOS" and "XBE"(Xbox executable) form. The features for the 2 versions are not all the same. Generally speaking, the XBE version contains less feature. 

Also note that this software can detect if a firmware replacement device is inserted onto the LPC port. Certain options specifically related to XBlast Mod will not be available if the proper hardware is not detected. However, HD44780 LCD ouput is supported on the whole range of SmartXX devices and on Xecuter 3(CE).

Notable feature available on both versions are:

* Change video settings (includes DVD,Game and Video region).
* 128MB RAM tester
* Reset user settings in Xbox EEPROM
* Flash LPC/TSOP with image from HDD/CD/HTTP (limited to current bank on non XBlast mod)
* Lock/Unlock HDD, Display HDD info, format drives (64KB clusters supported)

Notable features only available in BIOS version

* LCD ouput supported on all SmartXX/Xecuter3 and XBlast mod
* Save OS settings and backup eeprom to flash (not supported on SmartXX)

Notable features only available on XBlast mod

* Control multiple flash banks
* Custom names for flash banks(outputs on LCD too)
* Quickboot bank(bypass OS)
* TSOP control for multiple BIOS banks (Xbox 1.0/1.1 only).

Details on features will follow.