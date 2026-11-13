XPRD examples
=============
File readme.txt                                    Oct 2012

--------------------------------------------------------------------------

This directory contains examples of running Mosel models remotely
using the XPRD library.

NOTES:
1. All models need to be configured for your local architecture.
   The selected remote machines need to have a suitable version of Xpress
   installed and licensed (the nodes may be any platform/architecture supported
   by Xpress).
2. Before executing models on remote nodes, you need to start the server
   'xprmsrv' on all nodes you wish to use: in a command line interpreter 
   window type: 
      xprmsrv 
   or alternatively, under Windows, double click on the 'xprmsrv' icon. 


Introductory examples
---------------------
(see whitepaper "Multiple models and parallel solving with Mosel")
* runrtdistr.[c|java] (requires: rtparams.mos) - running a single model on 
           a remote machine
           - XPRD version of runrtparams.mos/runrtdistr.mos -
* runrtpardistr.[c|java] (requires: rtparams3.mos) - running several submodel
           instances in parallel in a distributed architecture
           - XPRD version of runrtparprl.mos/runrtpardistr.mos -
* runrtparqueued.[c|java] (requires: rtparams.mos) - queue management for
           parallel execution in a distributed architecture with one or 
           several models per node
           - XPRD version of runrtparqueue.mos/runrtparqueued.mos -
* findservers.[c|java] - check for available remote Mosel servers


Advanced examples
-----------------
* runelsd.[c|java] (requires: elsd.mos, readelsdem.mos, els.dat) 
           parallel execution of remote model instances (solving 
           the same ELS problem with different algorithm configurations)
           with updates during the optimization runs
           - XPRD version of runels.mos/runelsd.mos -
* mandelbrot.java (requires: mandelbrot.sub) - calculation of the
           Mandelbrot function, subdividing the space into squares of points
           to be solved by the submodel instances; graphical representation
           of function values using Java graphing functionality 
           - XPRD version of mandelbrotmain.mos -
* runfoliodistr.[c|java] (requires: foliomemio.mos, folio10.dat) 
           run an optimization model and retrieve detailed solution info,
           defining a file manager for data exchange in memory
           - XPRD version of runfoliodistr.mos (see Getting_Started) -
* distfolio.[c|java] (requires: foliomemio.mos, folio250.dat) 
           run an optimization model and retrieve detailed solution info,
           reading binary solution files
           - XPRD version of runfoliodistr.mos (see Getting_Started) -
* distfoliopar.[c|java] (requires: foliomemio.mos, folio250.dat) 
           run several optimization models on different remote Mosel instances
           and retrieve detailed solution info, reading binary solution files
           - XPRD version of runfoliopardistr.mos (see Getting_Started) -
* distfoliocbioev.[c|java] (requires: foliocbioev.mos, folio250.dat) 
           retrieve solution info during optimization model run,
           coordination via events
