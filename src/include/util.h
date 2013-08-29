/*
** Module   :UTIL.H
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
** Log: Wed  10/03/1998 Created
**      Wed  15/07/1998 Updated
*/

#ifndef __UTIL_H
#define __UTIL_H

#define BAR_HORISONTAL  1
#define BAR_VERTICAL    2
#define QUAD_TOP    	0
#define QUAD_BOTTOM 	1
#define QUAD_LEFT   	2
#define QUAD_RIGHT  	3
#define GRID_STEP       3
#define GRID_SIZE       1024

typedef struct
{
    int iFreq;
    int iDur;
} Sound;

typedef Sound* PSound;

void Play(PSound pSound);
int GetMax(int *pValue, int iFilled);

void AddMenuItem(HWND hwndMenu,
                 USHORT usIDparent,
                 USHORT usIDitem,
                 PSZ pszMenuText, int iChecked);

void CheckMenuItem(HWND hwndMenu, USHORT usID);
void CheckSubMenuItem(HWND hwndMenu, USHORT usIDlarent, USHORT usID);
void GetPPFont(HWND hwnd, char *buff);
void SetPPFont(HWND hwnd, char *buff);
void GetIniName(char *cName);
void RunProg(char *prog, char *parm);
void CopyFontSettings(HPS hpsSrc, HPS hpsDst);
void DrawBar(HPS hpsBuffer, RECTL rclPaint, ULONG ulValue,
             ULONG ulMax, int iType, LONG lColor);
void DrawDoubleBar(HPS hpsBuffer, RECTL rclPaint, ULONG ulValue1, ULONG ulValue2,
             ULONG ulMax, int iType, LONG lColor1, LONG lColor2);
/*
void DrawGrid(HPS hpsBuffer, RECTL rclPaint,
              int *pValue1, int *pValue2,
              int iMax1, int iMax2,
              int iFill, int iStart,
              int lColorGD, int lColorG1, int lColorG2);
*/
void HalfRect(RECTL& prclSource, RECTL& prclDest, int quadrant);
void ScaledPrint(char *cBuff, ULONG value);
HWND GetWindowAboveWarpCenter(HWND*);
void TextBounds(HPS hpsTemp, char *cText, LONG *lCx, LONG *lCy);

void ThTerm(HAB hab, HMQ hmq);
int  ThInit(HAB& hab, HMQ& hmq);
void Sleep(int);

void aRect(HPS hpsPaint, RECTL rclPaint, int iMode);

//----------------------------------------------------------------------
//

class PaintBuffer
{
        HPS hps;
        HPS hpsBuffer;
        HDC hdc;
        HDC hdcBuffer;
        RECTL  rclPaint;

        int init(RECTL, HPS src);
        int iEndPaint;

    public:

        PaintBuffer(HWND hwnd);
        PaintBuffer(HWND hwnd, HPS hps);
        ~PaintBuffer();

        RECTL getRect() { return rclPaint;}
        HPS   getPS()   { return hpsBuffer;}

        void SetPal(LONG *aPal, int iStart, int iEnd);

        void DrawText(char *, RECTL&, int, int = DT_CENTER | DT_VCENTER);
        int TextCx(char *cText);
        int TextCy(char *cText);

        void DrawGrid(RECTL rclPaint, int iStart, int lColorGD);
        void DrawGraph(RECTL rclPaint, int *pValues, int iMax, int iFilled,
                       int lColorG1);

        void DrawDoubleGraph(RECTL rclPaint,
                             int *pValue1, int *pValue2,
                             int iMax1, int iMax2,
                             int iFill, int iStart,
                             int lColorGD, int lColorG1, int lColorG2);

        void DrawDoubleColorGraph(RECTL rclPaint, int *pValues1,
                                  int *pValues2,
                                  int iMax, int iFilled,
                                  int lColorG1, int lColorG2);

        void DrawDoubleColorDoubleGraph(RECTL rclPaint,
                                        int *pValue1, int *piValue1,
                                        int *pValue2, int *piValue2,
                                        int iMax1, int iMax2,
                                        int iFill, int iStart,
                                        int lColorGD, int lColorG1, int lColorG2);
};


#endif  /* __UTIL_H */

