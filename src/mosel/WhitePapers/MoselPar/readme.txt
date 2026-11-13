Contents of this directory
==========================

Most of the examples in this directory are explained in the whitepaper
`Multiple models and parallel solving with Mosel'.

*** Models in this directory run on a single platform/Mosel instance, 
*** see subdirectory "Distr" for model versions running on distributed systems.

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
    type the following command (assuming that the main model is called
    'mymod.mos')
      mosel exec mymod
(c) Mosel libraries:
    in a C or VBA program use XPRMexecmod, and in a Java or C# program use 
    the sequence XPRM.compile/XPRM.loadModel/XPRMModel.run to execute a 
    model (see the `Mosel User Guide' or `Mosel Libraries Reference Manual' 
    for details)

*** ATTENTION: Models involving concurrent execution of submodels     ***
*** will not work if only a single Xpress licence is available        ***
*** (concerned model files: runrtparprl.mos, runrtparqueue.mos,       ***
***         runels.mos, cocoMs.mos, benders_main.mos, jobshopasp.mos) ***


1. Basic tasks
   * Running a submodel: runtestsub.mos (main model executing testsub.mos) 
   * Retrieving an event: runsubevnt.mos (main model executing testsub.mos)
   * Stopping a submodel: runsubwait.mos (main model executing testsub.mos)
   * Compiling to memory: runsubmem.mos (main model executing testsub.mos)
   * Setting runtime parameters: runrtparam*.mos (main model executing
        rtparams.mos)
   * Sequential execution of submodels: runrtparseq.mos (main model 
        executing rtparams.mos)
   * Parallel execution of submodels: runrtparprl.mos (main model 
        executing rtparams.mos)
   * Job queue for parallel execution of submodels: runrtparqueue.mos 
        (main model executing rtparams.mos)
   * Using the shmem driver for data exchange: runsubshm.mos (main model 
        executing testsubshm.mos)
   * Using the mempipe driver for data exchange: runsubpip.mos (main model 
        executing testsubpip.mos)

2. Column generation: solving different models in sequence
   paperp.mos - main model
   knapsack.mos - submodel
   papersn.mos - single model with main problem and subproblem
   
3. Solving several model instances in parallel 
   runels.mos - main model
   elsp.mos - submodel
   els*.dat - data files 
   
4. Dantzig-Wolfe decomposition: combining sequential and parallel solving
   cocoMs.mos - main model
   cocoSubFs.mos - submodel
   coco*.dat - data files    
   cocoMn.mos/cocoSubFn.mos - variant using mempipe notification in place of
   event sent by submodel

5. Benders decomposition: sequential solving of several different submodels
   benders_main.mos - main model
   benders_dual.mos - submodel (start solution + step 2, fixed integers)
   benders_cont.mos - submodel (step 2, fixed integers)
   benders_int.mos - submodel (step 1, fixed continuous variables)
   benders_single.mos - single model containing main problem + subproblems
   bprob*.dat - data files    

6. Generating start solutions via (sequential or parallel) subproblem solving
   jobshopas.mos - single model with main problem and subproblems
   jobshopasp.mos - main model
   jobseq.mos - submodel
   mt*.dat - data files

7. Outer approximation: solving different problems in sequence
   quadfacloc.mos - model formulated as single MIQP problem
   quadfaclocoa.mos - iterative sequential solving of MIP and QP subproblems  
