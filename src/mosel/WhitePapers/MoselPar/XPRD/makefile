#**********************************
# XPRD examples makefile          *
#                                 *
# (c) 2024 Fair Isaac Corporation *
#     author: Y. Colombani, 2012  *
#**********************************
.SUFFIXES: .java .class

all: allC allJ

allC:runelsd.exe runrtdistr.exe runrtpardistr.exe runfoliodistr.exe runrtparqueued.exe distfolio.exe distfoliopar.exe distfoliocbioev.exe findservers.exe
allJ:mandelbrot.class runelsd.class runrtdistr.class runrtpardistr.class runfoliodistr.class runrtparqueued.class distfolio.class distfoliopar.class distfoliocbioev.class findservers.class

XCFLAGS=-nologo -MD -GS- -I"$(XPRESSDIR)\include"

.java.class:
	javac -classpath "$(XPRESSDIR)/lib/xprd.jar;$(XPRESSDIR)/lib/bindrv.jar";. $*.java

runelsd.exe: runelsd.c
	$(CC) $(XCFLAGS) $(CFLAGS) runelsd.c "$(XPRESSDIR)\lib\xprd.lib"  "$(XPRESSDIR)\lib\bindrvMD.lib"

runrtdistr.exe: runrtdistr.c
	$(CC) $(XCFLAGS) $(CFLAGS) runrtdistr.c "$(XPRESSDIR)\lib\xprd.lib"

runrtpardistr.exe: runrtpardistr.c
	$(CC) $(XCFLAGS) $(CFLAGS) runrtpardistr.c "$(XPRESSDIR)\lib\xprd.lib"

runrtparqueued.exe: runrtparqueued.c
	$(CC) $(XCFLAGS) $(CFLAGS) runrtparqueued.c "$(XPRESSDIR)\lib\xprd.lib"

runfoliodistr.exe: runfoliodistr.c
	$(CC) $(XCFLAGS) $(CFLAGS) runfoliodistr.c "$(XPRESSDIR)\lib\xprd.lib" "$(XPRESSDIR)\lib\bindrvMD.lib"

distfolio.exe: distfolio.c
	$(CC) $(XCFLAGS) $(CFLAGS) distfolio.c "$(XPRESSDIR)\lib\xprd.lib" "$(XPRESSDIR)\lib\bindrvMD.lib"

distfoliopar.exe: distfoliopar.c
	$(CC) $(XCFLAGS) $(CFLAGS) distfoliopar.c "$(XPRESSDIR)\lib\xprd.lib" "$(XPRESSDIR)\lib\bindrvMD.lib"

distfoliocbioev.exe: distfoliocbioev.c
	$(CC) $(XCFLAGS) $(CFLAGS) distfoliocbioev.c "$(XPRESSDIR)\lib\xprd.lib" "$(XPRESSDIR)\lib\bindrvMD.lib"

findservers.exe: findservers.c
	$(CC) $(XCFLAGS) $(CFLAGS) findservers.c "$(XPRESSDIR)\lib\xprd.lib" ws2_32.lib

clean:
	del *.exe
	del *.obj
	del *.bim
	del *.class
