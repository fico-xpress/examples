# Windows makefile for Optimizer nonlinear examples
# Will build all C library examples if no target is specified

# The following variables control behavior and must be set (for example
# on the command line or as an environment variable)
#   XPRESSDIR  The root of the Xpress installation

# Usage: nmake [all]       Compiles all examples
#        nmake NAME.exe    Compiles NAME.c
#        nmake NAME.run    Runs example NAME (NAME.exe must exist).
#        nmake all.run     Build and run all examples

.SUFFIXES: .c .exe .run

all: *.c
	!$(MAKE) -nologo $(**B).exe
all.run: *.c
	!$(MAKE) -nologo $(**B).exe
	!$(MAKE) -nologo $(**B).run

CPPFLAGS=-nologo /I$(XPRESSDIR)/include
LIBS=$(XPRESSDIR)/lib/xprs.lib
LINKFLAGS=

.c.exe:
	$(CC) /W4 /WX $(CFLAGS) $(CPPFLAGS) $< /link $(LINKFLAGS) $(LIBS)

.exe.run:
	copy "$(XPRESSDIR)\bin\xprl.dll" xprl.dll
	copy "$(XPRESSDIR)\bin\xprs.dll" xprs.dll
	IF EXIST "$(XPRESSDIR)\bin\xknitro.dll" (copy "$(XPRESSDIR)\bin\xknitro.dll" xknitro.dll)
	IF EXIST "$(XPRESSDIR)\bin\libiomp5md.dll" (copy "$(XPRESSDIR)\bin\libiomp5md.dll" libiomp5md.dll)
	$*

clean:
	del *.exe 2>NUL
	del *.dll 2>NUL
