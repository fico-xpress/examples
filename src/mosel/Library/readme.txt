Mosel Libraries Examples
========================

This directory contains examples that show how to use the Mosel
model compiler and run time libraries.

mmexcomp.c      compiling a model file into a BIM file
                (requires burglari.mos)

mmexlib.c       working with models and accessing dynamic libraries in Mosel
mmexlib.java    (requires burglari.bim, chess2.bim, trans.bim)
                * load and unload BIM models
                * run a model in Mosel
                * display information about loaded models
                * display information about additional libraries required 
                  by the loaded models

mmexset.c       using sets in Mosel
mmexset.java    (requires burglari.bim)
                * retrieve a set by its model name
                * get the set size
                * get first and last set element
                * get the name or index of a set element

mmexlst.c       using lists in Mosel
mmexlst.java    (requires euler.mos and euler.dat)
                * retrieve a list by its model name
                * get the list size
                * enumerate the list elements
                * get value of list element

mmexrec.c       using records in Mosel
mmexrec.java    (requires burglar_rec.mos and burglar_rec.dat)
                * retrieve an array of records (user type) by its model name
                * retrieve the record field information
                * enumerate the array of records
                * for each array entry (record) get the value of all its fields

mmexarr.c       using arrays in Mosel
                (requires trans.bim)
                * compare index tuples
                * retrieve an array by its model name
                * get array size and number of dimensions
                * get indices of first and last array entries
                * get (value of) array entries
                * check whether an index tuple lies in the range of an array
                
mmexas.c        using arrays with index sets
mmexas.java     (requires trans.bim)
                * get indexing sets of an array
                * get array type
                * enumerate array entries in usual and transposed order
                * enumerate true array entries
                
mmexprob.c      accessing problems and solution information with Mosel
mmexprob.java   (requires blend2.bim)
                * export problem to a file (MPS or LP format)
                * get problem status
                * get objective function value
                * get primal/dual solution values, and constraint activity

mmexctc.c       interrupting a running model using Ctrl-C
                (requires toolong.bim)

mmdispmod.c     display the list of parameters and all symbols published
mmdispmod.java  by a model. The information is read from a bim file

mmdispdso.c     display the list of constants, control parameters and
mmdispdso.java  subroutines defined by a module

mmexdrvs.c      use IO drivers to handle Mosel output with a callback function,
mmexdrvs.java   compile a model from memory to memory, load a bim file from
                memory, initialise arrays in the Model program from C data
                or Java objects and retrieve information from the model
                through memory

mmexcbdrv.c     use the 'cb' IO driver to handle Mosel output and
mmexcbdrv.java  provide initial data to the model.

mmcover.c       use the debugger interface to make a source coverage testing
mmcover.java    tool: this program executes the given model and reports
                whether all statements have been executed and, for each
                procedure or function, how many time it has been called

Running the examples
====================
To run these examples, Mosel and the Xpress-Optimizer must have been installed
and the related environment variables set up correctly (XPRESSDIR, PATH on Windows, LD_LIBRARY_PATH on Linux or DYLD_LIBRARY_PATH on Macos).

To build all examples, run:
 mosel make

Then, C examples can be executed directly. For instance:
 ./mmexcomp

For a Java example, on Linux or Macos:
 java -classpath ${XPRESSDIR}/lib/xprm.jar:. mmexdrvs
On Windows:
 java -classpath %XPRESSDIR%\lib\xprm.jar;. mmexdrvs

The following command may be used to delete all generated files:
 model make TODO=clean
