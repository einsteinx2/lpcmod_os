/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/init.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "netif/etharp.h"
#include "netif/ppp/pppoe.h"
#include "lwip/dhcp.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/timeouts.h"


#include "httpd.h"
#include "ftpd.h"

#include "WebServerOps.h"
#include "xblast/settings/xblastSettingsDefs.h"
#include "lib/cromwell/cromString.h"
#include "Gentoox.h"
#include "HDDMenuActions.h"
#include "EepromEditMenuActions.h"
#include "Gentoox.h"
#include "FlashUi.h"
#include "FlashDriver.h"
#include "MenuActions.h"
#include "BootIde.h"
#include "video.h"
#include "string.h"
#include "stdlib.h"
#include "lib/LPCMod/xblastDebug.h"
#include "lib/cromwell/CallbackTimer.h"
#include <stdarg.h>

bool netFlashOver;
WebServerOps currentWebServerOp;
unsigned char* postProcessBuf;
unsigned int postProcessBufSize;

#define DHCP_WAIT_MS 10000 /* Max allowed time as per RFC2131 */

#define LINK_SPEED_OF_YOUR_NETIF_IN_BPS 100000000

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'b'

NetworkState currentNetworkState = NetworkState_Idle;
static struct ip4_addr ipaddr, netmask, gw;
static struct netif *netif = NULL;
static unsigned int callbackTimerId;

void
eth_transmit (const char *d, unsigned int t, unsigned int s, const void *p);
int
eth_poll_into (char *buf, int *len);

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
#if 0
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};
#endif

/* Forward declarations. */
static void  ethernetif_input(struct netif *netif);

extern char forcedeth_hw_addr[6];

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init(struct netif *netif)
{
#if 0
  struct ethernetif *ethernetif = netif->state;
#endif

  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* set MAC hardware address */
  memcpy (netif->hwaddr, forcedeth_hw_addr, ETHARP_HWADDR_LEN);

  /* maximum transfer unit */
  netif->mtu = 1500;
  
  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

#if LWIP_IPV6 && LWIP_IPV6_MLD
  /*
   * For hardware/netifs that implement MAC filtering.
   * All-nodes link-local is handled by default, so we must let the hardware know
   * to allow multicast packets in.
   * Should set mld_mac_filter previously. */
  if (netif->mld_mac_filter != NULL) {
    ip6_addr_t ip6_allnodes_ll;
    ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
    netif->mld_mac_filter(netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
  }
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */
 
  /* Do whatever else is needed to initialize interface. */  
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
#if 0
  struct ethernetif *ethernetif = netif->state;
#endif
  char buf[1500];
  char *bufptr;
  struct eth_hdr *h;
  struct pbuf *q;
  
  MIB2_STATS_NETIF_ADD(netif, ifoutoctets, p->tot_len);
  if (((u8_t*)p->payload)[0] & 1) {
    /* broadcast or multicast packet*/
    MIB2_STATS_NETIF_INC(netif, ifoutnucastpkts);
  } else {
    /* unicast packet */
    MIB2_STATS_NETIF_INC(netif, ifoutucastpkts);
  }
  /* increase ifoutdiscards or ifouterrors on error */


#if ETH_PAD_SIZE
  pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
  bufptr = &buf[0];
  for(q = p; q != NULL; q = q->next) {
    /* Send the data from the pbuf to the interface, one pbuf at a
       time. The size of the data in each pbuf is kept in the ->len
       variable. */
    //send data from(q->payload, q->len);
	memcpy (bufptr, q->payload, q->len);
	bufptr += q->len;
  }

  h = (struct eth_hdr *) buf;

  //signal that packet should be sent();
  eth_transmit (&h->dest.addr[0], ntohs (h->type), p->tot_len - sizeof(struct eth_hdr), &buf[sizeof(struct eth_hdr)]);

#if ETH_PAD_SIZE
  pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
  
  LINK_STATS_INC(link.xmit);

  return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *
low_level_input(struct netif *netif)
{
  struct pbuf *p, *q;
  char *bufptr;
  char buf[1500];
  int len;

  /* Obtain the size of the packet and put it into the "len"
     variable. */
  if (!eth_poll_into (buf, &len))
    return NULL;

  LWIP_DEBUGF(NETIF_DEBUG, ("Got new packet from low level nic.\n"));
#if ETH_PAD_SIZE
  len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

  /* We allocate a pbuf chain of pbufs from the pool. */
  p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  
  if (p != NULL) {
	  
	bufptr = &buf[0];

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    /* We iterate over the pbuf chain until we have read the entire
     * packet into the pbuf. */
    for(q = p; q != NULL; q = q->next) {
      /* Read enough bytes to fill this pbuf in the chain. The
       * available data in the pbuf is given by the q->len
       * variable.
       * This does not necessarily have to be a memcpy, you can also preallocate
       * pbufs for a DMA-enabled MAC and after receiving truncate it to the
       * actually received size. In this case, ensure the tot_len member of the
       * pbuf is the sum of the chained pbuf len members.
       */
      //read data into(q->payload, q->len);
	  memcpy (q->payload, bufptr, q->len);
      bufptr += q->len;
    }

    //acknowledge that packet has been read();

    MIB2_STATS_NETIF_ADD(netif, ifinoctets, p->tot_len);
    if (((u8_t*)p->payload)[0] & 1) {
      /* broadcast or multicast packet*/
      MIB2_STATS_NETIF_INC(netif, ifinnucastpkts);
    } else {
      /* unicast packet*/
      MIB2_STATS_NETIF_INC(netif, ifinucastpkts);
    }
#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    LINK_STATS_INC(link.recv);
  } else {
    //drop packet();
    LWIP_DEBUGF(NETIF_DEBUG, ("Couldn't allocate pbuf... Drop packet\n"));
    LINK_STATS_INC(link.memerr);
    LINK_STATS_INC(link.drop);
    MIB2_STATS_NETIF_INC(netif, ifindiscards);
  }

  return p;  
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
static void
ethernetif_input(struct netif *netif)
{
#if 0
  struct ethernetif *ethernetif;
#endif
  struct eth_hdr *ethhdr;
  struct pbuf *p;

  //ethernetif = netif->state;

  /* move received packet into a new pbuf */
  p = low_level_input(netif);
  /* if no packet could be read, silently ignore this */
    if (p != NULL) {
      /* pass all packets to ethernet_input, which decides what packets it supports */
      if (netif->input(p, netif) != ERR_OK) {
        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
        pbuf_free(p);
        p = NULL;
      }
    }
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
ethernetif_init(struct netif *netif)
{
#if 0
  struct ethernetif *ethernetif;
#endif

  LWIP_ASSERT("netif != NULL", (netif != NULL));

#if 0
  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }
#endif

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;//ebd_output
#if LWIP_IPV6
  netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */
  netif->linkoutput = low_level_output;//ebd_low_level_output
  
  
  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}

static void dhcpFailCallback(void)
{
    if (dhcp_supplied_address(netif) == false)
    {
        //Not necessary, but polite.
        dhcp_stop (netif);
        IP4_ADDR(&gw, LPCmodSettings.OSsettings.staticGateway[0],
                 LPCmodSettings.OSsettings.staticGateway[1],
                 LPCmodSettings.OSsettings.staticGateway[2],
                 LPCmodSettings.OSsettings.staticGateway[3]);
        IP4_ADDR(&ipaddr, LPCmodSettings.OSsettings.staticIP[0],
                 LPCmodSettings.OSsettings.staticIP[1],
                 LPCmodSettings.OSsettings.staticIP[2],
                 LPCmodSettings.OSsettings.staticIP[3]);
        IP4_ADDR(&netmask, LPCmodSettings.OSsettings.staticMask[0],
                 LPCmodSettings.OSsettings.staticMask[1],
                 LPCmodSettings.OSsettings.staticMask[2],
                 LPCmodSettings.OSsettings.staticMask[3]);
        netif_set_addr(netif, &ipaddr, &netmask, &gw);

        dhcp_inform (netif);
    }
    currentNetworkState = NetworkState_ServerInit;
    cromwellWarning();
    XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "currentNetworkState == NetworkState_ServerInit");
}

void run_lwip(void)
{


    switch(currentNetworkState)
    {
    case NetworkState_Idle:

    	break;
    case NetworkState_Init:
        printk ("\n            TCP/IP initialization. ");
    	lwip_init();
    	cromwellSuccess();

		netif = netif_find("eb");   //Trying to find previously configured network interface

		if(netif == NULL)
		{
		    XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "No configured network interface found. Creating one.");
			netif = (struct netif *)malloc(sizeof(struct netif));
			//Will never be removed for entire duration of program execution so no free()...

			//These will be overwritten by DHCP anyway if need be.
			IP4_ADDR(&gw, 0,0,0,0);
			IP4_ADDR(&ipaddr, 0,0,0,0);
			IP4_ADDR(&netmask, 0,0,0,0);

			netif_add (netif, &ipaddr, &netmask, &gw, NULL, ethernetif_init, ethernet_input);
		}
		else
		{
		    XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "Found previously configured network interface.");
		}

		netif_set_up(netif);

		if (LPCmodSettings.OSsettings.useDHCP)
		{
			//Re-run DHCP discover anyways just in case lease was revoked.
		    printk ("\n            Starting DHCP. ");
			dhcp_start (netif);
			cromwellSuccess();
			printk ("\n            Acquiring IP address. ");

			callbackTimerId = newCallbackTimer(dhcpFailCallback, DHCP_WAIT_MS, IsSingleUseTimer);
			currentNetworkState = NetworkState_DHCPStart;
			XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "currentNetworkState == NetworkState_DHCPStart");
		}
		else
		{
			//Not necessary, but polite.
			dhcp_stop (netif);
			IP4_ADDR(&gw, LPCmodSettings.OSsettings.staticGateway[0],
					 LPCmodSettings.OSsettings.staticGateway[1],
					 LPCmodSettings.OSsettings.staticGateway[2],
					 LPCmodSettings.OSsettings.staticGateway[3]);
			IP4_ADDR(&ipaddr, LPCmodSettings.OSsettings.staticIP[0],
					 LPCmodSettings.OSsettings.staticIP[1],
					 LPCmodSettings.OSsettings.staticIP[2],
					 LPCmodSettings.OSsettings.staticIP[3]);
			IP4_ADDR(&netmask, LPCmodSettings.OSsettings.staticMask[0],
					 LPCmodSettings.OSsettings.staticMask[1],
					 LPCmodSettings.OSsettings.staticMask[2],
					 LPCmodSettings.OSsettings.staticMask[3]);
			netif_set_addr(netif, &ipaddr, &netmask, &gw);

			dhcp_inform (netif);

			currentNetworkState = NetworkState_ServerInit;
			XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "currentNetworkState == NetworkState_ServerInit");
		}
		netif_set_default (netif);
    	break;
    case NetworkState_DHCPStart:
	    ethernetif_input(netif);

    	if(dhcp_supplied_address(netif))
    	{
    	    stopCallbackTimer(callbackTimerId);
    		currentNetworkState = NetworkState_ServerInit;
			cromwellSuccess();
			XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "currentNetworkState == NetworkState_ServerInit");
    	}
    	break;
    case NetworkState_ServerInit:
        printk ("\n\n            Go to 'http://%u.%u.%u.%u' to flash your BIOS.\n",
            ((netif->ip_addr.addr) & 0xff),
            ((netif->ip_addr.addr) >> 8 & 0xff),
            ((netif->ip_addr.addr) >> 16 & 0xff),
            ((netif->ip_addr.addr) >> 24 & 0xff));

		httpd_init();
		ftpd_init();

		currentNetworkState = NetworkState_ServerRunning;
		XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "currentNetworkState == NetworkState_ServerRunning");
    	break;
    case NetworkState_ServerRunning:
	    ethernetif_input(netif);

    	if(netFlashOver == true)
    	{
    		currentNetworkState = NetworkState_ServerShuttingDown;
    		XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "currentNetworkState == NetworkState_ServerShuttingDown");
    	}
    	break;
    case NetworkState_ServerShuttingDown:
    	dhcp_stop(netif);
    	currentNetworkState = NetworkState_Cleanup;
    	XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "currentNetworkState == NetworkState_Cleanup");
    	break;
    case NetworkState_Cleanup:
        switch(currentWebServerOp)
        {
        case WebServerOps_BIOSFlash:
        {
            FlashProgress flashProgress = Flash_getProgress();
            if(flashProgress.currentFlashOp == FlashOp_Idle)
            {
                currentNetworkState = NetworkState_Idle;
            }
        }
        break;
        default:
            currentNetworkState = NetworkState_Idle;
            break;
        }
    	break;
    }

    sys_check_timeouts();
}

void startNetFlash(WebServerOps flashType)
{
    currentWebServerOp = flashType;
    //Init a couple of Lwip globals because they seem to assume declared pointers are set to NULL by default.
    //From tcp.c
    tcp_bound_pcbs = NULL;
    tcp_listen_pcbs.listen_pcbs = NULL;
    tcp_active_pcbs = NULL;
    tcp_tw_pcbs = NULL;
    //tcp_tmp_pcb = NULL; //Not needed anymore in 2.0.0

    //from netif.c
    netif_list = NULL;
    netif_default = NULL;

    //From udp.c
    udp_pcbs = NULL;

    currentNetworkState = NetworkState_Init;
    postProcessBuf = NULL;
    postProcessBufSize = 0;
    netFlashOver = false;
    XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "currentNetworkState == NetworkState_Init");
}

bool newPostProcessData(WebServerOps op, const unsigned char* buf, unsigned int size)
{
    //op param will be used in the future
    //Maybe chain up operations?

    // Do not overwrite a pending operation
    if(postProcessBuf != NULL || postProcessBufSize != 0)
    {
        return false;
    }

    postProcessBuf = malloc(size);
    memcpy(postProcessBuf, buf, size);
    postProcessBufSize = size;

    return true;
}

bool netflashPostProcess(void)
{
    extern void ClearScreen (void);

    if(netFlashOver)
    {
        switch(currentWebServerOp)
        {
        case WebServerOps_BIOSFlash:
            ClearScreen ();
            FlashFileFromBuffer(postProcessBuf, postProcessBufSize, false);
            break;
        case WebServerOps_EEPROMFlash:
            ClearScreen ();
            updateEEPROMEditBufferFromInputBuffer(postProcessBuf, postProcessBufSize, true);
            UIFooter();
            break;
        case WebServerOps_HDD0Lock:
            ClearScreen ();

            if((tsaHarddiskInfo[0].m_securitySettings &0x0002)==0x0002)     //Drive is already locked
            {
                UnlockHDD(0, 0, postProcessBuf, false);    //Attempt Unlock only if SECURITY_UNLOCK was successful.
            }
            else
            {
                LockHDD(0, 0, postProcessBuf);
            }
            break;
        case WebServerOps_HDD1Lock:
            ClearScreen ();

            if((tsaHarddiskInfo[1].m_securitySettings &0x0002)==0x0002)     //Drive is already locked
            {
                UnlockHDD(1, 1, postProcessBuf, false);       //Attempt Unlock only if SECURITY_UNLOCK was successful.
            }
            else
            {
                LockHDD(1, 1, postProcessBuf);
            }
            break;
        }

        free(postProcessBuf);
        postProcessBuf = NULL;
        postProcessBufSize = 0;

        return true;
    }

    return false;
}
