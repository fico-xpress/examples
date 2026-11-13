Getting Started Examples
========================
File readme.txt (Optimizer)                                 September 2022

--------------------------------------------------------------------------

The directory "Optimizer" contains the examples discussed in the part
"Getting Started with the Optimizer" of "Getting Started with Xpress":

Chapter 14 "Matrix input"
* folioinput.c (requires Folio.mps): loading an LP problem via matrix input

Chapter 15 "Inputting and Solving a Linear Programming problem"
* foliolp.c: loading a small LP problem via XPRSloadlp

Chapter 16 "Mixed Integer Programming"
* foliomip1.c: loading a small MIP problem via XPRSloadmip (binary
  variables)
* foliomip2.c: loading a small MIP problem via XPRSloadmip
  (semi-continuous variables)

Chapter 17 "Quadratic Programming"
* folioqp.c: loading a small QP problem via XPRSloadqp

--------------------------------------------------------------------------

Advanced Evaluators Guide examples
==================================

***Multiple MIP solutions***
foliomatenumsol.c - using the MIP solution enumerator
foliomatsolpool.c - using the MIP solution pool

--------------------------------------------------------------------------

To run these examples with the provided makefile, the environment variable
XPRESSDIR must be set to the installation directory of Xpress.

