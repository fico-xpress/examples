#**************************************************
# Mosel examples makefile                         *
#                                                 *
# (c) 2008 Fair Isaac Corporation                 *
#     author: Y. Colombani, 2001, rev. July 2026  *
#**************************************************
.SUFFIXES: .bim .mos .java .class

XPLIBS="$(XPRESSDIR)\lib\xprm.lib"

all: annotdisplay.exe annotdisplay.class

.c.exe:
	$(CC) -nologo -Zi -MD "-I$(XPRESSDIR)\include" $(CFLAGS) $< $(XPLIBS)

.mos.bim:
	mosel compile -g -D "$*"

.java.class:	
	javac -classpath "$(XPRESSDIR)/lib/xprm.jar";. $*.java

clean:
	del *.exe
	del *.obj
	del *.class
