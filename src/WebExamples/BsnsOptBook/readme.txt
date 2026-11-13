File readme.txt                                       November 2020
===============

FICO Xpress Mosel implementations of examples discussed in:
   J. Kallrath: Business Optimization Using Mathematical Programming -
   An Introduction with Case Studies and Solutions in Various Algebraic 
   Modeling Languages. 2nd edition, Springer Nature, Cham, 2021
   DOI 10.1007/978-3-030-73237-0


License
-------
All files in this directory are licensed under the Apache License, Version 2.0. See http://www.apache.org/licenses/LICENSE-2.0 for the full license text.


Running the examples
--------------------
1. Install the FICO Xpress software
   Free Community Edition download: 
   https://community.fico.com/optimization

2. Run a Mosel model
   (a) From the command line:
       type the following command (assuming the model is called mymodel.mos)
         mosel mymodel.mos
   (b) With Xpress Workbench:
         start up Workbench (double click on the model file)
         to run the model click on the `run' button or alternatively,
	 select the 'Run mymodel.mos' entry from the 'Run' menu
   (c) Mosel libraries:
       in a C or VBA program use XPRMexecmod, and in a Java or C# program use 
       the sequence XPRM.compile/XPRM.loadModel/XPRMModel.run to execute a 
       model (see the `Mosel User Guide' or `Mosel Libraries Reference Manual' 
       for details)       
   Online documentation for Xpress Mosel: 
   https://www.fico.com/fico-xpress-optimization/docs/latest/mosel


List of provided model files
----------------------------
(Examples marked with * are newly introduced in the 2nd edition,
 all other models have been converted from the mp-model versions
 that were provided with the 1st edition of the book in 1997)

 Filename       Problem description                                 Section
 --------       -------------------                                 -------
 absval.mos     modeling absolute value terms (linearization)       6.5
*absval2.mos    modeling absolute value as general constraints 
 bench101.mos   solution to Exercise 10.1 in Chapter 10             10.9
 bench102.mos   solution to Exercise 10.2 in Chapter 10             10.9
 blendx.mos     ore blending problem                                2.7.1
                (data files: blendx.csv, blendx.dat) 
*boat.mos       solution for Exercise 3.3 in Chapter 3              3.7
*boat2.mos      multi-period solution for Exercise 3.3 in Chapter 3 3.7
 boatdual.mos   dual of the "Boat Renting" problem (Ex. 3.3)        3.7
 brewery.mos    brewery planning                                    8.3
                (data files: brewery.xlsx, brewdata.dat) 
 burgap.mos     knapsack exercise (Exercise 7.3)                    7.10
 burglar.mos    knapsack problem                                    7.1.1
 buscrew.mos    bus crew scheduling                                 7.8.4
 calves.mos     calves and pigs problem                             3.3.1
 carton.mos     carton scheduling problem problem                   10.3
                (data file: carton.dat) 
*ch-tri2.mos    minimal perimeter convex hull for two triangles     10.3
*contract.mos   contract allocation (semi-continuous variables)     10.2.1  
 couples.mos    solution to Exercise 6.5 in Chapter 6               6.11
 dea.mos        data envelopment analysis (Exercise 5.2)            5.3
 dual.mos       solution to Exercise 3.1(b) in Chapter 3            3.5.1
*dynbigm.mos    dynamic computation of big-M coefficients           14.1.2.1
*dynbigm2.mos   formulation using indicator constraints             14.1.2.1
*ea_smpld.mos   evolutionary algorithm for supply management        14.1.3.3
 euro.mos       solution to Exercise 7.5 in Chapter 7               7.10
 flowshop.mos   solution to Exercise 7.7 in Chapter 7               7.10
*fracprog.mos   fractional programming example                      11.1
 gap.mos        generalized assignment problem (Ex. 7.2)            7.10
*goalprog.mos   example exploiting a hierarchy of goals             5.4.3
*lagrel.mos     Lagrange relaxation applied to the GAP              14.1.3.3
 lim1.mos       solution to Exercise 5.1 in Chapter 5               5.3
 lim2.mos       solution to Exercise 6.7 in Chapter 6               6.11
*manufact.mos   solution to Exercise 6.9 in Chapter 6               6.11
 multk.mos      exercise on multiple knapsack (Exercise 7.4)        7.10
 network.mos    network problem (Exercise 4.3)                      4.7
 network2.mos   generic formulation network problem (Exercise 4.3)  4.7
*newsvendor.mos newsvendor problem: 2-stage stochastic programming  11.3.2.1
 npv.mos        solution to Exercise 6.8 in Chapter 6               6.11
*optgrid.mos    optimal breakpoints for piecewise linear approx.    14.2.3
*portfolio.mos  EEV for multi-stage stochastic problems             11.3.2.5
                (submodels: portfolio_det.mos, portfolio_deq.mos) 
 primal.mos     primal problem solution to Exercise 3.1(a)          3.7     
 prodx.mos      simple production planning exercise                 2.5.2
 projschd.mos   project scheduling case study                       10.2.3
 quadrat.mos    linearized quadratic programming example            11.4
*quadrat2.mos   quadratic programming example solved as NLP         11.4
 set.mos        set covering problem (Exercise 7.6)                 7.10
 simple1.mos    solution to Exercise 2.2 in Chapter 2               2.13
 simple2.mos    solution to Exercise 2.3 in Chapter 2               2.13
 slab.mos       solution to Exercise 6.6 in Chapter 6               6.11
 sludge.mos     example illustrating recursion                      11.2.1
*sludge2.mos    recursion example solved as NLP                     11.2.1
*teams.mos      solution to Exercise 6.4 in Chapter 6               6.11
 trim1.mos      trimloss problem (Exercise 4.1)                     4.1.1
 trim2.mos      trimloss problem (Exercise 4.2)                     4.1.2
*trimminlp.mos  trimloss problem formulated as a MINLP problem      13.3
*trimminlp2.mos alternative NLP solver choice                       13.3
 tsp.mos        traveling salesman problem (Exercise 7.1)           7.10
*vehroute.mos   vehicle routing - heating oil delivery              7.2.3
*woods.mos      solution to Exercise 2.1 in Chapter 2               2.13
 yldmgmt.mos    yield management, financial modeling                8.4.2

