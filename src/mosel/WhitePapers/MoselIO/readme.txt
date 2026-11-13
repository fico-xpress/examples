Contents of this directory
==========================

All examples in this directory are explained in the whitepaper
`Generalized file handling in Mosel'.

To run these examples, Mosel and the Xpress Solver must have
been installed and the related environment variables (MOSEL, PATH,
LD_LIBRARY_PATH, XPRESSDIR) set up correctly. 


The Mosel models may be run in any of the standard ways of running models:
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

For the C programs, please use the provided makefiles.


Standard version of the example problem
  burglar.mos - data in the model, integer indices
  burglari.mos - data in the model, string indices
  burglar2.mos, burglar.dat - reading data from a text file
  
Data source access from Mosel models
  burglar2o.mos, burglar.mdb, burglar.xls - reading data with mmodbc.odbc 
                (no database is provided under Unix) 
  burglar2sql.mos, burglar.sqlite - reading data from SQLite, using SQL or ODBC 
  burglar2e.mos, burglar.xls - reading data with mmsheet.excel 
                (Windows only) 
  burglar2dd.mos, burglardd.dat - reading data with mmetc.diskdata
  burglar2ff.mos, burglarff.dat - reading data in free format
  burglar2x.mos, burglar.xml - reading data in XML format
  burglar2j.mos, burglar.json - reading data in JSON format


To run the following C examples, the environment variable MOSEL_DSO 
must be set to this directory for Mosel to be able to locate the modules.


Exchange of information with embedded models
  iodrvmem.c  - working in memory ('mem' driver): compilation of a model
                held in memory to memory
  iodrvmem2.c - working in memory ('mem' driver): compilation of a model
                file to memory
  iodrvraw.c  - binary data format ('raw' driver): compilation of a model
                held in memory to memory
  iodrvraw2.c - binary data format ('raw' driver): working with physical 
                model files
  iodrvcb.c   - output to callback functions ('cb' driver); working with
                system file descriptors ('sysfd' driver)

Java and .NET versions
  See subdirectories C2 and D3 of the Mosel User guide examples folder


Bindrv / 'bin:'
  burgbindata.[c|java], burglar2m.mos - use of BinDrv for reading and writing 


mmjobs versions
  runburglar.mos, burglar.mos      - compile/load/run another model
  runburglario.mos, burglar2r.mos  - data exchange between models in memory 
                                     using bin format
  runburglarior.mos, burglar2r.mos - data exchange between models in memory
                                     using raw format
  runburglardistr.mos, burglar2m.mos  - data exchange between models in memory 
                                        on the local host
  runburglardistr2.mos, burglar2m.mos - data exchange between models in memory
                                        on the remote Mosel instance
  runburglarmem.mos                - compiling a model source from memory
  

User-defined drivers
  The compression and code generation examples are located in the 'modules'
  subdirectory of the standard set of Mosel examples.
