Extra Libraries Examples
========================

This directory contains examples that show how to use XPRD and bindrv libraries.

bdrv.c          generate and read data files using the 'bin:' data format
bdrv.java

fmgr.c          example using the XPRD library
fmgr.java       * start instances
                * retrieve system information
                * compile/load/run model
                * implement file manager to exchange data with model
                * send:receive Mosel events

piap.c          implementation of piapprox.mos in C/Java using XPRD
piap.java       (requires piapprox.mos)

Running the examples
====================
To run these examples, Mosel and the Xpress-Optimizer must have been installed
and the related environment variables set up correctly (XPRESSDIR, PATH on Windows, LD_LIBRARY_PATH on Linux or DYLD_LIBRARY_PATH on Macos).

To build all examples, run:
 mosel make

Then, C examples can be executed directly. For instance:
 ./bdrv

For a Java example, on Linux or Macos:
 java -classpath ${XPRESSDIR}/lib/bindrv.jar:${XPRESSDIR}/lib/xprd.jar:. bdrv
On Windows:
 java -classpath %XPRESSDIR%\lib\bindrv.jar;%XPRESSDIR%\lib\xprd.jar;. mmexdrvs

The following command may be used to delete all generated files:
 model make TODO=clean
