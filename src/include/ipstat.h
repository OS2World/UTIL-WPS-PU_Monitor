/*
** Module   :IPSTAT.H
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
** Copyright (c) knut st. osmundsen (knut.stange.osmundsen@pmsc.no)
**
** Log: Fri  05/03/1998     Created
**           22/11/2001     Updated by knut st. osmundsen
*/

#include <ip_hdrs.h>

#ifndef __IPSTAT_H
#define __IPSTAT_H

class IPStatsMonitor
{
        int sock;
        struct ifmib one;
        unsigned long ulTotalIn[IFMIB_ENTRIES];
        unsigned long ulTotalOut[IFMIB_ENTRIES];
        unsigned long ulEstIn[IFMIB_ENTRIES];
        unsigned long ulEstOut[IFMIB_ENTRIES];
        unsigned long ulLastCheck;

    public:

        IPStatsMonitor();
        ~IPStatsMonitor();

        void GrabStat();

        unsigned long getInTotal(int i)  { return ulTotalIn[i];}
        unsigned long getOutTotal(int i) { return ulTotalOut[i];}
        unsigned long getInEst(int i)    { return ulEstIn[i];}
        unsigned long getOutEst(int i)   { return ulEstOut[i];}
        unsigned long getMaxCPS(int i)   { return (one.iftable[i].ifSpeed) ? (one.iftable[i].ifSpeed / 8):1;}
        unsigned long getIFtype(int i)   { return one.iftable[i].ifType;}
        char * getName(int i)            { return one.iftable[i].ifDescr;}
        int IOctl(int cmd, void* d, int s) { return ioctl(sock, cmd, (caddr_t)d, s);}

        void ListItems(HWND hwndDlg);
        void Next();
};

int GetMessageCount(char *server, char *port, char *user, char *password);
#endif  /* __IPSTAT_H */

