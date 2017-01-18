#include "etherboot_config.h"
#include "lib/cromwell/cromString.h"
#include "lib/LPCMod/xblastDebug.h"
#include "osdep.h"

#include	"if_ether.h"

//#define TFTM_DEBUG
#ifdef TFTM_DEBUG
#define dprintf(x) debugSPIPrint x
#else
#define dprintf(x)
#endif

#define ARP_CLIENT	0
#define ARP_SERVER	1
#define ARP_GATEWAY	2
#define MAX_ARP		ARP_GATEWAY+1

struct arptable_t {
	//in_addr ipaddr;
	uint8_t node[6];
};

struct rom_info {
	unsigned short	rom_segment;
	unsigned short	rom_length;
};

#define PACKED __attribute__((packed))

struct e820entry {
	uint64_t addr;
	uint64_t size;
	uint32_t type;
#define E820_RAM	1
#define E820_RESERVED	2
#define E820_ACPI	3 /* usable as RAM once ACPI tables have been read */
#define E820_NVS	4
} PACKED;
#define E820MAX 32
struct meminfo {
	uint16_t basememsize;
	uint16_t pad;
	uint32_t memsize;
	uint32_t map_count;
	struct e820entry map[E820MAX];
} PACKED;
extern struct meminfo meminfo;

extern void udelay(unsigned int usecs);

#define printf printk

