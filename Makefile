#CC	= gcc-3.30 #Left for legacy purpose
PREFIX = #i686-linux-gnu-
CC	= ${PREFIX}gcc  #Builds find using gcc 5.4.0 on a x86 Ubuntu system 

# prepare check for gcc 3.3, $(GCC_3.3) will either be 0 or 1
GCC_3.3 := $(shell expr `$(CC) -dumpversion` \>= 3.3)

GCC_4.2 := $(shell expr `$(CC) -dumpversion` \>= 4.2)

DEBUG ?= 0 #run make with "DEBUG=1" argument to enable extra debug
TSOPCTRL ?= 0 #Override TSOP control availability based on Xbox Revision
VGA ?= 0 #Generates VGA enabled by default image. Does not override existing setting in flash.
ETHERBOOT := yes
LWIPFOLDER := lwip-2.0.1

INCLUDE = -I$(TOPDIR)/grub -I$(TOPDIR)/include -I$(TOPDIR)/ -I./ -I$(TOPDIR)/fs/cdrom \
	-I$(TOPDIR)/fs/fatx -I$(TOPDIR)/fs/grub -I$(TOPDIR)/lib/eeprom -I$(TOPDIR)/lib/crypt \
	-I$(TOPDIR)/drivers/video -I$(TOPDIR)/drivers/ide -I$(TOPDIR)/drivers/flash -I$(TOPDIR)/lib/misc \
	-I$(TOPDIR)/boot_xbe/ -I$(TOPDIR)/fs/grub -I$(TOPDIR)/lib/cromwell/font \
	-I$(TOPDIR)/startuploader -I$(TOPDIR)/drivers/cpu -I$(TOPDIR)/menu \
	-I$(TOPDIR)/lib/jpeg/ -I$(TOPDIR)/menu/actions -I$(TOPDIR)/menu/textmenu \
	-I$(TOPDIR)/menu/iconmenu -I$(TOPDIR)/$(LWIPFOLDER) -I$(TOPDIR)/$(LWIPFOLDER)/src/include \
	-I$(TOPDIR)/$(LWIPFOLDER)/src/include/ipv4 -I$(TOPDIR)/$(LWIPFOLDER)/src/include/lwip/apps

#These are intended to be non-overridable.
CROM_CFLAGS=$(INCLUDE)

#You can override these if you wish.
CFLAGS= -Os -march=pentium -m32 -Werror -Wstrict-prototypes -Wreturn-type -pipe -fomit-frame-pointer  -DIPv4 -fpack-struct -ffreestanding
2BL_CFLAGS= -O2 -march=pentium -m32 -Werror -Wstrict-prototypes -Wreturn-type -pipe -fomit-frame-pointer -fpack-struct -ffreestanding
# add the option for gcc 3.3 only, again, non-overridable
ifeq ($(GCC_3.3), 1)
CROM_CFLAGS += -fno-zero-initialized-in-bss
endif

# add the option for gcc 4.2 only, again, non-overridable
ifeq ($(GCC_4.2), 1)
CFLAGS += -fno-stack-protector -U_FORTIFY_SOURCE
endif

LD      = ${PREFIX}ld
OBJCOPY = ${PREFIX}objcopy

export CC

TOPDIR  := $(shell /bin/pwd)
SUBDIRS	= fs drivers lib boot menu $(LWIPFOLDER) xblast
#### Etherboot specific stuff
ifeq ($(ETHERBOOT), yes)
ETH_SUBDIRS = etherboot
CROM_CFLAGS	+= -DETHERBOOT
ETH_INCLUDE = 	-I$(TOPDIR)/etherboot/include -I$(TOPDIR)/etherboot/arch/i386/include -I$(TOPDIR)
ETH_CFLAGS  = 	-Os -march=pentium -m32 -Werror -Wreturn-type $(ETH_INCLUDE) -Wstrict-prototypes -fomit-frame-pointer -pipe -ffreestanding
# add the option for gcc 4.2 only, again, non-overridable
ifeq ($(GCC_4.2), 1)
ETH_CFLAGS += -fno-stack-protector -U_FORTIFY_SOURCE
endif

ifeq ($(DEBUG), 1)
DEBUG_FLAGS = -DDEV_FEATURES -DSPITRACE
CROM_CFLAGS += $(DEBUG_FLAGS)
ETH_CFLAGS += $(DEBUG_FLAGS)
endif

ifeq ($(TSOPCTRL), 1)
CROM_CFLAGS += -DCUSTOM_TSOP
ETH_CFLAGS += -DCUSTOM_TSOP
endif

ifeq ($(VGA), 1)
CROM_CFLAGS += -DDEFAULT_ENABLE_VGA
endif
endif

LDFLAGS-ROM     = -s -S -T $(TOPDIR)/scripts/ldscript-crom.ld
LDFLAGS-XBEBOOT = -s -S -T $(TOPDIR)/scripts/xbeboot.ld
LDFLAGS-ROMBOOT = -s -S -T $(TOPDIR)/boot_rom/bootrom.ld
LDFLAGS-VMLBOOT = -s -S -T $(TOPDIR)/boot_vml/vml_start.ld
ifeq ($(ETHERBOOT), yes)
LDFLAGS-ETHBOOT = -s -S -T $(TOPDIR)/boot_eth/eth_start.ld
endif

# add the option for gcc 3.3 only
ifeq ($(GCC_3.3), 1)
ETH_CFLAGS += -fno-zero-initialized-in-bss
endif
#### End Etherboot specific stuff

OBJECTS-XBE = $(TOPDIR)/boot_xbe/xbeboot.o

OBJECTS-VML = $(TOPDIR)/boot_vml/vml_Startup.o
ifeq ($(ETHERBOOT), yes)
OBJECTS-ETH = $(TOPDIR)/boot_eth/eth_Startup.o
endif
                                             
OBJECTS-ROMBOOT = $(TOPDIR)/obj/2bBootStartup.o
OBJECTS-ROMBOOT += $(TOPDIR)/obj/2bPicResponseAction.o
OBJECTS-ROMBOOT += $(TOPDIR)/obj/2bBootStartBios.o
OBJECTS-ROMBOOT += $(TOPDIR)/obj/sha1.o
OBJECTS-ROMBOOT += $(TOPDIR)/obj/2bBootLibrary.o
OBJECTS-ROMBOOT += $(TOPDIR)/obj/misc.o
#OBJECTS-ROMBOOT += $(TOPDIR)/obj/LED.o
                                             
OBJECTS-CROM = $(TOPDIR)/obj/BootStartup.o
OBJECTS-CROM += $(TOPDIR)/obj/BootResetAction.o
OBJECTS-CROM += $(TOPDIR)/obj/i2cio.o
OBJECTS-CROM += $(TOPDIR)/obj/pci.o
OBJECTS-CROM += $(TOPDIR)/obj/BootVgaInitialization.o
OBJECTS-CROM += $(TOPDIR)/obj/VideoInitialization.o
OBJECTS-CROM += $(TOPDIR)/obj/conexant.o
OBJECTS-CROM += $(TOPDIR)/obj/focus.o
OBJECTS-CROM += $(TOPDIR)/obj/xcalibur.o
OBJECTS-CROM += $(TOPDIR)/obj/BootIde.o
OBJECTS-CROM += $(TOPDIR)/obj/BootHddKey.o
OBJECTS-CROM += $(TOPDIR)/obj/rc4.o
OBJECTS-CROM += $(TOPDIR)/obj/sha1.o
OBJECTS-CROM += $(TOPDIR)/obj/BootVideoHelpers.o
OBJECTS-CROM += $(TOPDIR)/obj/cromString.o
OBJECTS-CROM += $(TOPDIR)/obj/string.o
OBJECTS-CROM += $(TOPDIR)/obj/sortHelpers.o
OBJECTS-CROM += $(TOPDIR)/obj/rand.o
OBJECTS-CROM += $(TOPDIR)/obj/vsprintf.o
OBJECTS-CROM += $(TOPDIR)/obj/timeManagement.o
OBJECTS-CROM += $(TOPDIR)/obj/Gentoox.o
OBJECTS-CROM += $(TOPDIR)/obj/LED.o
OBJECTS-CROM += $(TOPDIR)/obj/IconMenu.o
OBJECTS-CROM += $(TOPDIR)/obj/IconMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/TextMenu.o
OBJECTS-CROM += $(TOPDIR)/obj/TextMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/BankSelectMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/VideoMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/ResetMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/HDDFlashMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/CDMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/InfoMenuInit.o
#OBJECTS-CROM += $(TOPDIR)/obj/FlashMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/HDDMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/LEDMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/SystemMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/ToolsMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/DeveloperMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/ModchipMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/NetworkMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/LCDMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/EepromEditMenuInit.o
#OBJECTS-CROM += $(TOPDIR)/obj/BFMBootMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/XBlastScriptMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/UncommittedChangesMenuInit.o
OBJECTS-CROM += $(TOPDIR)/obj/MenuActions.o
OBJECTS-CROM += $(TOPDIR)/obj/VideoMenuActions.o
OBJECTS-CROM += $(TOPDIR)/obj/InfoMenuActions.o
OBJECTS-CROM += $(TOPDIR)/obj/ResetMenuActions.o
OBJECTS-CROM += $(TOPDIR)/obj/FlashMenuActions.o
OBJECTS-CROM += $(TOPDIR)/obj/HDDMenuActions.o
OBJECTS-CROM += $(TOPDIR)/obj/CDMenuActions.o
OBJECTS-CROM += $(TOPDIR)/obj/LEDMenuActions.o
OBJECTS-CROM += $(TOPDIR)/obj/ToolsMenuActions.o
#OBJECTS-CROM += $(TOPDIR)/obj/Confirm.o
OBJECTS-CROM += $(TOPDIR)/obj/OnScreenKeyboard.o
OBJECTS-CROM += $(TOPDIR)/obj/ModchipMenuActions.o
OBJECTS-CROM += $(TOPDIR)/obj/LCDMenuActions.o
OBJECTS-CROM += $(TOPDIR)/obj/SystemMenuActions.o
OBJECTS-CROM += $(TOPDIR)/obj/DeveloperMenuActions.o
OBJECTS-CROM += $(TOPDIR)/obj/NetworkMenuActions.o
OBJECTS-CROM += $(TOPDIR)/obj/EepromEditMenuActions.o
#OBJECTS-CROM += $(TOPDIR)/obj/BFMBootMenuActions.o
OBJECTS-CROM += $(TOPDIR)/obj/XBlastScriptMenuActions.o
OBJECTS-CROM += $(TOPDIR)/obj/UncommittedChangesMenuActions.o
OBJECTS-CROM += $(TOPDIR)/obj/LoadLinux.o
OBJECTS-CROM += $(TOPDIR)/obj/setup.o
OBJECTS-CROM += $(TOPDIR)/obj/iso9660.o
OBJECTS-CROM += $(TOPDIR)/obj/BootLibrary.o
OBJECTS-CROM += $(TOPDIR)/obj/cputools.o
OBJECTS-CROM += $(TOPDIR)/obj/microcode.o
OBJECTS-CROM += $(TOPDIR)/obj/ioapic.o
OBJECTS-CROM += $(TOPDIR)/obj/BootInterrupts.o
OBJECTS-CROM += $(TOPDIR)/obj/nanojpeg.o
OBJECTS-CROM += $(TOPDIR)/obj/FlashLowLevel.o
OBJECTS-CROM += $(TOPDIR)/obj/FlashDriver.o
OBJECTS-CROM += $(TOPDIR)/obj/FlashUi.o
OBJECTS-CROM += $(TOPDIR)/obj/BootEEPROM.o
OBJECTS-CROM += $(TOPDIR)/obj/BootLPCMod.o
OBJECTS-CROM += $(TOPDIR)/obj/BootLCD.o
OBJECTS-CROM += $(TOPDIR)/obj/BootFATX.o
OBJECTS-CROM += $(TOPDIR)/obj/ProgressBar.o
OBJECTS-CROM += $(TOPDIR)/obj/ConfirmDialog.o
OBJECTS-CROM += $(TOPDIR)/obj/md5.o
OBJECTS-CROM += $(TOPDIR)/obj/crc32.o
OBJECTS-CROM += $(TOPDIR)/obj/strtol.o
OBJECTS-CROM += $(TOPDIR)/obj/xblastScriptEngine.o
OBJECTS-CROM += $(TOPDIR)/obj/xblastSettings.o
OBJECTS-CROM += $(TOPDIR)/obj/xblastSettingsChangeTracker.o
OBJECTS-CROM += $(TOPDIR)/obj/xblastSettingsImportExport.o
OBJECTS-CROM += $(TOPDIR)/obj/PowerManagement.o
OBJECTS-CROM += $(TOPDIR)/obj/HardwareIdentifier.o
#USB
OBJECTS-CROM += $(TOPDIR)/obj/config.o 
OBJECTS-CROM += $(TOPDIR)/obj/hcd-pci.o
OBJECTS-CROM += $(TOPDIR)/obj/hcd.o
OBJECTS-CROM += $(TOPDIR)/obj/hub.o
OBJECTS-CROM += $(TOPDIR)/obj/message.o
OBJECTS-CROM += $(TOPDIR)/obj/ohci-hcd.o
OBJECTS-CROM += $(TOPDIR)/obj/buffer_simple.o
OBJECTS-CROM += $(TOPDIR)/obj/urb.o
OBJECTS-CROM += $(TOPDIR)/obj/usb-debug.o
OBJECTS-CROM += $(TOPDIR)/obj/usb.o
OBJECTS-CROM += $(TOPDIR)/obj/BootUSB.o
OBJECTS-CROM += $(TOPDIR)/obj/usbwrapper.o
OBJECTS-CROM += $(TOPDIR)/obj/linuxwrapper.o
OBJECTS-CROM += $(TOPDIR)/obj/xpad.o
#OBJECTS-CROM += $(TOPDIR)/obj/xremote.o
#OBJECTS-CROM += $(TOPDIR)/obj/usbkey.o
OBJECTS-CROM += $(TOPDIR)/obj/risefall.o
#ETHERBOOT
ifeq ($(ETHERBOOT), yes)
OBJECTS-CROM += $(TOPDIR)/obj/nic.o
OBJECTS-CROM += $(TOPDIR)/obj/xbox.o
OBJECTS-CROM += $(TOPDIR)/obj/forcedeth.o
OBJECTS-CROM += $(TOPDIR)/obj/xbox_pci.o
OBJECTS-CROM += $(TOPDIR)/obj/etherboot_config.o
endif

OBJECTS-LWIP = $(addprefix $(TOPDIR)/obj/,def.o err.o ethernetif.o inet_chksum.o init.o mem.o memp.o netif.o pbuf.o raw.o stats.o sys.o tcp.o tcp_in.o tcp_out.o timeouts.o udp.o dhcp.o icmp.o ip4.o ip4_addr.o ip4_frag.o etharp.o fs.o httpd.o ethernet.o ip.o)
#OBJECTS-LWIP = $(addprefix $(TOPDIR)/obj/,def.o err.o ethernetif.o inet_chksum.o init.o mem.o memp.o netif.o pbuf.o raw.o stats.o sys.o tcp.o tcp_in.o tcp_out.o timers.o udp.o dhcp.o icmp.o ip.o inet.o ip_addr.o ip_frag.o etharp.o httpd.o)


OBJECTS-CROM += $(OBJECTS-LWIP)

RESOURCES = $(TOPDIR)/obj/backdrop.elf

export INCLUDE
export TOPDIR

ifeq ($(ETHERBOOT), yes)
BOOT_ETH_DIR = boot_eth/ethboot
BOOT_ETH_SUBDIRS = ethsubdirs
endif

.PHONY: all clean

all: makefsdata
	@$(MAKE) -j16 --no-print-directory resources $(BOOT_ETH_SUBDIRS) cromsubdirs xbeboot xromwell.xbe vml_startup vmlboot $(BOOT_ETH_DIR) obj/image-crom.bin cromwell.bin imagecompress 256KBBinGen crcbin

ifeq ($(ETHERBOOT), yes)
ethsubdirs: $(patsubst %, _dir_%, $(ETH_SUBDIRS))
$(patsubst %, _dir_%, $(ETH_SUBDIRS)) : dummy
	$(MAKE) CFLAGS="$(ETH_CFLAGS)" -C $(patsubst _dir_%, %, $@)
endif

cromsubdirs: $(patsubst %, _dir_%, $(SUBDIRS))
$(patsubst %, _dir_%, $(SUBDIRS)) : dummy
	$(MAKE) CFLAGS="$(CFLAGS) $(CROM_CFLAGS)" -C $(patsubst _dir_%, %, $@)
	
2blsubdirs: $(patsubst %, _dir_%, boot_rom)
$(patsubst %, _dir_%, boot_rom) : dummy
	$(MAKE) CFLAGS="$(2BL_CFLAGS) $(INCLUDE)" -C $(patsubst _dir_%, %, $@)

dummy:

resources:
	# Background
	${LD} -r --oformat elf32-i386 -o $(TOPDIR)/obj/backdrop.elf -T $(TOPDIR)/scripts/backdrop.ld -b binary $(TOPDIR)/pics/backdrop.jpg	

clean:
	find . \( -name '*.[oas]' -o -name core -o -name '.*.flags' \) -type f -print \
		| grep -v lxdialog/ | xargs rm -f
	rm -f $(TOPDIR)/obj/*.gz 
	rm -f $(TOPDIR)/obj/*.bin 
	rm -f $(TOPDIR)/obj/*.elf
	rm -f $(TOPDIR)/obj/*.map
	rm -f $(TOPDIR)/image/*.bin 
	rm -f $(TOPDIR)/image/*.xbe 
	rm -f $(TOPDIR)/xbe/*.xbe $(TOPDIR)/xbe/*.bin
	rm -f $(TOPDIR)/xbe/*.elf
	rm -f $(TOPDIR)/image/*.bin
	rm -f $(TOPDIR)/bin/imagebld*
	rm -f $(TOPDIR)/bin/crcbin*
	rm -f $(TOPDIR)/bin/scriptChecker*
	rm -f $(TOPDIR)/bin/makefsdata
	rm -f $(TOPDIR)/boot_vml/disk/vmlboot
	rm -f $(TOPDIR)/$(LWIPFOLDER)/src/apps/httpd/fsdata.c
	rm -f boot_eth/ethboot
	mkdir -p $(TOPDIR)/xbe 
	mkdir -p $(TOPDIR)/image
	mkdir -p $(TOPDIR)/obj 
	mkdir -p $(TOPDIR)/bin

obj/image-crom.bin: cromsubdirs resources
	${LD} -o obj/image-crom.elf ${OBJECTS-CROM} ${RESOURCES} ${LDFLAGS-ROM} -Map $(TOPDIR)/obj/image-crom.map
	${OBJCOPY} --output-target=binary --strip-all obj/image-crom.elf $@

vmlboot: vml_startup 
	${LD} -o $(TOPDIR)/obj/vmlboot.elf ${OBJECTS-VML} ${LDFLAGS-VMLBOOT}
	${OBJCOPY} --output-target=binary --strip-all $(TOPDIR)/obj/vmlboot.elf $(TOPDIR)/boot_vml/disk/$@
	
vml_startup:
	$(CC) ${CFLAGS} -c -o ${OBJECTS-VML} boot_vml/vml_Startup.S

ifeq ($(ETHERBOOT), yes)
boot_eth/ethboot: ethboot obj/image-crom.bin
	${LD} -o obj/ethboot.elf ${OBJECTS-ETH} -b binary obj/image-crom.bin ${LDFLAGS-ETHBOOT} -Map $(TOPDIR)/obj/ethboot.map
	${OBJCOPY} --output-target=binary --strip-all obj/ethboot.elf obj/ethboot.bin
	perl -I boot_eth boot_eth/mknbi.pl --output=$@ obj/ethboot.bin
	
ethboot:
	$(CC) ${CFLAGS} -c -o ${OBJECTS-ETH} boot_eth/eth_Startup.S
endif

xromwell.xbe: xbeboot
	${LD} -o $(TOPDIR)/obj/xbeboot.elf ${OBJECTS-XBE} ${LDFLAGS-XBEBOOT}
	${OBJCOPY} --output-target=binary --strip-all $(TOPDIR)/obj/xbeboot.elf $(TOPDIR)/xbe/XBlast\ OS.xbe
	
xbeboot:
	$(CC) ${CFLAGS} -c -o ${OBJECTS-XBE} boot_xbe/xbeboot.S

cromwell.bin: cromsubdirs 2blsubdirs
	${LD} -o $(TOPDIR)/obj/2lbimage.elf ${OBJECTS-ROMBOOT} ${LDFLAGS-ROMBOOT} -Map $(TOPDIR)/obj/2lbimage.map
	${OBJCOPY} --output-target=binary --strip-all $(TOPDIR)/obj/2lbimage.elf $(TOPDIR)/obj/2blimage.bin

# This is a local executable, so don't use a cross compiler...
bin/imagebld:
	gcc -Ilib/crypt -o bin/sha1.o -c lib/crypt/sha1.c
	gcc -Ilib/crypt -o bin/md5.o -c lib/crypt/md5.c
	gcc -Ilib/crypt -o bin/imagebld.o -c pc_tools/imagebld/imagebld.c
	gcc -o bin/imagebld bin/imagebld.o bin/sha1.o bin/md5.o

# Same here.
crcbin:
	gcc -o bin/crcbin.o -c pc_tools/crcbin/crcbin.c
	gcc -o bin/crc32.o -c lib/misc/crc32.c
	gcc -o bin/crcbin bin/crcbin.o bin/crc32.o
	
scriptchecker:
	gcc -g -o bin/scriptChecker pc_tools/scriptChecker/scriptChecker.c
	
imagecompress: obj/image-crom.bin bin/imagebld
	cp obj/image-crom.bin obj/c
	gzip -9 obj/c
	bin/imagebld -xbe xbe/XBlast\ OS.xbe obj/image-crom.bin
	bin/imagebld -vml boot_vml/disk/vmlboot obj/image-crom.bin f

256KBBinGen: imagecompress crcbin cromwell.bin
	bin/imagebld -rom obj/2blimage.bin obj/c.gz image/cromwell.bin
	bin/crcbin image/cromwell.bin image/crcwell.bin
	
makefsdata: clean
	gcc -I"$(TOPDIR)/$(LWIPFOLDER)" -I"$(TOPDIR)/$(LWIPFOLDER)/src/include" -O2 -Wall -c -o "obj/makefsdata.o" "$(TOPDIR)/$(LWIPFOLDER)/src/apps/httpd/makefsdata/makefsdata.c"
	gcc -O2 -Wall -c -o "obj/findfirst.o" "$(TOPDIR)/$(LWIPFOLDER)/src/apps/httpd/makefsdata/findfirst.c"
	gcc -O2 -Wall -c -o "obj/spec.o" "$(TOPDIR)/$(LWIPFOLDER)/src/apps/httpd/makefsdata/spec.c"
	gcc -o bin/makefsdata obj/findfirst.o obj/spec.o obj/makefsdata.o
	bin/makefsdata "$(TOPDIR)/$(LWIPFOLDER)/src/apps/httpd/fs" -e -nossi
	mv fsdata.c "$(TOPDIR)/$(LWIPFOLDER)/src/apps/httpd/fsdata.c"
	
