Examples of using Mosel on distributed architecture
===================================================

NOTES:
1. All models need to be configured for your local architecture.
   The selected remote machines need to have a suitable version of Xpress
   installed and licensed (where compilation is done on the root node, the
   remote versions should be all the same as the version on the root node,
   but in all cases the nodes may be any platform/architecture supported
   by Xpress).
2. Before executing models on remote nodes, you need to start the server
   'xprmsrv' on all nodes you wish to use: in a command line interpreter 
   window type: 
      xprmsrv 
   or alternatively, under Windows, double click on the 'xprmsrv' icon.
      

Introductory examples
---------------------

findservers.mos - check for available remote Mosel servers

runrtdistr.mos - run a single model on a remote machine  
  (requires: rtparams.mos)

runrtdistrconf.mos - run a single model on a remote machine with
                     configuration options 
  (requires: rtparams.mos)

runrtpardistr.mos - running parallel submodels in a distributed architecture
  (requires: rtparams3.mos)
  
runrtparqueued.mos - queuing submodels for parallel execution in a distributed
                     architecture with one or several models per node  
  (requires: rtparams.mos)

runrtpartree.mos - 3-level tree of (parallel) submodels
  (requires: rtparams2.mos)

runrtdetach.mos - running a submodel that detaches itself from its parent
  (requires: rtparams4.mos)



Advanced examples
-----------------
cocoMd[p].mos - Dantzig-Wolfe decomposition (several parallel submodels)
  (requires: coco2.dat, cocoSubFd[p].mos)

runelsd.mos - Parallel execution of remote model instances with updates
              during the optimization runs
  (requires: elsd.mos, els.dat)
  
tspmain.mos  - decomposition algorithm for solving the TSP problem by a 
              series of small subproblems, "glued" together by partially
              unfixing tours of neighbouring subproblems;
              tspmain_graph.mos: graphical representation 
  (requires: tspsub.mos)
