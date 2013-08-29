/*
** Module   :VARS.H
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sun  17/05/1998 Created
**      Wed  15/07/1998 Updated
*/

#include <cvars.h>

#ifndef __VARS_H
#define __VARS_H

/* General */

extern IPStatsMonitor ipStat;
extern UptimeInfo Uptime;
extern int iCPULoad;
extern int iCPULoad1;
extern int iCPUKLoad;
extern int iCPUKLoad1;
extern int iNumCPU;
extern ULONG uFreeRAM;
extern ULONG uIn ;
extern ULONG uOut;
extern ULONG uInTotal ;
extern ULONG uOutTotal;
extern ULONG uMaxIO;

extern int iMails[];
extern int iLastMails[];
extern int iNeedBeep[];

extern LONG lColorFG;
extern LONG lColorBG;
extern LONG lColorGR;
extern LONG lColorG1;
extern LONG lColorG2;
extern LONG lColorGD;

extern int iIPStartGrid;
extern int iCPUStartGrid;

extern HMTX hmtxSites;

extern int iValuesFilled;
extern int iInValues [];
extern int iOutValues[];
extern int iInRealValues [];
extern int iOutRealValues[];
extern int iCValuesFilled;
extern int iCPUValues [];
extern int iCPUValues1[];
extern int iCPUKValues [];
extern int iCPUKValues1[];

extern int iPal0;
extern int iPal1;
extern int iPal2;
extern int iPal3;
extern int iPal4;
extern int iPal5;

extern HWND hwndMainFrame;

extern int iSplitType0;
extern int iSplitType1;
extern int iSplitType2;

/* INI interface */

void LoadVars(void);
void StoreVars(void);
void PrintVars(void);

#endif  /*__VARS_H*/

