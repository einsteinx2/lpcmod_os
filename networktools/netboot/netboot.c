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
#include "memory_layout.h"
#include <shared.h>

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

	void *data_start;
	int data_len;

};

static int postNum=0;
static int initrdSize = 0;

static char http_file0[] = 
#include "webContent/main.html.h"
;

static char http_file1[] = 
#include "webContent/kernel.html.h"
;

static char http_file2[] = 
#include "webContent/initrd.html.h"
;

static char http_file3[] = 
#include "webContent/append.html.h"
;

static char http_file4[] = 
#include "webContent/booting.html.h"
;

static char http_file5[] =
#include "webContent/gentoox.png.h"
;

static char http_file404[] = "HTTP/1.1 404 NOT FOUND\r\nContent-Type: text/html\r\nContent-Length: 11\r\n\r\nHello 404!\n";
static char http_file500[] = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\nContent-Length: 11\r\n\r\nError 500!\n";

struct http_file {
	int len;
	char *data;
};

static struct http_file http_files[8]={
	{sizeof (http_file0) - 1, http_file0},       /* 0 */
	{sizeof (http_file1) - 1, http_file1},       /* 1 */
	{sizeof (http_file2) - 1, http_file2},       /* 2 */
	{sizeof (http_file3) - 1, http_file3},       /* 3 */
	{sizeof (http_file4) - 1, http_file4},       /* 4 */
	{sizeof (http_file5) - 1, http_file5},       /* 4 */
	{sizeof (http_file404) - 1, http_file404},   /* 5 */
	{sizeof (http_file500) - 1, http_file500},   /* 6 */
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
  if(hs->data_start) {
		if(postNum == 1) {
			inputLED();
			cromwellSuccess();
//			printk("           Got Kernel: %i\n", hs->data_len);
			memPlaceKernel(hs->data_start, hs->data_len);
		} else if(postNum == 2) {
			if(hs->data_len > 0) {
				cromwellSuccess();
			} else {
				printk("\t[ skipped ]\n");
			}
			inputLED();
//			printk("           Got Initrd: %i\n", hs->data_len);
			initrdSize = hs->data_len;
			memcpy((u8*)INITRD_START, hs->data_start, hs->data_len);
		} else if(postNum == 3) {
			cromwellSuccess();
			goodLED();
//			printk("           Got Append: %i\n", hs->data_len);
			char *append = (char*)malloc(hs->data_len+1);
			memset(append, 0, hs->data_len+1);
			memcpy(append, hs->data_start, hs->data_len);
//			printk("%s", append);
			eth_disable();
			ExittoLinuxFromNet(initrdSize, append);
			while(1);
		}
		hs->data_start = NULL;
		hs->data_len = 0;
  }


  if (hs->postdata)
    hs->postdata = NULL;

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
			if(fno == 0) {
				fno = 100;
			}
			if(fno > 105 || fno < 100) {
				fno = 106; /* 404 */
			}

			hs->file = http_files[fno-100].data;
			hs->left = http_files[fno-100].len;
		} else if (strncmp (hs->lineBuf, "POST /", 5) == 0) {
			hs->ispost = 1;
			// Send the client back the next step (which happens to be
			// postNum+2)
			hs->file = http_files[postNum+2].data;
			hs->left = http_files[postNum+2].len;
		} else {
			return 0;
		}
		hs->gotfirst = 1;
	} else {

		/* end of header empty line? */
		if (hs->lineBuf[0] == '\0') {
			if (hs->ispost && hs->postlen) {
				hs->postdata = (u8*)(INITRD_START + (15*1024*1024)); //(char *)malloc (hs->postlen);
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

	postNum += 1;
	hs->data_start = start;
	hs->data_len = len;

	return 1;
}

static err_t http_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
  int i;
  char *data;
  struct http_state *hs;
  static int beginPost = 1;

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
				  if(postNum == 0 && beginPost) {
					   downloadingLED();
						printk("           Receiving kernel...");
				  } else if(postNum == 1 && beginPost) {
					   downloadingLED();
						printk("           Receiving initrd...");
				  } else if(postNum == 2 && beginPost) {
					   downloadingLED();
						printk("           Receiving append...");
				  }

				  beginPost = 0;

				  if (hs->postpos == hs->postlen) {
					  beginPost = 1;
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

  hs->data_start = NULL;
  hs->data_len = 0;

  /* Tell TCP that this is the structure we wish to be passed for our
     callbacks. */
  tcp_arg(pcb, hs);

  /* Tell TCP that we wish to be informed of incoming data by a call
     to the http_recv() function. */
  tcp_recv(pcb, http_recv);

  tcp_err(pcb, conn_err);
  
  return ERR_OK;
}

void netboot_init(void) {
  struct tcp_pcb *pcb;

  pcb = tcp_new();
  tcp_bind(pcb, IP_ADDR_ANY, 80);
  pcb = tcp_listen(pcb);
  tcp_err(pcb, conn_err);
  tcp_accept(pcb, http_accept);
  cromwellSuccess();
  printk("\n\n\n\n\n           Go to 'http://ip.address.shown.above' to begin.\n\n");
  inputLED();
}
