Contents of this directory
==========================

This directory contains application examples of the use of 
Xpress Kalis from Mosel.

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


bridgescheduling.mos - Project scheduling problem with disjunctive resource 
                  constraints and minimum and maximum delays between tasks.
bridgesched_graph.mos - Same model as previous, graphical solution 
                  representation.
bridgescheduling_alt.mos - Alternative model formulation using resource and 
                  task objects.

jobshop.mos     - Solving the classical job-shop scheduling problem.
jobshop_alt.mos - Alternative model formulation using resource and 
                  task objects.

mrcpsp.mos      - Solving the multi-mode resource constrained 
                  project scheduling problem (MRCPSP).

rcpsp.mos       - Solving the classical resource constrained project 
                  scheduling problem (RCPSP).

runfapp.mos, fappmod.mos - Frequency Assignment Problem with Polarity;
                  model runs are started by executing runfapp.mos

 
