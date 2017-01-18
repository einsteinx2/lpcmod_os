/**************************************************************************
Etherboot -  Network Bootstrap Program

Literature dealing with the network protocols:
	ARP - RFC826
	RARP - RFC903
        IP - RFC791
	UDP - RFC768
	BOOTP - RFC951, RFC2132 (vendor extensions)
	DHCP - RFC2131, RFC2132 (options)
	TFTP - RFC1350, RFC2347 (options), RFC2348 (blocksize), RFC2349 (tsize)
	RPC - RFC1831, RFC1832 (XDR), RFC1833 (rpcbind/portmapper)
	NFS - RFC1094, RFC1813 (v3, useful for clarifications, not implemented)
	IGMP - RFC1112, RFC2113, RFC2365, RFC2236, RFC3171

**************************************************************************/
#include "etherboot.h"
#include "nic.h"


struct arptable_t	arptable[MAX_ARP];

/* Currently no other module uses rom, but it is available */
struct rom_info		rom;


static int dummy(void *unused __unused)
{
	return (0);
}

/* Careful.  We need an aligned buffer to avoid problems on machines
 * that care about alignment.  To trivally align the ethernet data
 * (the ip hdr and arp requests) we offset the packet by 2 bytes.
 * leaving the ethernet data 16 byte aligned.  Beyond this
 * we use memmove but this makes the common cast simple and fast.
 */
static char	packet[ETH_FRAME_LEN + ETH_DATA_ALIGN] __aligned;

struct nic	nic =
{
	{
		0,				/* dev.disable */
		{
			0,			/* Vendor ID */
			0,			/* Device ID */
			PCI_BUS_TYPE, /*Bus type */
		},				/* dev.devid */
		0,				/* index */
		0,				/* type */
		PROBE_FIRST,			/* how_pobe */
		PROBE_NONE,			/* to_probe */
		0,				/* failsafe */
		0,				/* type_index */
		{},				/* state */
	},
	(int (*)(struct nic *))dummy,		/* poll */
	(void (*)(struct nic *, const char *,
		unsigned int, unsigned int,
		const char *))dummy,		/* transmit */
	0,					/* flags */
	&rom,					/* rom_info */
	arptable[ARP_CLIENT].node,		/* node_addr */
	packet + ETH_DATA_ALIGN,		/* packet */
	0,					/* packetlen */
	0,					/* priv_data */
};


int eth_probe(struct dev *dev)
{
	return probe(dev);
}

int eth_poll(void)
{
	return ((*nic.poll)(&nic));
}

int eth_poll_into(char *buf, int *len)
{
	if (eth_poll()) {
		*len = nic.packetlen;
		memcpy (buf, nic.packet, nic.packetlen);
		return 1;
	}
	return 0;
}

void eth_transmit(const char *d, unsigned int t, unsigned int s, const void *p)
{
	(*nic.transmit)(&nic, d, t, s, p);
}

void eth_disable(void)
{
	disable(&nic.dev);
}
