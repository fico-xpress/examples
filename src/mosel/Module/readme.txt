Mosel Native Interface Examples
===============================

This directory contains examples that show how to write modules that
define new constants, subroutines, types, services, or parameters for 
Mosel programs. For every module, an example Mosel program is provided
that demonstrates the use of the new functionality.

myconstants.c  definition of constants of different types
               * constants: integer, real, string, boolean
               example program using the new constants:
                 myconst_test.mos

solarray.c     definition of a procedure for getting solution values
               into an array
               * subroutine: procedure
               example program using the new subroutine:
                 solarr_test.mos

complex.c      definition of complex numbers and operators to work with
               numbers of this type
               * type: mathematical
               * operators: arithmetic, constructors, assignment, comparator
               * service: reset
               example program using the new type:
                 complex_test.mos

task.c         definition of type task and subroutines for accessing it
               * constants: boolean
               * type: collection of data
               * subroutines: procedures and functions
               * operators: constructors, assignment, comparator
               * services: reset, accessing and enumerating parameters
               * parameters: real, integer
               example program using the new type:
                 task_test.mos

date.c         definition of type date and subroutines for accessing it
               * constants: boolean
               * type: system
               * subroutines: procedures and functions
               * operators: constructor, assignment, comparators
               * services: reset, accessing and enumerating parameters
               * parameter: boolean
               example program using the new type:
                 date_test.mos

euro.c         definition of three new types and operators to work with them
               * types: 3 currencies
               * operators: constructors, assignment (with type conversion),
                 comparators, addition (with type conversion), multiplication
                 with numerical types (real,integer)
               * service: reset   
               example program using the new types:
                 euro_test.mos

myxprs.c       basic LP/MIP solver interface for Xpress Optimizer   
               definition of constants, parameters, subroutines
               * type: mpproblem.mxp (extension of mpproblem)
               * subroutines: procedures and functions
	       * implementation of a callback function
               * services: reset, unload, accessing and enumerating parameters 
	       * parameters: real, integer, boolean
               example program using the new functionality:
                 myxprs_test.mos

mmstatdso.c    declaring a static module (the module is embeded in the
               program instead of being a dso file) and initializing
               an array from data stored in the C program

zlib.c         use 'zlib' library (see: http://www.gzip.org) to provide
               I/O driver 'compress' (e.g. "zlib.compress:model.bim")
	       for stream compression and 'gzip' (e.g. "zlib.gzip:data.gz")
	       for handling gzip files

export.c       a module implementing driver 'toC' to save the result of
               a compilation to a C file: the execution of the generated
	       program perform the model processing.
	       For instance from Mosel console: 
	        compile model '' export.toC:model.c

dsodbg.c       this program includes a module in order to compile
               it as static. 'dsodbg' can then be executed from a
               debugger to work on the module.
               In the following lines of 'dsodbg.c'
                #include "my_module.c"
                #define SRC my_module
               the module name (here: my_module) has to be replaced
               by the name of the module to compile.

               Then from a debugger one can execute:
                dsodbg complex_test
               to analyse the behaviour of the module.

Running the examples
====================
To run these examples, Mosel and the Xpress-Optimizer must have been installed
and the related environment variables  set up correctly (XPRESSDIR, PATH on Windows, LD_LIBRARY_PATH on Linux or DYLD_LIBRARY_PATH on Macos).
In addition, the environment variable MOSEL_DSO must be set to
this directory for Mosel to be able to locate the modules.

To build all modules, run:
 mosel make

The following command may be used to delete all generated files:
 model make TODO=clean
