#***********************************************
# Mosel examples makefile
#  
# (c) 2024 Fair Isaac Corporation
#     author: Y. Colombani, 2002, rev. July 2026
#***********************************************
.SUFFIXES: .bim .mos .java .class

all: iodrvcb.exe iodrvmem.exe iodrvmem2.exe iodrvraw.exe iodrvraw2.exe burgbindata.exe burgbindata.class

XPLIBS="$(XPRESSDIR)\lib\xprm.lib" "$(XPRESSDIR)\lib\bindrvMD.lib"

.c.exe:
	$(CC) -nologo -Zi -MD "-I$(XPRESSDIR)\include" $(CFLAGS) $< $(XPLIBS)

.mos.bim:
	mosel compile -g "$*"

.java.class:
	javac -classpath "$(XPRESSDIR)/lib/xprm.jar;$(XPRESSDIR)/lib/bindrv.jar";. $*.java

clean:
	del *.exe
	del *.obj
	del *.bim
	del *.class
