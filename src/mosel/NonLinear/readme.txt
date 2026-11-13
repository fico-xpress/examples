readme.txt (mmxnlp examples)                                   November 2020
============================

In order to be able to run the examples in this directory, Xpress Nonlinear 
must be installed and licensed. 


Polygon - trigonometric constraints and objective
-------
(Examples documented in the Xpress Nonlinear Manual)
* polygon1.mos     - formulation using direct algebraic expressions
* polygon1_graph.mos - same as polygon1.mos + graph drawing
* polygon2.mos     - defining a simple Mosel user function  
* polygon3.mos     - defining a multi-valued Mosel user function
* polygon8.mos, polygon8_delta.mos (requires Polygon.java) 
                   - using a Mosel user function redirected into Java
		     (simple function or returning partial derivatives)
  >>compile Polygon.java into Polygon.class before running this example


Airport - QCQP
-------
Locate airports each within a specified distance of a city centre.
(Example documented in the Mosel User Guide)
* airport_nl.mos (requires airport.dat), airport_nl_graph.mos

Bookdisc - discrete NLP
--------
Purchasing goods with discounts depending on previous purchase amounts. 
* bookdisc.mos

Boxes - discrete NLP
-----
Planning the production of boxes with variable design. 
* boxes02.mos

Catenary - QCQP
--------
Find the shape of a hanging chain.
* catenary.mos, catenary_graph.mos

EMFL - Convex NLP
----
Euclidean facility location problem.
* emfl.mos, emfl_graph.mos

Expfit - Convex NLP
------
Approximating a function by a polynomial.
* expfita.mos (requires expfita.dat), expfita_graph.mos

Glider - NLP
------
Maximize the total horizontal distance a hang glider flies.
* glidert.mos (requires glider.dat), glidert_graph.mos

Grasping - Convex/Nonconvex NLP
--------
Find the smallest amount of force required to lift an object.
* grasp.mos, grasp_graph.mos

Loadbal - Convex NLP
-------
Static load balancing in a tree computer network.
* loadbal.mos (requires loadbal.dat), loadbal_graph.mos

Minsurf - Convex NLP
-------
Minimizing the surface between given boundaries with an optional obstacle.
* minsurf.mos, minsurf_graph.mos

Moonshot - Nonconvex NLP
--------
Flying a rocket from Earth to the Moon.
* moonshot.mos (requires (moonshot.dat), moonshot_graph.mos

Portfoliorisk - QCQP
-------------
Portfolio optimization using different risk expressions. 
* portfoliorisk.mos (requires portfoliorisk.dat)

Pricechange - Discrete NLP
-----------
Determine quantities and prices for given total amounts.
* pricechange.mos, pricechange_graph.mos

Sphere - Nonconvex NLP 
------
Find the eqilibrium state distribution of electrons on a sphere.
* sphere.mos, sphere_graph.mos

Springs - SOCP
-------
Find the shape of a hanging chain where each chain link is a spring.
* springs.mos, springs_graph.mos

Steiner - SOCP
-------
Steiner tree problem.
* springs.mos, springs_graph.mos (requires steiner.dat)

Trafequil - Convex NLP
---------
Determining a trafic equilibrium  for a given network.
* trafequil.mos (requires trafequil.dat), trafequil_graph.mos

