# README #

This is the code repository for the LPCMod<s Xbox Modchip "OS".
This code is based on the excellent Gentoox Loader software which is based on Cromwell.

### Required setup to build ###

For now, I was only able to build a working binary under Linux.
Here's my working setup:
Lubuntu x86 14.04
Make 3.81
gcc-3.3 "3.3.6 (Ubuntu 1:3.3.6-15ubuntu6)" i486-linux-gnu
ld 2.24


### Important information ###

This program is in it's infancy stage. Expect lots of updates.

There is no code size protection inside the 256KB range. When building the image, it is important that the imagebld tool report at least 4KB of unused space. Settings for the OS are dynamically written in the last 4 KB of flash of this image.

### Who do I talk to? ###

* bennydiamond on AssemblerGames forums
* psyko_chewbacca on XBMC4Xbox forums