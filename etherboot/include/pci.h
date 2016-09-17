#if !defined(PCI_H) && defined(CONFIG_PCI)
#define PCI_H

/*
** Support for NE2000 PCI clones added David Monro June 1997
** Generalised for other PCI NICs by Ken Yap July 1997
**
** Most of this is taken from:
**
** /usr/src/linux/drivers/pci/pci.c
** /usr/src/linux/include/linux/pci.h
** /usr/src/linux/arch/i386/bios32.c
** /usr/src/linux/include/linux/bios32.h
** /usr/src/linux/drivers/net/ne.c
*/

/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 */

#include "pci_ids.h"

#define PCI_COMMAND_IO			0x1	/* Enable response in I/O space */
#define PCI_COMMAND_MEM			0x2	/* Enable response in mem space */
#define PCI_COMMAND_MASTER		0x4	/* Enable bus mastering */
#define PCI_LATENCY_TIMER		0x0d	/* 8 bits */
#define PCI_COMMAND_SPECIAL		0x8	/* Enable response to special cycles */
#define PCI_COMMAND_INVALIDATE		0x10	/* Use memory write and invalidate */
#define  PCI_COMMAND_VGA_PALETTE 0x20	/* Enable palette snooping */
#define  PCI_COMMAND_PARITY	0x40	/* Enable parity checking */
#define  PCI_COMMAND_WAIT 	0x80	/* Enable address/data stepping */
#define  PCI_COMMAND_SERR	0x100	/* Enable SERR */
#define  PCI_COMMAND_FAST_BACK	0x200	/* Enable back-to-back writes */

#define PCI_VENDOR_ID           0x00    /* 16 bits */
#define PCI_DEVICE_ID           0x02    /* 16 bits */
#define PCI_COMMAND             0x04    /* 16 bits */

#define PCI_REVISION            0x08    /* 8 bits  */
#define PCI_REVISION_ID         0x08    /* 8 bits  */
#define PCI_CLASS_REVISION      0x08    /* 32 bits  */
#define PCI_CLASS_CODE          0x0b    /* 8 bits */
#define PCI_SUBCLASS_CODE       0x0a    /* 8 bits */
#define PCI_HEADER_TYPE         0x0e    /* 8 bits */

#define PCI_BASE_ADDRESS_0      0x10    /* 32 bits */
#define PCI_BASE_ADDRESS_1      0x14    /* 32 bits */
#define PCI_BASE_ADDRESS_2      0x18    /* 32 bits */
#define PCI_BASE_ADDRESS_3      0x1c    /* 32 bits */
#define PCI_BASE_ADDRESS_4      0x20    /* 32 bits */
#define PCI_BASE_ADDRESS_5      0x24    /* 32 bits */

#define PCI_BASE_ADDRESS_MEM_TYPE_MASK 0x06
#define PCI_BASE_ADDRESS_MEM_TYPE_32	0x00	/* 32 bit address */
#define PCI_BASE_ADDRESS_MEM_TYPE_1M	0x02	/* Below 1M [obsolete] */
#define PCI_BASE_ADDRESS_MEM_TYPE_64	0x04	/* 64 bit address */

#ifndef	PCI_BASE_ADDRESS_IO_MASK
#define	PCI_BASE_ADDRESS_IO_MASK       (~0x03)
#endif
#ifndef	PCI_BASE_ADDRESS_MEM_MASK
#define	PCI_BASE_ADDRESS_MEM_MASK       (~0x0f)
#endif
#define	PCI_BASE_ADDRESS_SPACE_IO	0x01
#define	PCI_ROM_ADDRESS		0x30	/* 32 bits */
#define	PCI_ROM_ADDRESS_ENABLE	0x01	/* Write 1 to enable ROM,
					   bits 31..11 are address,
					   10..2 are reserved */

#define PCI_SLOT(devfn)		  ((devfn) >> 3)
#define PCI_FUNC(devfn)           ((devfn) & 0x07)

struct pci_device;
struct dev;
typedef int (*pci_probe_t)(struct dev *, struct pci_device *);

struct pci_device {
	uint32_t		class;
	uint16_t		vendor, dev_id;
	const char		*name;
	/* membase and ioaddr are silly and depricated */
	unsigned int		membase;
	unsigned int		ioaddr;
	unsigned int		romaddr;
	unsigned char		devfn;
	unsigned char		bus;
	const struct pci_driver	*driver;
};

extern void scan_pci_bus(int type, struct pci_device *dev);
extern void find_pci(int type, struct pci_device *dev);

extern int pcibios_read_config_byte(unsigned int bus, unsigned int device_fn, unsigned int where, uint8_t *value);
extern int pcibios_write_config_byte (unsigned int bus, unsigned int device_fn, unsigned int where, uint8_t value);
extern int pcibios_read_config_word(unsigned int bus, unsigned int device_fn, unsigned int where, uint16_t *value);
extern int pcibios_write_config_word (unsigned int bus, unsigned int device_fn, unsigned int where, uint16_t value);
extern int pcibios_read_config_dword(unsigned int bus, unsigned int device_fn, unsigned int where, uint32_t *value);
extern int pcibios_write_config_dword(unsigned int bus, unsigned int device_fn, unsigned int where, uint32_t value);
extern unsigned long pcibios_bus_base(unsigned int bus);
extern void adjust_pci_device(struct pci_device *p);


static inline int 
pci_read_config_byte(struct pci_device *dev, unsigned int where, uint8_t *value)
{
	return pcibios_read_config_byte(dev->bus, dev->devfn, where, value);
}
static inline int 
pci_write_config_byte(struct pci_device *dev, unsigned int where, uint8_t value)
{
	return pcibios_write_config_byte(dev->bus, dev->devfn, where, value);
}
static inline int 
pci_read_config_word(struct pci_device *dev, unsigned int where, uint16_t *value)
{
	return pcibios_read_config_word(dev->bus, dev->devfn, where, value);
}
static inline int 
pci_write_config_word(struct pci_device *dev, unsigned int where, uint16_t value)
{
	return pcibios_write_config_word(dev->bus, dev->devfn, where, value);
}
static inline int 
pci_read_config_dword(struct pci_device *dev, unsigned int where, uint32_t *value)
{
	return pcibios_read_config_dword(dev->bus, dev->devfn, where, value);
}
static inline int 
pci_write_config_dword(struct pci_device *dev, unsigned int where, uint32_t value)
{
	return pcibios_write_config_dword(dev->bus, dev->devfn, where, value);
}

/* Helper functions to find the size of a pci bar */
extern unsigned long pci_bar_start(struct pci_device *dev, unsigned int bar);
extern unsigned long pci_bar_size(struct pci_device *dev, unsigned int bar);
/* Helper function to find pci capabilities */
extern int pci_find_capability(struct pci_device *dev, int cap);
struct pci_id {
	unsigned short vendor, dev_id;
	const char *name;
};

struct dev;
/* Most pci drivers will use this */
struct pci_driver {
	int type;
	const char *name;
	pci_probe_t probe;
	struct pci_id *ids;
	int id_count;

/* On a few occasions the hardware is standardized enough that
 * we only need to know the class of the device and not the exact
 * type to drive the device correctly.  If this is the case
 * set a class value other than 0.
 */
	unsigned short class;
};

#define __pci_driver  
extern struct pci_driver* pci_drivers;
extern struct pci_driver* pci_drivers_end;

#define PCI_ROM(VENDOR_ID, DEVICE_ID, IMAGE, DESCRIPTION) \
	{ VENDOR_ID, DEVICE_ID, IMAGE, }

#endif	/* PCI_H */
