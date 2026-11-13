Contents of this directory
==========================

The examples in this directory are explained in the whitepaper
`Hybrid MIP/CP solving with Xpress Optimizer and Xpress Kalis'.

All examples may be run in any of the standard ways of running Mosel
models:
(a) with Xpress Workbench (desktop version):
      start up Workbench (double click on the model file)
      to run the model click on the `run' button (white triangle on green)  
    with Xpress Workbench (online version):
      connect to Workbench and create a new workspace
      upload the model file and all required data files
      to run the model click on the `run' button (white triangle on green)  
(b) Mosel command line:
    type the following command (assuming that the [main] model is called
    'mymod.mos')
      mosel exec mymod
(c) Mosel libraries:
    in a C or VBA program use XPRMexecmod, and in a Java or C# program use 
    the sequence XPRM.compile/XPRM.loadModel/XPRMModel.run to execute a 
    model (see the `Mosel User Guide' or `Mosel Libraries Reference Manual' 
    for details)
    
*** ATTENTION: The model versions with concurrent submodels will not run ***
*** properly if no sufficient number of Xpress licences is available.    ***
*** (Concerned files: sched_mainp.mos, sched_mainmp.mos)                 ***

1. CP propagation as preprocessor
   CP version: b1stadium_ka.mos
   CP + LP: b1stadium_main.mos (main model executing b1stadium_sub.mos)
   
2. Combining CP and MIP
   Sequential solving of subproblems: sched_main.mos (main model executing
      sched_sub.mos)
   Parallel solving of subproblems: sched_mainp.mos (main model executing
      sched_subp.mos)   
   Distributed parallel solving of subproblems: sched_mainpd.mos (main model
      executing sched_subpd.mos)   
   Definition of subproblems within a single model file (sequential solving):
      sched_single*.mos

3. CP generating start solutions for MIP
   CP model run before MIP Branch-and-bound search: f5tour3.mos
   CP model run at nodes of MIP Branch-and-bound search:
   f5tour4.mos/f5tour4m.mos
   Heuristic start solutions and subtour elimination constraints: f5tourcbrandom.mos
