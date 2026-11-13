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
      

runrtdistr.mos   - run a single model on a remote machine  
  (requires: rtparams.mos)

remotefiles.mos  - demonstrates access to files on remote (parent or
                   child) nodes; 
                   this model requires write access on remote machines  

piapprox.mos     - approxmating pi by calculating the area of small rectangles,
                   the rectangles are distributed evenly across all submodels,
                   results are passed back through an event message

mandelhttp.mos - queued execution of remote models with result graphic
                   in a web browser controlled by root model
  (requires: mandelhttpsub.mos,mandelhttp.html,jquery-2.0.0.min.js) 
