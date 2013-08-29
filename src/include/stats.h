/*
** Module   :STATS.H
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sun  17/05/1998 Created
**      Wed  15/07/1998 Updated
**
*/

#ifndef __STATS_H
#define __STATS_H

#ifndef QSV_NUMPROCESSORS
#define QSV_NUMPROCESSORS   26
#endif

#define LL2F(high, low)       (4294967296.0*(high)+(low))
#define CPUS                  (4)
#define CMD_KI_RDCNT          (0x63)
#define CMD_KI_ENABLE         (0x60)


struct UptimeInfo
{
    ULONG uSeconds;
    ULONG uMins;
    ULONG uHours;
    ULONG uDays;
};

typedef struct UptimeInfo* PUptime;

typedef void APIENTRY TThread(ULONG data);
typedef TThread* PThread;

/* Measurement threads */

void APIENTRY thUptime(ULONG data);
void APIENTRY thRam(ULONG data);
void APIENTRY thCPU(ULONG data);
void APIENTRY thMAIL(ULONG data);
void APIENTRY thIP(ULONG data);
void APIENTRY thCONN(ULONG data);

#ifdef __cplusplus
extern "C" {
#endif

APIRET16 APIENTRY16 Dos16MemAvail(PULONG pulAvailMem);
APIRET APIENTRY DosPerfSysCall(ULONG ulCommand, ULONG ulParm1,
                               ULONG ulParm2, ULONG ulParm3);
#ifdef __cplusplus
}
#endif


/* Measurement calls */
/* NOTE: Most of these calls is not thread safe */

APIRET GetUptime(PUptime);
APIRET GetFreeMem(ULONG * pMem);
APIRET GetCPULoad(int *load, int *kernel, int CPU);

int PingSite(char *site, int iterations, int packetsize);
int ping(char *site, int packetsize);

void ChkMail(HWND hwndFrame);
void APIENTRY CheckMailOnce(ULONG);

#endif  /*__STATS_H*/

