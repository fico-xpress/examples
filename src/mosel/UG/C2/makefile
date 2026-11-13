#**********************************
# Mosel examples makefile         *
#                                 *
# (c) 2024 Fair Isaac Corporation *
#     author: Y. Colombani, 2002  *
#**********************************
.SUFFIXES: .java .class

# This makefile will build the Java examples but not the .NET.
# For a .NET examples, change into the subdirectory for that example (e.g. "cd ugcb-cs") and
# type "dotnet build" to compile, or "dotnet run" to execute.  The supplied example projects 
# can also be opened in Visual Studio.

all: ugarray.class ugcb.class ugcomp.class ugcomptmp.class ugdefstream.class ugiocb.class ugiodense.class ugiosparse.class ugioscalar.class ugparam.class ugrun.class ugsol.class ugstreamdense.class ugstreamdensescrmt.class ugstreamsparse.class

.java.class:	
	javac -classpath "$(XPRESSDIR)/lib/xprm.jar;." $*.java

clean:
	del *.class
