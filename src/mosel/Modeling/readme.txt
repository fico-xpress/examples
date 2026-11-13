Mosel Example Problems
======================

The directory Modeling contains a set of introductory
examples that show how to write models using the Mosel language.

The subdirectory Data contains any input data files required by the
examples.   

blend
-----
  A model for mineral blending.
blend.mos:
* simple LP problem
* data input from file
* bounds on variables

burglar
-------
  The 'Burglar Bill' knapsack model.
burglar.mos:
* small MIP problem
* alternative use of number-valued ranges and sets of strings for
  indexing variables and data
* definition of binary variables 
* forall statement 
burglari.mos:
* using string indices instead of integer ranges
burglarl.mos:
* defining indicator and logical constraints

chess
-----
  Manufacturing of chess boards. Simple production planning problem.
chess.mos
* simple LP/IP problem
chess2.mos
* simple LP problem  
* defining a set of variables and an array of descriptions for the 
  variables (to be used in the output printing)

coco
----
  The Coco multi-period, multi-item production planning problem.
coco.mos:
* complete problem, data defined in the model definition
* if-then, if-then-else, if-then-elif statements

firestns
--------
  A set-covering model for emergency service provision.
fstns.mos:
* simple LP problem
* definition of Booleans 
* logical conditions in constraint definition  

lcsp
----
  A labour constrained scheduling problem (LCSP). See text at the end
  of the model for details.
lscp1.mos:
* difficult MIP problem
* dynamic creation of variables
* constraint definitions with logical conditions
* using parameters in the optimisation function
* if-then-else statement

setops
------
  Basic set operations.
setops.mos:
* declaring constant and dynamic sets
* assigning and adding elements to sets
* using predefined set operations: union, intersection, subtraction 
* printing sets 

pplan
-----
  A project planning problem with resource constraints.
pplan.mos:
* simple MIP problem
* alternative formulation using binary variables or SOS-1
* if-then-else statement
 
purchase
--------
  A model for optimal purchasing with price-breaks.
purch.mos:
* complex MIP model 
* data input from file, including parameters for dimensioning
* splitting the declaration section to read first the parameters that
  are used to declare the data arrays
* using SOS-2

catenary
--------
  A QCQP model to determine the shape of a hanging chain.
catenary.mos:
* QCQP model (linear objective, convex quadratic constraints) 

transpl2
--------
  A transportation problem with piecewise linear cost expressions.
transpl2.mos:
* MIP model 
* data input from file
* using 'pwlin' to formulate piecewse linear cost functions

piecewiselin
------------
  Approximating nonlinear univariate functions using
  piecewise linear constraints.
piecewise_linear.mos:
* formulating pwlin constraints via piecewise linear segments (pws) or
  via slopes
piecewise_linear_graph.mos:
* representation of nonlinear function approxmiations as SVG graphs

maxsat
------
  MAXSAT solving via MIP.
nrm_10_4_1.mos:
* MAXSAT model
* using boolvar with and/or/not constraints
* retrieving the mpvar for boolvar
* solution reporting for boolvar

