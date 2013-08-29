/*
** Module   :IPSTAT.CPP
** Abstract :IP Statistics
**
** Copyright (C) Sergey I. Yevtushenko
** Log: Fri  05/03/1998 Created
**      Thu  25/06/1998 Updated
**      Wed  15/07/1998 Upadted
*/

#define INCL_DOS
#define INCL_WIN
#include <os2.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ipstat.h>
#include <util.h>
#include <stats.h>
#include <vars.h>

int sockets_init_complete = 0;

IPStatsMonitor::IPStatsMonitor()
{
    if(!sockets_init_complete)
    {
        sock_init();
        sockets_init_complete = 1;
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);

    if(sock > 0)
    {
    	ioctl(sock, SIOSTATIF, (caddr_t)&one, sizeof(one));

        for(int i = 0; i < IFMIB_ENTRIES; i++)
        {
            ulTotalIn[i]  = one.iftable[i].ifInOctets;
            ulTotalOut[i] = one.iftable[i].ifOutOctets;
            ulEstIn[i] = ulEstOut[i] = 0;
        }

	    DosQuerySysInfo(QSV_MS_COUNT,
    	                QSV_MS_COUNT,
        	            (PVOID)&ulLastCheck,
                        sizeof(ULONG));
    }
}

IPStatsMonitor::~IPStatsMonitor()
{
    soclose(sock);
}

void IPStatsMonitor::GrabStat()
{
    if(sock < 0)
        return;

    unsigned long ulMilliseconds;
    unsigned long ulNewEstIn;
    unsigned long ulNewEstOut;
    unsigned long ulDivisor;

    ioctl(sock, SIOSTATIF, (caddr_t)&one, sizeof(one));

    DosQuerySysInfo(QSV_MS_COUNT,
                    QSV_MS_COUNT,
                    (PVOID)&ulMilliseconds,
                    sizeof(ULONG));

    ulDivisor   = (ulMilliseconds - ulLastCheck);
    if(!ulDivisor)
        ulDivisor = 1;

    for(int i = 0; i < IFMIB_ENTRIES; i++)
    {
        ulNewEstIn  = (double(one.iftable[i].ifInOctets  - ulTotalIn[i] )*1000)/ulDivisor;
        ulNewEstOut = (double(one.iftable[i].ifOutOctets - ulTotalOut[i])*1000)/ulDivisor;

        ulTotalIn [i] = one.iftable[i].ifInOctets;
        ulTotalOut[i] = one.iftable[i].ifOutOctets;

        ulEstIn[i]  = ulNewEstIn;
        ulEstOut[i] = ulNewEstOut;
    }

    ulLastCheck = ulMilliseconds;
}

void IPStatsMonitor::ListItems(HWND hwndDlg)
{
    if(sock < 0)
        return;

    for(int i = 0; i < IFMIB_ENTRIES; i++)
    {
        if(one.iftable[i].ifDescr[0])
        {
            int iNdx = (LONG) WinSendMsg(hwndDlg,
                	                     LM_INSERTITEM,
                    	                 MPFROMSHORT(LIT_END),
                                         MPFROMP(one.iftable[i].ifDescr));

            WinSendMsg(hwndDlg,
                       LM_SETITEMHANDLE,
                       MPFROMLONG(iNdx),
                       MPFROMLONG(i));

            if (i == iIPCurrent)
                WinSendMsg(hwndDlg,
                           LM_SELECTITEM,
                           MPFROMLONG(iNdx),
                           MPFROMLONG(TRUE));
        }
    }
}

int init_pop(char *cHost, char *cPort)
{
    int sock;
    unsigned short port;
    struct hostent *hostnm;
    struct sockaddr_in server;

    if(!sockets_init_complete)
    {
        sock_init();
        sockets_init_complete = 1;
    }

    hostnm = gethostbyname(cHost);

    if (!hostnm)
        return -1;

    port = (unsigned short) atoi(cPort);

    if(port <= 0)
        return -2;

    server.sin_family      = AF_INET;
    server.sin_port        = htons(port);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    sock = socket(PF_INET, SOCK_STREAM, 0);

    if(sock < 0)
        return -3;

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        soclose(sock);
        return -4;
    }
    return sock;
}

int send_line(int sock, char *str)
{
    int len;
    if(sock < 0 || !str)
        return -1;

    len = strlen(str);
    if(send(sock, (char *)str, len, 0) < 0)
        return -2;
    return len;
}

int recv_line(int sock, char *str, int max_len)
{
    int len = 0;
    char last_char = 0;

    if(sock < 0 || !str)
        return -1;
    do
    {
        if(recv(sock, &last_char, 1, 0) < 0)
        {
            str[len] = 0;
            return len;
        }
        str[len++] = last_char;
    }
    while(last_char != '\n' && len < (max_len - 1));

    str[len] = 0;
    return len;
}

int done_pop(int sock)
{
    shutdown(sock,2);
    soclose(sock);
    return 0;
}

int put_get(int sock, char *str, int len)
{
    int rc;

    if(!str)
        return -1;

    rc = send_line(sock, str);

    if(rc < 0)
        return rc;

    rc = recv_line(sock, str, len);

    if(rc < 0)
        return rc;

    if(str[0]=='+' && str[1]=='O' && str[2]=='K')
        return 1;

    return 0;
}

int GetMessageCount(char *server, char *port, char *user, char *password)
{
    char line[512];
    int sock;
    int rc;
    int num = 0;

    sock = init_pop(server, port);

    if(sock < 0)
        return 0;

    rc = recv_line(sock, line, sizeof(line));

    if(rc <= 0)
    {
        done_pop(sock);
        return 0;
    }

    if(line[0] != '+' || line[1] != 'O' || line[2] != 'K')
    {
        done_pop(sock);
        return 0;
    }


    sprintf(line, "USER %s\r\n", user);

    rc = put_get(sock, line, sizeof(line));

    if(rc <= 0)
    {
        done_pop(sock);
        return 0;
    }

    sprintf(line, "PASS %s\r\n", password);

    rc = put_get(sock, line, sizeof(line));

    if(rc <= 0)
    {
        done_pop(sock);
        return 0;
    }

    strcpy(line, "STAT\r\n");

    rc = put_get(sock, line, sizeof(line));

    if(rc <= 0)
    {
        done_pop(sock);
        return 0;
    }

    //Now line contain message in following format:
    //+OK Mailbox open, xxx yyy
    //printf("%s", line);

    for(rc = 3; line[rc] && !(line[rc] >= '0' && line[rc] <= '9'); )
        rc++;

    while(line[rc] && line[rc] >= '0' && line[rc] <= '9')
    {
        num *= 10;
        num += line[rc] - '0';
        rc++;
    }

    strcpy(line, "QUIT\r\n");
    put_get(sock, line, sizeof(line));  //Ignore rc

    done_pop(sock);
    return num;
}

