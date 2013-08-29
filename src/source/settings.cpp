/*
** Module   :SETTINGS.C
** Abstract :Settings notebook implementation for 'PU Monitor 2.0'
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sat  16/05/1998 Created
**      Thu  25/06/1998 Updated
**      Wed  15/07/1998 Updated
*/

#define INCL_WIN
#define INCL_NLS
#define INCL_DOS
#define INCL_GPI
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <settings.h>
#include <id.h>
#include <ipstat.h>
#include <stats.h>
#include <vars.h>
#include <cell.h>

MRESULT EXPENTRY DlgProc_Mail(int, HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY DlgProc_Mail1(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY DlgProc_Mail2(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY DlgProc_Mail3(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY DlgProc_Mail4(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY DlgProc_Mail5(HWND, ULONG, MPARAM, MPARAM);

MRESULT EXPENTRY DlgProc_CPU(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY DlgProc_TCP(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY DlgProc_Ping(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY DlgProc_Memory(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY DlgProc_About(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY DlgProc_General(HWND, ULONG, MPARAM, MPARAM);

MRESULT EXPENTRY ButtProc(HWND, ULONG, MPARAM, MPARAM);

typedef MRESULT EXPENTRY ADlgProc(HWND,ULONG,MPARAM,MPARAM);
typedef ADlgProc *PDlgProc;

struct NBDef
{
    char *name;
    char *sname;
    int  id;
    PDlgProc proc;
};

struct NBDef nbSettings[]=
{
    {"General"       , "General",ID_P_7FORM, DlgProc_General},
    {"CPU Meter"     , "CPU"    ,ID_P_2FORM, DlgProc_CPU	},
    {"Memory Meter"  , "Memory" ,ID_P_5FORM, DlgProc_Memory	},
    {"TCP/IP traffic", "TCP/IP" ,ID_P_3FORM, DlgProc_TCP	},
    {"Mail Checker 1", "Mail 1" ,ID_P_1FORM, DlgProc_Mail1	},
    {"Mail Checker 2", "Mail 2" ,ID_P_1FORM, DlgProc_Mail2  },
    {"Mail Checker 3", "Mail 3" ,ID_P_1FORM, DlgProc_Mail3  },
    {"Mail Checker 4", "Mail 4" ,ID_P_1FORM, DlgProc_Mail4  },
    {"Mail Checker 5", "Mail 5" ,ID_P_1FORM, DlgProc_Mail5  },
    {"Connectivity"  , "Ping"   ,ID_P_4FORM, DlgProc_Ping   },
    {"About"         , "About"  ,ID_P_6FORM, DlgProc_About  },
    {0,0,0}
};

static char* cSites = 0;

char *cGraphText[]=
{
    "None",
    "Bar",
    "Graph"
};

char *cTextText[]=
{
    "None",
    "In/Out",
    "In/Out/Totals"
};

struct stColorSetup
{
    char *name;
    int  *value;
};

struct stColorSetup ColorSetup[]=
{
    { "Text      ", &iPal0},
    { "Background", &iPal1},
    { "Bar graph ", &iPal2},
    { "Graph 1   ", &iPal3},
    { "Graph 2   ", &iPal4},
    { "Grid      ", &iPal5},
    { 0, 0}
};

extern HWND hwndPane1;
extern HWND hwndPane2;
extern HWND hwndPane3;
extern HWND hwndPane4;

//-----------------------------------------
// Code
//-----------------------------------------

void Apply(HWND hwndNb)
{
    ULONG ulPageID;
    char *pTmp;

    ulPageID = (ULONG)WinSendMsg(hwndNb,
                                 BKM_QUERYPAGEID,
                                 0,
                                 MPFROM2SHORT(BKA_FIRST,0));
    while(ulPageID)
    {
        HWND hwndPage = (HWND)WinSendMsg(hwndNb,
                                         BKM_QUERYPAGEWINDOWHWND,
                                         MPFROMLONG(ulPageID),
                                         0);

        WinSendMsg(hwndPage, WM_COMMAND, MPFROMSHORT(ID_B_APPLY), 0);

        ulPageID = (ULONG)WinSendMsg(hwndNb,
                                     BKM_QUERYPAGEID,
                                     MPFROMLONG(ulPageID),
                                     MPFROM2SHORT(BKA_NEXT,0));
    }

    DosRequestMutexSem(hmtxSites, (ULONG) SEM_INDEFINITE_WAIT);

    pTmp = cConnSites;

    ULONG uMaxLen = strlen(cSites);

    cConnSites  = new char[uMaxLen+1];
    cConnSites[uMaxLen] = 0;

    if(uMaxLen)
        memcpy(cConnSites, cSites, uMaxLen);

    DosReleaseMutexSem(hmtxSites);

    if(pTmp)
        delete pTmp;

    int iMEnabled = 0;

    for(int i = 0; i < 5; i++)
        iMEnabled += iMailEnabled[i];

    ShowCell(hwndMainFrame, ID_MAIL, iMEnabled);
    ShowCell(hwndMainFrame, ID_TCP , iIPGraphEnabled + iIPTotals);

//Refresh windows
    WinInvalidateRect(hwndMainFrame, NULL, TRUE);
}

PDlgProc pButton = 0;

MRESULT EXPENTRY ButtProc(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    HWND hwndFrame = WinQueryWindow(hwndDlg, QW_PARENT);

    switch (msg)
    {
        case WM_PRESPARAMCHANGED:

            LONG lMode = (LONG)(mp1);
            ULONG AttrFound;
            ULONG cbRetLen;
            LONG  AttrValue[1];

            if(lMode != PP_BACKGROUNDCOLOR &&
               lMode != PP_BACKGROUNDCOLORINDEX &&
               lMode != PP_FOREGROUNDCOLOR &&
               lMode != PP_FOREGROUNDCOLORINDEX)
            {
                break;
            }

            int iOp = (lMode == PP_FOREGROUNDCOLOR ||
                       lMode == PP_FOREGROUNDCOLORINDEX) ?
                        PP_FOREGROUNDCOLOR : PP_BACKGROUNDCOLOR;

            cbRetLen = WinQueryPresParam(hwndDlg,
                                         iOp,
                                         0,
                                         &AttrFound,
                                         sizeof(AttrValue),
                                         &AttrValue[0],
                                         QPF_PURERGBCOLOR  |
                                         QPF_NOINHERIT     |
                                         0);
            if(!cbRetLen)
                break;

            LONG iNdx = (LONG) WinSendDlgItemMsg(hwndFrame, 1000,
                                                 LM_QUERYSELECTION,
                                                 MPFROMLONG(LIT_FIRST),
                                                 MPFROMLONG(0));
            if (iNdx != LIT_NONE)
            {
                //Everything seems OK, but palette should be
                //set to user defined, because this palette
                //is actual holder the color

                iCurrPalette = ID_MENU_CLR_USR - ID_MENU_CLR_0;

                WinCheckButton(hwndFrame, 1020 + iCurrPalette, TRUE);

                WinSendDlgItemMsg(hwndFrame, 1000,
                                  LM_SETITEMHANDLE,
                                  MPFROMLONG(iNdx),
                                  MPFROMLONG(AttrValue[0]));

                for(iNdx = 1000; iNdx <= 1009; iNdx++)
                    WinEnableControl(hwndFrame, iNdx, TRUE);

                //Set spin buttons to appropriate values
                for(iNdx = 0; iNdx < 3; iNdx++)
                {
                    int iValue = (AttrValue[0] >> (iNdx * 8)) & 0xFF;

                    WinSendDlgItemMsg(hwndFrame, 1001+iNdx,
                                      SPBM_SETCURRENTVALUE,
                                      MPFROMLONG(iValue),
                                      0);
                }

                WinInvalidateRect(hwndDlg, NULL, TRUE);
            }
            break;
    }

    if(pButton)
        return pButton(hwndDlg, msg, mp1, mp2);

    return (MRESULT) FALSE;
}

//-----------------------------------------
// Common behavior
//-----------------------------------------

MRESULT EXPENTRY S_1DlgProc(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg)
    {
        case WM_CLOSE:
            WinDismissDlg(hwndDlg, DID_CANCEL);
            break;

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case ID_B_APPLY :
                    Apply(WinWindowFromID(hwndDlg,1000));
                    break;

                case ID_B_OK    :
                    Apply(WinWindowFromID(hwndDlg,1000));
                    WinDismissDlg(hwndDlg, ID_B_OK);
                    break;

                case ID_B_CANCEL:
                    WinDismissDlg(hwndDlg, ID_B_CANCEL);
                    break;
            }
            break;

        case WM_CONTROL:
            break;

        default:
            return WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }
    return (MRESULT) FALSE;
}

void DoSettings(HWND hwndFrame, int iSetPage)
{
    HWND  hwndDlg;
    HWND  hNewFrame;
    ULONG uMaxLen;
    int i;
    ULONG Ver = 3;

    static int iInside = 0;

    if(iInside)
        return;

    iInside++;

    DosQuerySysInfo(QSV_VERSION_MINOR,
                    QSV_VERSION_MINOR,
                    (PVOID)&Ver, sizeof(ULONG));

    Ver /= 10;

    DosRequestMutexSem(hmtxSites, (ULONG) SEM_INDEFINITE_WAIT);

    uMaxLen = strlen(cConnSites);
    cSites  = new char[uMaxLen+1];
    cSites[uMaxLen] = 0;

    if(uMaxLen)
        memcpy(cSites, cConnSites, uMaxLen);

    DosReleaseMutexSem(hmtxSites);

    hwndDlg = WinLoadDlg(HWND_DESKTOP, hwndFrame, S_1DlgProc, 0, ID_S_1FORM, 0);

    {
    	SWP swpCenter;
    	RECTL rclParent;

        WinQueryWindowPos(hwndDlg,&swpCenter);
        WinQueryWindowRect(hwndFrame, &rclParent);

    	swpCenter.x=(rclParent.xRight-swpCenter.cx)/2;
    	swpCenter.y=(rclParent.yTop-swpCenter.cy)/2;


        WinSetWindowPos(hwndDlg,0, swpCenter.x, swpCenter.y, 0, 0, SWP_MOVE);
    }

    WinSendDlgItemMsg(hwndDlg, 1000,
                      BKM_SETNOTEBOOKCOLORS,
                      MPFROMLONG(SYSCLR_DIALOGBACKGROUND),
                      MPFROMLONG(BKA_BACKGROUNDPAGECOLORINDEX));

    LONG lMaxWidth = 0;
    LONG lMaxHight = 0;
    LONG lFlg      = BKA_STATUSTEXTON | BKA_AUTOPAGESIZE | BKA_MAJOR;

    for (i = 0; nbSettings[i].id ; i++)
    {
        ULONG ulPageId;
        hNewFrame = WinLoadDlg(hwndDlg,
                               WinWindowFromID(hwndDlg, 1000),
                               nbSettings[i].proc,
                               0,
                               nbSettings[i].id,
                               0);

        if(Ver < 4)
        {
            HPS hpsTemp;
            LONG lWidth;
            LONG lHight;
            POINTL txtPointl[TXTBOX_COUNT];

            hpsTemp = WinGetPS(hNewFrame);

            GpiQueryTextBox(hpsTemp,
                            strlen(nbSettings[i].sname),
                            nbSettings[i].sname,
                            TXTBOX_COUNT,
                            txtPointl);

            WinReleasePS(hpsTemp);

            lWidth = txtPointl[TXTBOX_TOPRIGHT].x -
                     txtPointl[TXTBOX_TOPLEFT ].x;

            lHight = txtPointl[TXTBOX_TOPLEFT   ].y -
                     txtPointl[TXTBOX_BOTTOMLEFT].y;

            if(lWidth > lMaxWidth)
                lMaxWidth = lWidth;

            if(lHight > lMaxHight)
                lMaxHight = lHight;
        }

        ulPageId = (ULONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                             BKM_INSERTPAGE,
                                             MPFROMLONG(0),
                                             MPFROM2SHORT(lFlg, BKA_LAST));
        if (ulPageId)
        {
            WinSendDlgItemMsg(hwndDlg, 1000,
                              BKM_SETPAGEWINDOWHWND,
                              MPFROMLONG(ulPageId),
                              MPFROMHWND(hNewFrame));


            WinSendDlgItemMsg(hwndDlg, 1000,
                              BKM_SETTABTEXT,
                              MPFROMLONG(ulPageId),
                              MPFROMP((Ver < 4) ? nbSettings[i].sname :
                                                  nbSettings[i].name));
            if(i == iSetPage)
            {
                WinSendDlgItemMsg(hwndDlg, 1000,
                                  BKM_TURNTOPAGE,
                                  MPFROMLONG(ulPageId),
                                  0);

            }
        }
    }

    if(Ver < 4)
        WinSendDlgItemMsg(hwndDlg, 1000,
                          BKM_SETDIMENSIONS,
                          MPFROM2SHORT(lMaxWidth+10, lMaxHight+10),
                          MPFROMSHORT(BKA_MAJORTAB));


    WinProcessDlg(hwndDlg);
    WinDestroyWindow(hwndDlg);

    if(cSites)
    {
        delete cSites;
        cSites = 0;
    }

    StoreVars();

    iInside--;
}

MRESULT EXPENTRY DlgProc_Mail1(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    return DlgProc_Mail(0, hwndDlg, msg, mp1, mp2);
}
MRESULT EXPENTRY DlgProc_Mail2(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    return DlgProc_Mail(1, hwndDlg, msg, mp1, mp2);
}
MRESULT EXPENTRY DlgProc_Mail3(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    return DlgProc_Mail(2, hwndDlg, msg, mp1, mp2);
}
MRESULT EXPENTRY DlgProc_Mail4(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    return DlgProc_Mail(3, hwndDlg, msg, mp1, mp2);
}
MRESULT EXPENTRY DlgProc_Mail5(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    return DlgProc_Mail(4, hwndDlg, msg, mp1, mp2);
}

MRESULT EXPENTRY DlgProc_Mail(int iN, HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    HWND hwndFrame = hwndDlg;

    switch (msg)
    {
        case WM_INITDLG:
            {
                WinCheckButton(hwndDlg, 1002, iMailBeep[iN]);
                WinCheckButton(hwndDlg, 1004, iMailRun[iN]);
                WinCheckButton(hwndDlg, 1005, iMailEnabled[iN]);
                WinCheckButton(hwndDlg, 1011, iMailLoop[iN]);
                WinCheckButton(hwndDlg, 1030, iMailSep[iN]);
                WinSetDlgItemText(hwndDlg, 1000, cMailProgParm[iN]);
                WinSetDlgItemText(hwndDlg, 1006, cMailServ[iN]);
                WinSetDlgItemText(hwndDlg, 1007, cMailUser[iN]);
                WinSetDlgItemText(hwndDlg, 1008, cMailRun[iN]);
                WinSetDlgItemText(hwndDlg, 1013, cMailProg[iN]);
                WinSetDlgItemText(hwndDlg, 1016, cMailPass[iN]);
                WinSetDlgItemText(hwndDlg, 1021, cMailPort[iN]);

                WinSendDlgItemMsg(hwndDlg, 1003,
                                  SPBM_SETLIMITS,
                                  MPFROMLONG(999),
                                  MPFROMLONG(10));

                WinSendDlgItemMsg(hwndDlg, 1003,
                                  SPBM_SETCURRENTVALUE,
                                  MPFROMLONG(iMailInterval[iN]),
                                  0);

                WinEnableControl(hwndDlg, 1000, iMailEnabled[iN]);
                WinEnableControl(hwndDlg, 1002, iMailEnabled[iN]);
                WinEnableControl(hwndDlg, 1003, iMailEnabled[iN]);
                WinEnableControl(hwndDlg, 1003, iMailEnabled[iN]);
                WinEnableControl(hwndDlg, 1004, iMailEnabled[iN]);
                WinEnableControl(hwndDlg, 1006, iMailEnabled[iN]);
                WinEnableControl(hwndDlg, 1007, iMailEnabled[iN]);
                WinEnableControl(hwndDlg, 1008, iMailEnabled[iN]);
                WinEnableControl(hwndDlg, 1011, iMailEnabled[iN]);
                WinEnableControl(hwndDlg, 1012, iMailEnabled[iN]);
                WinEnableControl(hwndDlg, 1013, iMailEnabled[iN]);
                WinEnableControl(hwndDlg, 1014, iMailEnabled[iN]);
                WinEnableControl(hwndDlg, 1016, iMailEnabled[iN]);
                WinEnableControl(hwndDlg, 1021, iMailEnabled[iN]);
                WinEnableControl(hwndDlg, 1008,
                                (iMailRun[iN] && iMailEnabled[iN]) ? TRUE : FALSE);
                WinEnableControl(hwndDlg, 1012,
                            	(iMailRun[iN] && iMailEnabled[iN]) ? TRUE : FALSE);
            }
            break;

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case ID_B_APPLY :
                    {
                        iMailBeep[iN]    = (ULONG)WinSendDlgItemMsg(hwndDlg, 1002, BM_QUERYCHECK, 0, 0);
                        iMailRun[iN]     = (ULONG)WinSendDlgItemMsg(hwndDlg, 1004, BM_QUERYCHECK, 0, 0);
                        iMailEnabled[iN] = (ULONG)WinSendDlgItemMsg(hwndDlg, 1005, BM_QUERYCHECK, 0, 0);
                        iMailLoop[iN]    = (ULONG)WinSendDlgItemMsg(hwndDlg, 1011, BM_QUERYCHECK, 0, 0);
                        iMailSep[iN]     = (ULONG)WinSendDlgItemMsg(hwndDlg, 1030, BM_QUERYCHECK, 0, 0);
                        WinQueryDlgItemText(hwndDlg, 1000, 261, cMailProgParm[iN]);
                        WinQueryDlgItemText(hwndDlg, 1006, 261, cMailServ[iN]);
                        WinQueryDlgItemText(hwndDlg, 1007, 261, cMailUser[iN]);
                        WinQueryDlgItemText(hwndDlg, 1008, 261, cMailRun[iN]);
                        WinQueryDlgItemText(hwndDlg, 1013, 261, cMailProg[iN]);
                        WinQueryDlgItemText(hwndDlg, 1016, 261, cMailPass[iN]);
                        WinQueryDlgItemText(hwndDlg, 1021, 261, cMailPort[iN]);

                        WinSendDlgItemMsg(hwndDlg, 1003,
                                          SPBM_QUERYVALUE,
                                          MPFROMP(&iMailInterval[iN]),
                                          MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));
                    }
                    break;

                case 1012:
                    GetAndSetFile(hwndDlg, 1008, "File to execute", "*.exe");
                    break;

                case 1014:
                    GetAndSetFile(hwndDlg, 1013, "File to execute", "*.exe");
                    break;
            }
            break;

        case WM_CONTROL:
            switch (SHORT1FROMMP(mp1))
            {
                case 1005:
                    if(SHORT2FROMMP(mp1) == BN_CLICKED)
                    {
                        int iTmp;
                        int iTmp2;

                        iTmp  = (ULONG)WinSendDlgItemMsg(hwndDlg, 1005, BM_QUERYCHECK, 0, 0);
                        iTmp2 = (ULONG)WinSendDlgItemMsg(hwndDlg, 1004, BM_QUERYCHECK, 0, 0);

                        WinEnableControl(hwndDlg, 1000, iTmp);
                        WinEnableControl(hwndDlg, 1002, iTmp);
                        WinEnableControl(hwndDlg, 1003, iTmp);
                        WinEnableControl(hwndDlg, 1003, iTmp);
                        WinEnableControl(hwndDlg, 1004, iTmp);
                        WinEnableControl(hwndDlg, 1006, iTmp);
                        WinEnableControl(hwndDlg, 1007, iTmp);
                        WinEnableControl(hwndDlg, 1008, iTmp);
                        WinEnableControl(hwndDlg, 1011, iTmp);
                        WinEnableControl(hwndDlg, 1012, iTmp);
                        WinEnableControl(hwndDlg, 1013, iTmp);
                        WinEnableControl(hwndDlg, 1014, iTmp);
                        WinEnableControl(hwndDlg, 1016, iTmp);
                        WinEnableControl(hwndDlg, 1021, iTmp);
                        WinEnableControl(hwndDlg, 1008, (iTmp2 && iTmp) ? TRUE : FALSE);
                        WinEnableControl(hwndDlg, 1012, (iTmp2 && iTmp) ? TRUE : FALSE);
                    }
                    break;

                case 1004:
                    if(SHORT2FROMMP(mp1) == BN_CLICKED)
                    {
                        int iTmp2;
                        iTmp2 = (ULONG)WinSendDlgItemMsg(hwndDlg, 1004,
                                                         BM_QUERYCHECK, 0, 0);

                        WinEnableControl(hwndDlg, 1008, iTmp2);
                        WinEnableControl(hwndDlg, 1012, iTmp2);
                    }
                    break;
            }
            break;

        default:
            return WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }
    return (MRESULT) FALSE;
}

MRESULT EXPENTRY DlgProc_CPU(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    HWND hwndFrame = hwndDlg;

    switch (msg)
    {
        case WM_INITDLG:
            {
                WinSendDlgItemMsg(hwndDlg, 1100,
                                  SPBM_SETLIMITS,
                                  MPFROMLONG(100),
                                  MPFROMLONG(1));

                WinSendDlgItemMsg(hwndDlg, 1100,
                                  SPBM_SETCURRENTVALUE,
                                  MPFROMLONG(iCPUVar),
                                  0);


                WinCheckButton(hwndDlg, 1002, iCPUGraph);
                WinCheckButton(hwndDlg, 1021, iCPUKernel);
                WinSetDlgItemText(hwndDlg, 1002, cGraphText[iCPUGraph]);
                WinCheckButton(hwndDlg, 1005, iCPUText);

                WinSendDlgItemMsg(hwndDlg, 1003,
                                  SPBM_SETLIMITS,
                                  MPFROMLONG(10),
                                  MPFROMLONG(1));

                WinSendDlgItemMsg(hwndDlg, 1003,
                                  SPBM_SETCURRENTVALUE,
                                  MPFROMLONG(iCPUInterval),
                                  0);
            }
            break;

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case ID_B_APPLY :
                    {
                        int iData;

                        WinSendDlgItemMsg(hwndDlg, 1003,
                                          SPBM_QUERYVALUE,
                                          MPFROMP(&iCPUInterval),
                                          MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));

                        iData     = (ULONG)WinSendDlgItemMsg(hwndDlg, 1002, BM_QUERYCHECK, 0, 0);
                        if(iData != iCPUGraph)
                            iCValuesFilled = 0;

                        iCPUGraph = iData;

                        iCPUKernel = (ULONG)WinSendDlgItemMsg(hwndDlg, 1021, BM_QUERYCHECK, 0, 0);
                        iCPUText  = (ULONG)WinSendDlgItemMsg(hwndDlg, 1005, BM_QUERYCHECK, 0, 0);

                        WinSendDlgItemMsg(hwndDlg, 1100,
                                          SPBM_QUERYVALUE,
                                          MPFROMP(&iCPUVar),
                                          MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));

                    }
                    break;
            }
        case WM_CONTROL:
            {
                if((SHORT2FROMMP(mp1) == BN_CLICKED ||
                    SHORT2FROMMP(mp1) == BN_DBLCLICKED) &&
                    SHORT1FROMMP(mp1) == 1002)
                {
                    int iData1;
                    iData1 = (ULONG)WinSendDlgItemMsg(hwndDlg, 1002, BM_QUERYCHECK, 0, 0);
                    WinSetDlgItemText(hwndDlg, 1002, cGraphText[iData1]);
                }
            }
            break;

        default:
            return WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }
    return (MRESULT) FALSE;
}

MRESULT EXPENTRY DlgProc_TCP(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    HWND hwndFrame = hwndDlg;

    switch (msg)
    {
        case WM_INITDLG:
            {
                WinSendDlgItemMsg(hwndDlg, 1100,
                                  SPBM_SETLIMITS,
                                  MPFROMLONG(100),
                                  MPFROMLONG(1));

                WinSendDlgItemMsg(hwndDlg, 1100,
                                  SPBM_SETCURRENTVALUE,
                                  MPFROMLONG(iIPVar),
                                  0);

                WinSendDlgItemMsg(hwndDlg, 1003,
                                  SPBM_SETLIMITS,
                                  MPFROMLONG(100),
                                  MPFROMLONG(1));

                WinSendDlgItemMsg(hwndDlg, 1003,
                                  SPBM_SETCURRENTVALUE,
                                  MPFROMLONG(iIPInterval),
                                  0);

                WinCheckButton(hwndDlg, 1002, iIPGraphEnabled);
                WinSetDlgItemText(hwndDlg, 1002, cGraphText[iIPGraphEnabled]);
                WinCheckButton(hwndDlg, 1005, iIPTotals);
                WinSetDlgItemText(hwndDlg, 1005, cTextText[iIPTotals]);
                WinCheckButton(hwndDlg, 1006, iIPAdaptive);
                WinEnableControl(hwndDlg, 1006, (iIPGraphEnabled == 1) ? TRUE:FALSE);

                extern IPStatsMonitor ipStat;
                ipStat.ListItems(WinWindowFromID(hwndDlg,1000));
            }
            break;

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case ID_B_APPLY :
                    {
                        int iNdx = (LONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                                            LM_QUERYSELECTION,
                                                            MPFROMLONG(LIT_FIRST),
                                                            MPFROMLONG(0));

                        if (iNdx != LIT_NONE)
                        {
                            int iData;
                            iData = (ULONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                                           LM_QUERYITEMHANDLE,
                                                           MPFROMLONG(iNdx),
                                                           0);
                            if(iData != iIPCurrent)
                                iValuesFilled = 0;

                            iIPCurrent = iData;
                        }

                        WinSendDlgItemMsg(hwndDlg, 1003,
                                          SPBM_QUERYVALUE,
                                          MPFROMP(&iIPInterval),
                                          MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));

                        WinSendDlgItemMsg(hwndDlg, 1100,
                                          SPBM_QUERYVALUE,
                                          MPFROMP(&iIPVar),
                                          MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));

                        iIPGraphEnabled = (ULONG)WinSendDlgItemMsg(hwndDlg, 1002, BM_QUERYCHECK, 0, 0);
                        iIPTotals       = (ULONG)WinSendDlgItemMsg(hwndDlg, 1005, BM_QUERYCHECK, 0, 0);
                        iIPAdaptive     = (ULONG)WinSendDlgItemMsg(hwndDlg, 1006, BM_QUERYCHECK, 0, 0);
                    }
                    break;
            }
        case WM_CONTROL:
            {
                if((SHORT2FROMMP(mp1) == BN_CLICKED ||
                    SHORT2FROMMP(mp1) == BN_DBLCLICKED) &&
                   (SHORT1FROMMP(mp1) == 1002 ||
                    SHORT1FROMMP(mp1) == 1005))
                {
                    int iData1;
                    int iData2;
                    iData1 = (ULONG)WinSendDlgItemMsg(hwndDlg, 1002, BM_QUERYCHECK, 0, 0);
                    iData2 = (ULONG)WinSendDlgItemMsg(hwndDlg, 1005, BM_QUERYCHECK, 0, 0);
                    WinSetDlgItemText(hwndDlg, 1002, cGraphText[iData1]);
                    WinSetDlgItemText(hwndDlg, 1005, cTextText[iData2]);

                    WinEnableControl(hwndDlg, 1006, (iData1 == 1) ? TRUE:FALSE);
                }
            }
            break;

        default:
            return WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }
    return (MRESULT) FALSE;
}

MRESULT EXPENTRY DlgProc_Ping(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    HWND hwndFrame = hwndDlg;

    switch (msg)
    {
        case WM_INITDLG:
            {
                WinCheckButton(hwndDlg,    1001, iConnRunProg);
                WinCheckButton(hwndDlg,    1002, iConnBeep);
                WinSetDlgItemText(hwndDlg, 1008, cConnRunProg);
                WinSetDlgItemText(hwndDlg, 1020, cConnProgParm);

                WinSendDlgItemMsg(hwndDlg, 1003,
                                  SPBM_SETLIMITS,
                                  MPFROMLONG(999),
                                  MPFROMLONG(1));

                WinSendDlgItemMsg(hwndDlg, 1003,
                                  SPBM_SETCURRENTVALUE,
                                  MPFROMLONG(iConnInterval),
                                  0);
                /* Setup List box */
                {
                    char *str;
                    char *ptr;

                    for(ptr = str = cSites; *str;)
                    {
                        while(*str && *str != '\n')
                            str++;

                        if(*str)
                        {
                            if(*str == '\n')
                                *str++ = 0;
                        }

                        WinSendDlgItemMsg(hwndDlg, 1000,
                                          LM_INSERTITEM,
                                          MPFROMSHORT(LIT_SORTASCENDING),
                                          MPFROMP(ptr));

                        ptr = str;
                    }
                }
                WinEnableControl(hwndDlg, 1005, FALSE);
                WinEnableControl(hwndDlg, 1006, FALSE);
                WinEnableControl(hwndDlg, 1008, iConnRunProg);
                WinEnableControl(hwndDlg, 1011, FALSE);
                WinEnableControl(hwndDlg, 1014, iConnRunProg);
                WinEnableControl(hwndDlg, 1020, iConnRunProg);

            }
            break;

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case ID_B_APPLY :
                    {
                        WinQueryDlgItemText(hwndDlg, 1008, 261, cConnRunProg);
                        WinQueryDlgItemText(hwndDlg, 1020, 261, cConnProgParm);

                        WinSendDlgItemMsg(hwndDlg, 1003,
                                          SPBM_QUERYVALUE,
                                          MPFROMP(&iConnInterval),
                                          MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));

                        iConnRunProg = (ULONG)WinSendDlgItemMsg(hwndDlg, 1001, BM_QUERYCHECK, 0, 0);
                        iConnBeep    = (ULONG)WinSendDlgItemMsg(hwndDlg, 1002, BM_QUERYCHECK, 0, 0);

                        /* Get list of sites */
                        int i;
                        int len = 0;
                        char *ptr;

                        delete cSites;
                        cSites = 0;

                        LONG sItemCount = (LONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                                                   LM_QUERYITEMCOUNT,
                                                                   0, 0);
                        for(i = 0; i < sItemCount; i++)
                        {
                            LONG sTextLen;
                            sTextLen = i;
                            sTextLen = (LONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                                                LM_QUERYITEMTEXTLENGTH,
                                                                MPFROMSHORT(sTextLen),
                                                                0);
                            len += (sTextLen + 1);
                        }

                        cSites = new char[len+1];
                        cSites[len] = 0;

                        for(i = 0, ptr = cSites; i < sItemCount; i++)
                        {
                            LONG sTextLen;
                            sTextLen = i;
                            sTextLen = (LONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                                                LM_QUERYITEMTEXTLENGTH,
                                                                MPFROMSHORT(sTextLen),
                                                                0);
                            sTextLen++;
                            WinSendDlgItemMsg(hwndDlg, 1000,
                                              LM_QUERYITEMTEXT,
                                              MPFROM2SHORT(i, sTextLen),
                                              MPFROMP(ptr));
                            ptr += (sTextLen - 1);
                            *ptr = '\n';
                            ptr++;
                        }
                    }
                    break;

                case 1005:
                    {
                        CHAR aTextValue[262];

                        WinQueryDlgItemText(hwndDlg, 1004, sizeof(aTextValue) - 1, aTextValue);
                        WinSendDlgItemMsg(hwndDlg, 1000,
                                          LM_INSERTITEM,
                                          MPFROMSHORT(LIT_SORTASCENDING),
                                          MPFROMP(aTextValue));

                        WinEnableControl(hwndDlg, 1005, FALSE);
                        WinEnableControl(hwndDlg, 1006, FALSE);
                        WinEnableControl(hwndDlg, 1011, FALSE);
                    }
                    break;

                case 1006:
                    {
                        int iNdx;

                        iNdx = (LONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                                        LM_QUERYSELECTION,
                                                        MPFROMLONG(LIT_FIRST),
                                                        MPFROMLONG(0));

                        if (iNdx != LIT_NONE)
                        {
                            WinSendDlgItemMsg(hwndDlg, 1000,
                                              LM_DELETEITEM,
                                              MPFROMLONG(iNdx),
                                              0);
                        }

                        WinEnableControl(hwndDlg, 1006, FALSE);
                        WinEnableControl(hwndDlg, 1011, FALSE);
                    }
                    break;

                case 1014:
                    GetAndSetFile(hwndDlg, 1008, "File to execute", "*.exe");
                    break;

                case 1011:
                    {
                        char aTextValue[262];
                        int iNdx;

                        iNdx = (LONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                                        LM_QUERYSELECTION,
                                                        MPFROMLONG(LIT_FIRST),
                                                        MPFROMLONG(0));

                        if (iNdx != LIT_NONE)
                        {
                            WinQueryDlgItemText(hwndDlg, 1004, 261, aTextValue);
                            WinSendDlgItemMsg(hwndDlg, 1000,
                                              LM_SETITEMTEXT,
                                              MPFROMLONG(iNdx),
                                              MPFROMP(aTextValue));
                        }
                        WinEnableControl(hwndDlg, 1005, FALSE);
                        WinEnableControl(hwndDlg, 1011, FALSE);
                    }
                    break;
            }
            break;

        case WM_CONTROL:
            switch (SHORT1FROMMP(mp1))
            {
                case 1000:
                    if(SHORT2FROMMP(mp1) == LN_SELECT)
                    {
                        int iNdx;

                        WinEnableControl(hwndDlg, 1006, TRUE);

                        iNdx = (LONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                                        LM_QUERYSELECTION,
                                                        MPFROMLONG(LIT_FIRST),
                                                        MPFROMLONG(0));

                        if (iNdx != LIT_NONE)
                        {
                            char aTextValue[262];

                            WinSendDlgItemMsg(hwndDlg, 1000,
                                              LM_QUERYITEMTEXT,
                                              MPFROM2SHORT(iNdx, 261),
                                              MPFROMP(aTextValue));
                            WinSetDlgItemText(hwndDlg, 1004, aTextValue);
                            WinEnableControl(hwndDlg, 1005, FALSE);
                            WinEnableControl(hwndDlg, 1011, FALSE);
                        }
                    }
                    break;

                case 1001:
                    if(SHORT2FROMMP(mp1) == BN_CLICKED)
                    {
                        int iTmp = (ULONG)WinSendDlgItemMsg(hwndDlg, 1001, BM_QUERYCHECK, 0, 0);

                        WinEnableControl(hwndDlg, 1008, iTmp);
                        WinEnableControl(hwndDlg, 1014, iTmp);
                        WinEnableControl(hwndDlg, 1020, iTmp);
                    }
                    break;

                case 1004:
                    if(SHORT2FROMMP(mp1) == EN_CHANGE)
                    {
                        int iNdx;
                        char aTextValue[262];

                        WinQueryDlgItemText(hwndDlg, 1004, 261, aTextValue);

                        for (iNdx = 0; aTextValue[iNdx]; iNdx++)
                        {
                            if (aTextValue[iNdx] > ' ')
                            {
                                WinEnableControl(hwndDlg, 1005, TRUE);
                                break;
                            }
                        }

                        if (!aTextValue[iNdx])
                            WinEnableControl(hwndDlg, 1005, FALSE);


                        iNdx = (LONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                                        LM_QUERYSELECTION,
                                                        MPFROMLONG(LIT_FIRST),
                                                        MPFROMLONG(0));

                        if (iNdx != LIT_NONE)
                            WinEnableControl(hwndDlg, 1011, TRUE);
                    }
                    break;
            }
            break;

        default:
            return WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }
    return (MRESULT) FALSE;
}

MRESULT EXPENTRY DlgProc_Memory(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    HWND hwndFrame = hwndDlg;

    switch (msg)
    {
        case WM_INITDLG:
            {
                WinSendDlgItemMsg(hwndDlg, 1003,
                                  SPBM_SETLIMITS,
                                  MPFROMLONG(100),
                                  MPFROMLONG(1));

                WinSendDlgItemMsg(hwndDlg, 1003,
                                  SPBM_SETCURRENTVALUE,
                                  MPFROMLONG(iRAMInterval),
                                  0);

                WinCheckButton(hwndDlg, 1000, iRAMUptime);
                WinCheckButton(hwndDlg, 1005, iRAMClock);
                WinCheckButton(hwndDlg, 1006, iRAMText);
                WinCheckButton(hwndDlg, 1007, iRAMAdaptive);

                if(iRAMMethod)
                    WinCheckButton(hwndDlg, 1009, 1);
                else
                    WinCheckButton(hwndDlg, 1008, 1);
            }
            break;

        case WM_CONTROL:
            break;

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case ID_B_APPLY :
                    {
                        WinSendDlgItemMsg(hwndDlg, 1003,
                                          SPBM_QUERYVALUE,
                                          MPFROMP(&iRAMInterval),
                                          MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));

                        iRAMUptime   = (ULONG)WinSendDlgItemMsg(hwndDlg, 1000, BM_QUERYCHECK, 0, 0);
                        iRAMClock    = (ULONG)WinSendDlgItemMsg(hwndDlg, 1005, BM_QUERYCHECK, 0, 0);
                        iRAMText     = (ULONG)WinSendDlgItemMsg(hwndDlg, 1006, BM_QUERYCHECK, 0, 0);
                        iRAMAdaptive = (ULONG)WinSendDlgItemMsg(hwndDlg, 1007, BM_QUERYCHECK, 0, 0);
                        iRAMMethod   = (ULONG)WinSendDlgItemMsg(hwndDlg, 1009, BM_QUERYCHECK, 0, 0);
                    }
                    break;
            }
            break;

        default:
            return WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }
    return (MRESULT) FALSE;
}

MRESULT EXPENTRY DlgProc_About(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    HWND hwndFrame = hwndDlg;

    switch (msg)
    {
        case WM_COMMAND:
            break;

        case WM_INITDLG:
            break;

        default:
            return WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }
    return (MRESULT) FALSE;
}

MRESULT EXPENTRY DlgProc_General(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    HWND hwndFrame = hwndDlg;

    switch (msg)
    {
        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case ID_B_APPLY :
                    {
                        //Color group
                        LONG sItemCount = (LONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                                                   LM_QUERYITEMCOUNT,
                                                                   0, 0);
                        for(int i = 0; i < sItemCount; i++)
                        {
                            int iData;
                            iData = (LONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                                             LM_QUERYITEMHANDLE,
                                                             MPFROMSHORT(i),
                                                             0);
                            *ColorSetup[i].value = iData;
                        }

                        //Rest of the group
                        iFloat      = (ULONG)WinSendDlgItemMsg(hwndDlg, 1010, BM_QUERYCHECK, 0, 0);
                        iAttach     = (ULONG)WinSendDlgItemMsg(hwndDlg, 1011, BM_QUERYCHECK, 0, 0);
                        iLock       = (ULONG)WinSendDlgItemMsg(hwndDlg, 1012, BM_QUERYCHECK, 0, 0);
                        iDrawBorder = (ULONG)WinSendDlgItemMsg(hwndDlg, 1014, BM_QUERYCHECK, 0, 0);
                        i3DFont     = (ULONG)WinSendDlgItemMsg(hwndDlg, 1030, BM_QUERYCHECK, 0, 0);

                        WinSendDlgItemMsg(hwndDlg, 1016,
                                          SPBM_QUERYVALUE,
                                          MPFROMP(&iBtLeft),
                                          MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));
                        WinSendDlgItemMsg(hwndDlg, 1017,
                                          SPBM_QUERYVALUE,
                                          MPFROMP(&iBtRight),
                                          MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));


                        //Palette group
                        for(int iCtl = 1020; iCtl <= 1028; iCtl++)
                        {
                            if((ULONG)WinSendDlgItemMsg(hwndDlg, iCtl, BM_QUERYCHECK, 0, 0) == 1)
                            {
                                SetPalette(iCtl - 1020);
                                break;
                            }
                        }
                    }
                    break;
            }
            break;

        case WM_CONTROL:
            switch (SHORT1FROMMP(mp1))
            {
                case 1001:
                case 1002:
                case 1003:
                    if(SHORT2FROMMP(mp1) == SPBN_CHANGE)
                    {
                        int iValue;
                        WinSendDlgItemMsg(hwndDlg, SHORT1FROMMP(mp1),
                                          SPBM_QUERYVALUE,
                                          MPFROMP(&iValue),
                                          MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));

                        LONG iNdx = (LONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                                        LM_QUERYSELECTION,
                                                        MPFROMLONG(LIT_FIRST),
                                                        MPFROMLONG(0));
                        if (iNdx != LIT_NONE)
                        {
                            int iData;
                            iData = (ULONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                                           LM_QUERYITEMHANDLE,
                                                           MPFROMLONG(iNdx),
                                                           0);
                            iValue <<= (SHORT1FROMMP(mp1) - 1001) * 8;
                            iData &= ~(0xFF << (SHORT1FROMMP(mp1) - 1001) * 8);
                            iData |= iValue;

                            WinSendDlgItemMsg(hwndDlg, 1000,
                                              LM_SETITEMHANDLE,
                                              MPFROMLONG(iNdx),
                                              MPFROMLONG(iData));

                            WinInvalidateRect(WinWindowFromID(hwndDlg, 1004),
                                              NULL, TRUE);
                        }
                    }
                    break;

                case 1000:
                    if(SHORT2FROMMP(mp1) == LN_SELECT)
                    {
                        LONG iNdx = (LONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                                             LM_QUERYSELECTION,
                                                             MPFROMLONG(LIT_FIRST),
                                                             MPFROMLONG(0));

                        if (iNdx != LIT_NONE)
                        {
                            int iData;
                            iData = (ULONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                                              LM_QUERYITEMHANDLE,
                                                              MPFROMLONG(iNdx),
                                                              0);
                            for(int i = 0; i < 3; i++)
                            {
                                int iValue = (iData >> (i * 8)) & 0xFF;

                                WinSendDlgItemMsg(hwndDlg, 1001+i,
                                                  SPBM_SETCURRENTVALUE,
                                                  MPFROMLONG(iValue),
                                                  0);
                            }
                        }

                        WinInvalidateRect(WinWindowFromID(hwndDlg, 1004),
                                          NULL, TRUE);
                    }
                    break;

                case 1004:
                    if(SHORT2FROMMP(mp1) == BN_PAINT)
                    {
                        PUSERBUTTON pButton = (PUSERBUTTON)mp2;
                        RECTL rclPaint;
                        HPS hpsBuffer = pButton->hps;
                        LONG lColor = CLR_BLACK;

                        WinQueryWindowRect(pButton->hwnd,
                                           &rclPaint);

                        int iNdx;

                        iNdx = (LONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                                        LM_QUERYSELECTION,
                                                        MPFROMLONG(LIT_FIRST),
                                                        MPFROMLONG(0));

                        if (iNdx != LIT_NONE)
                        {
                            int iData;
                            iData = (ULONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                                           LM_QUERYITEMHANDLE,
                                                           MPFROMLONG(iNdx),
                                                           0);
                            LONG alTable[16];

                            GpiQueryLogColorTable(hpsBuffer, 0L, 0L, 16L, alTable);

                            alTable[1] = iData;
                            lColor = 1;

                            GpiCreateLogColorTable(hpsBuffer, 0L, LCOLF_CONSECRGB,
                                                   0L, 16, alTable);
                        }

                        WinFillRect(hpsBuffer, &rclPaint, lColor);
                    }
                    break;

                case 1011:
                    {
                        int iState = (ULONG)WinSendDlgItemMsg(hwndDlg,
                                                              SHORT1FROMMP(mp1),
                                                              BM_QUERYCHECK,
                                                              0, 0);

                        for(int iCtl = 1015; iCtl <= 1019; iCtl++)
                            WinEnableControl(hwndDlg, iCtl, iState);
                    }
                    break;

                case 1020:
                case 1021:
                case 1022:
                case 1023:
                case 1024:
                case 1025:
                case 1026:
                case 1027:
                case 1028:
                    {
                        int iState = (ULONG)WinSendDlgItemMsg(hwndDlg,
                                                              SHORT1FROMMP(mp1),
                                                              BM_QUERYCHECK,
                                                              0, 0);
                        if(!iState)
                            break;

                        for(int iCtl = 1000; iCtl <= 1009; iCtl++)
                            WinEnableControl(hwndDlg, iCtl,
                                             (SHORT1FROMMP(mp1) == 1028) ?
                                              TRUE:FALSE);
                    }
                    break;
            }
            break;

        case WM_INITDLG:
            {
                //Color group
                WinSendDlgItemMsg(hwndDlg, 1001, SPBM_SETLIMITS, MPFROMLONG(255), MPFROMLONG(0));
                WinSendDlgItemMsg(hwndDlg, 1002, SPBM_SETLIMITS, MPFROMLONG(255), MPFROMLONG(0));
                WinSendDlgItemMsg(hwndDlg, 1003, SPBM_SETLIMITS, MPFROMLONG(255), MPFROMLONG(0));

                for(int i = 0; ColorSetup[i].name; i++)
                {
                    int iNdx;

                    iNdx = (LONG) WinSendDlgItemMsg(hwndDlg, 1000,
                                                    LM_INSERTITEM,
                                                    MPFROMSHORT(LIT_END),
                                                    MPFROMP(ColorSetup[i].name));
                    WinSendDlgItemMsg(hwndDlg, 1000,
                                      LM_SETITEMHANDLE,
                                      MPFROMLONG(iNdx),
                                      MPFROMLONG(*ColorSetup[i].value));
                    if(!i)
                        WinSendDlgItemMsg(hwndDlg, 1000,
                                          LM_SELECTITEM,
                                          MPFROMLONG(iNdx),
                                          MPFROMLONG(TRUE));
                }
                //Behavior group
                WinCheckButton(hwndDlg, 1010, iFloat);
                WinCheckButton(hwndDlg, 1011, iAttach);
                WinCheckButton(hwndDlg, 1012, iLock);
                WinCheckButton(hwndDlg, 1014, iDrawBorder);
                WinCheckButton(hwndDlg, 1030, i3DFont);

                static PSZ pLeft[]={"None", "Shutdown", "Find/Shutdown","Lock/Find/Shutdown", "All"};
                static PSZ pRight[]={"None", "Tray List"};

                WinSendDlgItemMsg(hwndDlg, 1016, SPBM_SETARRAY, pLeft, MPFROMLONG(5));
                WinSendDlgItemMsg(hwndDlg, 1017, SPBM_SETARRAY, pRight, MPFROMLONG(2));
                WinSendDlgItemMsg(hwndDlg, 1016, SPBM_SETCURRENTVALUE, MPFROMLONG(iBtLeft), NULL);
                WinSendDlgItemMsg(hwndDlg, 1017, SPBM_SETCURRENTVALUE, MPFROMLONG(iBtRight), NULL);

                //Palette group
                WinCheckButton(hwndDlg, 1020 + iCurrPalette, TRUE);

                for(int iCtl = 1015; iCtl <= 1019; iCtl++)
                    WinEnableControl(hwndDlg, iCtl, iAttach);

                for(iCtl = 1000; iCtl <= 1009; iCtl++)
                    WinEnableControl(hwndDlg, iCtl,
                                     ((1020 + iCurrPalette) == 1028) ?
                                        TRUE:FALSE);

                pButton = WinSubclassWindow(WinWindowFromID(hwndDlg, 1004), ButtProc);
            }
            break;

        default:
            return WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }
    return (MRESULT) FALSE;
}

void GetAndSetFile(HWND hwndDlg, int iID, char *hdr, char *ext)
{
    FILEDLG FileDlg;
    HWND hwndFile;

    memset(&FileDlg, 0, sizeof(FILEDLG));
    FileDlg.cbSize = sizeof(FILEDLG);
    FileDlg.fl = FDS_HELPBUTTON | FDS_CENTER | FDS_OPEN_DIALOG;
    FileDlg.pszTitle = (!hdr) ? "" : hdr;
    FileDlg.szFullFile[0] = 0;

    WinQueryDlgItemText(hwndDlg, iID, sizeof(FileDlg.szFullFile), FileDlg.szFullFile);

    if (!FileDlg.szFullFile[0])
        strcpy(FileDlg.szFullFile, (!ext) ? "*.exe" : ext);

    hwndFile = WinFileDlg(HWND_DESKTOP, hwndDlg, &FileDlg);

    if (hwndFile && (FileDlg.lReturn == DID_OK))
    {
        WinSetDlgItemText(hwndDlg, iID, FileDlg.szFullFile);
    }
}

