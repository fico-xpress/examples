#**********************************
# XPRD examples makefile          *
#                                 *
# (c) 2024 Fair Isaac Corporation *
#     author: Y. Colombani, 2012  *
#**********************************
.SUFFIXES: .java .class

all: allC allJ

allC:runprimedistr.exe runprimeiodistr.exe runprimeiodistr2.exe runprimeiodistr3.exe
allJ:runprimedistr.class runprimeiodistr.class runprimeiodistr2.class runprimeiodistr3.class

XCFLAGS=-nologo -MD -GS- -I"$(XPRESSDIR)\include"

.java.class:
	javac -classpath "$(XPRESSDIR)/lib/xprd.jar;$(XPRESSDIR)/lib/bindrv.jar";. $*.java

runprimedistr.exe: runprimedistr.c
	$(CC) $(XCFLAGS) $(CFLAGS) runprimedistr.c "$(XPRESSDIR)\lib\xprd.lib"

runprimeiodistr.exe: runprimeiodistr.c
	$(CC) $(XCFLAGS) $(CFLAGS) runprimeiodistr.c "$(XPRESSDIR)\lib\xprd.lib" "$(XPRESSDIR)\lib\bindrvMD.lib"

runprimeiodistr2.exe: runprimeiodistr2.c
	$(CC) $(XCFLAGS) $(CFLAGS) runprimeiodistr2.c "$(XPRESSDIR)\lib\xprd.lib" "$(XPRESSDIR)\lib\bindrvMD.lib"

runprimeiodistr3.exe: runprimeiodistr3.c
	$(CC) $(XCFLAGS) $(CFLAGS) runprimeiodistr3.c "$(XPRESSDIR)\lib\xprd.lib" "$(XPRESSDIR)\lib\bindrvMD.lib"

clean:
	del *.exe
	del *.obj
	del *.bim
	del *.class
