Contents of this directory
==========================

This directory contains examples of the use of the Xpress Kalis Mosel
module described in the `Xpress Kalis Mosel User Guide'.

The examples may be run in any of the standard ways of running Mosel
models:
(a) with Xpress Workbench (desktop version):
      start up Workbench (double click on the model file)
      to run the model click on the `run' button (white triangle on green)  
    with Xpress Workbench (online version):
      connect to Workbench and create a new workspace
      upload the model file and all required data files
      to run the model click on the `run' button (white triangle on green)  
(b) Mosel command line:
    type the following command (assuming that your model is called mymod.mos)
      mosel exec mymod
(c) Mosel libraries:
    in a C or VBA program use XPRMexecmod, and in a Java or C# program use 
    the sequence XPRM.compile/XPRM.loadModel/XPRMModel.run to execute a 
    model (see the `Mosel User Guide' or `Mosel Libraries Reference Manual' 
    for details)


Modeling
--------
meeting.mos    - Introductory example: disequality constraints; default
                 enumeration
i4exam_ka.mos  - Scheduling exams: disequality constraints; data input 
                 from file (static arrays)
i4exam2_ka.mos, i4exam2b_ka.mos - Data input from file (dynamic arrays) 
i4exam3_ka.mos - Optimization
i4exam4_ka.mos - Definition of branching strategy over decision variables

Constraints
-----------
a4sugar_ka.mos - Production of cane sugar: linear, 'ocurrence', and 'element'
                 constraints; branching strategy for variables
a4sugar2_ka.mos - Alternative formulation using 'distribute' constraints

b4seq_ka.mos   - Sequencing jobs on a bottleneck machine: linear and 
                 'element' constraints; branching strategy for variables;
		 consecutive solving with 3 different objectives
b4seq2_ka.mos  - Alternative formulation using 'disjunctive' constraint,
                 branching over variables and constraints

b5paint_ka.mos - Planning of paint production: linear, 'element', 'implies',
                 and 'all-different' constraints
b5paint2_ka.mos - Alternative formulation using 'disjunctive' and 
                 2-dimensional 'element' constraints
b5paint3_ka.mos - Alternative formulation using 'cycle' constraint

eulerkn.mos    - Euler knight tour problem: 'all-different' and generic
                 binary constraints; branching strategy for variables
eulerkn2.mos   - Alternative implementation using subtour elimination
                 constraints ('implies')
eulerkn2_graph.mos - Drawing a user graph with "mmsvg"   
eulerkn3.mos   - Alternative implementation using 'cycle' constraints 
eulerkn3b.mos  - Alternative implementation using a 'cycle' constraint 
                 with successor and predecessor variables

freqasgn.mos   - Frequency assignment problem: 'abs', 'distance', and
                 'all-different' constraints; branching strategy for
		 variables; solution callback; interrupting and restarting
		 the search
freqasgn_graph.mos - Drawing a user graph with "mmsvg"

j5tax_ka.mos   - Choice of locations for income tax offices: linear,
                 'element', 'occurrence', 'equiv' constraints; search
		 startegy for variables

meeting2.mos   - Constraint handling: immediate posting of (unnamed) constraints
meeting3.mos   - Named constraints posted without control of result
meeting4.mos   - Constraint posting with 'cp_post' to retrieve the result

nonlinctr.mos  - Nonlinear constraints over discrete and continuous variables

persplan.mos   - Personnel planning problem: 'all_different', 'implies',
                 'occurrence', and 'distribute' constraints; branching
		 strategy for variables
persplan_graph.mos - Drawing a user graph with "mmsvg"

scenesalloc.mos - Scene allocation: 'implies', 'distribute', 'maximum',
                 symmetry breaking

sudoku_ka.mos  - Sudoku puzzle: 'all-different' constraints; retrieving
                 status information through parameters
sudoku2_ka.mos - Same model; changing the propagation algorithm; time
                 measures with "gettime"

Enumeration
-----------
i1assign_ka.mos - Assigning workers to machines: linear, 'all-different',
                  and 'element' constraints; branching strategy for variables;
                  consecutive solving with 2 different objectives; 
		  heuristic solution algorithm without enumeration
i1assign2_ka.mos - Same model; user-defined search (variable and value 
                  selection strategies)
tracereverse.mos - Displaying the values of reversible numbers during search.
sudoku_conflict.mos - Analyzing infeasibility

Scheduling
----------
altresource.mos  - Scheduling with resource choice; 'requires' constraints 
                  with sets of 'resusage'
altresource_graph.mos - Drawing a user graph with "mmsvg"

b1stadium_ka.mos - Project planning problem: linear constraints, effect
                  of constraint propagation
b1stadium2_ka.mos - Scheduling with precedence constraints, modeled with tasks

b4seq3_ka.mos   - Disjunctive scheduling / sequencing problem, modeled with 
                  tasks and resources
                 
b5paint4_ka.mos - Disjunctive scheduling with setup times, modeled with 
                  tasks and resources; 'equiv', 'element', 'maximum' constraints
		  enumeration strategy for tasks and variables

d4backup2_ka.mos - Binpacking problem: 'cumulative' constraints, search
                  strategy for variables
d4backup3_ka.mos - Alternative formulation using tasks and resources;
                  default scheduling search
d4backup3a_ka.mos - Variable-based search strategy for cp_minimize		 
d4backup3b_ka.mos - Variable-based search strategy for cp_schedule
d4backup3c_ka.mos - Choice of the propagation algorithm
		 
jobshop2.mos    - Job-shop scheduling problem modeled with tasks and resources;
                  default scheduling search  
jobshop2a.mos   - Task-based user search strategy for cp_minimize 
jobshop2b.mos   - Task-based user search strategy for cp_schedule 
jobshop2c.mos   - Choice of the propagation algorithm 

renewa.mos      - Provision/requirement of a renewable resource.

renewb.mos      - Production/consumption of a non-renewable resource.
renewb2.mos     - Alternative formulation with producer_consumer constraint
                 
residle.mos     - Preemptive scheduling ('resource idle times');
                  definition of resource usage profiles.
resprofile.mos  - Scheduling with resource usage profiles; 
                  'requires' constraints with sets of 'resusage'
resprofile_graph.mos - Drawing a user graph with "mmsvg"

Relaxations
-----------
knapsackalld.mos - Knapsack problem with 'alldifferent' constraint solved
                   by automatic linear relaxations (requires Xpress Optimizer)
knapsackalld2.mos - Configuring linear relaxations (requires Xpress Optimizer)
knapsackalld3a.mos - Selecting constraints for linear relaxations
                   (requires Xpress Optimizer)
knapsackalld3b.mos - User-defined linear relaxations (requires Xpress Optimizer)
