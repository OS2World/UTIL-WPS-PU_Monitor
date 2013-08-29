/*
** Module   :VARS.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sun  17/05/1998 Created
**      Thu  25/06/1998 Updated
**      Wed  15/07/1998 Updated
*/
#define INCL_PM
#define INCL_WIN
#define INCL_DOS
#include <os2.h>
#include <string.h>
#include <stdio.h>
#include <util.h>
#include <cell.h>
#include <id.h>

/**/
int iEncripted            =   0;
int iCurrPalette          =   4;
int iFloat                =   1;
int iLock                 =   0;
int iAttach               =   1;
int iDrawBorder           =   1;
int i3DFont               =   0;
int iBtLeft               =   3;
int iBtRight              =   1;
int iSplit1               =  50;
int iSplit2               =  80;
int iSplit3               =  70;
int iStartX               =  78;
#ifndef PRODUCTION
int iStartY               =  30;
#else
int iStartY               =   0;
#endif
int iSizeX                = 318;
int iSizeY                =  26;

int iCPUInterval          =  10;
int iCPUGraph             =   2;
int iCPUText              =   1;
int iCPUVar               =   1;
int iCPUKernel            =   0;

int iIPInterval           =  10;
int iIPVar                =   1;
int iIPGraphEnabled       =   2;
int iIPTotals             =   1;
int iIPCurrent            =   0;
int iIPAdaptive           =   1;

int iRAMInterval          =   1;
int iRAMUptime            =   0;
int iRAMClock             =   0;
int iRAMMethod            =   1;
int iRAMText              =   1;
int iRAMAdaptive          =   1;

int iConnRunProg          =   0;
int iConnBeep             =   0;
int iConnInterval         =  60;
int iBarType              = BAR_HORISONTAL;
int iSplitType0           =   CELL_VSPLIT;
int iSplitType1           =   CELL_VSPLIT;
int iSplitType2           =   CELL_VSPLIT;

char cConnRunProg [262];
char cConnProgParm[262];

char cFont1       [262];
char cFont2       [262];
char cFont3       [262];
char cFont4       [262];

int iMailSep[5]      = {0};
int iMailEnabled[5]  = {0};
int iMailLoop[5]     = {0};
int iMailBeep[5]     = {0};
int iMailRun[5]      = {0};
int iMailInterval[5] = {30, 30, 30, 30, 30} ;

static char cDT1[5][300];
static char cDT2[5][300];
static char cDT3[5][300];

static char cDT4[5][300] = {"your.domain.com", "your.domain.com", "your.domain.com", "your.domain.com", "your.domain.com"};
static char cDT5[5][300] = {"userid", "userid", "userid", "userid", "userid"};
static char cDT6[5][300] = {"password", "password", "password", "password", "password"};
static char cDT7[5][300] = {"110", "110", "110", "110", "110"};

char* cMailProgParm[5] = { cDT1[0], cDT1[1], cDT1[2], cDT1[3], cDT1[4]};
char* cMailProg    [5] = { cDT2[0], cDT2[1], cDT2[2], cDT2[3], cDT2[4]};
char* cMailRun     [5] = { cDT3[0], cDT3[1], cDT3[2], cDT3[3], cDT3[4]};
char* cMailServ    [5] = { cDT4[0], cDT4[1], cDT4[2], cDT4[3], cDT4[4]};
char* cMailUser    [5] = { cDT5[0], cDT5[1], cDT5[2], cDT5[3], cDT5[4]};
char* cMailPass    [5] = { cDT6[0], cDT6[1], cDT6[2], cDT6[3], cDT6[4]};
char* cMailPort    [5] = { cDT7[0], cDT7[1], cDT7[2], cDT7[3], cDT7[4]};

char *cConnSites   = 0;

unsigned uChecker = 0;
LONG lColorFG;
LONG lColorBG;
LONG lColorGR;
LONG lColorG1;
LONG lColorG2;
LONG lColorGD;

int iPal0 = 0x00000000; //Text
int iPal1 = 0x00CCCCCC; //Back
int iPal2 = 0x00808080; //Bar
//int iPal3 = 0x0000FFFF; //Graph1
int iPal3 = 0x00C8FF00; //Graph1
int iPal4 = 0x000000FF; //Graph2
int iPal5 = 0x00BEBEBE; //Grid

/**/

HMTX hmtxSites;

//char cFontDefault[] = "8.Helv";
char cFontDefault[] = "9.WarpSans Bold";

struct INIVars
{
    char *cName;
    void *pData;
};

//Type of variable depends from first letter of name
// i - means INTEGER
// c - means CHAR

struct INIVars variables[]=
{
    {"iEncripted",      &iEncripted },
    {"cConnRunProg",    cConnRunProg },
    {"cConnProgParm",   cConnProgParm },
    {"cFont1",          cFont1},
    {"cFont2",      	cFont2},
    {"cFont3",      	cFont3},
    {"cFont4",      	cFont4},

    {"iCurrPalette",    &iCurrPalette},
    {"iPal0",           &iPal0},
    {"iPal1",           &iPal1},
    {"iPal2",           &iPal2},
    {"iPal3",           &iPal3},
    {"iPal4",           &iPal4},
    {"iPal5",           &iPal5},
    {"iFloat",          &iFloat},
    {"iLock" ,          &iLock},
    {"iAttach" ,        &iAttach},
    {"iDrawBorder",     &iDrawBorder},
    {"i3DFont",         &i3DFont},
    {"iBtLeft",         &iBtLeft},
    {"iBtRight",        &iBtRight},
    {"iSplit1",         &iSplit1},
    {"iSplit2",     	&iSplit2},
    {"iSplit3",         &iSplit3},
    {"iStartX",         &iStartX},
    {"iStartY",     	&iStartY},
    {"iSizeX",      	&iSizeX},
    {"iSizeY",      	&iSizeY},

    {"iCPUInterval",    &iCPUInterval},
    {"iCPUGraph",       &iCPUGraph},
    {"iCPUText",        &iCPUText},
    {"iCPUVar",         &iCPUVar},
    {"iCPUKernel",      &iCPUKernel},
    {"iIPInterval",     &iIPInterval},
    {"iIPVar",          &iIPVar},
    {"iIPGraphEnabled", &iIPGraphEnabled},
    {"iIPTotals",       &iIPTotals},
    {"iIPCurrent",      &iIPCurrent},
    {"iIPAdaptive",     &iIPAdaptive},
    {"iRAMInterval", 	&iRAMInterval},
    {"iRAMUptime",  	&iRAMUptime},
    {"iRAMText",        &iRAMText},
    {"iRAMAdaptive",    &iRAMAdaptive},
    {"iConnRunProg",    &iConnRunProg},
    {"iConnBeep",   	&iConnBeep},
    {"iConnInterval", 	&iConnInterval},
    {"iRAMClock",       &iRAMClock},
    {"iRAMMethod",      &iRAMMethod},
    {"iSplitType0",     &iSplitType0},
    {"iSplitType1",     &iSplitType1},
    {"iSplitType2",     &iSplitType2},

    {"cMailProgParm",   cMailProgParm[0]},
    {"cMailProg",       cMailProg[0]},
    {"cMailRun",        cMailRun[0]},
    {"cMailServ",       cMailServ[0]},
    {"cMailUser",       cMailUser[0]},
    {"cMailPass",       cMailPass[0]},
    {"cMailPort",       cMailPort[0]},

    {"iMailEnabled",    &iMailEnabled[0]},
    {"iMailLoop",       &iMailLoop[0]},
    {"iMailBeep",       &iMailBeep[0]},
    {"iMailRun",        &iMailRun[0]},
    {"iMailInterval",   &iMailInterval[0]},

#define MK_PAIR(var,n)  {#var#n, var[n]}
#define MK_PAIR2(var,n) {#var#n, &var[n]}

#define MK_MAILVARS(n)  \
    MK_PAIR(cMailProgParm,n), \
    MK_PAIR(cMailProg,n),     \
    MK_PAIR(cMailRun ,n),     \
    MK_PAIR(cMailServ,n),     \
    MK_PAIR(cMailUser,n),     \
    MK_PAIR(cMailPass,n),     \
    MK_PAIR(cMailPort,n),     \
    MK_PAIR2(iMailSep,n),     \
    MK_PAIR2(iMailEnabled,n), \
    MK_PAIR2(iMailLoop,n),    \
    MK_PAIR2(iMailBeep,n),    \
    MK_PAIR2(iMailRun,n),     \
    MK_PAIR2(iMailInterval,n)

    MK_MAILVARS(1),
    MK_MAILVARS(2),
    MK_MAILVARS(3),
    MK_MAILVARS(4),

/*
    {"cMailProgParm1",  cMailProgParm[1]},
    {"cMailProg1",      cMailProg[1]},
    {"cMailRun1",       cMailRun[1]},
    {"cMailServ1",      cMailServ[1]},
    {"cMailUser1",      cMailUser[1]},
    {"cMailPass1",      cMailPass[1]},
    {"cMailPort1",      cMailPort[1]},

    {"iMailEnabled1",   &iMailEnabled[1]},
    {"iMailLoop1",      &iMailLoop[1]},
    {"iMailBeep1",      &iMailBeep[1]},
    {"iMailRun1",       &iMailRun[1]},
    {"iMailInterval1",  &iMailCheckingInterval[1]},
*/
    {0, 0}
};

char *pEncriptVars[]=
{
    cMailPass[0],
    cMailPass[1],
    cMailPass[2],
    cMailPass[3],
    cMailPass[4],
    0
};


HINI OpenINI(void)
{
    char cName[262];
    HINI hIni;

    GetIniName(cName);
    hIni = PrfOpenProfile(0, cName);
    return hIni;
}

void CloseINI(HINI hIni)
{
    PrfCloseProfile(hIni);
}

/* Restore all variables from INI file */

void EncriptVars()
{
    for(int i = 0; pEncriptVars[i]; i++)
    {
        char *str = pEncriptVars[i];

        while(*str)
        {
            char h = *str >> 4;
            char l = (*str & 0x0F) << 4;
            *str = l | h;
            str++;
        }
    }
}

void DecriptVars()
{
    for(int i = 0; pEncriptVars[i]; i++)
    {
        char *str = pEncriptVars[i];

        while(*str)
        {
            char h = *str >> 4;
            char l = (*str & 0x0F) << 4;
            *str = l | h;
            str++;
        }
    }
}

void LoadVars(void)
{
    HINI hIni;
    ULONG uMaxLen;
    BOOL rc;
    int i;

    strcpy(cFont1, cFontDefault);
    strcpy(cFont2, cFontDefault);
    strcpy(cFont3, cFontDefault);
    strcpy(cFont4, cFontDefault);

    hIni = OpenINI();

    if(!hIni)
        return;

    for(i = 0; variables[i].cName; i++)
    {
        if(variables[i].cName[0] == 'c')
        {
            uMaxLen = 261;
            ((char *)(variables[i].pData))[261] = 0;
        }
        else
            uMaxLen = sizeof(int);

        rc = PrfQueryProfileData(hIni,
                                 "PUMon",
                                 variables[i].cName,
                                 variables[i].pData,
                                 &uMaxLen);
    }

    /* */

    {
        char *pBuffer;

        uMaxLen = 0;

        rc = PrfQueryProfileSize(hIni,
                                 "PUMon",
                                 "cConnSites",
                                 &uMaxLen);
        if(rc == TRUE)
        {
            if(uMaxLen > (1024 * 262))
                uMaxLen = (1024 * 262);

            pBuffer = new char[uMaxLen + 1];

            if(pBuffer)
            {
                char *pTmp;

                pBuffer[uMaxLen] = 0;
                rc = PrfQueryProfileData(hIni,
                                         "PUMon",
                                         "cConnSites",
                                         pBuffer,
                                         &uMaxLen);

                DosRequestMutexSem(hmtxSites, (ULONG) SEM_INDEFINITE_WAIT);
                pTmp = cConnSites;
                cConnSites = pBuffer;
                DosReleaseMutexSem(hmtxSites);
                if(pTmp)
                    delete pTmp;
            }
        }
        else
        {
            char *pTmp;
            pBuffer = new char[1];
            pBuffer[0] = 0;

            DosRequestMutexSem(hmtxSites, (ULONG) SEM_INDEFINITE_WAIT);
            pTmp = cConnSites;
            cConnSites = pBuffer;
            DosReleaseMutexSem(hmtxSites);

            if(pTmp)
                delete pTmp;
        }
    }

    if(iEncripted)
    {
        //decript password and key.
        DecriptVars();
    }
    else
        iEncripted = 1;

    /* */

    CloseINI(hIni);
}

/* Save all variables to INI file */

void StoreVars(void)
{
    HINI hIni;
    int i;
    BOOL rc;


    hIni = OpenINI();
    if(!hIni)
        return;

    //encript password and key.
    EncriptVars();

    for(i = 0; variables[i].cName; i++)
    {
        rc = PrfWriteProfileData(hIni,
                                 "PUMon",
                                 variables[i].cName,
                                 variables[i].pData,
                                 (variables[i].cName[0] == 'c') ? 262:4);
    }

    //decript password and key.
    DecriptVars();

    {
        ULONG uMaxLen;

        DosRequestMutexSem(hmtxSites, (ULONG) SEM_INDEFINITE_WAIT);

        uMaxLen = strlen(cConnSites)+1;

        rc = PrfWriteProfileData(hIni,
                                 "PUMon",
                                 "cConnSites",
                                 cConnSites,
                                 uMaxLen);

        DosReleaseMutexSem(hmtxSites);
    }

    CloseINI(hIni);

}

