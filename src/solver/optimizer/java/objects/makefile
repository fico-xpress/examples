# Windows makefile for Optimizer examples

# The following variables control behavior and must be set (for example
# on the command line or as an environment variable)
#   XPRESSDIR  The root of the Xpress installation

# Usage: nmake [all]       Compiles all examples
#        nmake NAME.class  Compiles NAME.java
#        nmake NAME.run    Runs example NAME (NAME.class must exist).
#        nmake all.run     Compile and run all examples.

.SUFFIXES: .java .class .run

all: *.java
	!$(MAKE) -nologo $(**B).class
all.run: *.java
	!$(MAKE) -nologo $(**B).class
	!$(MAKE) -nologo $(**B).run

.java.class:
	javac $(JAVAFLAGS) -classpath "$(XPRESSDIR)/lib/xprs.jar;." $*.java

# We copy the DLLs to the execution directory.
# Alternatively, we could add $(XPRESSDIR)\bin to the PATH environment variable.
.class.run:
	copy "$(XPRESSDIR)\bin\xprs.dll" xprs.dll
	copy "$(XPRESSDIR)\bin\xprl.dll" xprl.dll
	copy "$(XPRESSDIR)\bin\javaxprs.dll" javaxprs.dll
	IF EXIST "$(XPRESSDIR)\bin\xknitro.dll" (copy "$(XPRESSDIR)\bin\xknitro.dll" xknitro.dll)
	IF EXIST "$(XPRESSDIR)\bin\libiomp5md.dll" (copy "$(XPRESSDIR)\bin\libiomp5md.dll" libiomp5md.dll)
	java -classpath "$(XPRESSDIR)\lib\xprs.jar;." $*

clean:
	del *.class 2>NUL
