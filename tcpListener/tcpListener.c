#include "lwip/stats.h"
#include "lwip/mem.h"
#include "netif/etharp.h"
#include "lwip/tcp.h"
#include "boot.h"

struct eth_addr ethaddr= {0,0x0d,0xff,0xff,0,0};

extern void cromwellSuccess(void);

void eth_transmit(const char *d, unsigned int t, unsigned int s, const void *p);
int eth_poll_into(char *buf, int *len);

int gotIP = 0;
int applicationStarted = 0;
int resetCount = 0;

static struct pbuf *
ebd_poll(struct netif *netif)
{
	struct pbuf *p, *q;
	char *bufptr;
	char buf[1500];
	int len;

	if (!eth_poll_into(buf, &len))
		return NULL;

	p = pbuf_alloc(PBUF_LINK, len, PBUF_POOL);
	if (p != NULL) {
		bufptr = &buf[0];
		for(q = p; q != NULL; q = q->next) {
			memcpy(q->payload, bufptr, q->len);
			bufptr += q->len;
		}
	} else {
		printk("           Could not allocate pbufs\n");
	}
	return p;
}

static err_t
ebd_low_level_output(struct netif *netif, struct pbuf *p)
{
	char buf[1500];
	char *bufptr;
	struct eth_hdr *h;
	struct pbuf *q;

	bufptr = &buf[0];
	h = (struct eth_hdr *)bufptr;
	for(q = p; q != NULL; q = q->next) {
		memcpy(bufptr, q->payload, q->len);
		bufptr += q->len;
	}
	eth_transmit (&h->dest.addr[0], ntohs (h->type), p->tot_len - 14, &buf[14]);
}

static err_t
ebd_output(struct netif *netif, struct pbuf *p,
		   struct ip_addr *ipaddr)
{
	p = etharp_output(netif, ipaddr, p);
	if (p != NULL) {
		return ebd_low_level_output(netif, p);
//	} else {
//		printk("b");
	}
	return ERR_OK;
}

int
ebd_wait(struct netif *netif, u16_t time)
{
  unsigned long delay_ticks;
  static unsigned long start_ticks = 0;
  extern unsigned long currticks(void);

  //delay_ticks = time * 3579;
  delay_ticks = time * 50;
  if (!start_ticks)
	  start_ticks = currticks();

  while (1) {
	  struct eth_hdr *ethhdr;
	  struct pbuf *p, *q;
	  
	  p = ebd_poll (netif);
	  if (p) {
		  ethhdr = p->payload;
		  q = NULL;
		  switch (htons(ethhdr->type)) {
		  case ETHTYPE_IP:
			  etharp_ip_input(netif, p);
			  pbuf_header(p, -14);
			  netif->input(p, netif);
			  break;
		  case ETHTYPE_ARP:
			  etharp_arp_input(netif, &ethaddr, p);
			  break;
		  default:
			  pbuf_free(p);
			  break;
		  }
		  if (q != NULL) {
			  ebd_low_level_output(netif, q);
			  pbuf_free(q);
		  }
		  return 1;
	  } else {
		  unsigned long ticks = currticks () - start_ticks;
		  if (ticks > delay_ticks) {
			  start_ticks = 0;
			  return 0;
		  }
	  }
  }
}

extern char forcedeth_hw_addr[6];

static err_t
ebd_init(struct netif *netif)
{
	netif->hwaddr_len = 6;
	memcpy (netif->hwaddr, forcedeth_hw_addr, 6);
	memcpy (ethaddr.addr, forcedeth_hw_addr, 6);
	netif->name[0] = 'e';
	netif->name[1] = 'b';
	netif->output = ebd_output;
	netif->linkoutput = ebd_low_level_output;
	return ERR_OK;
}

int run_lwip(int A, int B, int C, int D, int P)
{
	struct ip_addr ipaddr, netmask, gw;
	struct netif netif;
	busyLED();
	printk("           Initialising TCP/IP...");
	mem_init();
	memp_init();
	pbuf_init(); 
	netif_init();
	ip_init();
	udp_init();
	tcp_init();
	etharp_init();
	cromwellSuccess();
	printk("           Waiting for IP...");
	printk(" ");
	applicationStarted = 0;
	gotIP = 0;
	resetCount = 0;
	
/*	IP4_ADDR(&gw, 192,168,99,1);
	IP4_ADDR(&ipaddr, 192,168,99,2);
	IP4_ADDR(&netmask, 255,255,255,0);
*/
	IP4_ADDR(&gw, 0,0,0,0);
	IP4_ADDR(&ipaddr, 0,0,0,0);
	IP4_ADDR(&netmask, 255,255,255,255);
	
	netif_add(&netif, &ipaddr, &netmask, &gw, NULL, ebd_init, ip_input);
   dhcp_start(&netif);

	netif_set_default(&netif);

	inputLED();
	
   int divisor = 0;
	int first = 1;
	while (1) {
		if(risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_C) == 1) {
			if(resetCount >= 3) {
				I2CRebootSlow();
			}
			resetCount++;
		}

		if (!ebd_wait(&netif, TCP_TMR_INTERVAL)) {
			if (divisor++ == 60 * 4) {
				if (first && netif.dhcp->state != DHCP_BOUND) {
					printk ("192.168.0.250 (DHCP failed)\n");
					dhcp_stop (&netif);
					IP4_ADDR(&gw, 192,168,0,1);
					IP4_ADDR(&ipaddr, 192,168,0,250);
					IP4_ADDR(&netmask, 255,255,255,0);
					netif_set_ipaddr(&netif, &ipaddr);
					netif_set_netmask(&netif, &netmask);
					netif_set_gw(&netif, &gw);
					gotIP = 1;
				}
				first = 0;
				dhcp_coarse_tmr();

				divisor=0;
			}
			if (divisor & 1) {
				dhcp_fine_tmr();
			}

			tcp_tmr();

			if(applicationStarted == 0) {
				if((gotIP == 1) || (netif.dhcp->state == DHCP_BOUND)) {
					wait_ms(2000);
					if(A == 1336) {
						applicationStarted = 1;
						printk("           Starting Net Flash...");
						netflash_init();
					} else if(A == 1337) {
						applicationStarted = 1;
						printk("           Starting Web Update...");
						webupdate_init();
					}/* else if(A == 1338) {
						applicationStarted = 1;
						printk("           Starting Net Boot...");
						netboot_init();
					} else {
						applicationStarted = 1;
						printk("           Starting Web Boot...");
					   webboot_init(A, B, C, D, P);
					}*/
				}
			}
		}
	}
}

