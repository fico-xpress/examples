Mosel Examples Programs
=======================

The directory Programming contains a set of examples that
demonstrate how Mosel can be used for writing programs.
The files in this directory implement certain well-known algorithms
that, a priori, have nothing to do with the formulation and solution 
of optimisation problems.

lcdiv.mos
---------
  Find the largest common divisor of two integer numbers.
  * recursive function calls
  
minmax.mos
----------
  Find the smallest and the largest value in a set of integers.
  * use of round, random, min, max

perfect.mos
-----------
  Calculate all perfect numbers up to a given upper bound.

prime.mos
---------
  Find all prime numbers up to a given bound using the Sieve 
  of Eratosthenes.
  * working with sets

readfile.mos
------------
  Read two tables and a set from a data file.

readcsv.mos
------------
  Parse a CSV file line by line.

shsort.mos
----------
  Sort an array of numbers into numerical order using Shell's method.    
  * nested loops: repeat-until, forall, while

qsort.mos
---------
  Sort an array of numbers into numerical order using Quicksort.    
  * nested loops: repeat-until, forall, while
  * recursive calls of procedures
  * overloading

bintree.mos
-----------
  Package that provides the functionality for handling binary trees
  * definition of a package
  * definition and operations on records

exbtree.mos
-----------
  Example of use of the package 'bintree'
  * use of a package
  * basic list operations

bpack.mos
---------
  Encapsulate a user-specified binary file into a BIM
  * datablock
  * submodel included in model source

translate.mos
-------------
  Example of use of the automatic translation system

mdbg.mos
--------
  Implements a basic debugger for Mosel programs (use of Remote Invocation Protocol)

mprf.mos
--------
  Implements a profiler for Mosel programs (use of Remote Invocation Protocol)

mcov.mos
--------
  Implements model code coverage with Mosel (use of Remote Invocation Protocol)

cnf2mos.mos
-----------
  Converting SAT or MAXSAT problems in CNF or WCNF format to Mosel models.
  The resulting Mosel file can be solved as a MIP problem.

xmltest.mos
-----------
  Creating and saving XML documents
  * adding nodes, specifying attributes
  * recursive display of subtrees

booksearch.mos, xmlrefexpl.mos
------------------------------
  Retrieving XML nodes via xpath search expressions, using XML-format files as
  databases; modifying and formatting XML files.

jparse.mos, xparse.mos
----------------------
  Parsing JSON or XML documents with unknown structure using the parsing routines
  'xmlparse' or 'jsonparse' of mmxml 
  * definition of callbacks

json.mos
--------
  Package that provides type definitions for representing JSON format files
  along with some access functions and a parsing routine for reading JSON
  documents with unknown structure

readjson.mos
------------
  Alternative ways of reading a JSON-format database file, saving it into
  (a) an xmldoc structure, or (b) a specific user-defined record structure, or
  (c) union-based types defined by the package json.mos
