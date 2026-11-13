#***********************************************
# Mosel examples makefile  
#                          
# (c) 2024 Fair Isaac Corporation 
#     author: Y. Colombani, 2002, rev. July 2026
#***********************************************
.SUFFIXES: .bim .mos

all: oclean.exe paper.exe paperio.exe paperiom.exe scenar.exe

XPLIBS="$(XPRESSDIR)\lib\xprm.lib"

.c.exe:
	$(CC) -nologo -Zi -MD "-I$(XPRESSDIR)\include" $(CFLAGS) $< $(XPLIBS)

.mos.bim:
	mosel compile -g "$*"

clean:
	del *.exe
	del *.obj
	del *.bim
