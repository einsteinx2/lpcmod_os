# README #

This is the code repository for the XBlast's Xbox Legal "OS".

It is a replacement firmware or "BIOS" to run on the Original Xbox video game console. It also serves as software control interface for the "Xblast" line of modding devices for Xbox.

This code is based on the excellent Gentoox Loader software which is based on Cromwell.
#### This software contains no copyrighted code ####
#### This software cannot circumvent security mechanism of an Xbox console ####

### Required setup to build ###

x86 or x86_64 Linux system with gcc installed

ia32 compatiblity layer packages must also be installed on x86_64 systems

Currently building successfully with gcc-4, gcc-5 and gcc-6.

A Virtual Machine image is available for download. It is ready to go for building Cromwell-Based software.

Link for dowwnload: [Google Drive](https://drive.google.com/drive/folders/1gb-PGhSQmvJLW4VFF1y_tesKdOr_IGHN?usp=sharing)

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