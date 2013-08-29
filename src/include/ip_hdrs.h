/*
** Module   :IP_HDRS.H
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Wed  10/03/1999 Created
**
*/
#ifndef __IP_HDRS_H
#define __IP_HDRS_H

extern "C" {

#define TCPV40HDRS  1
#define OS2
#define BSD_SELECT

#include <types.h>
#include <netdb.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <net/route.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>
}

#endif  /*__IP_HDRS_H*/
