Contents of this directory
==========================

This directory contains examples of the use of Xpress Kalis from Mosel
described in the `Xpress Kalis Mosel Reference Manual'.

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


ac2001.mos      - Generic binary constraints.

alldiff.mos     - All-different constraint; solving an assignment problem.

altresource_scheduling.mos - Scheduling jobs with resource choice and variable 
                  durations; task-based branching strategy with user-defined 
                  resource selection.

binpacking_table_constraint.mos - Using the 'table_constraint' to state
                  a binpacking problem. 

branching.mos   - Branching strategies using the branching schemes
                  'assign_and_forbid', 'assign_var', and 'split_domain';
                  user-defined variable and value selection heuristics.

conjdisj.mos    - Conjunctions and disjunctions (logical 'and' and 'or').

cumulative.mos  - Using the 'cumulative' constraint to formulate a
                  scheduling problem with resource constraints.

customrelax.mos - Defining a customized linear relaxation.

cycle.mos       - 'cycle' constraint example, solving a small TSP problem.
cycle_graph.mos - Graphical representation of solutions as user graph.

disjunctive.mos - Disjunctive constraint; implementing a sequencing problem.

distance.mos    - 'abs' and 'distance' constraints.

distribute.mos  - 'distribute' and 'occurrence' constraints; solving a
                  personnel planning problem.

element.mos     - One- and two-dimensional 'element' constraints.

gac2001.mos     - Generic n-ary constraints.

groupserializer.mos - Defining a task group serializer branching strategy 
                  for the problem of "producer_consumer.mos".

impequiv.mos    - Implication and equivalence constraints.

knapsackalld_cp.mos - Integer knapsack problem with 'alldifferent' constraint
                      solved with standard CP search.
knapsackalld_relax.mos - Defining a linear relaxation and corresponding search.

linctr.mos      - Defining, posting and propagating linear constraints.

minmax.mos      - Minimum and maximum constraints.

nlinctr.mos     - Non-linear constraints over real-valued decision variables.

occurrence.mos  - 'occurrence' (=cardinality) constraints.

probeac2001.mos - Branching scheme 'probe_assign_var' and definition of
                  generic binary constraints; solving the Euler knight
                  tour problem.
probeac2001_nary.mos - Defining a generic nary constraint

probesettledisjunction.mos - Branching scheme 'probe_settle_disjunction'; same
                  problem as in "disjunctive.mos" but modeled by pairs
                  of individual disjunctions (using 'or').

producer_consumer.mos - Resource-constrained project planning problem
                 (construction of a house) modeled with task and resource
		 objects.
producer_consumer_alt.mos - Alternative formulation using a 'producer_consumer'
                  constraint.
producer_consumer_alt_graph.mos - Graphical repesentation of solutions as 
                  user graph.

resource_altern.mos - Alternative resources and non-constant resource usage 
                  profiles. 
resource_altern_graph.mos - Graphical repesentation of solutions as 
                  user graph. 

resource_capacity.mos - Setting different capacity limits over time for 
                  a cumulative resource.

resource_coupled_setup_times.mos - Specifying resource-dpendent setup times.

resource_profile.mos - Scheduling tasks with non-constant resource usage 
                  profiles. 
resource_profile_graph.mos - Graphical repesentation of solutions as 
                  user graph. 

reversibles.mos - Working with "reversible" objects (scalars and arrays).

scalar_product.mos - Use 'dot' for multiplying an array of decision variables
                  with an array of numbers.

settledisjunction.mos - Branching scheme 'settle_disjunction'; same
                  problem as in "disjunctive.mos" but modeled by pairs
                  of individual disjunctions (using 'or').

solution.mos    - Use of callbacks to output the search tree; same problem
                  as in "disjunctive.mos".   
solution_graph.mos - Use of callbacks to draw a user graph.   

taskserializer.mos - Defining a task-based branching strategy for the problem
                  of "producer_consumer.mos".
