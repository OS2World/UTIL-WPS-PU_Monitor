/*
** Module   :STATS.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sat  16/05/1998 Created
**      Thu  25/06/1998 Updated
**      Wed  15/07/1998 Updated
*/

#define INCL_DOS
#define INCL_WIN
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include <ipstat.h>
#include <id.h>
#include <cell.h>
#include <stats.h>
#include <vars.h>
#include <util.h>
#include <win32k.h>

#define printf(a)

typedef struct _CPUUTIL
{
    ULONG ulTimeLow;     /* Low 32 bits of time stamp      */
    ULONG ulTimeHigh;    /* High 32 bits of time stamp     */
    ULONG ulIdleLow;     /* Low 32 bits of idle time       */
    ULONG ulIdleHigh;    /* High 32 bits of idle time      */
    ULONG ulBusyLow;     /* Low 32 bits of busy time       */
    ULONG ulBusyHigh;    /* High 32 bits of busy time      */
    ULONG ulIntrLow;     /* Low 32 bits of interrupt time  */
    ULONG ulIntrHigh;    /* High 32 bits of interrupt time */
} CPUUTIL;

typedef CPUUTIL *PCPUUTIL;


IPStatsMonitor ipStat;
UptimeInfo Uptime;
int iNumCPU        = 1;
int iCPULoad       = 0;
int iCPULoad1      = 0;
int iCPUKLoad      = 0;
int iCPUKLoad1     = 0;
ULONG uFreeRAM     = 0;
ULONG uIn          = 0;
ULONG uOut         = 0;
ULONG uInTotal     = 0;
ULONG uOutTotal    = 0;
ULONG uMaxIO       = 1;
int iMails[5]      = {0};
int iLastMails[5]  = {0};
int iNeedBeep[5]   = {0};
int iValuesFilled = 0;
int iInValues [GRID_SIZE];
int iOutValues[GRID_SIZE];
int iInRealValues [GRID_SIZE];
int iOutRealValues[GRID_SIZE];

int iCValuesFilled = 0;
int iCPUValues [GRID_SIZE];
int iCPUValues1[GRID_SIZE];

int iCPUValuesR [GRID_SIZE];
int iCPUValues1R[GRID_SIZE];

int iCPUKValues [GRID_SIZE];
int iCPUKValues1[GRID_SIZE];

int iCPUKValuesR [GRID_SIZE];
int iCPUKValues1R[GRID_SIZE];

int iIPStartGrid  = 0;
int iCPUStartGrid = 0;

Sound NewMail[]=
{
    {1700, 32},
    {   1, 64},
    {1700, 32},
    {0,0}
};

Sound ConnDown[]=
{
    {1700, 164},
    {1200, 164},
    {1700, 164},
    {1200, 164},
    {0,0}
};

/* dummy variable */

volatile short ForEver = 1;

/*--------------------------------------------------*/

void AddInOutValues(int iIn, int iOut)
{
    if(iIn < 0 || iOut < 0)
        return;

    memmove(&iInValues[1]     , iInValues     , sizeof(int)*(GRID_SIZE - 1));
    memmove(&iOutValues[1]    , iOutValues    , sizeof(int)*(GRID_SIZE - 1));
    memmove(&iInRealValues[1] , iInRealValues , sizeof(int)*(GRID_SIZE - 1));
    memmove(&iOutRealValues[1], iOutRealValues, sizeof(int)*(GRID_SIZE - 1));

    iInRealValues[0]  = iIn ;
    iOutRealValues[0] = iOut;
    iInValues[0]      = iIn ;
    iOutValues[0]     = iOut;

//S = S - S/n + X/n

    if(iValuesFilled < (GRID_SIZE - 1))
        iValuesFilled++;

    iIPStartGrid++;
    iIPStartGrid %= GRID_STEP;

    {
        //Make iIn/OutValues
        int num = min(iValuesFilled, iIPVar);
        int sum = 0;
        iIn = 0;
        iOut = 0;

        for (int i = 0; i < num; i++)
        {
            int mul = min((num >> 1) + 1 , 8);
            iIn    += iInRealValues[i] * mul;
            iOut   += iOutRealValues[i] *mul;
            sum    += mul;
        }

        iInValues[0] = iIn / sum;
        iOutValues[0] = iOut / sum;
    }
}

void AddCPUValues(int iIn, int iIn1, int kIn, int kIn1)
{
    if(iIn < 0 || iIn1 < 0)
        return;

    memmove(&iCPUValues[1] , iCPUValues , sizeof(int)*(GRID_SIZE - 1));
    memmove(&iCPUValues1[1], iCPUValues1, sizeof(int)*(GRID_SIZE - 1));

    memmove(&iCPUValuesR[1] , iCPUValuesR , sizeof(int)*(GRID_SIZE - 1));
    memmove(&iCPUValues1R[1], iCPUValues1R, sizeof(int)*(GRID_SIZE - 1));

    iCPUValues[0]   = iIn ;
    iCPUValues1[0]  = iIn1;
    iCPUValuesR[0]  = iIn ;
    iCPUValues1R[0] = iIn1;

    if(iCPUKernel)
    {
      	memmove(&iCPUKValues[1] , iCPUKValues , sizeof(int)*(GRID_SIZE - 1));
      	memmove(&iCPUKValues1[1], iCPUKValues1, sizeof(int)*(GRID_SIZE - 1));

      	memmove(&iCPUKValuesR[1] , iCPUKValuesR , sizeof(int)*(GRID_SIZE - 1));
      	memmove(&iCPUKValues1R[1], iCPUKValues1R, sizeof(int)*(GRID_SIZE - 1));

      	iCPUKValues[0]   = kIn ;
      	iCPUKValues1[0]  = kIn1;
      	iCPUKValuesR[0]  = kIn ;
      	iCPUKValues1R[0] = kIn1;
    }

    if(iCValuesFilled < (GRID_SIZE - 1))
        iCValuesFilled++;

    iCPUStartGrid++;
    iCPUStartGrid %= GRID_STEP;

    {
        //Make iIn/OutValues
        int num = min(iCValuesFilled, iCPUVar);
        int sum = 0;
        iIn  = 0;
        iIn1 = 0;

        for (int i = 0; i < num; i++)
        {
            int mul = min((num >> 1) + 1 , 8);
            iIn    += iCPUValuesR[i] * mul;
            iIn1   += iCPUValues1R[i] * mul;

            if(iCPUKernel)
            {
                kIn  += iCPUKValuesR[i] * mul;
                kIn1 += iCPUKValues1R[i] * mul;
            }

            sum    += mul;
        }

        iCPUValues[0]  = iIn / sum;
        iCPUValues1[0] = iIn1 / sum;

        if(iCPUKernel)
        {
        	iCPUKValues[0]  = kIn / sum;
          	iCPUKValues1[0] = kIn1 / sum;
        }
    }
}

/*--------------------------------------------------*/

APIRET GetUptimeSeconds(PULONG pulSeconds);
APIRET GetUptimeSeconds2(PULONG pulSeconds);

/*--------------------------------------------------*/

int PingSite(char *site, int num, int psize)
{
    int success = 0;

    for(int i = 0; i < num; i++)
    {
        int j = ping(site, psize);

        if(j >= 0)
            success++;
        Sleep(32);
    }

    return success;
}

/*--------------------------------------------------*/

void APIENTRY thUptime(ULONG data)
{
    HAB hab;
    HMQ hmq;
    HWND hwndFrame = (HWND)data;

    if(ThInit(hab, hmq))
        return;
/*----------------------------------------*/
    do
    {
        GetUptime(&Uptime);
        if(iRAMUptime || iRAMClock)
            WinInvalidateRect(hwndFrame, NULL, TRUE);
        Sleep(iRAMInterval * 1000);
    }
    while(ForEver);
/*----------------------------------------*/
    ThTerm(hab, hmq);
}

/*--------------------------------------------------*/

void APIENTRY thRam(ULONG data)
{
    HAB hab;
    HMQ hmq;
    HWND hwndFrame = (HWND)data;

    if(ThInit(hab, hmq))
        return;
/*----------------------------------------*/
    do
    {
        GetFreeMem(&uFreeRAM);
        WinInvalidateRect(hwndFrame, NULL, TRUE);
        Sleep(iRAMInterval * 1000);
    }
    while(ForEver);
/*----------------------------------------*/
	ThTerm(hab, hmq);
}

/*--------------------------------------------------*/

static void ChkMail(HWND hwndFrame, int iN)
{
    iMails[iN] = GetMessageCount(cMailServ[iN], cMailPort[iN], cMailUser[iN], cMailPass[iN]);

    if(iMails[iN] != iLastMails[iN] || iNeedBeep[iN])
    {
        if(iMailBeep[iN] || iNeedBeep[iN])
        {
            Play(NewMail);
        }
        if(!iMailLoop[iN])
            iNeedBeep[iN] = 0;

        if(iMails[iN] != iLastMails[iN])
        {
            if(iMailLoop[iN] && iMails[iN])
                iNeedBeep[iN] = 1;
            else
                iNeedBeep[iN] = 0;

            if(iMailEnabled[iN])
            	WinInvalidateRect(hwndFrame, NULL, TRUE);

            if(iMailRun[iN] && (iMails[iN] > iLastMails[iN]))
            {
                WinPostMsg(hwndMainFrame, WM_USER+ID_MENU_START_APP,
                           MPFROMP(cMailRun[iN]),
                           MPFROMP(""));
            }

            iLastMails[iN] = iMails[iN];
        }
    }
}

void ChkMail(HWND hwndFrame)
{
    for(int i = 0; i < 5; i++)
        ChkMail(hwndFrame, i);
}

/*--------------------------------------------------*/

void APIENTRY thCPU(ULONG data)
{
    HAB hab;
    HMQ hmq;
    HWND hwndFrame = (HWND)data;
    int iInterval = 0;

    if(ThInit(hab, hmq))
        return;

/*----------------------------------------*/
    WinInvalidateRect(hwndFrame, NULL, TRUE);

    do
    {
        int iTmpLoad;
        int iTmpLoad1;
        int iTmpKLoad;
        int iTmpKLoad1;

        GetCPULoad(&iTmpLoad , &iTmpKLoad, 0);
        GetCPULoad(&iTmpLoad1, &iTmpKLoad1, 1);

        if(iCPUGraph == 2)
        {
            iInterval += iCPUInterval * 10;

            if(iInterval >= (iCPUInterval * 100))
            {
                iCPULoad  = iTmpLoad;
                iCPULoad1 = iTmpLoad1;
                iCPUKLoad  = iTmpKLoad;
                iCPUKLoad1 = iTmpKLoad1;

                AddCPUValues(iTmpLoad, iTmpLoad1, iTmpKLoad, iTmpKLoad1);

                WinInvalidateRect(hwndFrame, NULL, TRUE);

                iInterval = 0;
            }
        }
        else
        {
            if((iTmpLoad > iCPULoad || iTmpLoad1 > iCPULoad1) &&
               (iTmpKLoad > iCPUKLoad || iTmpKLoad1 > iCPUKLoad1) &&
               (iTmpLoad > 15 || iTmpLoad1 > 15) &&
               (iTmpKLoad > 15 || iTmpKLoad1 > 15))
            {
                iCPULoad  = iTmpLoad;
                iCPULoad1 = iTmpLoad1;
                iCPUKLoad  = iTmpKLoad;
                iCPUKLoad1 = iTmpKLoad1;
                WinInvalidateRect(hwndFrame, NULL, TRUE);
            }

            iInterval += iCPUInterval * 10;

            if(iInterval >= (iCPUInterval * 100))
            {
                iCPULoad  = iTmpLoad;
                iCPULoad1 = iTmpLoad1;
                iCPUKLoad  = iTmpKLoad;
                iCPUKLoad1 = iTmpKLoad1;
                WinInvalidateRect(hwndFrame, NULL, TRUE);
                iInterval = 0;
            }
    	}

        Sleep(iCPUInterval * 10);
    }
    while(ForEver);
/*----------------------------------------*/
    ThTerm(hab, hmq);
}

/*--------------------------------------------------*/

void APIENTRY thMAIL(ULONG data)
{
    HAB hab;
    HMQ hmq;
    HWND hwndFrame = (HWND)data;

    int i = 0;
    int iTime[5] = {0};

    if(ThInit(hab, hmq))
        return;

/*----------------------------------------*/

    for(i = 0; i < 5; i++)
    {
        if(iMailEnabled[i])
        {
            WinInvalidateRect(hwndFrame, NULL, TRUE);
            break;
        }
    }

    do
    {
	    for(i = 0; i < 5; i++)
    	{
            if(iMailEnabled[i] && !iTime[i])
                ChkMail(hwndFrame, i);

            if(!iTime[i])
                iTime[i] = (iMailInterval[i] > 0) ?
                           iMailInterval[i]:10;
            else
                iTime[i]--;
        }

        //Sleep(iMailInterval[iN] * 1000);
        Sleep(1000);
    }
    while(ForEver);
/*----------------------------------------*/
    ThTerm(hab, hmq);
}

void APIENTRY CheckMailOnce(ULONG data)
{
    HAB hab;
    HMQ hmq;
    HWND hwndFrame = (HWND)data;

    if(ThInit(hab, hmq))
        return;
/*----------------------------------------*/
    ChkMail(hwndFrame);
/*----------------------------------------*/
    ThTerm(hab, hmq);
}

/*--------------------------------------------------*/

void APIENTRY thIP(ULONG data)
{
    HAB hab;
    HMQ hmq;
    HWND hwndFrame = (HWND)data;

    if(ThInit(hab, hmq))
        return;

/*----------------------------------------*/
    if(iIPGraphEnabled + iIPTotals)
    	WinInvalidateRect(hwndFrame, NULL, TRUE);

    ipStat.GrabStat();
    Sleep(iIPInterval);
    do
    {
        ipStat.GrabStat();

        ULONG uInTmp    = ipStat.getInEst (iIPCurrent);
        ULONG uOutTmp   = ipStat.getOutEst(iIPCurrent);
        ULONG uMaxIOTmp = ipStat.getMaxCPS(iIPCurrent);

        AddInOutValues(uInTmp, uOutTmp);

        uInTmp 	= iInValues[0];
        uOutTmp = iOutValues[0];

        if(uInTmp    != uIn    ||
           uOutTmp   != uOut   ||
           uMaxIOTmp != uMaxIO ||
           iIPTotals != 0      ||
           iIPGraphEnabled == 2)
        {
            uIn       = uInTmp   ;
            uOut      = uOutTmp  ;
            uMaxIO    = uMaxIOTmp;
            uInTotal  = ipStat.getInTotal(iIPCurrent);
            uOutTotal = ipStat.getOutTotal(iIPCurrent);
        }

        if(iIPGraphEnabled + iIPTotals)
        	WinInvalidateRect(hwndFrame, NULL, TRUE);

        Sleep(iIPInterval * 100);
    }
    while(ForEver);
/*----------------------------------------*/
    ThTerm(hab, hmq);
}

/*--------------------------------------------------*/

void APIENTRY thCONN(ULONG data)
{
    HAB hab;
    HMQ hmq;
    HWND hwndFrame = (HWND)data;

    if(ThInit(hab, hmq))
        return;

/*----------------------------------------*/
    do
    {
        static char *sList = 0;
        ULONG uMaxLen;

        if(sList)
            delete sList;

        DosRequestMutexSem(hmtxSites, (ULONG) SEM_INDEFINITE_WAIT);

        uMaxLen = strlen(cConnSites);
        sList = new char[uMaxLen+1];
        sList[uMaxLen] = 0;

        if(uMaxLen)
            memcpy(sList, cConnSites, uMaxLen);

        DosReleaseMutexSem(hmtxSites);

        /**/
        /* Count list items */

        char *str;
        char *ptr;
        int nSites = 0;
        int iDelay;

        for(ptr = str = sList; *str;)
        {
            while(*str && *str != '\n')
                str++;

            if(*str)
            {
                if(*str == '\n')
                    *str++;
            }
            nSites++;
        }

        if(nSites <= 0)
        {
            Sleep(iConnInterval * 1000);
            continue;
        }

        iDelay = (iConnInterval * 1000)/nSites;

        if(iDelay < 32)
            iDelay = 32; //Minimal interval is 32ms

        for(ptr = str = sList; *str;)
        {
            int Rate;

            while(*str && *str != '\n')
                str++;

            if(*str)
            {
                if(*str == '\n')
                    *str++ = 0;
            }
            //Ping one site with 5 packets and check results.
            //If response rate = 0 then connection is lost

            Rate = PingSite(ptr, 2, 56);

            if(Rate == 0)
            {
                //Warn user
                if(iConnBeep)
                {
                    Play(ConnDown);
                }

                //Run program
                if(iConnRunProg)
                {
                    char parms[2262];
                    char *pPtr;
                    char *pStr;


                    for(pStr = parms, pPtr = cConnProgParm; *pPtr; pPtr++)
                    {
                        if(*pPtr == '%')
                        {
                            pPtr++;
                            switch(*pPtr)
                            {
                                case 's':
                                case 'S':
                                    {
                                        char *pTmp = ptr;
                                        while(*pTmp)
                                            *pStr++ = *pTmp++;
                                    }
                                    continue;

                                default:
                                    *pStr++ = '%';
                                    break;
                            }
                        }
                        *pStr++ = *pPtr;
                    }
                    *pStr = 0;

                    RunProg(cConnRunProg, parms);
                }
            }

            //Go to next site
            Sleep(iDelay);
            ptr = str;
        }

        /**/
        //delete sList;
    }
    while(ForEver);
/*----------------------------------------*/
    ThTerm(hab, hmq);
}

/*--------------------------------------------------*/

APIRET GetUptime(PUptime pData)
{
    ULONG ulSeconds = 0;

    APIRET rc;

    if(iRAMMethod)
        rc = GetUptimeSeconds(&ulSeconds); //HiRes
    else
        rc = GetUptimeSeconds2(&ulSeconds); //Standard

    if(rc)
        return rc;

    pData->uSeconds = ulSeconds % 60;
    pData->uMins    = (ulSeconds/60)%60;
    pData->uHours   = (ulSeconds/3600) % 24;
    pData->uDays    = (ulSeconds/3600) / 24;

    return 0;
}

/*--------------------------------------------------*/

APIRET GetFreeMem(ULONG * pMem)
{
    if(libWin32kInstalled())
    {
        K32SYSTEMMEMINFO MemInfo;
        int rc;

        MemInfo.cb = sizeof(MemInfo);
        MemInfo.flFlags = K32_SYSMEMINFO_PAGING;

        rc = W32kQuerySystemMemInfo(&MemInfo);

        if (!rc)
        {
            *pMem = MemInfo.cbPhysAvail;
            return 0;
        }
    }
    return Dos16MemAvail(pMem);
}

APIRET GetCPULoad(int *load, int *kernel, int CPU)
{
    static double  ts_val,   ts_val_prev[CPUS]  ={0.,0.,0.,0.};
    static double  idle_val, idle_val_prev[CPUS]={0.,0.,0.,0.};
    static double  busy_val, busy_val_prev[CPUS]={0.,0.,0.,0.};
    static double  intr_val, intr_val_prev[CPUS]={0.,0.,0.,0.};
    static CPUUTIL CPUUtil[CPUS];
    static int     CPULoad[CPUS];
	static int     CPUKernelLoad[CPUS];
    static int iter = 0;
    APIRET rc;

    if(CPU > 0) //Just return value, needless to measure it again
    {
        *load = (CPU >= 0 && CPU < CPUS) ? CPULoad[CPU]:0;
        *kernel = (CPU >= 0 && CPU < CPUS) ? CPUKernelLoad[CPU]:0;
        return 0;
    }

    do
    {
        rc = DosPerfSysCall(CMD_KI_RDCNT,(ULONG) &CPUUtil[0],0,0);
        if (rc)
            return rc;

        for(int i=0; i < CPUS; i++)
        {
            ts_val   = LL2F(CPUUtil[i].ulTimeHigh, CPUUtil[i].ulTimeLow);
            idle_val = LL2F(CPUUtil[i].ulIdleHigh, CPUUtil[i].ulIdleLow);
            busy_val = LL2F(CPUUtil[i].ulBusyHigh, CPUUtil[i].ulBusyLow);
            intr_val = LL2F(CPUUtil[i].ulIntrHigh, CPUUtil[i].ulIntrLow);

            if(!ts_val)
                continue;

            if(iter > 0)
            {
                double ts_delta = ts_val - ts_val_prev[i];
                double usage = ((busy_val - busy_val_prev[i])/ts_delta)*100.0;
                double kusage = ((intr_val - intr_val_prev[i])/ts_delta)*100.0;

                if(usage > 99.0)
                    usage = 100;
                if(kusage > usage)
                    usage = kusage;

                CPULoad[i] = (int)usage;
                CPUKernelLoad[i] = (int)kusage;
            }

            ts_val_prev[i]   = ts_val;
            idle_val_prev[i] = idle_val;
            busy_val_prev[i] = busy_val;
            intr_val_prev[i] = intr_val;
        }
        if(!iter)
            Sleep(32);

        if(iter < 3)
            iter++;
    }
    while(iter < 2);

    *load = (CPU >= 0 && CPU < CPUS) ? CPULoad[CPU]:0;
    *kernel = (CPU >= 0 && CPU < CPUS) ? CPUKernelLoad[CPU]:0;
    return 0;
}

/*--------------------------------------------------*/

APIRET GetUptimeSeconds(PULONG pulSeconds)
{
    QWORD qwTmrTime;
    APIRET rc;
    ULONG ulSeconds = 0;
    static ULONG ulTmrFreq = 0;

    if(!ulTmrFreq)
    {
        rc = DosTmrQueryFreq(&ulTmrFreq);
        if(rc)
        {
            ULONG ulMsCount;
            rc = DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, (PVOID)&ulMsCount, sizeof(ULONG));

            if(rc)
                return rc;

            ulSeconds = ulMsCount/1000;
        }
    }
    if(!ulSeconds)
    {
        rc = DosTmrQueryTime(&qwTmrTime);

        if(rc)
            return rc;

        ulSeconds  = LL2F(qwTmrTime.ulHi, qwTmrTime.ulLo) / ((double)ulTmrFreq);
    }
    *pulSeconds = ulSeconds;
    return 0;
}

APIRET GetUptimeSeconds2(PULONG pulSeconds)
{
    APIRET rc;
    ULONG ulSeconds = 0;
    ULONG ulMsCount;
    rc = DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, (PVOID)&ulMsCount, sizeof(ULONG));

    if(rc)
        return rc;

    ulSeconds = ulMsCount/1000;
    *pulSeconds = ulSeconds;
    return 0;
}

