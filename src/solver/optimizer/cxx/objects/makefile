# Windows makefile for Optimizer examples
# Will build all C library examples if no target is specified

# The following variables control behavior and must be set (for example
# on the command line or as an environment variable)
#   XPRESSDIR  The root of the Xpress installation

# Usage: nmake [all]       Compiles all examples
#        nmake NAME.exe    Compiles NAME.cpp
#        nmake NAME.run    Runs example NAME (NAME.exe must exist).
#        nmake all.run     Build and run all examples

.SUFFIXES: .cpp .exe .run

all: *.cpp
	!$(MAKE) -nologo $(**B).exe
all.run: *.cpp
	!$(MAKE) -nologo $(**B).exe
	!$(MAKE) -nologo $(**B).run

CPPFLAGS=-nologo -I$(XPRESSDIR)/include -Zc:__cplusplus -std:c++17 -EHsc
LIBS=$(XPRESSDIR)/lib/xprs.lib
LINKFLAGS=

.cpp.exe:
	$(CXX) /W4 /WX $(CXXFLAGS) $(CPPFLAGS) $< /link $(LINKFLAGS) $(LIBS)

# Before running an executable, we copy all required DLLs into the directory.
# An alternative is to set the PATH environment variable to contain
# $(XPRESSDIR)\bin
.exe.run:
	copy "$(XPRESSDIR)\bin\xprl.dll" xprl.dll
	copy "$(XPRESSDIR)\bin\xprs.dll" xprs.dll
	IF EXIST "$(XPRESSDIR)\bin\xknitro.dll" (copy "$(XPRESSDIR)\bin\xknitro.dll" xknitro.dll)
	IF EXIST "$(XPRESSDIR)\bin\libiomp5md.dll" (copy "$(XPRESSDIR)\bin\libiomp5md.dll" libiomp5md.dll)
	$*

clean:
	del *.exe 2>NUL
	del *.dll 2>NUL
