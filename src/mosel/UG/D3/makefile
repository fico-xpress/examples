#**************************************************
# Mosel examples makefile                         *
#                                                 *
# (c) 2024 Fair Isaac Corporation                 *
#     author: Y. Colombani, 2001, rev. July 2026  *
#**************************************************
.SUFFIXES: .bim .mos .java .class

all: ugcompmem.exe ugcompmem.class ugcompfrmem.exe ugcompfrmem.class

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
	del *.class
