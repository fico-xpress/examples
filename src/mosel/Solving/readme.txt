Mosel Example Problems
======================

The directory Solving contains a set of examples that 
implement various solution heuristics using the more advanced
solving features of Mosel.

The subdirectory Data contains any input data files required by the
examples.

colgen
------
  Column generation for a cutting stock problem.
cutstk.mos:
* creating new variables and adding them to constraints and objective  
* basis in- and output
* sorting algorithm
* heuristic single knapsack algorithm
* repeat-until statement
* use of functions

papersn.mos:
* use of multiple problems in a single model

els
---
  An economic lot-sizing (ELS) problem. See text at the end of the 
  model file for details.
els.mos:
* use of model parameters
* adding new constraints and resolving the LP-problem
* basis in- and output
* defining the cut manager callback
* use of 'addcuts'
* if statement
* repeat-until statement
* procedure

fixbv2
------
  Implements a binary fixing heuristic for the Coco problem.
fixbv.mos:
* changing bounds on variables
* basis in- and output
* setting optimiser parameters

gabbp
-----
  Branch-and-Price for the Generalized Assignment Problem.
GAPbp3.mos (requires submodel GAPsubDP.mos): 
* user type definitions for data structures (records)
* concurrent solving of a set of subproblems coordinated via events and 
  exchanging data (including data with user type) via shared memory
genGapDataD.mos
* data instance generator

goal
----
  Goal programming examples.
goalctr.mos:
  Pre-emptive and Archimedian goal programming using constraints
* (un)hiding constraints 
* adding and deleting constraint terms
* getting solution information 
* case statement
* use of procedures
goalobj.mos:
  Archimedian and pre-emptive goal programming using objective functions
* changing constraint type
* changing the objective function
goalobjmo.mos:
  Same as goalobj.mos, using multi-objective solving functionality

multiobj
--------
  Examples using the multi-objective solving functionality
lexgoalprog.mos:
  Lexicographic goal programming for solving a small production planning
  example
* configuration of multiple objectives via 'objconfig' constructors 
  in list form
* detailed status check via XPRS_SOLVESTATUS and XPRS_SOLSTATUS
markowitzmo.mos:
  Markowitz portfolio optimization, multi-objective quadratic programming example
* list combining 'nlctr' and 'linctr' types
* display of the optimal frontier as SVG graph
multiobjknapsack.mos:
  Knapsack problem with two objectives
* configuration of multiple objectives in array structure using priorities
* multi-objective logging settings

recurse
-------
  An example of a non-linear problem modelled as a successive linear 
  programming (SLP) model. (SLP is also known as 'recursion'.)  
recurse.mos:
* modifying constraint coefficients
* while statement
* basis in- and output and problem reloading
* setting/accessing optimiser parameters
* procedure

transprt
--------
  Simple transportation model.
transprt.mos:
* small LP problem  
* generate a nice printed output
* reading several data arrays from a single data file
