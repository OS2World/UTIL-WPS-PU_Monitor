# Makefile generated by BUILD.CMD V2.2.7
# 19 Feb 2002 at 13:13:11
# Mode VA

# Compiler
CC=icc -O+ -Oc+ -Ss+ -G5 -Q+ -Gm+ -I..\include -B"/ST:327680 /NOE /E:2" -B"/PMTYPE:PM"

# Librarian
LIBRARIAN=ilib /nofr /noi

# Resource compiler
RC=rc -i ..\include -x2

# Rules

.path.obj=.\obj
.path.lib=.\obj
.path.exe=.\obj
.path.dll=.\obj
.path.cpp=.\source
.path.res=.\obj
.path.rc=.\source
.path.c=.\source
.path.h=.\include
.path.hpp=.\include

.c.obj:
	@cd .\obj
	$(CC) -c .$<
	@cd ..

.cpp.obj:
	@cd .\obj
	$(CC) -c .$<
	@cd ..

.rc.res:
	@cd .\source
	$(RC) -r .$< .$@
	@cd ..

# Libary dependencies

OBJS = \
	cell.obj\
	ipstat.obj\
	itemproc.obj\
	ping.obj\
	settings.obj\
	stats.obj\
	util.obj\
	vars.obj

# Targets 
all :  pumon2.exe

COMMON.LIB : $(OBJS)
	@cd .\obj
	-@del COMMON.LIB >nul 2>nul
	@$(LIBRARIAN) COMMON.LIB @..\library.rsp;
	@cd ..

pumon2.exe : COMMON.LIB pumon2.obj pumon2.res
	@cd .\obj
	$(CC) @&&|
pumon2.obj COMMON.LIB   ..\source\si2.lib ..\source\win32k.lib so32dll.lib tcp32dll.lib pumon2.def
|
	@$(RC) pumon2.res pumon2.exe
	@cd ..


# Individual dependencies

cell.obj    : cell.h     cvars.h    util.h     cell.cpp
ipstat.obj  : cvars.h    ip_hdrs.h  ipstat.h   stats.h    util.h     vars.h     ipstat.cpp
itemproc.obj: cell.h     cvars.h    id.h       ip_hdrs.h  ipstat.h   settings.h stats.h    util.h     vars.h     itemproc.cpp
ping.obj    : ip_hdrs.h  ping.cpp
pumon2.obj  : cell.h     cvars.h    id.h       ip_hdrs.h  ipstat.h   settings.h stats.h    util.h     vars.h     win32k.h   pumon2.cpp
settings.obj: cell.h     cvars.h    id.h       ip_hdrs.h  ipstat.h   settings.h stats.h    vars.h     settings.cpp
stats.obj   : cell.h     cvars.h    id.h       ip_hdrs.h  ipstat.h   stats.h    util.h     vars.h     win32k.h   stats.cpp
util.obj    : base.h     cvars.h    util.h     util.cpp
vars.obj    : cell.h     id.h       util.h     vars.cpp
pumon2.res  : id.h       id2.h      pumon2.rc .\source\pumon2.ico .\source\pumon2.ico .\source\mailer.ico .\source\inet.ico .\source\tcpip.ico .\source\cpu.ico .\source\ram.ico

# End
