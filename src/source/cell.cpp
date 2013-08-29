/*
** Module   :CELL.C
** Abstract :Cell Toolkit procedures
**
** Copyright (C) Sergey I. Yevtushenko
** Log: Sun  08/02/1998 Created
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
#include <cell.h>
#include <cvars.h>
#include <util.h>

#define TKM_SEARCH_ID       WM_USER+0x1000
#define TKM_QUERY_FLAGS     WM_USER+0x1001
#define TKM_SEARCH_PARENT   WM_USER+0x1002

#define CELL_TOP_LIMIT      98
#define CELL_BOTTOM_LIMIT    2
/*****************************************************************************
** Static data
*/

static CHAR CELL_CLIENT[] = "UCC";


/*****************************************************************************
** Internal prototypes
*/

MRESULT EXPENTRY CellProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY CellClientProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/*****************************************************************************
** Internal data types
*/

    /* Cell data, used by subclass proc of splitted window. */

typedef struct
{
    PFNWP pOldProc;
    RECTL rclBnd;
    LONG lType;
    LONG lSplit;
    LONG lSize;
    HWND hwndSplitbar;
    HWND hwndPanel1;
    HWND hwndPanel2;
} CellCtlData;

static HAB hab;

/*
******************************************************************************
** Cell (Splitted view) implementation
******************************************************************************
*/

LONG GetSplit(HWND hwnd)
{
    CellCtlData* pCtlData = 0;
    pCtlData = (CellCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    if(!pCtlData)
        return 0;

    return pCtlData->lSplit;
}

LONG SetSplit(HWND hwnd, LONG lNewSplit)
{
    CellCtlData* pCtlData = 0;

    pCtlData = (CellCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    if(!pCtlData)
        return 0;

    if(!(pCtlData->lType & CELL_FIXED))
    {
        pCtlData->lSplit = lNewSplit;
        if(pCtlData->lSplit  > CELL_TOP_LIMIT)
            pCtlData->lSplit = CELL_TOP_LIMIT;

        if(pCtlData->lSplit  < CELL_BOTTOM_LIMIT)
            pCtlData->lSplit = CELL_BOTTOM_LIMIT;

        WinSendMsg(hwnd, WM_UPDATEFRAME, 0, 0);
    }

    return pCtlData->lSplit;
}

LONG GetSplitType(HWND hwnd)
{
    CellCtlData* pCtlData = 0;

    pCtlData = (CellCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    if(!pCtlData)
        return 0;

    return (pCtlData->lType & (CELL_VSPLIT | CELL_HSPLIT | CELL_SWAP));
}

void SetSplitType(HWND hwnd, LONG lNewSplit)
{
    CellCtlData* pCtlData = 0;

    pCtlData = (CellCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    if(!pCtlData)
        return;

    pCtlData->lType &= ~(CELL_VSPLIT | CELL_HSPLIT);
    pCtlData->lType |= lNewSplit & (CELL_VSPLIT | CELL_HSPLIT);

    if(lNewSplit & CELL_SWAP) //Swap required?
    {
        if(!(pCtlData->lType & CELL_SWAP)) //Not swapped yet
        {
        	//Swap subwindows
        	HWND hwndTmp = pCtlData->hwndPanel1;
        	pCtlData->hwndPanel1 = pCtlData->hwndPanel2;
        	pCtlData->hwndPanel2 = hwndTmp;
        }

        pCtlData->lType |= CELL_SWAP;
    }
    else
    {
        if(pCtlData->lType & CELL_SWAP) //Already swapped
        {
            //Restore original state
            HWND hwndTmp = pCtlData->hwndPanel1;
            pCtlData->hwndPanel1 = pCtlData->hwndPanel2;
            pCtlData->hwndPanel2 = hwndTmp;
        }

        pCtlData->lType &= ~CELL_SWAP;
    }
}

void ShowCell(HWND hwnd, LONG lID, BOOL bAction)
{
    HWND hwndMain = hwnd;
    CellCtlData* pCtlData = 0;
    LONG lCell = 0;

    hwnd = CellParentWindowFromID(hwnd, lID);

    if(!hwnd)
        return;

    pCtlData = (CellCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    if(!pCtlData)
        return;

    if(WinQueryWindowUShort(pCtlData->hwndPanel1, QWS_ID) == lID)
        lCell = CELL_HIDE_1;

    if(WinQueryWindowUShort(pCtlData->hwndPanel2, QWS_ID) == lID)
        lCell = CELL_HIDE_2;

    switch(lCell)
    {
        case CELL_HIDE_1:
            if(bAction == FALSE)
                pCtlData->lType |= CELL_HIDE_1;
            else
                pCtlData->lType &= ~CELL_HIDE_1;
            break;

        case CELL_HIDE_2:
            if(bAction == FALSE)
                pCtlData->lType |= CELL_HIDE_2;
            else
                pCtlData->lType &= ~CELL_HIDE_2;
            break;
    }

    if(lCell)
        WinSendMsg(hwnd, WM_UPDATEFRAME, 0, 0);
}

/* Function: CellProc
** Abstract: Subclass procedure for frame window
*/

MRESULT EXPENTRY CellProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    CellCtlData* pCtlData = 0;
    ULONG itemCount;

    pCtlData = (CellCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    if(!pCtlData)
    {
        return 0;
    }
    switch(msg)
    {
        case TKM_SEARCH_PARENT:
            {
                HWND hwndRC;

                if(WinQueryWindowUShort(hwnd, QWS_ID) == (ULONG)mp1)
                    return MRFROMSHORT(FALSE);

                if(WinQueryWindowUShort(pCtlData->hwndPanel1, QWS_ID)
                    == (ULONG)mp1)
                    return (MPARAM)hwnd;

                hwndRC = (HWND)WinSendMsg(pCtlData->hwndPanel1,
                                          TKM_SEARCH_PARENT,
                                          mp1, 0);
                if(hwndRC)
                    return (MPARAM)hwndRC;

                if(WinQueryWindowUShort(pCtlData->hwndPanel2, QWS_ID)
                    == (ULONG)mp1)
                    return (MPARAM)hwnd;

                hwndRC = (HWND)WinSendMsg(pCtlData->hwndPanel2,
                                          TKM_SEARCH_PARENT,
                                          mp1, 0);
                if(hwndRC)
                    return (MPARAM)hwndRC;

            }
            return MRFROMSHORT(FALSE);

        case TKM_SEARCH_ID:
            {
                HWND hwndRC;

                if(WinQueryWindowUShort(hwnd, QWS_ID) == (ULONG)mp1)
                    return (MPARAM)hwnd;

                if(WinQueryWindowUShort(pCtlData->hwndPanel1, QWS_ID)
                	== (ULONG)mp1)
                    return (MPARAM)pCtlData->hwndPanel1;

                hwndRC = (HWND)WinSendMsg(pCtlData->hwndPanel1,
                                          TKM_SEARCH_ID,
                                          mp1, 0);
                if(hwndRC)
                    return (MPARAM)hwndRC;

                if(WinQueryWindowUShort(pCtlData->hwndPanel2, QWS_ID)
                	== (ULONG)mp1)
                    return (MPARAM)pCtlData->hwndPanel2;

                hwndRC = (HWND)WinSendMsg(pCtlData->hwndPanel2,
                                          TKM_SEARCH_ID,
                                          mp1, 0);
                if(hwndRC)
                    return (MPARAM)hwndRC;
            }
            return MRFROMSHORT(FALSE);

        case WM_QUERYTRACKINFO:
            {
                PTRACKINFO ptInfo;

                itemCount = (ULONG)(pCtlData->pOldProc(hwnd, msg, mp1, mp2));
                ptInfo = (PTRACKINFO)(mp2);

                ptInfo->ptlMinTrackSize.y -= 8;
                ptInfo->ptlMinTrackSize.x  = ptInfo->ptlMinTrackSize.y;

                ptInfo->rclBoundary.xLeft   = -30000;
                ptInfo->rclBoundary.xRight  =  30000;
                ptInfo->rclBoundary.yBottom = -30000;
                ptInfo->rclBoundary.yTop    =  30000;
            }
            return MRFROMLONG(itemCount);

        case WM_QUERYFRAMECTLCOUNT:
            {
                itemCount = (ULONG)(pCtlData->pOldProc(hwnd, msg, mp1, mp2));

                if(pCtlData->hwndPanel1 && !(pCtlData->lType & CELL_HIDE_1))
                    itemCount++;

                if(pCtlData->hwndPanel2 && !(pCtlData->lType & CELL_HIDE_2))
                    itemCount++;

            }
            return MRFROMLONG(itemCount);

        case WM_FORMATFRAME:
            {
                PSWP   pSWP     = 0;
                USHORT usClient = 0;

                itemCount = (ULONG)(pCtlData->pOldProc(hwnd, msg, mp1, mp2));

                if(!pCtlData->hwndPanel1 && !pCtlData->hwndPanel2)
                    return MRFROMLONG(itemCount);

                pSWP = (PSWP)PVOIDFROMMP(mp1);

                while (pSWP[usClient].hwnd != WinWindowFromID(hwnd,
                                                              FID_CLIENT))
                    usClient++;

                /*
                ** Placing panels.
                ** Remember client rect for future use
                ** They will save time when we start moving splitbar
                */

                pCtlData->rclBnd.xLeft   = pSWP[usClient].x;
                pCtlData->rclBnd.xRight  = pSWP[usClient].x +
                                           pSWP[usClient].cx;
                pCtlData->rclBnd.yTop    = pSWP[usClient].y +
                                           pSWP[usClient].cy;
                pCtlData->rclBnd.yBottom = pSWP[usClient].y;

                if(!pCtlData->hwndPanel1 || !pCtlData->hwndPanel2 ||
                   (pCtlData->lType & CELL_HIDE))
                {
                    /*
                    **single subwindow;
                    **In this case we don't need a client window,
                    **because of lack of splitbar.
                    **Just copy all data from pSWP[usClient]
                    **and replace some part of it
                    */

                    pSWP[itemCount]     = pSWP[usClient];
                    pSWP[itemCount].fl |= SWP_MOVE | SWP_SIZE;
                    pSWP[itemCount].hwndInsertBehind = HWND_TOP;
                    pSWP[usClient].cy = 0;

                    pSWP[itemCount].hwnd = 0;

                    if(pCtlData->hwndPanel1 && !(pCtlData->lType & CELL_HIDE_1))
                        pSWP[itemCount].hwnd = pCtlData->hwndPanel1;

                    if(pCtlData->hwndPanel2 && !(pCtlData->lType & CELL_HIDE_2))
                        pSWP[itemCount].hwnd = pCtlData->hwndPanel2;

                    /* Increase number of controls */

                    if(pSWP[itemCount].hwnd)
                        itemCount++;
                }
                else
                {
                    ULONG usPanel1 = itemCount;
                    ULONG usPanel2 = itemCount + 1;
                    ULONG usWidth1 = 0;
                    ULONG usWidth2 = 0;

                    /* Just like case of one panel */

                    pSWP[usPanel1] = pSWP[usClient];
                    pSWP[usPanel2] = pSWP[usClient];

                    pSWP[usPanel1].fl |= SWP_MOVE | SWP_SIZE;
                    pSWP[usPanel2].fl |= SWP_MOVE | SWP_SIZE;

                    pSWP[usPanel1].hwndInsertBehind = HWND_TOP;
                    pSWP[usPanel2].hwndInsertBehind = HWND_TOP;

                    pSWP[usPanel1].hwnd = pCtlData->hwndPanel1;
                    pSWP[usPanel2].hwnd = pCtlData->hwndPanel2;

                    if(pCtlData->lType & CELL_VSPLIT)
                    {
                        if((pCtlData->lType & CELL_FIXED) &&
                           (pCtlData->lType & (CELL_SIZE1 | CELL_SIZE2)) &&
                           (pCtlData->lSize > 0))
                        {
                            /* Case of fixed panel with exact size */

                            if(pCtlData->lType & CELL_SIZE1)
                            {
                                usWidth1 = pCtlData->lSize;
                                usWidth2 = pSWP[usClient].cx - usWidth1;
                            }
                            else
                            {
                                usWidth2 = pCtlData->lSize;
                                usWidth1 = pSWP[usClient].cx - usWidth2;
                            }
                        }
                        else
                        {
                        	usWidth1 = (pSWP[usClient].cx * pCtlData->lSplit)
                            	        / 100;
                        	usWidth2 = pSWP[usClient].cx - usWidth1;
                        }

                        if(pCtlData->lType & CELL_SPLITBAR)
                        {
                            if(!(pCtlData->lType & CELL_SIZE1))
                                usWidth2 -= SPLITBAR_WIDTH;
                            else
                                usWidth1 -= SPLITBAR_WIDTH;

                            pSWP[usClient].cx = SPLITBAR_WIDTH;
                            pSWP[usClient].x  = pSWP[usClient].x + usWidth1;
                        }
                        else
                            pSWP[usClient].cx = 0;

                        pSWP[usPanel1].cx  = usWidth1;
                        pSWP[usPanel2].x  += usWidth1 + pSWP[usClient].cx;
                        pSWP[usPanel2].cx  = usWidth2;
                    }
                    else
                    {
                        if((pCtlData->lType & CELL_FIXED) &&
                           (pCtlData->lType & (CELL_SIZE1 | CELL_SIZE2)) &&
                           (pCtlData->lSize > 0))
                        {
                            /* Case of fixed panel with exact size */

                            if(pCtlData->lType & CELL_SIZE1)
                            {
                                usWidth1 = pCtlData->lSize;
                                usWidth2 = pSWP[usClient].cy - usWidth1;
                            }
                            else
                            {
                                usWidth2 = pCtlData->lSize;
                                usWidth1 = pSWP[usClient].cy - usWidth2;
                            }
                        }
                        else
                        {
    	                    usWidth1 = (pSWP[usClient].cy * pCtlData->lSplit)
                                       / 100;
                            usWidth2 = pSWP[usClient].cy - usWidth1;
                        }

                        if(pCtlData->lType & CELL_SPLITBAR)
                        {
                            if(!(pCtlData->lType & CELL_SIZE1))
                                usWidth2 -= SPLITBAR_WIDTH;
                            else
                                usWidth1 -= SPLITBAR_WIDTH;
                            pSWP[usClient].cy = SPLITBAR_WIDTH;
                            pSWP[usClient].y  = pSWP[usClient].y + usWidth1;
                        }
                        else
                            pSWP[usClient].cy = 0;

                        pSWP[usPanel1].cy  = usWidth1;
                        pSWP[usPanel2].y  += usWidth1 + pSWP[usClient].cy;
                        pSWP[usPanel2].cy  = usWidth2;
                    }

                    itemCount += 2;
                }
            }
            return MRFROMSHORT(itemCount);
    }
    return pCtlData->pOldProc(hwnd, msg, mp1, mp2);
}

/* Function: CellClientProc
** Abstract: Window procedure for Cell Client Window Class (splitbar)
*/

MRESULT EXPENTRY CellClientProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    HWND hwndFrame = 0;
    CellCtlData *pCtlData = 0;

    hwndFrame = WinQueryWindow(hwnd, QW_PARENT);
    if(hwndFrame)
        pCtlData = (CellCtlData *)WinQueryWindowULong(hwndFrame, QWL_USER);

    switch (msg)
    {
        case WM_PAINT:
            {
                HPS hpsPaint;
                RECTL rclPaint;

                hpsPaint = WinBeginPaint(hwnd, 0, 0);

                WinQueryWindowRect(hwnd, &rclPaint);
                WinFillRect(hpsPaint, &rclPaint, CLR_PALEGRAY);

                rclPaint.yTop     -= 1;
                rclPaint.xRight   -= 1;

                aRect(hpsPaint, rclPaint, (iDrawBorder) ? 0:1);
                //aRect(hpsPaint, rclPaint, (pCtlData->lType & CELL_FIXED) ? 0:1);

                WinEndPaint(hpsPaint);

            }
            return MRFROMSHORT(FALSE);

        case WM_MOUSEMOVE:
            {
                if(iLock)
                    break;

                if(pCtlData->lType & CELL_FIXED)
                    break;

                if(pCtlData->lType & CELL_VSPLIT)
                    WinSetPointer(HWND_DESKTOP,
                                  WinQuerySysPointer(HWND_DESKTOP,
                                                     SPTR_SIZEWE,
                                                     FALSE));
                else
                    WinSetPointer(HWND_DESKTOP,
                                  WinQuerySysPointer(HWND_DESKTOP,
                                                     SPTR_SIZENS,
                                                     FALSE));
            }
            return MRFROMSHORT(FALSE);

        case WM_BUTTON1DOWN:
            {
                TRACKINFO track;
                APIRET rc;

                if(iLock)
                    break;

                if(pCtlData->lType & CELL_FIXED)
                    break;

                track.cxBorder = 2;
                track.cyBorder = 2;
                track.cxGrid   = 1;
                track.cyGrid   = 1;
                track.cxKeyboard = 8;
                track.cyKeyboard = 8;
                track.rclBoundary = pCtlData->rclBnd;

                WinMapWindowPoints(hwndFrame,
                                   HWND_DESKTOP,
                                   (PPOINTL)&track.rclBoundary,
                                   2);

                WinQueryWindowRect(hwnd, &track.rclTrack);
                WinMapWindowPoints(hwnd,
                                   HWND_DESKTOP,
                                   (PPOINTL)&track.rclTrack,
                                   2);

                track.ptlMinTrackSize.x = track.rclTrack.xRight
                                        - track.rclTrack.xLeft;
                track.ptlMinTrackSize.y = track.rclTrack.yTop
                                    	- track.rclTrack.yBottom;
                track.ptlMaxTrackSize.x = track.rclTrack.xRight
                                    	- track.rclTrack.xLeft;
                track.ptlMaxTrackSize.y = track.rclTrack.yTop
                                    	- track.rclTrack.yBottom;

                track.fs = TF_MOVE | TF_ALLINBOUNDARY;

                rc = WinTrackRect(HWND_DESKTOP, 0, &track);

                if(rc == TRUE)
                {
                    ULONG usNewRB;
                    ULONG usSize;

                    if(pCtlData->lType & CELL_VSPLIT)
                    {
                        usNewRB = track.rclTrack.xLeft
                                - track.rclBoundary.xLeft;
                        usSize  = track.rclBoundary.xRight
                                - track.rclBoundary.xLeft;
                    }
                    else
                    {
                        usNewRB = track.rclTrack.yBottom
                                - track.rclBoundary.yBottom;
                        usSize  = track.rclBoundary.yTop
                                - track.rclBoundary.yBottom;
                    }

                    pCtlData->lSplit = (usNewRB * 100)/usSize;
                    if(pCtlData->lSplit  > CELL_TOP_LIMIT)
                        pCtlData->lSplit = CELL_TOP_LIMIT;

                    if(pCtlData->lSplit  < CELL_BOTTOM_LIMIT)
                        pCtlData->lSplit = CELL_BOTTOM_LIMIT;

                    WinSendMsg(hwndFrame, WM_UPDATEFRAME, 0, 0);
                }
            }
            return MRFROMSHORT(FALSE);

        case WM_BUTTON2DOWN:

            if(pCtlData->lType & CELL_FIXED)
                break;

            {
                ULONG lType = pCtlData->lType & (CELL_VSPLIT | CELL_HSPLIT);

                pCtlData->lType &= ~(CELL_VSPLIT | CELL_HSPLIT);

                if(lType & CELL_VSPLIT)
                {
                    pCtlData->lType |= CELL_HSPLIT;
                }
                else
                {
                    pCtlData->lType |= CELL_VSPLIT;
                }

                //Swap subwindows

                if(lType & CELL_VSPLIT)
                {
                    HWND hwndTmp = pCtlData->hwndPanel1;
                    pCtlData->hwndPanel1 = pCtlData->hwndPanel2;
                    pCtlData->hwndPanel2 = hwndTmp;

	                pCtlData->lType ^= CELL_SWAP;
                }

                if(pCtlData->lType & CELL_HIDE_1)
                {
                    pCtlData->lType &= ~CELL_HIDE_1;
                    pCtlData->lType |=  CELL_HIDE_2;
                }
                else
                {
                    if(pCtlData->lType & CELL_HIDE_2)
                    {
                        pCtlData->lType &= ~CELL_HIDE_2;
                        pCtlData->lType |=  CELL_HIDE_1;
                    }
                }

                if(pCtlData->lType & CELL_SIZE1)
                {
                    pCtlData->lType &= ~CELL_SIZE1;
                    pCtlData->lType |=  CELL_SIZE2;
                }
                else
                {
                    if(pCtlData->lType & CELL_SIZE2)
                    {
                        pCtlData->lType &= ~CELL_SIZE2;
                        pCtlData->lType |=  CELL_SIZE1;
                    }
                }

                WinSendMsg(hwndFrame, WM_UPDATEFRAME, 0, 0);
            }
            return MRFROMSHORT(FALSE);
    }
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/* Function: CreateCell
** Abstract: Creates a subwindows tree for a given CellDef
** Note: If hWndOwner == NULLHANDLE, and first CellDef is frame,
**       all subwindows will have this frame window as Owner.
*/

HWND CreateCell(CellDef* pCell, HWND hWndParent, HWND hWndOwner)
{
    HWND hwndFrame        = NULLHANDLE;
    CellCtlData* pCtlData = 0;
    WindowCellCtlData * pWCtlData = 0;

    if(!pCell)
        return hwndFrame;

    switch(pCell->lType & (CELL_VSPLIT | CELL_HSPLIT | CELL_WINDOW))
    {
        case CELL_WINDOW:
            hwndFrame = WinCreateWindow(hWndParent,
                                        pCell->pszClass,
                                        pCell->pszName,
                                        pCell->ulStyle,
                                    	0, 0, 0, 0,
                                        hWndOwner,
                                    	HWND_TOP,
                                        pCell->ulID,
                                    	NULL,
                                        NULL);

            if(pCell->pClassProc && hwndFrame)
            {
                pWCtlData = (WindowCellCtlData *)
                             malloc(sizeof(WindowCellCtlData));
                if(!pWCtlData)
                	return hwndFrame;

                memset(pWCtlData, 0, sizeof(WindowCellCtlData));
                pWCtlData->pOldProc = WinSubclassWindow(hwndFrame,
                                                        pCell->pClassProc);
                WinSetWindowULong(hwndFrame, QWL_USER, (ULONG)pWCtlData);
            }
            break;

        case CELL_HSPLIT:
        case CELL_VSPLIT:

            pCtlData = (CellCtlData *)malloc(sizeof(CellCtlData));
            if(!pCtlData)
                return hwndFrame;

            memset(pCtlData, 0, sizeof(CellCtlData));

            pCtlData->lType = pCell->lType & CELL_SPLIT_MASK;
            pCtlData->lSize = (pCell->lType & (CELL_SIZE1
                                               | CELL_SIZE2
                                               | CELL_FIXED))
                                ? pCell->lSize
                                : 0;

            pCtlData->lSplit = 50;

            switch(pCell->lType & CELL_SPLIT_REL)
            {
                case CELL_SPLIT10x90: pCtlData->lSplit = 10; break;
                case CELL_SPLIT20x80: pCtlData->lSplit = 20; break;
                case CELL_SPLIT30x70: pCtlData->lSplit = 30; break;
                case CELL_SPLIT40x60: pCtlData->lSplit = 40; break;
                case CELL_SPLIT50x50: pCtlData->lSplit = 50; break;
                case CELL_SPLIT60x40: pCtlData->lSplit = 60; break;
                case CELL_SPLIT70x30: pCtlData->lSplit = 70; break;
                case CELL_SPLIT80x20: pCtlData->lSplit = 80; break;
                case CELL_SPLIT90x10: pCtlData->lSplit = 90; break;
            }

            hwndFrame = WinCreateStdWindow(hWndParent,
                                           WS_VISIBLE,
                                           &pCell->ulStyle,
                                           (PSZ)CELL_CLIENT,
                                           pCell->pszName,
                                           0L, 0,
                                           pCell->ulID,
                                           &pCtlData->hwndSplitbar);

            if(pCell->pClassProc)
                pCtlData->pOldProc = WinSubclassWindow(hwndFrame,
                                                       pCell->pClassProc);
            else
                pCtlData->pOldProc = WinSubclassWindow(hwndFrame, CellProc);

            if(pCell->pClientClassProc)
            {
                pWCtlData = (WindowCellCtlData *)
                             malloc(sizeof(WindowCellCtlData));
                if(!pWCtlData)
                	return hwndFrame;

                memset(pWCtlData, 0, sizeof(WindowCellCtlData));

                pWCtlData->pOldProc = WinSubclassWindow(pCtlData->hwndSplitbar,
                                                       pCell->pClientClassProc);
                WinSetWindowULong(pCtlData->hwndSplitbar,
                                  QWL_USER,
                                  (ULONG)pWCtlData);
            }

            if(!hWndOwner)
                hWndOwner = hwndFrame;
            else
                WinSetOwner(pCtlData->hwndSplitbar, hWndOwner);

            pCtlData->hwndPanel1 = CreateCell(pCell->pPanel1,
                                              hwndFrame,
                                              hWndOwner);
            pCtlData->hwndPanel2 = CreateCell(pCell->pPanel2,
                                              hwndFrame,
                                              hWndOwner);

            WinSetWindowULong(hwndFrame, QWL_USER, (ULONG)pCtlData);
            break;
    }

    return hwndFrame;
}

/* Function: ToolkitInit
** Abstract: Registers classes needed for toolkit
*/

void ToolkitInit(HAB aHab)
{
    hab = aHab;
    WinRegisterClass(hab,
                     (PSZ)CELL_CLIENT,
                     CellClientProc,
                     CS_SIZEREDRAW,
                     sizeof(ULONG));
}

/* Function: CellWindowFromID
** Abstract: Locate control window with given ID
*/

HWND CellWindowFromID(HWND hwndCell, ULONG ulID)
{
    return (HWND)WinSendMsg(hwndCell, TKM_SEARCH_ID, MPFROMLONG(ulID), 0);
}

HWND CellParentWindowFromID(HWND hwndCell, ULONG ulID)
{
    return (HWND)WinSendMsg(hwndCell, TKM_SEARCH_PARENT, MPFROMLONG(ulID), 0);
}

