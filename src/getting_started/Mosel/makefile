#************************************************
#  Getting Started
#  ===============
#  Makefile for Mosel examples
# 
#  (c) 2008 Fair Isaac Corporation
#      author: Y. Colombani, 2003, rev. July 2026
#************************************************
#
# Before using this makefile remember to set the environment variable
# XPRESSDIR to the directory where the DLL kit is installed on your computer.
# 
# This makefile will not build .NET examples.  For these, you should move into
# the example's subdirectory (e.g. "cd folioexec-cs") and type "dotnet build" to
# compile, or "dotnet run" to compile and execute.  Alternatively, the supplied
# project files can be opened from Visual Studio.
#
.SUFFIXES: .bim .mos .java .class

all: foliodata.bim folioexec.exe folioexec.class foliomat.exe foliomat.class folioobj.exe folioobj.class folioparam.exe folioparam.class foliorun.exe foliorun.class runfolio.exe runfolio.class runfolio2.exe runfolio2.class runfoliob.class runfoliod.exe runfoliod.class runfoliocbio.exe runfoliocbio.class usrpkg.bim

XPLIBS="$(XPRESSDIR)\lib\xprm.lib"

.c.exe:
	$(CC) -nologo -Zi -MD "-I$(XPRESSDIR)\include" $(CFLAGS) $< $(XPLIBS)

.mos.bim:
	mosel compile -g "$*"

.java.class:	
	javac -classpath "$(XPRESSDIR)/lib/xprm.jar";. $*.java

clean:
	del *.exe
	del *.obj
	del *.bim
	del *.class
