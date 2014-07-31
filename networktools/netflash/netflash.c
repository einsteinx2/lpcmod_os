/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
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
 * Modified: Thomas Pedley <gentoox@shallax.com>
 */

#include "boot.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"

struct http_state {
	unsigned char retries;
	char *file;
	int left;
	
	int ispost;
	char *postdata;
	int postlen;
	int postpos;

	char lineBuf[256];
	char lineBufpos;
	char gotfirst;

	void *bios_start;
	int bios_len;

};

static char http_file0[] = 
#include "webContent/main.html.h"
;
static char http_file1[] = 
#include "webContent/gentoox.png.h"
;
static char http_file2[] = 
#include "webContent/ok.html.h"
;
static char http_file3[] = 
#include "webContent/fail.html.h"
;
static char http_file404[] = "HTTP/1.1 404 NOT FOUND\r\nContent-Type: text/html\r\nContent-Length: 11\r\n\r\nHello 404!\n";
static char http_file500[] = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\nContent-Length: 11\r\n\r\nError 500!\n";

struct http_file {
	int len;
	char *data;
};
static struct http_file http_files[6]={
	{sizeof (http_file0) - 1, http_file0},       /* 0 */
	{sizeof (http_file1) - 1, http_file1},       /* 1 */
	{sizeof (http_file2) - 1, http_file2},       /* 2 */
	{sizeof (http_file3) - 1, http_file3},       /* 3 */
	{sizeof (http_file404) - 1, http_file404},   /* 4 */
	{sizeof (http_file500) - 1, http_file500},   /* 5 */
};

static void conn_err(void *arg, err_t err) {
  struct http_state *hs;

  hs = arg;
  mem_free(hs);
}

static void close_conn(struct tcp_pcb *pcb, struct http_state *hs) {
  tcp_arg(pcb, NULL);
  tcp_sent(pcb, NULL);
  tcp_recv(pcb, NULL);

  if (hs->bios_start) {
		extern void ClearScreen (void);
		ClearScreen ();
		busyLED();
		//printk ("\nGot BIOS-image over http, %d bytes\n", hs->bios_len);
		memcpy ((void*)0x100000, hs->bios_start, hs->bios_len);
		BootReflashAndReset((void*)0x100000,0,hs->bios_len);
		while (1)
			;
  }

  if (hs->postdata)
	  free (hs->postdata);

  mem_free(hs);
  tcp_close(pcb);
}

static void send_data(struct tcp_pcb *pcb, struct http_state *hs) {
  err_t err;
  u16_t len;

  /* We cannot send more data than space available in the send
     buffer. */     
  if (tcp_sndbuf(pcb) < hs->left) {
    len = tcp_sndbuf(pcb);
  } else {
    len = hs->left;
  }

  do {
    err = tcp_write(pcb, hs->file, len, 0);
    if (err == ERR_MEM) {
      len /= 2;
    }
  } while (err == ERR_MEM && len > 1);  
  
  if (err == ERR_OK) {
    hs->file += len;
    hs->left -= len;
    /*  } else {
	printf("send_data: error %s len %d %d\n", lwip_strerr(err), len, tcp_sndbuf(pcb));*/
  }
}

static err_t http_poll(void *arg, struct tcp_pcb *pcb) {
  struct http_state *hs;

  hs = arg;
  
  /*  printf("Polll\n");*/
  if (hs == NULL) {
	  //printk("p");
	  /*tcp_abort(pcb);*/
    return ERR_OK;
  } else {
	  //printk("P");
    ++hs->retries;
    if (hs->retries == 4) {
      tcp_abort(pcb);
      return ERR_ABRT;
    }
    send_data(pcb, hs);
  }

  return ERR_OK;
}

static err_t http_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
  struct http_state *hs;

  hs = arg;

  hs->retries = 0;
  
  if (hs->left > 0) {    
    send_data(pcb, hs);
  } else {
    close_conn(pcb, hs);
  }

  return ERR_OK;
}

static int handle_line(struct tcp_pcb *pcb, struct http_state *hs) {
	if (!hs->gotfirst) {
		if (strncmp (hs->lineBuf, "GET /", 4) == 0) {
			unsigned long fno = simple_strtoul (&hs->lineBuf[5], NULL, NULL);
			if (fno > 2) {
				fno = 4; /* 404 */
			}
			hs->file = http_files[fno].data;
			hs->left = http_files[fno].len;
		} else if (strncmp (hs->lineBuf, "POST /", 5) == 0) {
			hs->ispost = 1;
			hs->file = http_files[5].data;
			hs->left = http_files[5].len;
		} else {
			return 0;
		}
		hs->gotfirst = 1;
	} else {

		/* end of header empty line? */
		if (hs->lineBuf[0] == '\0') {
			if (hs->ispost && hs->postlen) {
				hs->postdata = (char *)malloc (hs->postlen);
				hs->postpos = 0;
			} else {
				send_data(pcb, hs);
				tcp_poll(pcb, http_poll, 4);
				tcp_sent(pcb, http_sent);
				hs->gotfirst = 0;
			}
		}

		if (strncmp (hs->lineBuf, "Content-Length:", 15) == 0) {
			unsigned long len = simple_strtoul (&hs->lineBuf[16], NULL, NULL);

			hs->postlen = len;
		}
	}
	return 1;
}

static int handle_post(struct http_state *hs) {
	int i, ncnt = 0, blen, len;
	char *start, *end;
	char *boundary = NULL;

	hs->file = http_files[5].data;
	hs->left = http_files[5].len;

	for (i = 0; i < hs->postlen; i++) {
		if (hs->postdata[i] == '\n' || hs->postdata[i] == '\r') {
			hs->postdata[i] = 0;
			boundary = hs->postdata;
			blen = i;
			break;
		}
	}
	if (!boundary)
		return 0;

	for (; i < hs->postlen; i++) {
		if (hs->postdata[i] == '\r')
			continue;

		if (hs->postdata[i] == '\n') {
			ncnt++;
			if (ncnt == 2) {
				start = &hs->postdata[i+1];
				break;
			}
		} else {
			ncnt = 0;
		}
	}
	if (!start) {
		//printk ("Could not find start...\n");
		return 0;
	}

	for (; i < hs->postlen; i++) {
		if ( memcmp(boundary,&hs->postdata[i],blen) == 0) {
			end = &hs->postdata[i];
			break;
		}
	}

	if (!end) {
		//printk ("Could not find end...\n");
		return 0;
	}

	end -= 2;

	len = end - start;


	if (len != 256*1024 && len != 512*1024 && len != 1024*1024) {
		hs->file = http_files[3].data;
		hs->left = http_files[3].len;
		//printk("Illegal size, NOT flashing\n");
		return 0;
	}

	hs->bios_start = start;
	hs->bios_len = len;

	hs->file = http_files[2].data;
	hs->left = http_files[2].len;
/*
*/
	return 1;
}

static err_t http_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
  int i;
  char *data;
  struct http_state *hs;

  hs = arg;

  if (err == ERR_OK && p != NULL) {
	  struct pbuf *q;
    /* Inform TCP that we have taken the data. */
    tcp_recved(pcb, p->tot_len);

	  for (q = p; q; q = q->next) {
		  for (i = 0; i < q->len; i++) {
			  char c = ((char *)q->payload)[i];

			  if (!hs->postdata) {
				  if (c == '\r') /* ignore \r */
					  continue;
				  
				  if (c == '\n') {
					  hs->lineBuf[hs->lineBufpos] = 0;
					  if (!handle_line (pcb, hs)) {
						  pbuf_free(p);
						  close_conn(pcb, hs);
						  return ERR_OK;
					  }
					  hs->lineBufpos = 0;
				  } else {
					  if (hs->lineBufpos < sizeof (hs->lineBuf) - 2)
						  hs->lineBuf[hs->lineBufpos++] = c;
				  }
			  } else {
				  hs->postdata[hs->postpos++] = c;
				  if (hs->postpos == hs->postlen) {
					  handle_post (hs);
					  send_data(pcb, hs);
					  tcp_poll(pcb, http_poll, 4);
					  tcp_sent(pcb, http_sent);
					  hs->gotfirst = 0;
				  }
			  }
		  }
	  }

	pbuf_free(p);
  }
  if (err == ERR_OK && p == NULL) {
    close_conn(pcb, hs);
  }
  return ERR_OK;
}

static err_t http_accept(void *arg, struct tcp_pcb *pcb, err_t err) {
  struct http_state *hs;

  tcp_setprio(pcb, TCP_PRIO_MIN);
  
  /* Allocate memory for the structure that holds the state of the
     connection. */
  hs = mem_malloc(sizeof(struct http_state));

  if (hs == NULL) {
    //printk("http_accept: Out of memory\n");
    return ERR_MEM;
  }

  /* Initialize the structure. */
  hs->file = NULL;
  hs->left = 0;
  hs->retries = 0;
  hs->lineBufpos = 0;
  hs->gotfirst = 0;
  hs->ispost = 0;
  hs->postdata = NULL;
  hs->postpos = 0;
  hs->postlen = 0;

  hs->bios_start = NULL;
  hs->bios_len = 0;

  /* Tell TCP that this is the structure we wish to be passed for our
     callbacks. */
  tcp_arg(pcb, hs);

  /* Tell TCP that we wish to be informed of incoming data by a call
     to the http_recv() function. */
  tcp_recv(pcb, http_recv);

  tcp_err(pcb, conn_err);
  
  return ERR_OK;
}
void netflash_init(void) {
  struct tcp_pcb *pcb;

  pcb = tcp_new();
  tcp_bind(pcb, IP_ADDR_ANY, 80);
  pcb = tcp_listen(pcb);
  tcp_accept(pcb, http_accept);
  cromwellSuccess();
  printk("\n\n\n\n\n           Go to 'http://ip.address.shown.above' to flash your BIOS.\n");
  downloadingLED();
}
