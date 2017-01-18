/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 */

#include	"etherboot.h"
#include	"nic.h"
#ifdef CONFIG_PCI
#include	"pci.h"
#endif

#ifdef CONFIG_PCI
static int pci_probe(struct dev *dev, const char *type_name)
{
/*
 *	NIC probing is in pci device order, followed by the 
 *      link order of the drivers.  A driver that matches 
 *      on vendor and device id will supersede a driver
 *      that matches on pci class.
 *
 *	If you want to probe for another device behind the same pci
 *      device just increment index.  And the previous probe call
 *      will be repeated.
 */
	struct pci_probe_state *state = &dev->state.pci;
/*	printf("Probing pci %s...\n", type_name);*/
	if (dev->how_probe == PROBE_FIRST) {
		state->advance    = 1;
		state->dev.driver = 0;
		state->dev.bus    = 0;
		state->dev.devfn  = 0;
		dev->index        = -1;
	}
	for(;;) {
		if ((dev->how_probe != PROBE_AWAKE) && state->advance) {
			find_pci(dev->type, &state->dev);
			dev->index = -1;
		}
		state->advance = 1;
		
		if (state->dev.driver == 0)
			break;
		
#if 0
		/* FIXME the romaddr code needs a total rethought to be useful */
		if (state->dev.romaddr != ((unsigned long) rom.rom_segment << 4)) {
			continue;
		}
#endif
		if (dev->how_probe != PROBE_AWAKE) {
			dev->type_index++;
		}
		dev->devid.bus_type = PCI_BUS_TYPE;
		dev->devid.vendor_id = htons(state->dev.vendor);
		dev->devid.device_id = htons(state->dev.dev_id);
		/* FIXME how do I handle dev->index + PROBE_AGAIN?? */
		
		//No need to print driver's name. We know it's an nForce!
		//printf("\n            [%s]", state->dev.name);
		if (state->dev.driver->probe(dev, &state->dev)) {
			state->advance = (dev->index == -1);
			return PROBE_WORKED;
		}
	}
	return PROBE_FAILED;
}
#else
#define pci_probe(d, tn) (PROBE_FAILED)
#endif

#define isa_probe(d, tn) (PROBE_FAILED)

static const char *driver_name[] = {
	"nic", 
	"disk", 
	"floppy",
};
int probe(struct dev *dev)
{
	const char *type_name;
	type_name = "";
	if ((dev->type >= 0) && 
		(dev->type < sizeof(driver_name)/sizeof(driver_name[0]))) {
		type_name = driver_name[dev->type];
	}
	if (dev->how_probe == PROBE_FIRST) {
		dev->to_probe = PROBE_PCI;
		memset(&dev->state, 0, sizeof(dev->state));
	}
	if (dev->to_probe == PROBE_PCI) {
		dev->how_probe = pci_probe(dev, type_name);
		if (dev->how_probe == PROBE_FAILED) {
			dev->to_probe = PROBE_ISA;
		}
	}
	if (dev->to_probe == PROBE_ISA) {
		dev->how_probe = isa_probe(dev, type_name);
		if (dev->how_probe == PROBE_FAILED) {
			dev->to_probe = PROBE_NONE;
		}
	}
	if ((dev->to_probe != PROBE_PCI) &&
		(dev->to_probe != PROBE_ISA)) {
		dev->how_probe = PROBE_FAILED;
		
	}
	return dev->how_probe;
}

void disable(struct dev *dev)
{
	if (dev->disable) {
		dev->disable(dev);
		dev->disable = 0;
	}
}
