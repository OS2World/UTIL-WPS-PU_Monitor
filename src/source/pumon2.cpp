/*
** Module   :PUMON2.CPP
** Abstract :Version 2.0
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Wed  24/06/1998 Created
**      Wed  15/07/1998 Updated
*/

#define INCL_WIN
#define INCL_NLS
#define INCL_DOS
#define INCL_GPI

#include <os2.h>
#include <stdio.h>
#include <cell.h>
#include <string.h>
#include <ipstat.h>
#include <util.h>
#include <id.h>
#include <settings.h>
#include <stats.h>
#include <vars.h>
#include <win32k.h>

#define USE_BIRD_FLOAT_ON_TOP 0

/* Local procedures */
void DrawGrid(HPS hpsBuffer, RECTL rclPaint,
              int *pValue1, int *pValue2, int iMax1, int iMax2,
              int iFill, int iStart);
MRESULT EXPENTRY MainClientProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ProcCPU (HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY ProcTCP (HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY ProcMail(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY ProcMem (HWND, ULONG, MPARAM, MPARAM);

CellDef cdMail =
{
    CELL_WINDOW, "Pumon.Mail",
    "",
    BS_PUSHBUTTON | BS_NOBORDER | BS_NOPOINTERFOCUS | BS_USERBUTTON,
    ID_MAIL, 0, 0
};

CellDef cdMem =
{
    CELL_WINDOW, "Pumon.Mem",
    "",
    BS_PUSHBUTTON | BS_NOBORDER | BS_NOPOINTERFOCUS | BS_USERBUTTON,
    ID_MEM, 0, 0
};

CellDef cdTCP =
{
    CELL_WINDOW, "Pumon.TCP",
    "",
    BS_PUSHBUTTON | BS_NOBORDER | BS_NOPOINTERFOCUS | BS_USERBUTTON,
    ID_TCP, 0, 0
};

CellDef cdCPU =
{
    CELL_WINDOW, "Pumon.CPU",
    "",
    BS_PUSHBUTTON | BS_NOBORDER | BS_NOPOINTERFOCUS | BS_USERBUTTON,
    ID_CPU, 0, 0
};

CellDef cdLeft =
{
    CELL_VSPLIT | CELL_SPLITBAR | CELL_SPLIT60x40,
    0, "",
    FCF_NOBYTEALIGN,
    ID_LPANE,
    &cdCPU,
    &cdTCP,
    0, MainClientProc
};

CellDef cdRight =
{
    CELL_VSPLIT | CELL_SPLITBAR | CELL_SPLIT80x20,
    0, "",
    FCF_NOBYTEALIGN,
    ID_RPANE,
    &cdMem,
    &cdMail,
    0, MainClientProc
};

CellDef mainClient =
{
    CELL_VSPLIT | CELL_SPLITBAR | CELL_SPLIT50x50,
    0, "PU Monitor",
    FCF_TASKLIST | FCF_SIZEBORDER | FCF_NOBYTEALIGN | FCF_ICON,
    MAIN_FRAME,
    &cdLeft,
    &cdRight,
    0, MainClientProc
};

/* Measuring threads */

TID pthUptime;
TID pthRam   ;
TID pthCPU   ;
TID pthMAIL  ;
TID pthIP    ;
TID pthCONN  ;

HWND hwndPane1;
HWND hwndPane2;
HWND hwndPane3;
HWND hwndPane4;
HWND hwndMainFrame;

HAB hab;

typedef struct
{
    LONG lColorFG;
    LONG lColorBG;
    LONG lColorGR;
    LONG lColorG1;
    LONG lColorG2;
    LONG lColorGD;
} stColorPalette;

stColorPalette colorPal[]=
{  //Characters  //Background  //Graph
    {CLR_BLACK   , CLR_DARKGRAY, CLR_PALEGRAY , CLR_BLUE    , CLR_BLACK   , CLR_PALEGRAY},
    {CLR_BLACK   , CLR_PALEGRAY, CLR_DARKGRAY , CLR_DARKPINK, CLR_DARKBLUE, CLR_DARKCYAN},
    {CLR_BLACK   , CLR_PALEGRAY, CLR_PALEGRAY , CLR_DARKPINK, CLR_DARKBLUE, CLR_DARKCYAN},
    {CLR_BLACK   , CLR_DARKGRAY, CLR_DARKGRAY , CLR_BLUE    , CLR_BLACK   , CLR_PALEGRAY},
    {CLR_GREEN   , CLR_BLACK   , CLR_DARKGREEN, CLR_PINK    , CLR_YELLOW  , CLR_DARKCYAN},
    {CLR_GREEN   , CLR_BLACK   , CLR_DARKGRAY , CLR_PINK    , CLR_YELLOW  , CLR_DARKCYAN},
    {CLR_PALEGRAY, CLR_BLACK   , CLR_DARKGRAY , CLR_PINK    , CLR_YELLOW  , CLR_DARKCYAN},
    {CLR_WHITE   , CLR_BLACK   , CLR_DARKGREEN, CLR_PINK    , CLR_YELLOW  , CLR_DARKCYAN},
    {CLR_TEXT    , CLR_BACK    , CLR_BAR      , CLR_GRAPH1  , CLR_GRAPH2  , CLR_GRID},
};

void FillPalette(LONG* alTable)
{
//RGB_BLUE               0x000000FFL
//RGB_GREEN              0x0000FF00L
//RGB_RED                0x00FF0000L

    alTable[CLR_TEXT  ] = iPal0;
    alTable[CLR_BACK  ] = iPal1;
    alTable[CLR_BAR   ] = iPal2;
    alTable[CLR_GRAPH1] = iPal3;
    alTable[CLR_GRAPH2] = iPal4;
    alTable[CLR_GRID  ] = iPal5;
}

void SetPalette(int i)
{
    iCurrPalette = i;
    lColorFG = colorPal[i].lColorFG;
    lColorBG = colorPal[i].lColorBG;
    lColorGR = colorPal[i].lColorGR;
    lColorG1 = colorPal[i].lColorG1;
    lColorG2 = colorPal[i].lColorG2;
    lColorGD = colorPal[i].lColorGD;

}

/* Start threads */

APIRET _osCreateThread(PTID ptid, PFNTHREAD pfn, ULONG param, ULONG flag, ULONG cbStack)
{
    return DosCreateThread(ptid, pfn, param, flag, cbStack);
}

void start_threads(HWND h1, HWND h2, HWND h3, HWND h4, HWND h5, HWND h6)
{
    _osCreateThread(&pthUptime, thUptime, (ULONG)h1, 0, 32768);
    _osCreateThread(&pthRam   , thRam   , (ULONG)h2, 0, 32768);
    _osCreateThread(&pthCPU   , thCPU   , (ULONG)h3, 0, 32768);
    _osCreateThread(&pthMAIL  , thMAIL  , (ULONG)h4, 0, 32768);
    _osCreateThread(&pthIP    , thIP    , (ULONG)h5, 0, 32768);
    _osCreateThread(&pthCONN  , thCONN  , (ULONG)h6, 0, 32768);
}

/* Main */

int main(int argc, char **argv)
{
    HMQ hmq;
	QMSG qmsg;
    HWND hwndFrame;
    SWP swp;

    libWin32kInit();

    hab = WinInitialize(0);

    if(!hab)
    {
        return -1;
    }

    hmq = WinCreateMsgQueue(hab, 0);

    if(!hmq)
    {
        WinTerminate(hab);
        return -2;
    }

    if(DosQuerySysInfo(QSV_NUMPROCESSORS, QSV_NUMPROCESSORS, &iNumCPU, sizeof(ULONG)))
        iNumCPU = 1;

    ToolkitInit(hab);

    WinRegisterClass(hab, "Pumon.CPU" , ProcCPU , CS_SIZEREDRAW, sizeof(ULONG));
    WinRegisterClass(hab, "Pumon.TCP" , ProcTCP , CS_SIZEREDRAW, sizeof(ULONG));
    WinRegisterClass(hab, "Pumon.Mail", ProcMail, CS_SIZEREDRAW, sizeof(ULONG));
    WinRegisterClass(hab, "Pumon.Mem" , ProcMem , CS_SIZEREDRAW, sizeof(ULONG));

    DosCreateMutexSem(NULL, &hmtxSites, DC_SEM_SHARED, FALSE);
    DosPerfSysCall(CMD_KI_ENABLE, 0, 0, 0);

    WinQueryWindowPos(HWND_DESKTOP, &swp);

    hwndFrame = CreateCell(&mainClient, HWND_DESKTOP, 0);

    if(hwndFrame)
    {
        hwndMainFrame = hwndFrame;

#ifndef DEBUGPOS
        iStartY = swp.y + swp.cy - 26;
#endif

        LoadVars();
        SetPalette(iCurrPalette);

#ifndef PRODUCTION

        if(argc == 2 && argv[1][0]=='-' && argv[1][1]=='p')
            PrintVars();
#endif

        SetSplit(CellWindowFromID(hwndFrame, ID_LPANE), iSplit1);
        SetSplit(CellWindowFromID(hwndFrame, ID_RPANE), iSplit2);
        SetSplit(hwndFrame, iSplit3);

        SetSplitType(CellWindowFromID(hwndFrame, ID_LPANE), iSplitType0);
        SetSplitType(CellWindowFromID(hwndFrame, ID_RPANE), iSplitType1);
        SetSplitType(hwndFrame, iSplitType2);

        int iMEnabled = 0;

        for(int m = 0; m < 5; m++)
            iMEnabled += iMailEnabled[m];

        ShowCell(hwndFrame, ID_MAIL, iMEnabled);
        ShowCell(hwndMainFrame, ID_TCP, iIPGraphEnabled + iIPTotals);

        WinStartTimer(hab,
                      WinWindowFromID(hwndFrame, FID_CLIENT),
                      ID_TIMER,
                      250);

        hwndPane1 = CellWindowFromID(hwndFrame, ID_CPU);
        hwndPane2 = CellWindowFromID(hwndFrame, ID_TCP);
        hwndPane3 = CellWindowFromID(hwndFrame, ID_MEM);
        hwndPane4 = CellWindowFromID(hwndFrame, ID_MAIL);

        SetPPFont(hwndPane1, cFont1);
        SetPPFont(hwndPane2, cFont2);
        SetPPFont(hwndPane3, cFont3);
        SetPPFont(hwndPane4, cFont4);

        WinSetWindowPos(hwndFrame, NULLHANDLE,
                        iStartX,
                        iStartY,
                        iSizeX,
                        iSizeY,
                        SWP_ACTIVATE | SWP_MOVE | SWP_SIZE | SWP_SHOW);

        WinSendMsg(hwndFrame,
                   WM_SETBORDERSIZE,
                   MPFROMSHORT(3),
                   MPFROMSHORT(3));

        WinSetWindowPos(hwndFrame, NULLHANDLE,
                        iStartX,
                        iStartY,
                        iSizeX,
                        iSizeY,
	                    SWP_ACTIVATE | SWP_MOVE | SWP_SIZE | SWP_SHOW);

    	/* Start measuring threads */

        start_threads(hwndPane3, hwndPane3, hwndPane1, hwndPane4, hwndPane2, 0);

        while (WinGetMsg(hab, &qmsg, 0, 0, 0))
        	WinDispatchMsg(hab, &qmsg);

        GetPPFont(hwndPane1, cFont1);
        GetPPFont(hwndPane2, cFont2);
        GetPPFont(hwndPane3, cFont3);
        GetPPFont(hwndPane4, cFont4);

        WinQueryWindowPos(hwndFrame, &swp);

        iStartX = swp.x;
        iStartY = swp.y;
        iSizeX  = swp.cx;
        iSizeY  = swp.cy;

        iSplit1 = GetSplit(CellWindowFromID(hwndFrame, ID_LPANE));
        iSplit2 = GetSplit(CellWindowFromID(hwndFrame, ID_RPANE));
        iSplit3 = GetSplit(hwndFrame);

        iSplitType0 = GetSplitType(CellWindowFromID(hwndFrame, ID_LPANE));
        iSplitType1 = GetSplitType(CellWindowFromID(hwndFrame, ID_RPANE));
        iSplitType2 = GetSplitType(hwndFrame);

        WinDestroyWindow(hwndFrame);

        StoreVars();
    }

    DosCloseMutexSem(hmtxSites);

	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);

    return 0;
}

MRESULT EXPENTRY MainClientProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    WindowCellCtlData *pWCtlData = 0;
    HWND hwndFrame;
    hwndFrame = WinQueryWindow(hwnd, QW_PARENT);

    pWCtlData = (WindowCellCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    switch(msg)
    {
#if USE_BIRD_FLOAT_ON_TOP

        case WM_VRNDISABLED:
            return NULL;

        case WM_VRNENABLED:
            WinSetVisibleRegionNotify(hwnd, FALSE);

            if(iFloat || iAttach)
            {
                HWND hwndWC = HWND_TOP;

                if(iAttach)
                    hwndWC = GetWindowAboveWarpCenter(0);

                if(iFloat || (iAttach && hwndWC != HWND_TOP))
                    WinSetWindowPos(hwndFrame, hwndWC, 0,0,0,0, SWP_ZORDER);
            }

            if(iAttach)
            {
                HWND hwndWC = 0;

                SWP swp;
                SWP swp2;
                SWP swp3;

                GetWindowAboveWarpCenter(&hwndWC);

                if(hwndWC)
                {
                    WinQueryWindowPos(hwndWC, &swp);
                    WinQueryWindowPos(hwndMainFrame, &swp2);

                    swp3 = swp;
                    swp3.x  += (swp.cy == 22) ? 41:50;
                    swp3.cx  =  (swp.cy == 22) ? 177:209;

                    if(iBtLeft < 4)
                        swp3.x++;
                    else
                        swp3.cx++;

                    swp3.x  += ((swp.cy == 22) ? 23:27)*(4 - iBtLeft);
                    swp3.cx += ((swp.cy == 22) ? 23:27)*(iBtLeft);
                    swp3.cx += ((swp.cy == 22) ? 23:27)*(iBtRight);

                    if((swp3.x != swp2.x   || swp3.y != swp2.y ||
                        swp3.cx != swp2.cx || swp3.cy != swp2.cy) &&
                        swp.cy < 50)
                    {
                        WinSetWindowPos(hwndMainFrame, 0,
                                        swp3.x, swp3.y, swp3.cx,swp3.cy,
                                        SWP_MOVE | SWP_SIZE);
                    }
                }
            }
            WinSetVisibleRegionNotify(hwnd, TRUE);
            return NULL;
#endif

        case WM_USER+ID_MENU_START_APP:
            RunProg((char *)mp1, (char *)mp2);
            break;

        case WM_TIMER:

#if 0
            /**/
            {
                SWP swp;

                WinQueryWindowPos(hwndMainFrame, &swp);

                if(swp.cx < swp.cy) //Vertically oriented
                {
                    iBarType = BAR_HORISONTAL;

                    if(GetSplitType(hwndMainFrame) != CELL_HSPLIT)
                    {
                        SetSplitType(CellWindowFromID(hwndMainFrame, ID_LPANE), CELL_HSPLIT);
                        SetSplitType(CellWindowFromID(hwndMainFrame, ID_RPANE), CELL_HSPLIT);
                        SetSplitType(hwndMainFrame, CELL_HSPLIT);
                        WinSendMsg(hwndMainFrame, WM_UPDATEFRAME, 0, 0);
                    }
                }
                else
                {
                    iBarType = BAR_VERTICAL;

                    if(GetSplitType(hwndMainFrame) != CELL_VSPLIT)
                    {
                        SetSplitType(CellWindowFromID(hwndMainFrame, ID_LPANE), CELL_VSPLIT);
                        SetSplitType(CellWindowFromID(hwndMainFrame, ID_RPANE), CELL_VSPLIT);
                        SetSplitType(hwndMainFrame, CELL_VSPLIT);
                        WinSendMsg(hwndMainFrame, WM_UPDATEFRAME, 0, 0);
                    }
                }
            }
#endif

#if !USE_BIRD_FLOAT_ON_TOP

            if(iFloat || iAttach)
            {
                HWND hwndWC = HWND_TOP;

                if(iAttach)
                    hwndWC = GetWindowAboveWarpCenter(0);

                if(iFloat || (iAttach && hwndWC != HWND_TOP))
                    WinSetWindowPos(hwndFrame, hwndWC, 0,0,0,0, SWP_ZORDER);
            }

            if(iAttach)
            {
                HWND hwndWC = 0;

                SWP swp;
                SWP swp2;
                SWP swp3;

                GetWindowAboveWarpCenter(&hwndWC);

                if(hwndWC)
                {
                    WinQueryWindowPos(hwndWC, &swp);
                    WinQueryWindowPos(hwndMainFrame, &swp2);

                    swp3 = swp;
                    swp3.x  += (swp.cy == 22) ? 41:50;
                    swp3.cx  =  (swp.cy == 22) ? 177:209;

                    if(iBtLeft < 4)
                        swp3.x++;
                    else
                        swp3.cx++;

                    swp3.x  += ((swp.cy == 22) ? 23:27)*(4 - iBtLeft);
                    swp3.cx += ((swp.cy == 22) ? 23:27)*(iBtLeft);
                    swp3.cx += ((swp.cy == 22) ? 23:27)*(iBtRight);

                    if((swp3.x != swp2.x   || swp3.y != swp2.y ||
                        swp3.cx != swp2.cx || swp3.cy != swp2.cy) &&
                         swp.cy < 50)
                    {
                        WinSetWindowPos(hwndMainFrame, 0,
        	                            swp3.x, swp3.y, swp3.cx,swp3.cy,
                    	                SWP_MOVE | SWP_SIZE);
                    }
                }
            }
#endif
            break;

        case WM_COMMAND:
            switch(SHORT1FROMMP(mp1))
            {
                case ID_MENU_CLOSE:
					WinPostMsg(hwnd, WM_QUIT, 0L, 0L);
                    return ((MRESULT) NULL);

                case ID_MENU_FLOAT:
                    iFloat = 1 - iFloat;
                    break;

                case ID_MENU_LOCK:
                    iLock = 1 - iLock;
                    break;

                case ID_MENU_ATT:
                    iAttach = 1 - iAttach;
                    break;

                case ID_MENU_SET_0:
                case ID_MENU_SET_1:
                case ID_MENU_SET_2:
                case ID_MENU_SET_3:
                case ID_MENU_SET_4:
                case ID_MENU_SET_5:
                case ID_MENU_SET_6:
                case ID_MENU_SET_7:
                case ID_MENU_SET_8:
                case ID_MENU_SET_9:
                case ID_MENU_ABOUT:
                    DoSettings(HWND_DESKTOP, SHORT1FROMMP(mp1) - ID_MENU_SET_0);
                    break;

                case ID_MENU_MAIL:
                    RunProg(cMailProg[0], cMailProgParm[0]);
                    break;

                case ID_MENU_CLR_0: case ID_MENU_CLR_1:
                case ID_MENU_CLR_2: case ID_MENU_CLR_3:
                case ID_MENU_CLR_4: case ID_MENU_CLR_5:
                case ID_MENU_CLR_6: case ID_MENU_CLR_7:
                case ID_MENU_CLR_USR:
                    SetPalette(SHORT1FROMMP(mp1) - ID_MENU_CLR_0);

                    WinInvalidateRect(hwndPane1, NULL, TRUE);
                    WinInvalidateRect(hwndPane2, NULL, TRUE);
                    WinInvalidateRect(hwndPane3, NULL, TRUE);
                    WinInvalidateRect(hwndPane4, NULL, TRUE);
                    break;
            }
            return ((MRESULT) NULL);

        case WM_CONTEXTMENU:
            {
                HWND hwndPopup;
                POINTL ptlPoint;

                if(iAttach)
                {
                    HWND hWndWC = 0;

                    GetWindowAboveWarpCenter(&hWndWC);

                    if(hWndWC)
                        WinSetWindowPos(hWndWC, HWND_TOP,
                                        0,0,0,0,
                                        SWP_ZORDER);
            	}

                WinQueryPointerPos(HWND_DESKTOP, &ptlPoint);

                hwndPopup = WinLoadMenu(hwndMainFrame, 0, ID_MENU);

                if(iFloat)
                    CheckMenuItem(hwndPopup, ID_MENU_FLOAT);

                if(iLock)
                    CheckMenuItem(hwndPopup, ID_MENU_LOCK);

                if(iAttach)
                    CheckMenuItem(hwndPopup, ID_MENU_ATT);

                CheckSubMenuItem(hwndPopup, ID_MENU_COLOR, ID_MENU_COLOR+iCurrPalette);

                WinPopupMenu(HWND_DESKTOP,
                             hwndMainFrame,
                             hwndPopup,
                             ptlPoint.x,
                             ptlPoint.y,
                             0,
                             0
                             | PU_NONE
                             | PU_KEYBOARD
                             | PU_MOUSEBUTTON1
                             | PU_MOUSEBUTTON2
                             | PU_HCONSTRAIN
                             | PU_VCONSTRAIN);
            }
            return ((MRESULT) NULL);
    }
    if(pWCtlData)
        return pWCtlData->pOldProc(hwnd, msg, mp1, mp2);

    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}


