/*
** Module   :SETTINGS.H
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sat  16/05/1998 Created
**      Wed  15/07/1998 Updated
*/

#ifndef __MAIN_H
#define __MAIN_H

#define CLR_TEXT    1
#define CLR_BACK    2
#define CLR_BAR     3
#define CLR_GRAPH1  4
#define CLR_GRAPH2  5
#define CLR_GRID    6

#define CLR_START   CLR_TEXT
#define CLR_END     CLR_GRID

void GetAndSetFile(HWND, int, char *, char *);
void DoSettings(HWND, int);
void SetPalette(int i);
void FillPalette(LONG* alTable);

#endif  /*__MAIN_H*/

