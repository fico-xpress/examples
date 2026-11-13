readme_xnlp.txt (xnlp examples)                                   
============================

In order to be able to run the examples in this directory, Xpress NLP must
be installed and licensed. 

These examples are exmplained in the XNLP reference manual. 


xnlp_convexity.mos : demonstrating the effects of convexity
------------------

This example first solves a convex quadratic problem, for which Xpress-NonLinear find 
a proven optimal solution, then solves a non-covex one for which a local optimum is 
found. 


xnlp_derivatives.mos : demonstrating the cost of numerical derivatives
--------------------

This example solves the same simple problem several times using different differentiation 
techniques and solvers.


xnlp_local_optima.mos : demonstrating the presence of locally optimal solutions, and the role of initial points
---------------------

This example performs a series of solves for the exact same problem having several local optimas, followed by a
solve with Xpress Global to find the global optimum.


xnlp_nlp_duals.mos : example for dual multipliers for non-linear problems
------------------

This example solves a series of problem, some of which do not have valid dual multipliers (Lagrange multipliers) 
in the absense of one of the regularity conditions (like Slater's condition).


xnlp_nonconnected.mos : demonstrating non-connected feasible regions and initial points
---------------------

This examples defines a feasible region that resembles a uniformly layed out mesh of feasible rectangles
2*2 experiments are carried out:
 - starting from a random rectangle, can the solver move to another one where the optimim is?
 - starting from a random rectangle, can the solver stay in the same rectangle if the optimim is located there?
The experiment is repeated with and without model transformation.


xnlp_penalty_breakers.mos : demonstrating the need for penalty feasibility breakers
-------------------------

This is an SLP specific example.
Using Xpress-SLP, this examples solves two problem types that relate to various infeasibilities in SLP:
 One originating from steps taken along the linearization
 and one originating from deviation from the linearization


xnlp_penalty_multipliers.mos : example demonstrating the role of multipliers
----------------------------

This is an SLP specific example.
In this example a hihgly combinatorial and hihgly nonlienar problem is solved.
The high constraint nonlinear gives rise to very large violations that needs attention otherwise
 SLP overcompensates.
The example also demonstrates how feasibility and integrality might interact in an MINLP search.


xnlp_supportset.mos : demonstrating the type of solutions returned by solvers
-------------------

This examples solves simple optimzation problems with different solvers, demonstrating the fundamental
properites of the solutions returned.


xnlp_unboundedlinearization.mos : demonstrating behaviour of problems with unbounded first order approximations
-------------------------------

In this example a simple but unconstraint optimization is solved from various starting points
 and solves, demonstrating their convergence properties on such problems.


xnlp_zero_placeholder.mos : demonstrating local optimality and the role of placeholders
-------------------------

This examples solves a simple, then a complex function with many
 local optimal solutions both with SLP and Knitro to demonstrate
 the role of placeholders and delta_z, which while allows SLP to
 make long steps in degenrate situations, also tends to drive it to 
 solutions with a large vertex index.

