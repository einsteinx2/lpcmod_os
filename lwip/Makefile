EXTRA_CFLAGS := -DIPv4 -fpack-struct -Ilwip/src/include -Ilwip/src/include/ipv4 -Ixbox/include -Os -march=pentium3m -m32 -msse -mfpmath=sse -mpc32 -Wstrict-prototypes -fomit-frame-pointer -pipe -Ui386

LWIPDIR=.

O_TARGET := dns.o mem.o memp.o netif.o pbuf.o raw.o stats.o sys.o tcp.o tcp_in.o tcp_out.o udp.o dhcp.o icmp.o ip.o inet.o ip_addr.o ip_frag.o etharp.o ebd.o webserver.o

include $(TOPDIR)/Rules.make

