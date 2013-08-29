/*
** Module   :CVARS.H
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Wed  30/01/2002 Created
**
*/

#ifndef __CVARS_H
#define __CVARS_H

/* Configuration */
extern int iCurrPalette;
extern int i3DFont;
extern int iDrawBorder;
extern int iBtLeft;
extern int iBtRight;
extern int iFloat;
extern int iLock;
extern int iAttach;
extern int iSplit1;
extern int iSplit2;
extern int iSplit3;
extern int iStartX;
extern int iStartY;
extern int iSizeX;
extern int iSizeY;

extern char cFont1[];
extern char cFont2[];
extern char cFont3[];
extern char cFont4[];

extern char* cMailProg[];
extern char* cMailRun[];
extern char* cMailProgParm[];
extern char* cMailServ[];
extern char* cMailUser[];
extern char* cMailPass[];
extern char* cMailPort[];
extern char cConnRunProg[];
extern char cConnProgParm[];
extern char *cConnSites;

extern int iMailSep[];
extern int iMailEnabled[];
extern int iMailLoop[];
extern int iMailBeep[];
extern int iMailRun[];
extern int iMailInterval[];

extern int iCPUInterval;
extern int iCPUGraph;
extern int iCPUText;
extern int iCPUVar;
extern int iCPUKernel;

extern int iIPInterval;
extern int iIPVar;
extern int iIPGraphEnabled;
extern int iIPTotals;
extern int iIPCurrent;
extern int iIPAdaptive;

extern int iRAMInterval;
extern int iRAMUptime;
extern int iRAMClock;
extern int iRAMMethod;
extern int iRAMText;
extern int iRAMAdaptive;

extern int iConnRunProg;
extern int iConnBeep;
extern int iConnInterval;
extern int iBarType;

extern unsigned uChecker;

#endif  /*__CVARS_H*/

