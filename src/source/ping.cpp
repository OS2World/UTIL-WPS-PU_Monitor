/*
** Module   :PING.CPP
** Abstract :Implementation of ping routine.
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Thu  06/08/1998 Created
**
*/

#define INCL_DOS
#include <os2.h>
#include <string.h>
#include <stdio.h>

#include <ip_hdrs.h>
#include <process.h>

#define delay   DosSleep

int in_cksum(u_short *addr, int len)
{
    int nleft = len;
    u_short *w = addr;
    int sum = 0;
    u_short answer = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1)
    {
        *(u_char *)(&answer) = *(u_char *)w ;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return(answer);
}

int pinger(int s, struct sockaddr* whereto, char *outpack, int datalen)
{
    struct icmp *icp;
    int cc;
    int i;
    static int ntransmitted = 0;

    icp = (struct icmp *)outpack;
    icp->icmp_type  = ICMP_ECHO;
    icp->icmp_code  = 0;
    icp->icmp_cksum = 0;
    icp->icmp_seq   = ntransmitted++;
    icp->icmp_id    = getpid();

    cc = datalen + 8;

    icp->icmp_cksum = in_cksum((u_short *)icp, cc);
    i = sendto(s, (char *)outpack, cc, 0, whereto, sizeof(struct sockaddr));

    if (i < 0 || i != cc)
    {
        if (i < 0)
            return -1;
    }
    return 0;
}

int ping(char *addr, int packetsize)
{
    int rc = 0;
    char *src_packet = 0;
    char *dst_packet = 0;
    ULONG ulTmrFreq  = 0;
    QWORD qwTmrTime;
    QWORD qwTmrTime1;

    DosTmrQueryFreq(&ulTmrFreq);

    int s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	if (s >= 0)
	{
        sockaddr_in a;

		a.sin_family = AF_INET;
		a.sin_port = htons(7);
        a.sin_addr.s_addr = inet_addr(addr);

		if (a.sin_addr.s_addr == INADDR_NONE)
		{
			hostent *p = gethostbyname(addr);

			if (p)
				memcpy(&a.sin_addr.s_addr, p->h_addr, p->h_length);
		}

        if (a.sin_addr.s_addr != 0 &&
            a.sin_addr.s_addr != 0xFFFFFFFF)
        {
            struct sockaddr_in from;
            int cc;
            int fromlen;
            fd_set readmask;
            timeval timeout;

            src_packet = new char[packetsize + sizeof(struct icmp)];
            dst_packet = new char[packetsize + sizeof(struct icmp)];

            memset(src_packet, 0x5A, packetsize + sizeof(struct icmp));

            DosTmrQueryTime(&qwTmrTime);

            FD_ZERO(&readmask);
            FD_SET(s, &readmask);
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;

            fromlen = sizeof(from);
            pinger(s, (struct sockaddr *)&a, src_packet, packetsize);

            cc = -1;

            if (select(s + 1, &readmask, 0, 0, &timeout) > 0)
                if (FD_ISSET(s, &readmask))
    	            cc = recvfrom(s, dst_packet, packetsize, 0, (struct sockaddr *)&from, &fromlen);

            if(cc != packetsize)
                rc = -1;
            else
            {
                double dStart;
                double dStop;

#define LL2F(high, low)       (4294967296.0*(high)+(low))

                DosTmrQueryTime(&qwTmrTime1);
                dStart = LL2F(qwTmrTime.ulHi, qwTmrTime.ulLo);
                dStop  = LL2F(qwTmrTime1.ulHi, qwTmrTime1.ulLo);
                rc = ((dStop - dStart)*1000.0) / ((double)ulTmrFreq);
            }

            delete src_packet;
            delete dst_packet;

            shutdown(s, 2);
        }
        else
            rc = -1;

        soclose(s);
    }
    else
        rc = -1;

    return rc;
}

