/*
** Module   :ITEMPROC.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
** Copyright (C) "Nick Kurshev" <nickols_k@mail.ru>
**
** Log: Sun  14/11/1999 Created
**      Sat  14/10/2000 Merged changes from NK
**
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

static void prepare(PaintBuffer& buf, RECTL& rclPaint, int& iBType)
{
    if((iCurrPalette + ID_MENU_CLR_0) == ID_MENU_CLR_USR)
    {
        LONG alTable[16];

        FillPalette(alTable);
        buf.SetPal(alTable, CLR_START, CLR_END);
    }

    rclPaint = buf.getRect();

    if((rclPaint.xRight - rclPaint.xLeft) <
       (rclPaint.yTop - rclPaint.yBottom))
    {
        iBType = BAR_VERTICAL;
    }

    WinFillRect(buf.getPS(), &rclPaint, lColorBG);

    if(iDrawBorder)
    {
        rclPaint.yTop    -= 1;
        rclPaint.xRight  -= 1;
        aRect(buf.getPS(), rclPaint, 1);
        rclPaint.yTop    -= 1;
        rclPaint.yBottom += 1;
        rclPaint.xRight  -= 1;
        rclPaint.xLeft   += 1;
    }
}

MRESULT EXPENTRY CommonProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg)
    {
        case WM_PRESPARAMCHANGED:
            WinInvalidateRect(hwnd, NULL, TRUE);
            break;

        case WM_BUTTON1DOWN:
            if(iLock)
                return MRFROMSHORT(FALSE);

            return WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),
                            WM_TRACKFRAME, MPFROMSHORT(TF_MOVE), MPVOID);

        case WM_BUTTON2DOWN:
            return WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),
                              WM_CONTEXTMENU, mp1, mp2);
    }
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY ProcCPU(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg)
    {
        case WM_PAINT:
            {
                RECTL rclPaint;
                RECTL rclPaint2;
                CHAR  cBuff[256];
                int iBType = BAR_HORISONTAL;

                PaintBuffer buf = hwnd;

                prepare(buf, rclPaint, iBType);

                LONG lCnt;
                static LONG lOldCPU = 0;
                LONG lKCnt;
                static LONG lOldKCPU = 0;

                if(iNumCPU > 1)
                {
                    lCnt = (iCPULoad + iCPULoad1)/2;

                    if(iCPUKernel)
                    	lKCnt = (iCPUKLoad + iCPUKLoad1)/2;
                }
                else
                {
                    lCnt  = iCPULoad;
                    lKCnt = iCPUKLoad;
                }

                if(lCnt > 100)
                    lCnt = 100;
                if(lCnt < 0)
                    lCnt = 0;

                if(iCPUKernel)
                {
                  	if(lKCnt > 100)
                      	lKCnt = 100;
                  	if(lKCnt < 0)
                      	lKCnt = 0;
                }
                if(lOldCPU > lCnt)
                {
                    lCnt = (lOldCPU + lCnt)/2;
                    if(lCnt > 100)
                        lCnt = 100;
                    if(lCnt < 0)
                        lCnt = 0;

                }

                lOldCPU = lCnt;

                if(iCPUKernel)
                {
                  	if(lOldKCPU > lKCnt)
                  	{
                    	lKCnt = (lOldKCPU + lKCnt)/2;

                    	if(lKCnt > 100)
                        	lKCnt = 100;
                    	if(lKCnt < 0)
                        	lKCnt = 0;

                  	}
                  	lOldKCPU = lKCnt;
                }

                if(iCPUGraph == 1)
                {
                    if(iNumCPU > 1)
                    {
                        HalfRect(rclPaint   , rclPaint2, (iBType == BAR_HORISONTAL) ? QUAD_TOP:QUAD_LEFT);
//                        DrawBar (buf.getPS(), rclPaint2, iCPULoad , 100, iBType, lColorGR);
                        if(iCPUKernel)
                        {
                            DrawDoubleBar(buf.getPS(), rclPaint2,
                                          iCPUKLoad, iCPULoad, 100, iBType,
                                          lColorG2, lColorG1);
                        }
                        else
                        {
                            DrawBar(buf.getPS(), rclPaint2,
                                    iCPULoad , 100, iBType, lColorGR);
                        }
                        HalfRect(rclPaint   , rclPaint2, (iBType == BAR_HORISONTAL) ? QUAD_BOTTOM:QUAD_RIGHT);
//                        DrawBar (buf.getPS(), rclPaint2, iCPULoad1, 100, iBType, lColorGR);
                        if(iCPUKernel)
                        {
                            DrawDoubleBar(buf.getPS(), rclPaint2,
                                          iCPUKLoad1, iCPULoad1, 100, iBType,
                                          lColorG2, lColorG1);
                        }
                        else
                        {
                            DrawBar(buf.getPS(), rclPaint2,
                                    iCPULoad1, 100, iBType, lColorGR);
                        }
                    }
                    else
                    {
//                        DrawBar(buf.getPS(), rclPaint, lCnt, 100, iBType, lColorGR);
                        if(iCPUKernel)
                            DrawDoubleBar(buf.getPS(), rclPaint, lKCnt, lCnt,
                                          100, iBType, lColorG2, lColorG1);
                        else
                            DrawBar(buf.getPS(), rclPaint, lCnt, 100,
                                    iBType, lColorGR);
                    }
                }

                if(iCPUGraph == 2)
                {
                  	if(iCPUKernel)
                  	{
                    	buf.DrawDoubleColorDoubleGraph(rclPaint,
                        	   (iNumCPU > 1) ? iCPUValues1:0,
                            	    (iNumCPU > 1) ? iCPUKValues1:0,
                                	iCPUValues,
	                                iCPUKValues,
                                    100, 100,
                                    iCValuesFilled,
                                	iCPUStartGrid,
                                	lColorGD, lColorG1, lColorG2);
                  	}
                    else
	                {
    	                buf.DrawDoubleGraph(rclPaint,
	                            (iNumCPU > 1) ? iCPUValues1:0, iCPUValues,
            	                 100, 100,
                	             iCValuesFilled,
        	                     iCPUStartGrid,
    	                         lColorGD, lColorG1, lColorG2);
	                }
                }

                if(iCPUText)
                {
//                    sprintf(cBuff,"%d%%", lCnt);
                    if(iCPUKernel)
                    	sprintf(cBuff,"(%d%%)%d%%", lKCnt, lCnt);
                    else
                    	sprintf(cBuff,"%d%%", lCnt);

                    buf.DrawText(cBuff, rclPaint, lColorFG);
                }
            }
            return MRFROMSHORT(FALSE);
    }
    return CommonProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY ProcTCP(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg)
    {
        case WM_PAINT:
            {
                RECTL rclPaint;
                RECTL rclPaint2;
                CHAR  cBuff[256];
                CHAR  cBuff2[256];
                CHAR  cTmp[128];

                int   iBType = BAR_HORISONTAL;
                int   iTwoLines = 0;

                cBuff[0] = cBuff2[0] = 0;

                PaintBuffer buf = hwnd;

                prepare(buf, rclPaint, iBType);

                if((buf.TextCy("X") * 2) <= (rclPaint.yTop - rclPaint.yBottom))
                    iTwoLines = 1;

                if(iIPGraphEnabled == 1)
                {
                    int iMaxIn  = uMaxIO;
                    int iMaxOut = uMaxIO;

                    if(iIPAdaptive)
                    {
                        iMaxIn  = 0;
                        iMaxOut = 0;
                        for(int k = 0; k < iValuesFilled; k++)
                        {
                            if(iMaxIn < iInValues[k])
                                iMaxIn = iInValues[k];
                            if(iMaxOut < iOutValues[k])
                                iMaxOut = iOutValues[k];
                        }
                        if(iMaxIn <= 0)
                            iMaxIn = 1;
                        if(iMaxOut <= 0)
                            iMaxOut = 1;
                        if(iMaxIn > iMaxOut)
                            iMaxOut = iMaxIn;
                        else
                            iMaxIn = iMaxOut;
                    }

                    HalfRect(rclPaint , rclPaint2, (iBType == BAR_HORISONTAL) ? QUAD_TOP:QUAD_LEFT);
                    DrawBar (buf.getPS(), rclPaint2, uIn, iMaxIn, iBType, lColorGR);
                    HalfRect(rclPaint , rclPaint2, (iBType == BAR_HORISONTAL) ? QUAD_BOTTOM:QUAD_RIGHT);
                    DrawBar (buf.getPS(), rclPaint2, uOut, iMaxOut, iBType, lColorGR);
                }
                else
                {
                    if(iIPGraphEnabled)
                        buf.DrawDoubleGraph(rclPaint,
                                            iInValues, iOutValues,
                                            -1, -1,
                                            iValuesFilled,
                                            iIPStartGrid,
                                            lColorGD, lColorG1, lColorG2);
                }

                if(iIPTotals)
                {
                    if(iIPTotals != 2 && iTwoLines)
                    {
                        ScaledPrint(cTmp, uIn);
                        strcpy(cBuff, cTmp);
                        strcat(cBuff," in");

                        ScaledPrint(cTmp, uOut);
                        strcpy(cBuff2,cTmp);
                        strcat(cBuff2," out");
                    }
                    else
                    {
                        ScaledPrint(cTmp, uIn);
                        strcpy(cBuff, cTmp);
                        strcat(cBuff,":");
                        ScaledPrint(cTmp, uOut);
                        strcat(cBuff,cTmp);
                    }
                }

                if(iIPTotals == 2)
                {
                    if(iTwoLines)
                    {
                        strcpy(cBuff2, "Total ");
                        ScaledPrint(cTmp, uInTotal);
                        strcat(cBuff2,cTmp);
                        strcat(cBuff2," in :");
                        ScaledPrint(cTmp, uOutTotal);
                        strcat(cBuff2,cTmp);
                        strcat(cBuff2," out");
                    }
                    else
                    {
                        strcat(cBuff, " (");
                        ScaledPrint(cTmp, uInTotal);
                        strcat(cBuff,cTmp);
                        strcat(cBuff,":");
                        ScaledPrint(cTmp, uOutTotal);
                        strcat(cBuff,cTmp);
                        strcat(cBuff,")");
                    }
                }

                if(!cBuff2[0])
                {
                    buf.DrawText(cBuff, rclPaint, lColorFG);
                }
                else
                {
                    HalfRect(rclPaint , rclPaint2, QUAD_TOP);
                    buf.DrawText(cBuff, rclPaint2, lColorFG);

                    HalfRect(rclPaint , rclPaint2, QUAD_BOTTOM);
                    buf.DrawText(cBuff2, rclPaint2, lColorFG);
                }

            }
            return MRFROMSHORT(FALSE);
    }
    return CommonProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY ProcMail(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg)
    {
        case WM_PAINT:
            {
                RECTL rclPaint;
                RECTL rclPaint1;
                CHAR  cBuff[256];
                CHAR  c0[32];
                int iBType = BAR_HORISONTAL;
                int i;

                PaintBuffer buf = hwnd;

                prepare(buf, rclPaint, iBType);

                //First pass: calculate summary

                int iSum = 0;

                for(i = 0; i < 5; i++)
                {
                    if(iMailEnabled[i]/* && !iMailSep[i]*/)
                    {
                        //if(iSum < 0)
                        //    iSum = 0;
                        iSum += iMails[i];
                    }
                }

                //if(iSum >= 0)
                    sprintf(cBuff, "[%d]", iSum);
                //else
                //    cBuff[0] = 0;

                for(i = 0; i < 5; i++)
                {
                    if(iMailEnabled[i] && iMailSep[i])
                    {
                        sprintf(c0, "%d", iMails[i]);

                        if(cBuff[0])
                            strcat(cBuff, (cBuff[0] != '[') ? " : ":" ");

                        strcat(cBuff,c0);
                    }
                }

                buf.DrawText(cBuff, rclPaint, lColorFG);
            }
            return MRFROMSHORT(FALSE);

        case WM_BUTTON1DBLCLK:
            {
                TID aTh;
                DosCreateThread(&aTh, CheckMailOnce, hwnd, 0, 32768);
            }
            return MRFROMSHORT(FALSE);
    }
    return CommonProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY ProcMem(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg)
    {
        case WM_PAINT:
            {
                RECTL rclPaint;
                RECTL rclPaint2;
                CHAR  cBuff[256];
                CHAR  cBuff2[256];
                CHAR  cTmp[128];

                int   iBType = BAR_HORISONTAL;
                int   iTwoLines = 0;

                cBuff[0] = cBuff2[0] = 0;

                PaintBuffer buf = hwnd;

                prepare(buf, rclPaint, iBType);

                if((buf.TextCy("X") * 2) <= (rclPaint.yTop - rclPaint.yBottom))
                	iTwoLines = 1;

                if(iRAMText)
                {
                    if(iRAMAdaptive && (uFreeRAM/1024) > 9999)
                        sprintf(cBuff, "%.1fM", double(uFreeRAM)/double(1024*1024));
                    else
                        sprintf(cBuff, "%dK", uFreeRAM/1024);
                }

                if(iRAMUptime)
                {
                    if(iRAMText)
                        strcat(cBuff, " ");

                    sprintf(cTmp, "%u:%02u:%02u:%02u",
                            Uptime.uDays, Uptime.uHours,
                            Uptime.uMins, Uptime.uSeconds);

                    if(iTwoLines)
                        strcpy(cBuff2, cTmp);
                    else
                    {
                        strcat(cBuff, " ");
                        strcat(cBuff, cTmp);
                    }
                }

                if(iRAMClock)
                {
                    DATETIME dt;

                    DosGetDateTime(&dt);

                    if(iRAMText || iRAMUptime)
                        strcat(cBuff, " ");

                    sprintf(cTmp,"%02u:%02u:%02u",
                            dt.hours, dt.minutes, dt.seconds);
                    strcat(cBuff, cTmp);
                }

                if(!cBuff2[0])
                    buf.DrawText(cBuff, rclPaint, lColorFG);
                else
                {
                    HalfRect(rclPaint , rclPaint2, QUAD_TOP);
                    buf.DrawText(cBuff, rclPaint2, lColorFG);

                    HalfRect(rclPaint , rclPaint2, QUAD_BOTTOM);
                    buf.DrawText(cBuff2, rclPaint2, lColorFG);
                }
            }
            return MRFROMSHORT(FALSE);
    }
    return CommonProc(hwnd, msg, mp1, mp2);
}

