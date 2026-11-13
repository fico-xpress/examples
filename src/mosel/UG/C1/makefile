#**************************************************
# Mosel examples makefile                         *
#                                                 *
# (c) 2024 Fair Isaac Corporation                 *
#     author: Y. Colombani, 2001, rev. July 2026  *
#**************************************************
.SUFFIXES: .bim .mos

all: ugarray1.exe ugarray2.exe ugcb.exe ugcomp.exe ugcomptmp.exe ugdefstream.exe ugexec.exe ugiocb.exe ugiodense.exe ugiosparse.exe ugioscalar.exe ugparam1.exe ugparam2.exe ugrun.exe ugsol1.exe ugsol2.exe ugxprs1.exe ugxprs2.exe burglar4.bim burglar3.bim prime2.bim transport.bim

XPLIBS="$(XPRESSDIR)\lib\xprm.lib" "$(XPRESSDIR)\lib\xprs.lib"

.c.exe:
	$(CC) -nologo -Zi -MD "-I$(XPRESSDIR)\include" $(CFLAGS) $< $(XPLIBS)

.mos.bim:
	mosel compile -g "$*"

clean:
	del *.exe
	del *.obj
	del *.bim
