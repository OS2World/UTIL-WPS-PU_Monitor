/*
** Module   :BASE.H
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Wed  15/07/1998 Updated
**
*/

#ifndef  __BASE_H
#define  __BASE_H

#define VER_MAJ     2
#define VER_MID     1
#define VER_MIN     165

#ifndef min
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

typedef unsigned char Byte;
typedef unsigned short Word;
typedef unsigned long DWord;
typedef void *Ptr;
typedef Byte *PByte;
typedef Word *PWord;
typedef DWord *PDWord;
typedef char *CPtr;
typedef Word word;
typedef DWord dword;

#define CLASSPTR(Class) typedef Class* P##Class;
#define CLASSREF(Class) typedef Class& R##Class;
#define CLASSDEF(Class) class Class;\
                        CLASSPTR(Class)\
                        CLASSREF(Class)
#define STRUCDEF(Struc) struct Struc;\
                        CLASSPTR(Struc)\
                        CLASSREF(Struc)

#endif //__BASE_H

