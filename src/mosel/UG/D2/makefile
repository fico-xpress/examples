#**********************************
# Mosel examples makefile         *
#                                 *
# (c) 2024 Fair Isaac Corporation *
#     author: Y. Colombani, 2001  *
#**********************************
.SUFFIXES: .bim .mos

all:arcpkg.bim myconstants.bim solarraypkg.bim solarrayanypkg.bim parpkg.bim mynspkg1.bim mynspkg2.bim versionedpkg.bim

.mos.bim:
	mosel compile -g "$*"

clean:
	del *.bim
