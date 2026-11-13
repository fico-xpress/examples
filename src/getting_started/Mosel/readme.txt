Getting Started Examples
========================
File readme.txt (Mosel)                                 June 2023

--------------------------------------------------------------------------

The directory "Mosel" contains the examples discussed in the part
"Getting Started with Mosel" of "Getting Started with Xpress":


Chapter 3 "Inputting and Solving a Linear Programming problem"
* foliolp.mos: modeling and solving a small LP problem
* foliolperr.mos: LP model with syntax errors
* foliolps.mos: LP model using string indices 

Chapter 4 "Working with data"
* foliodata.mos (data file: folio.dat): data input from file, result 
  output to a file, model parameters
* folioodbc.mos (data files: folio.xls, folio.mdb, folio.sqlite): data input
  from a spreadsheet or database, result output to a spreadsheet or database,
  model parameters
* folioexcel.mos (data file: folio.xls): same as folioodbc.mos but
  with Excel-specific data input and output
* foliosheet.mos (data file: folio.xls): same as folioodbc.mos but
  with data input and output through generic spreadsheet access
* foliocsv.mos (data file: folio.csv): same as folioodbc.mos but
  with data input and output through generic spreadsheet access in CSV format

Chapter 5 "Drawing user graphs"
* folioloop.mos (data files: folio.dat, foliodev.dat): re-solving with 
  varied parameter settings
* folioloop_graph.mos (data files: folio.dat, foliodev.dat): folioloop with
  graph drawing
* foliolps_graph.mos: foliolps with graph drawing

Chapter 6 "Mixed Integer Programming"
* foliomip1.mos (data file: folio.dat): modeling and solving a small 
  MIP problem (binary variables)
* foliomip2.mos (data file: folio.dat): modeling and solving a small 
  MIP problem (semi-continuous variables)

Chapter 7 "Quadratic Programming"
* folioqp.mos (data file: folioqp.dat): modeling and solving a QP and 
  a MIQP problem
* folioqp_graph.mos (data files: folioqp.dat, folioqpgraph.dat):
  re-solving a QP problem with varied parameter settings, graphical solution 
  display
* folioqc.mos (data file: folioqp.dat): modeling and solving a QCQP 
* foliomiqc.mos (data file: folioqp.dat): modeling and solving a MIQCQP 

Chapter 8 "Heuristics"
* folioheur.mos (data file: folio.dat): heuristic solution of a MIP problem

Chapter 9 "Embedding a Mosel model in an application"
* folioexec.bas, folioexec.c, folioexec.java, folioexec-cs/folioexec.cs
  (requires foliodata.mos and folio.dat): executing a Mosel model 
* foliorun.bas, foliorun.c, foliorun.java, foliorun-cs/foliorun.cs
  (requires foliodata.bim and folio.dat): loading and running a BIM file
* folioparam.bas, folioparam.c, folioparam.java, folioparam-cs/folioparam.cs
  (requires foliodata.mos and folio.dat): parameterized model execution
* foliomat.c, foliomat.java, foliomat-cs/foliomat.cs (requires foliodata.mos and folio.dat):
  matrix output
* folioobj.c, folioobj.java, folioobj-cs/folioobj.cs (requires foliodata.mos and folio.dat):
  accessing model results
* runfolio.xlsm (requires folioexcel.mos):
  launching a Mosel model from Excel via VBA marcos

--------------------------------------------------------------------------


Advanced Evaluators Guide examples
==================================

foliomip3.mos - enlarged version of model foliomip.mos from the
                'Getting started' guide (more constraints, larger data set) 
		employed by all examples in the directory
		(to be used with data set folio10.dat or folio250.dat)

foliocb.mos   - defining an integer solution callback


***Multiple MIP solutions***
folioenumsol.mos  - using the solution enumerator


***Handling infeasibility***
folioinfeas.mos   - handling infeasibility through deviation variables
folioinfcause.mos - retrieving infeasible row/column from presolve
folioiis.mos      - retrieving IIS (LP infeasible)
foliomiis.mos     - retrieving IIS (MIP infeasible)
folionliis.mos    - retrieving IIS (NLP infeasible)
foliorep.mos      - using the built-in infeasibility repair functionality
foliorep_sol.mos  - same as foliorep, using an 'mpsol' object


***Data transfer in memory***
foliomemio.mos  - model version for demonstrating different data input methods
foliomemio2.mos - same as foliomemio.mos, grouping tables with identical
                  index sets in "initializations" blocks
foliocbio.mos   - model version writing out solutions via a MIP callback

runfolio.mos    - running foliomemio.mos with data transfer in memory
runfolio2.mos   - same running foliomemio2.mos
runfoliopar.mos - main model running several model instances in parallel

runfolio.c      - C application to run foliomemio.mos
runfolio2.c     - same running foliomemio2.mos
runfoliod.c     - C application to run foliomemio.mos using data callbacks
runfoliocbio.c  - C application retrieving solution output from Optimizer
                  callbacks in foliocbio.mos during optimization

runfolio.java   - Java application to run foliomemio.mos, passing data to/from
                  Java structures representing arrays
runfolio2.java  - same running foliomemio2.mos 
runfoliob.java  - Java application to run foliomemio.mos, passing data through 
                  buffers
runfoliod.java  - Java application to run foliomemio.mos using data callbacks
runfoliocbio.java  - Java application retrieving solution output from Optimizer
                  callbacks in foliocbio.mos during optimization

runfolio-cs     - C# application to run foliomemio.mos, passing data to/from
                  C# structures representing arrays
runfolio2-cs    - same running foliomemio2.mos 
runfoliob-cs    - C# application to run foliomemio.mos, passing data through 
                  buffers
runfoliocbio-cs  - C# application retrieving solution output from Optimizer
                  callbacks in foliocbio.mos during optimization


***Remote models on distributed architecture***
runfoliodistr.mos    - running foliomemio.mos on a remote instance of Mosel 
runfoliopardistr.mos - main model running several model instances in parallel,
                       each on a different (remote) instance of Mosel 


***Remote execution via XPRD***
See examples located in the Mosel Whitepapers directory MoselPar/XPRD:
distfolio.[c|java]    - running foliomemio.mos on a remote instance of Mosel,
                        data exchange via binary files
distfoliopar.[c|java] - running several model instances in parallel,
                        each on a different (remote) instance of Mosel
                        data exchange via binary files
runfoliodistr.[c|java] - running foliomemio.mos on a remote instance of Mosel,
                        data exchange via streams
distfoliocbioev.[c|java] - (model file: foliocbioev.mos)
                        data exchange during model run coordinated via
                        event messages


***XML & JSON data formats***
folioxml.mos    - (data file: folio.xml) same as foliodata.mos but using 
                  XML-format data files, reading data from an XML file, 
                  solution output in XML format on screen and to a new file 
folioxmlqp.mos  - (data files: folioqp.xml, folioqpvar.xml) same as folioqp.mos 
                  but using XML-format data files, reading XML files for
                  input, output of several solutions to a new XML file
runfolioxml.mos - (submodel: foliomemio.mos, data file: folio10.dat)
                  generate HTML output file as an XML document
foliojson.mos   - (data file: folio.json) same as foliodata.mos but using 
                  JSON-format data files, reading data from a JSON file, 
                  solution output in JSON format on screen and to a new file 


***HTTP***
foliohttpsrv.mos - (submodel: folioxml.mos, data file: folio.xml) 
                   starting an HTTP server managing requests from HTTP clients
foliohttpclient.mos - (data file: folio.xml) HTTP client exchanging XML data
                   files with an HTTP server 


***Mosel packages***
foliomipusrpkg.mos - (packages: usrpkg.mos or usrpkgtempl.mos, data file:
                   folio.dat)
                   working with dynamic package loading in order to allow 
                   users to modify the definition of an optimization
                   problem without disclosing the actual model source

--------------------------------------------------------------------------

To run the C examples with the provided makefile, the environment variable 
XPRESSDIR must be set to the installation directory of Xpress.
