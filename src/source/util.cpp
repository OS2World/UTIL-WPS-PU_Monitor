/*
** Module   :UTIL.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
** Log: Wed  10/03/1998     Created
**      Thu  25/06/1998     Updated
**      Wed  15/07/1998     Updated
*/

#define INCL_DOSPROCESS
#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#include <os2.h>
#include <string.h>
#include <stdio.h>
#include <base.h>
#include <cvars.h>
#include <util.h>

/* Utility */

int GetMax(int *pValue, int iFilled)
{
    int i;
    int iMax = 0;

    for(i = 0; i < iFilled; i++)
        if(iMax < pValue[i])
            iMax = pValue[i];

    return (iMax > 0) ? iMax:1;
}

void GetPPFont(HWND hwnd, char *buff)
{
    ULONG AttrFound;
    BYTE  AttrValue[128];
    ULONG cbRetLen;

    cbRetLen = WinQueryPresParam(hwnd,
                                 PP_FONTNAMESIZE,
                                 0,
                                 &AttrFound,
                                 sizeof(AttrValue),
                                 AttrValue,
                                 QPF_NOINHERIT);

    if(PP_FONTNAMESIZE == AttrFound && cbRetLen)
    {
        memcpy(buff, AttrValue, cbRetLen);
    }
}

void SetPPFont(HWND hwnd, char *buff)
{
    WinSetPresParam(hwnd,
                    PP_FONTNAMESIZE,
                    strlen(buff) + 1,
                    buff);
}

void CheckMenuItem(HWND hwndMenu, USHORT ulID)
{
    WinSendMsg(hwndMenu,
               MM_SETITEMATTR,
               (MPARAM)ulID,
               MPFROM2SHORT(MIA_CHECKED, MIA_CHECKED));
}

void CheckSubMenuItem(HWND hwndMenu, USHORT usIDparent, USHORT usID)
{
    MENUITEM mi;

    WinSendMsg(hwndMenu,
               MM_QUERYITEM,
               MPFROM2SHORT(usIDparent, TRUE),
               (MPARAM)&mi);

    hwndMenu = mi.hwndSubMenu;

    WinSendMsg(hwndMenu,
               MM_SETITEMATTR,
               (MPARAM)usID,
               MPFROM2SHORT(MIA_CHECKED, MIA_CHECKED));
}

void AddMenuItem(HWND hwndMenu,
                 USHORT usIDparent,
                 USHORT usIDitem,
                 PSZ pszMenuText, int iChecked)
{
    MENUITEM mi;

    WinSendMsg(hwndMenu,
               MM_QUERYITEM,
               MPFROM2SHORT(usIDparent, TRUE),
               (MPARAM)&mi);

    hwndMenu = mi.hwndSubMenu;

    mi.iPosition   = MIT_END;
    mi.afStyle     = MIS_TEXT;
    mi.afAttribute = (iChecked) ? MIA_CHECKED : 0;
    mi.id          = usIDitem;
    mi.hwndSubMenu = 0;
    mi.hItem       = 0;

    WinSendMsg(hwndMenu,
               MM_INSERTITEM,
               (MPARAM) &mi,
               (MPARAM) pszMenuText);
}

void GetIniName(char *cName)
{
    PTIB   pTib;
    PPIB   pPib;
    APIRET rc;
    char cProxName[261];

    rc = DosGetInfoBlocks(&pTib, &pPib);
    rc = DosQueryModuleName(pPib->pib_hmte, sizeof(cProxName), cProxName);

    if(rc)
        return;

    if(cProxName[0])
    {
        strcpy(cName, cProxName);

        char* lastdot   = strrchr(cName, '.');
        char* lastslash = strrchr(cName, '\\');

        if(lastdot > lastslash) //dot really in extension, not in path
            strcpy(lastdot, ".ini");
        else
            strcat(cName, ".ini");
    }
}

void RunProg(char *prog, char *parm)
{
    PROGDETAILS Details;
    char   cArgs[1024];
    char   cPath[CCHMAXPATH];

    strcpy(cPath, prog);
    char* lastslash = strrchr(cPath, '\\');

    if(lastslash)
        *lastslash = 0;
    else
        cPath[0] = 0;


    Details.Length                      = sizeof(PROGDETAILS);
    Details.progt.progc                 = PROG_DEFAULT;
    Details.progt.fbVisible             = SHE_VISIBLE;
    Details.pszTitle                    = 0;
    Details.pszExecutable               = prog;
    Details.pszParameters               = parm;
    Details.pszStartupDir               = cPath;
    Details.pszIcon                     = 0;
    Details.pszEnvironment              = 0;
    Details.swpInitial.fl               = SWP_ACTIVATE;        /* Window positioning   */
    Details.swpInitial.cy               = 0;                   /* Width of window      */
    Details.swpInitial.cx               = 0;                   /* Height of window     */
    Details.swpInitial.y                = 0;                   /* Lower edge of window */
    Details.swpInitial.x                = 0;                   /* Left edge of window  */
    Details.swpInitial.hwndInsertBehind = HWND_TOP;
    Details.swpInitial.hwnd             = 0;
    Details.swpInitial.ulReserved1      = 0;
    Details.swpInitial.ulReserved2      = 0;

    WinStartApp(0, &Details, 0, 0, SAF_INSTALLEDCMDLINE);

}

void CopyFontSettings(HPS hpsSrc, HPS hpsDst)
{
    FONTMETRICS fm;
    FATTRS fat;
    SIZEF sizf;

    GpiQueryFontMetrics(hpsSrc, sizeof(FONTMETRICS), &fm);

    memset(&fat, 0, sizeof(fat));

    fat.usRecordLength  = sizeof(FATTRS);
    fat.lMatch          = fm.lMatch;
    strcpy(fat.szFacename, fm.szFacename);

    GpiCreateLogFont(hpsDst, 0, 1L, &fat);
    GpiSetCharSet(hpsDst, 1L);

    sizf.cx = MAKEFIXED(fm.lEmInc,0);
    sizf.cy = MAKEFIXED(fm.lMaxBaselineExt,0);
    GpiSetCharBox(hpsDst, &sizf );
}

void DrawBar(HPS hpsBuffer, RECTL rclPaint, ULONG ulValue,
             ULONG ulMax, int iType, LONG lColor)
{
    ULONG ulFilled;
    RECTL rclPaint2;

    if(ulValue > ulMax)
        ulValue = ulMax;

    if(iType == BAR_HORISONTAL)
        ulFilled  = (ULONG)((double)(rclPaint.xRight - rclPaint.xLeft) *
                            ((double)ulValue/(double)ulMax));
    else
        ulFilled  = (ULONG)((double)(rclPaint.yTop - rclPaint.yBottom) *
                            ((double)ulValue/(double)ulMax));
    if (ulFilled < 3)
        ulFilled = 3;

    if(iType == BAR_HORISONTAL)
        rclPaint.xRight = rclPaint.xLeft + ulFilled;
    else
        rclPaint.yTop   = rclPaint.yBottom + ulFilled;

    rclPaint2 = rclPaint; rclPaint2.yBottom = rclPaint2.yTop - 1;
    WinFillRect(hpsBuffer, &rclPaint2, CLR_WHITE);

    rclPaint2 = rclPaint; rclPaint2.xRight = rclPaint2.xLeft + 1;
    WinFillRect(hpsBuffer, &rclPaint2, CLR_WHITE);

    rclPaint2 = rclPaint; rclPaint2.xLeft = rclPaint2.xRight - 1;
    WinFillRect(hpsBuffer, &rclPaint2, CLR_BLACK);

    rclPaint2 = rclPaint; rclPaint2.yTop = rclPaint2.yBottom + 1;
    WinFillRect(hpsBuffer, &rclPaint2, CLR_BLACK);

    rclPaint.yTop    -= 1; rclPaint.xRight  -= 1;
    rclPaint.xLeft   += 1; rclPaint.yBottom += 1;

    WinFillRect(hpsBuffer, &rclPaint, lColor);
}

/* Paints bar as two rectange with different colors (NK) */
void DrawDoubleBar(HPS hpsBuffer, RECTL rclPaint, ULONG ulValue1, ULONG ulValue2,
             ULONG ulMax, int iType, LONG lColor1, LONG lColor2)
{
    ULONG ulFilled1,ulFilled2;
    RECTL rclPaint2,orgPaint;

    if(ulValue1 > ulMax)
        ulValue1 = ulMax;
    if(ulValue2 > ulMax)
        ulValue2 = ulMax;

    if(iType == BAR_HORISONTAL)
    {
        ulFilled1  = (ULONG)((double)(rclPaint.xRight - rclPaint.xLeft) *
                            ((double)ulValue1/(double)ulMax));
        ulFilled2  = (ULONG)((double)(rclPaint.xRight - rclPaint.xLeft) *
                            ((double)ulValue2/(double)ulMax));
    }
    else
    {
        ulFilled1  = (ULONG)((double)(rclPaint.yTop - rclPaint.yBottom) *
                            ((double)ulValue1/(double)ulMax));
        ulFilled2  = (ULONG)((double)(rclPaint.yTop - rclPaint.yBottom) *
                            ((double)ulValue2/(double)ulMax));
    }
    if (ulFilled1 < 3) ulFilled1 = 3;
    if (ulFilled2 < 3) ulFilled2 = 3;

    orgPaint = rclPaint;
    if(iType == BAR_HORISONTAL)
        rclPaint.xRight = rclPaint.xLeft + ulFilled1;
    else
        rclPaint.yTop   = rclPaint.yBottom + ulFilled1;

    rclPaint2 = rclPaint; rclPaint2.yBottom = rclPaint2.yTop - 1;
    WinFillRect(hpsBuffer, &rclPaint2, CLR_WHITE);

    rclPaint2 = rclPaint; rclPaint2.xRight = rclPaint2.xLeft + 1;
    WinFillRect(hpsBuffer, &rclPaint2, CLR_WHITE);

    rclPaint2 = rclPaint; rclPaint2.xLeft = rclPaint2.xRight - 1;
    WinFillRect(hpsBuffer, &rclPaint2, CLR_BLACK);

    rclPaint2 = rclPaint; rclPaint2.yTop = rclPaint2.yBottom + 1;
    WinFillRect(hpsBuffer, &rclPaint2, CLR_BLACK);

    rclPaint.yTop    -= 1; rclPaint.xRight  -= 1;
    rclPaint.xLeft   += 1; rclPaint.yBottom += 1;

    WinFillRect(hpsBuffer, &rclPaint, lColor1);

    /* second rectangle */
    rclPaint = orgPaint;
    if(iType == BAR_HORISONTAL)
    {
        rclPaint.xLeft  = orgPaint.xLeft + ulFilled1;
        rclPaint.xRight = rclPaint.xLeft + ulFilled2;
    }
    else
    {
        rclPaint.yBottom = orgPaint.yBottom + ulFilled1;
        rclPaint.yTop    = rclPaint.yBottom + ulFilled2;
    }
    rclPaint2 = rclPaint; rclPaint2.yBottom = rclPaint2.yTop - 1;
    WinFillRect(hpsBuffer, &rclPaint2, CLR_WHITE);

    rclPaint2 = rclPaint; rclPaint2.xRight = rclPaint2.xLeft + 1;
    WinFillRect(hpsBuffer, &rclPaint2, CLR_WHITE);

    rclPaint2 = rclPaint; rclPaint2.xLeft = rclPaint2.xRight - 1;
    WinFillRect(hpsBuffer, &rclPaint2, CLR_BLACK);

    rclPaint2 = rclPaint; rclPaint2.yTop = rclPaint2.yBottom + 1;
    WinFillRect(hpsBuffer, &rclPaint2, CLR_BLACK);

    rclPaint.yTop    -= 1; rclPaint.xRight  -= 1;
    rclPaint.xLeft   += 1; rclPaint.yBottom += 1;

    WinFillRect(hpsBuffer, &rclPaint, lColor2);
}

void HalfRect(RECTL& prclSource, RECTL& prclDest, int quadrant)
{
    LONG lDx;
    LONG lDy;

    prclDest.yTop    = prclSource.yTop   ;
    prclDest.yBottom = prclSource.yBottom;
    prclDest.xLeft   = prclSource.xLeft  ;
    prclDest.xRight  = prclSource.xRight ;

    lDx = (prclDest.xRight - prclDest.xLeft)/2 + 1;
    lDy = (prclDest.yTop - prclDest.yBottom)/2 + 1;

    switch(quadrant)
    {
        case QUAD_TOP:    prclDest.yBottom  += lDy; break;
        case QUAD_BOTTOM: prclDest.yTop     -= lDy; break;
        case QUAD_LEFT:   prclDest.xRight   -= lDx; break;
        case QUAD_RIGHT:  prclDest.xLeft    += lDx; break;
    }
}

void ScaledPrint(char *cBuff, ULONG value)
{
    if(value >= (1024*1024))
        sprintf(cBuff, "%.1fM", ((double)(value))/(1024.0*1024.0));
    else
    {
        if(value >= 1024)
            sprintf(cBuff, "%.1fK", ((double)(value))/1024.0);
        else
            sprintf(cBuff, "%u", value);
    }
}

HWND GetWindowAboveWarpCenter(HWND *pWC)
{
    ULONG uItems, uBufSize;
    PSWBLOCK pSWB;
    HWND hWnd = HWND_TOP;
    int i;

    uItems   = WinQuerySwitchList(0, 0, 0);
    uBufSize = (uItems * sizeof(SWENTRY)) + sizeof(HSWITCH);

    pSWB = (PSWBLOCK ) new char[uBufSize];

    if(!pSWB)
        return hWnd;

    WinQuerySwitchList(0, pSWB, uBufSize);

    for (i = 0; i <= pSWB->cswentry; i++)
    {
        //if (pSWB->aswentry[i].swctl.uchVisibility == SWL_VISIBLE)
        {
            if(!stricmp(pSWB->aswentry[i].swctl.szSwtitle, "WarpCenter") ||
               !stricmp(pSWB->aswentry[i].swctl.szSwtitle, "SmartCenter"))
            {
                if(pWC)
                    *pWC = pSWB->aswentry[i].swctl.hwnd;
                hWnd = WinQueryWindow(pSWB->aswentry[i].swctl.hwnd, QW_PREV);
                if(!hWnd)
                    hWnd = HWND_TOP;
                break;
            }
        }
    }

    delete pSWB;

    return hWnd;
}

void TextBounds(HPS hpsTemp, char *cText, LONG *lCx, LONG *lCy)
{
    POINTL txtPointl[TXTBOX_COUNT];
    GpiQueryTextBox(hpsTemp, strlen(cText), cText, TXTBOX_COUNT, txtPointl);

    if(lCx)
        *lCx = txtPointl[TXTBOX_TOPRIGHT].x - txtPointl[TXTBOX_TOPLEFT   ].x;
    if(lCy)
    	*lCy = txtPointl[TXTBOX_TOPLEFT ].y - txtPointl[TXTBOX_BOTTOMLEFT].y;
}

MRESULT _inSendMsg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    return WinSendMsg(hwnd, msg, mp1, mp2);
}

int ThInit(HAB& hab, HMQ& hmq)
{
    hab = WinInitialize(0);

    if(!hab)
        return -1;

    hmq = WinCreateMsgQueue(hab, 0);

    if(!hmq)
    {
        WinTerminate(hab);
        return -1;
    }
    return 0;
}

void ThTerm(HAB hab, HMQ hmq)
{
	WinDestroyMsgQueue(hmq);
    WinTerminate(hab);
}

void Sleep(int iDelay)
{
    DosSleep(iDelay);
}

void aRect(HPS hpsPaint, RECTL rclPaint, int iMode)
{
    POINTL ptlWork;

    GpiSetColor(hpsPaint, (iMode) ? CLR_BLACK : CLR_WHITE);

    ptlWork.x = rclPaint.xLeft;
    ptlWork.y = rclPaint.yBottom;
    GpiMove(hpsPaint, &ptlWork);
    ptlWork.y = rclPaint.yTop;
    GpiLine(hpsPaint, &ptlWork);
    ptlWork.x = rclPaint.xRight;
    GpiLine(hpsPaint, &ptlWork);

    GpiSetColor(hpsPaint, (!iMode) ? CLR_BLACK : CLR_WHITE);

    ptlWork.y = rclPaint.yBottom;
    GpiLine(hpsPaint, &ptlWork);
    ptlWork.x = rclPaint.xLeft;
    GpiLine(hpsPaint, &ptlWork);
}

static void APIENTRY thPlay(ULONG data)
{
    PSound pSound = (PSound)data;

    if(!data)
        return;

    for(int i = 0; pSound[i].iFreq && pSound[i].iDur; i++)
    {
        if(pSound[i].iFreq == 1)
            DosSleep(pSound[i].iDur);
        else
            DosBeep(pSound[i].iFreq, pSound[i].iDur);
    }
}

void Play(PSound pSound)
{
    TID tPlay;

    if(pSound)
        DosCreateThread(&tPlay, thPlay, (ULONG)pSound, 0, 8192);
}

//----------------------------------------------------------------------
//

PaintBuffer::PaintBuffer(HWND hwnd)
{
    RECTL rectl;
    HPS src;

    src = WinBeginPaint(hwnd, 0, 0);
    WinQueryWindowRect(hwnd, &rectl);

    iEndPaint = 1;

    init(rectl, src);
}

PaintBuffer::PaintBuffer(HWND hwnd, HPS src)
{
    RECTL rectl;

    WinQueryWindowRect(hwnd, &rectl);

    iEndPaint = 0;

    init(rectl, src);
}

int PaintBuffer::init(RECTL rectl, HPS src)
{
    LONG cPlanes;
    LONG cBitCount;
    ULONG ulFlags;
    HBITMAP hbm;
    BITMAPINFOHEADER bmp;
    SIZEL sizl;
    FONTMETRICS fm;
    FATTRS fat;
    SIZEF sizf;

    rclPaint = rectl;
    hps      = src;

    hdc     = GpiQueryDevice(hps);
    ulFlags = GpiQueryPS(hps, &sizl);

    hdcBuffer = DevOpenDC(0, OD_MEMORY, "*", 0L, NULL, hdc);
    hpsBuffer = GpiCreatePS(0, hdcBuffer, &sizl, ulFlags | GPIA_ASSOC);

    DevQueryCaps(hdc, CAPS_COLOR_PLANES  , 1L, &cPlanes);
    DevQueryCaps(hdc, CAPS_COLOR_BITCOUNT, 1L, &cBitCount);

    bmp.cbFix     = sizeof(BITMAPINFOHEADER);
    bmp.cx        = (SHORT)(rclPaint.xRight - rclPaint.xLeft);
    bmp.cy        = (SHORT)(rclPaint.yTop - rclPaint.yBottom);
    bmp.cPlanes   = (SHORT)cPlanes;
    bmp.cBitCount = (SHORT)cBitCount;

    hbm = GpiCreateBitmap(hpsBuffer,
                          (PBITMAPINFOHEADER2)&bmp,
                          0x0000,
                          (PBYTE)NULL,
                          (PBITMAPINFO2)NULL);

    GpiSetBitmap(hpsBuffer, hbm);

    GpiQueryFontMetrics(hps, sizeof(FONTMETRICS), &fm);

    memset(&fat, 0, sizeof(fat));

    fat.usRecordLength  = sizeof(FATTRS);
    fat.lMatch          = fm.lMatch;
    strcpy(fat.szFacename, fm.szFacename);

    GpiCreateLogFont(hpsBuffer, 0, 1L, &fat);
    GpiSetCharSet(hpsBuffer, 1L);

    sizf.cx = MAKEFIXED(fm.lEmInc,0);
    sizf.cy = MAKEFIXED(fm.lMaxBaselineExt,0);
    GpiSetCharBox(hpsBuffer, &sizf );

    return 0;
}

void PaintBuffer::SetPal(LONG *aPal, int iStart, int iEnd)
{
    LONG alTable[16];

    GpiQueryLogColorTable(hpsBuffer, 0L, 0L, 16, alTable);

    if(aPal)
        for(int k = iStart ; k < 16 && k <= iEnd ; k++)
            alTable[k] = aPal[k];

    GpiCreateLogColorTable(hpsBuffer, 0L, LCOLF_CONSECRGB, 0L, 16, alTable);
}

PaintBuffer::~PaintBuffer()
{
    HBITMAP hbm;
    POINTL aptl[3];

    aptl[0].x = 0;
    aptl[0].y = 0;
    aptl[1].x = rclPaint.xRight - rclPaint.xLeft;
    aptl[1].y = rclPaint.yTop - rclPaint.yBottom;
    aptl[2].x = 0;
    aptl[2].y = 0;

    GpiBitBlt(hps, hpsBuffer, 3L, aptl, ROP_SRCCOPY, BBO_IGNORE);

    hbm = GpiSetBitmap (hpsBuffer, NULLHANDLE);

    if (hbm != NULLHANDLE)
        GpiDeleteBitmap (hbm);

    GpiDestroyPS(hpsBuffer);
    DevCloseDC  (hdcBuffer);

    if(iEndPaint)
        WinEndPaint(hps);
}

void PaintBuffer::DrawText(char *cText, RECTL& aRect, int iColor, int iAlign)
{
    if(i3DFont)
    {
	    RECTL aRect1 = aRect;
        aRect1.yTop    -= 1;
        aRect1.yBottom -= 1;
        aRect1.xLeft   += 1;
        aRect1.xRight  += 1;
        WinDrawText(hpsBuffer, -1, cText, &aRect1, CLR_WHITE, 0, iAlign);
    }
    WinDrawText(hpsBuffer, -1, cText, &aRect, iColor, 0, iAlign);
}

int PaintBuffer::TextCx(char *cText)
{
    POINTL txtPointl[TXTBOX_COUNT];
    GpiQueryTextBox(hpsBuffer, strlen(cText), cText, TXTBOX_COUNT, txtPointl);

    return txtPointl[TXTBOX_TOPRIGHT].x - txtPointl[TXTBOX_TOPLEFT].x;
}

int PaintBuffer::TextCy(char *cText)
{
    POINTL txtPointl[TXTBOX_COUNT];
    GpiQueryTextBox(hpsBuffer, strlen(cText), cText, TXTBOX_COUNT, txtPointl);

    return txtPointl[TXTBOX_TOPLEFT].y - txtPointl[TXTBOX_BOTTOMLEFT].y;
}

void PaintBuffer::DrawGrid(RECTL rclPaint, int iStart, int lColorGD)
{
    int i;
    POINTL ptlStart;
    POINTL ptlEnd;

//Draw grid

    //Horizontal
    ptlStart.x = rclPaint.xLeft  ;
    ptlEnd  .x = rclPaint.xRight ;

    ptlStart.y = rclPaint.yBottom + (GRID_STEP * 3)/2;
    ptlEnd  .y = ptlStart.y;

    GpiSetColor(hpsBuffer, lColorGD);

    for(i = 0; ptlStart.y <= rclPaint.yTop; i++)
    {
        GpiMove(hpsBuffer, &ptlStart);
        GpiLine(hpsBuffer, &ptlEnd);
        ptlStart.y += GRID_STEP * 3;
        ptlEnd  .y += GRID_STEP * 3;
    }

    //Vertical
    ptlStart.x = rclPaint.xRight - (iStart)*4;
    ptlEnd  .x = ptlStart.x;

    ptlStart.y = rclPaint.yTop   ;
    ptlEnd  .y = rclPaint.yBottom;

    for(i = 0; ptlStart.x >= rclPaint.xLeft; i++)
    {
        GpiMove(hpsBuffer, &ptlStart);
        GpiLine(hpsBuffer, &ptlEnd);
        ptlStart.x -= GRID_STEP * 4;
        ptlEnd  .x -= GRID_STEP * 4;
    }
}

void PaintBuffer::DrawGraph(RECTL rclPaint,
                            int *pValues, int iMax, int iFilled,
                            int lColorG1)
{
    int i;
    int iLen;
    int iHig;
    POINTL *ptGraph;

    iHig = rclPaint.yTop - rclPaint.yBottom - 1;
    iLen = (rclPaint.xRight - rclPaint.xLeft)/GRID_STEP + 1;

    if(iLen <= 0)
        return;

    ptGraph = new POINTL[iLen+1];

    GpiSetColor(hpsBuffer, lColorG1);

    for(i = 0; i < iLen ; i++)
    {
        int iVal;
        int iShift;

        if(i < iFilled)
            iVal = pValues[i];
        else
            iVal = 0;

        iShift = (ULONG)((double)(iHig) * ((double)iVal/(double)iMax));

        ptGraph[i].y = rclPaint.yBottom + iShift + 1;
        ptGraph[i].x = rclPaint.xRight  - GRID_STEP*i;
    }

    GpiMove(hpsBuffer, &ptGraph[0]);
    GpiPolyLine(hpsBuffer, iLen-1, &ptGraph[1]);

    delete ptGraph;
}

void PaintBuffer::DrawDoubleGraph(RECTL rclPaint,
              int *pValue1, int *pValue2,
              int iMax1, int iMax2,
              int iFill, int iStart,
              int lColorGD, int lColorG1, int lColorG2)
{
    int iMaxIn  = 0;
    int iMaxOut = 0;

    int *iValArray;
    int *iIValues ;
    int *iOValues ;
    int *iIValues1;
    int *iOValues1;

    int iCx = rclPaint.xRight - rclPaint.xLeft;

    if(iCx <= 1)
        return;

    if(iCx < iFill)
        iFill = iCx;

    iValArray = new int [GRID_SIZE * 4];

    iIValues  = &iValArray[GRID_SIZE * 0];
    iOValues  = &iValArray[GRID_SIZE * 1];
    iIValues1 = &iValArray[GRID_SIZE * 2];
    iOValues1 = &iValArray[GRID_SIZE * 3];

    if(pValue1)
    {
        memcpy(iIValues , pValue1 , GRID_SIZE*sizeof(int));
        memcpy(iIValues1, iIValues, GRID_SIZE*sizeof(int));
    }
    if(pValue2)
    {
        memcpy(iOValues , pValue2 , GRID_SIZE*sizeof(int));
        memcpy(iOValues1, iOValues, GRID_SIZE*sizeof(int));
    }

    if(pValue1)
        iMaxIn = (iMax1 >= 0) ? iMax1: GetMax(iIValues, iFill);

    if(pValue2)
        iMaxOut = (iMax2 >= 0) ? iMax2 : GetMax(iOValues, iFill);

    if(!iMaxIn)
        iMaxIn = 1;
    if(!iMaxOut)
        iMaxOut = 1;

    iMaxOut = max(iMaxIn, iMaxOut);
    iMaxIn = iMaxOut;

    DrawGrid(rclPaint, iStart, lColorGD);

    if(pValue1)
        DrawGraph(rclPaint, iIValues, iMaxIn, iFill, lColorG1);

    if(pValue2)
        DrawGraph(rclPaint, iOValues, iMaxOut, iFill, lColorG2);

    delete iValArray;
}

/* Paints two grids in one plane with different colors. (NK) */
void PaintBuffer::DrawDoubleColorGraph(RECTL rclPaint, int *pValues1,
                                       int *pValues2,
                                       int iMax, int iFilled,
                                       int lColorG1, int lColorG2)
{
    int i;
    int iLen;
    int iHig;
    POINTL *ptGraph;

    iHig = rclPaint.yTop - rclPaint.yBottom - 1;
    iLen = (rclPaint.xRight - rclPaint.xLeft)/GRID_STEP + 1;

    if(iLen <= 0)
        return;

    ptGraph = new POINTL[iLen+1];

    GpiSetColor(hpsBuffer, lColorG1);

    for(i = 0; i < iLen ; i++)
    {
        int iVal;
        int iShift;

        if(i < iFilled)
            iVal = pValues1[i];
        else
            iVal = 0;

        iShift = (ULONG)((double)(iHig) * ((double)iVal/(double)iMax));

        ptGraph[i].y = rclPaint.yBottom + iShift + 1;
        ptGraph[i].x = rclPaint.xRight  - GRID_STEP*i;
    }

    GpiMove(hpsBuffer, &ptGraph[0]);
    GpiPolyLine(hpsBuffer, iLen-1, &ptGraph[1]);

    GpiSetColor(hpsBuffer, lColorG2);

    for(i = 0; i < iLen ; i++)
    {
        int iVal;
        int iShift;

        if(i < iFilled)
            iVal = pValues2[i];
        else
            iVal = 0;

        iShift = (ULONG)((double)(iHig) * ((double)iVal/(double)iMax));

        ptGraph[i].y = rclPaint.yBottom + iShift + 1;
        ptGraph[i].x = rclPaint.xRight  - GRID_STEP*i;
    }

    GpiMove(hpsBuffer, &ptGraph[0]);
    GpiPolyLine(hpsBuffer, iLen-1, &ptGraph[1]);

    delete ptGraph;
}

/* Paints four grids as pair of grids with different colors (NK) */
void PaintBuffer::DrawDoubleColorDoubleGraph(RECTL rclPaint,
                                        int *pValue1, int *piValue1,
                                        int *pValue2, int *piValue2,
                                        int iMax1, int iMax2,
                                        int iFill, int iStart,
                                        int lColorGD, int lColorG1, int lColorG2)
{
    int iMaxIn  = 0;
    int iMaxOut = 0;

    int *iValArray;
    int *iIValues ;
    int *iOValues ;
    int *iIValues1;
    int *iOValues1;
    int *pValArray;
    int *pIValues ;
    int *pOValues ;
    int *pIValues1;
    int *pOValues1;

    int iCx = rclPaint.xRight - rclPaint.xLeft;

    if(iCx <= 1)
        return;

    if(iCx < iFill)
        iFill = iCx;

    iValArray = new int [GRID_SIZE * 4];

    iIValues  = &iValArray[GRID_SIZE * 0];
    iOValues  = &iValArray[GRID_SIZE * 1];
    iIValues1 = &iValArray[GRID_SIZE * 2];
    iOValues1 = &iValArray[GRID_SIZE * 3];

    pValArray = new int [GRID_SIZE * 4];

    pIValues  = &pValArray[GRID_SIZE * 0];
    pOValues  = &pValArray[GRID_SIZE * 1];
    pIValues1 = &pValArray[GRID_SIZE * 2];
    pOValues1 = &pValArray[GRID_SIZE * 3];

    if(pValue1)
    {
        memcpy(iIValues , pValue1 , GRID_SIZE*sizeof(int));
        memcpy(pIValues , piValue1, GRID_SIZE*sizeof(int));
        memcpy(iIValues1, iIValues, GRID_SIZE*sizeof(int));
        memcpy(pIValues1, pIValues, GRID_SIZE*sizeof(int));
    }
    if(pValue2)
    {
        memcpy(iOValues , pValue2 , GRID_SIZE*sizeof(int));
        memcpy(pOValues , piValue2, GRID_SIZE*sizeof(int));
        memcpy(iOValues1, iOValues, GRID_SIZE*sizeof(int));
        memcpy(pOValues1, pOValues, GRID_SIZE*sizeof(int));
    }

    if(pValue1)
        iMaxIn = (iMax1 >= 0) ? iMax1: GetMax(iIValues, iFill);

    if(pValue2)
        iMaxOut = (iMax2 >= 0) ? iMax2 : GetMax(iOValues, iFill);

    if(!iMaxIn)
        iMaxIn = 1;
    if(!iMaxOut)
        iMaxOut = 1;

    iMaxOut = max(iMaxIn, iMaxOut);
    iMaxIn = iMaxOut;

    DrawGrid(rclPaint, iStart, lColorGD);

    if(pValue1)
        DrawDoubleColorGraph(rclPaint, iIValues, pIValues, iMaxIn, iFill, lColorG1, lColorG2);

    if(pValue2)
        DrawDoubleColorGraph(rclPaint, iOValues, pOValues, iMaxOut, iFill, lColorG1, lColorG2);

    delete iValArray;
    delete pValArray;
}

