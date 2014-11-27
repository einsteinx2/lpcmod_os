/**
 * @file
 * DNS module
 *
 */


/* dns.c
 *
 * Stripped DNS module.
 *
 */

#include "lwip/opt.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"

#include "lwip/def.h"
#include "lwip/memp.h"
#include "lwip/inet.h"
#include "lwip/netif.h"
#include "lwip/udp.h"
#include "lwip/icmp.h"
#include "lwip/ip_addr.h"

#include "lwip/stats.h"

#include "arch/perf.h"
#include "lwip/snmp.h"

#define SIZEOF_DNS_QUERY 4

#define SIZEOF_DNS_ANSWER 10

#define SIZEOF_DNS_HDR 12

#define DNS_MAX_NAME_LENGTH             256

/** DNS server IP address */
#ifndef DNS_SERVER_ADDRESS
#define DNS_SERVER_ADDRESS(ipaddr)        (ip4_addr_set_u32(ipaddr, ipaddr_addr("208.67.222.222"))) /* resolver1.opendns.com */
#endif

/** DNS server port address */
#ifndef DNS_SERVER_PORT
#define DNS_SERVER_PORT           53
#endif

/** DNS maximum number of retries when asking for a name, before "timeout". */
#ifndef DNS_MAX_RETRIES
#define DNS_MAX_RETRIES           4
#endif

/** DNS resource record max. TTL (one week as default) */
#ifndef DNS_MAX_TTL
#define DNS_MAX_TTL               604800
#endif

/* DNS protocol flags */
#define DNS_FLAG1_RESPONSE        0x80
#define DNS_FLAG1_OPCODE_STATUS   0x10
#define DNS_FLAG1_OPCODE_INVERSE  0x08
#define DNS_FLAG1_OPCODE_STANDARD 0x00
#define DNS_FLAG1_AUTHORATIVE     0x04
#define DNS_FLAG1_TRUNC           0x02
#define DNS_FLAG1_RD              0x01
#define DNS_FLAG2_RA              0x80
#define DNS_FLAG2_ERR_MASK        0x0f
#define DNS_FLAG2_ERR_NONE        0x00
#define DNS_FLAG2_ERR_NAME        0x03

/* DNS protocol states */
#define DNS_STATE_UNUSED          0
#define DNS_STATE_NEW             1
#define DNS_STATE_ASKING          2
#define DNS_STATE_DONE            3

/*-----------------------------------------------------------------------------
 * Globales
 *----------------------------------------------------------------------------*/

/* DNS variables */
static struct udp_pcb        *dns_pcb;
static struct ip_addr        dns_servers[DNS_MAX_SERVERS];

PACK_STRUCT_BEGIN
/** DNS message header */
struct dns_hdr {
  PACK_STRUCT_FIELD(u16_t id);
  PACK_STRUCT_FIELD(u8_t flags1);
  PACK_STRUCT_FIELD(u8_t flags2);
  PACK_STRUCT_FIELD(u16_t numquestions);
  PACK_STRUCT_FIELD(u16_t numanswers);
  PACK_STRUCT_FIELD(u16_t numauthrr);
  PACK_STRUCT_FIELD(u16_t numextrarr);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END

/**
 * Send a DNS query packet.
 *
 * @param numdns index of the DNS server in the dns_servers table
 * @param name hostname to query
 * @param id index of the hostname in dns_table, used as transaction ID in the
 *        DNS query packet
 * @return ERR_OK if packet is sent; an err_t indicating the problem otherwise
 */
static err_t
dns_send(u8_t numdns, const char* name, u8_t id)
{
  err_t err;
  struct dns_hdr *hdr;
  struct dns_query qry;
  struct pbuf *p;
  char *query, *nptr;
  const char *pHostname;
  u8_t n;

  LWIP_DEBUGF(DNS_DEBUG, ("dns_send: dns_servers[%"U16_F"] \"%s\": request\n",
              (u16_t)(numdns), name));
  //LWIP_ASSERT("dns server out of array", numdns < DNS_MAX_SERVERS);
  //LWIP_ASSERT("dns server has no IP address set", !ip_addr_isany(&dns_servers[numdns]));

  /* if here, we have either a new query or a retry on a previous query to process */
  p = pbuf_alloc(PBUF_TRANSPORT, SIZEOF_DNS_HDR + DNS_MAX_NAME_LENGTH +
                 SIZEOF_DNS_QUERY, PBUF_RAM);
  if (p != NULL) {
    //LWIP_ASSERT("pbuf must be in one piece", p->next == NULL);
    /* fill dns header */
    hdr = (struct dns_hdr*)p->payload;
    memset(hdr, 0, SIZEOF_DNS_HDR);
    hdr->id = htons(id);
    hdr->flags1 = DNS_FLAG1_RD;
    hdr->numquestions = PP_HTONS(1);
    query = (char*)hdr + SIZEOF_DNS_HDR;
    pHostname = name;
    --pHostname;

    /* convert hostname into suitable query format. */
    do {
      ++pHostname;
      nptr = query;
      ++query;
      for(n = 0; *pHostname != '.' && *pHostname != 0; ++pHostname) {
        *query = *pHostname;
        ++query;
        ++n;
      }
      *nptr = n;
    } while(*pHostname != 0);
    *query++='\0';

    /* fill dns query */
    qry.type = PP_HTONS(DNS_RRTYPE_A);
    qry.cls = PP_HTONS(DNS_RRCLASS_IN);
    SMEMCPY(query, &qry, SIZEOF_DNS_QUERY);

    /* resize pbuf to the exact dns query */
        pbuf_realloc(p, (u16_t)((query + SIZEOF_DNS_QUERY) - ((char*)(p->payload))));

        /* connect to the server for faster receiving */
        udp_connect(dns_pcb, &dns_servers[numdns], DNS_SERVER_PORT);
        /* send dns packet */
        err = udp_sendto(dns_pcb, p, &dns_servers[numdns], DNS_SERVER_PORT);

        /* free pbuf */
        pbuf_free(p);
      } else {
        err = ERR_MEM;
      }

      return err;
    }
