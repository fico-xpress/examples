Examples from the "Embedding" whitepaper
----------------------------------------

Cleaning problem:
* oclean.c, cleana.mos, clparam.dat, cldata.dat

Scenarios:
* scenar.c, scenar.mos, Data/*.dat

Cutting stock problem:
* paper.c, paperm.mos: original version (defining a static module)
* paperio.c, paperio.mos: using I/O drivers instead of a static module
* paperiom.c, paperiom.mos: keeping the static module for pattern printing,
                            data input/output in memory via I/O drivers
