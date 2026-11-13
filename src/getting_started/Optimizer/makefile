#******************************************
#  Getting Started
#  ===============
#  Makefile for Xpress Optimizer examples
# 
#  (c) 2024 Fair Isaac Corporation
#      author: Y. Colombani, 2003
#******************************************
#
# Before using this makefile remember to set the environment variable
# XPRESSDIR to the directory where the DLL kit is installed on your computer.
#
.SUFFIXES: .java .class

all: folioinput.exe foliolp.exe foliomatenumsol.exe foliomatsolpool.exe foliomip1.exe foliomip2.exe folioqp.exe

.c.exe:
	$(CC) -nologo -Zi -MD "-I$(XPRESSDIR)\include" $(CFLAGS) $< "$(XPRESSDIR)\lib\xprs.lib"

.java.class:	
	javac -classpath "$(XPRESSDIR)/lib/xprs.jar";. $*.java

clean:
	del *.exe
	del *.class
	del *.obj
