Contents of this directory
==========================

All examples in this directory are explained in the whitepaper
`Using ODBC and other database interfaces with Mosel'.

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
    type the following command (assuming that the model is called 'mymod.mos')
      mosel exec mymod
(c) Mosel libraries:
    in a C or VBA program use XPRMexecmod, and in a Java or C# program use 
    the sequence XPRM.compile/XPRM.loadModel/XPRMModel.run to execute a 
    model (see the `Mosel User Guide' or `Mosel Libraries Reference Manual' 
    for details)

Introductory examples:

* duo.mos, douexc.mos (data files: data.mdb, data.xls, data.sqlite),
  duosheet.mos (data files: data.csv, data.xls, data.xlsx), duooci.mos:  
    data input using an ODBC connection ('odbc' driver or SQL statements), 
    the 'excel' IO driver, generic spreadsheet access (module 'mmsheet'),
    or OCI ('oci' driver or SQL statements)

* duo_out.mos, duoexc_out.mos (data files: data.mdb, data.xls, data.sqlite),
  duosheet_out.mos (data files: data.csv, data.xls, data.xlsx), 
  duooci_out.mos: 
    data output using an ODBC connection ('odbc' driver or SQL statements), 
    the 'excel' IO driver, generic spreadsheet access (module 'mmsheet'),
    or OCI ('oci' driver or SQL statements)

* dataframecsv.mos, dataframedb.mos, dataframesht.mos (data files:
  dframedata.csv, dframedata.xls, dframedata.xlsx): dataframe-format reading
  of CSV-format files, spreadsheets, and database tables


The following examples have several versions: 
'*' = [] - 'odbc' driver,
       1 - working with text data files,
       2 - ODBC connection using SQL statements,
       3 - 'excel' driver (Windows only),
       4 - Oracle (OCI) interface,
       5 - 'xls'/'xlsx' driver,
       6 - 'csv' driver

* soleg*.mos (data files: soleg.mdb, soleg.xls, soleg.csv, soleg.sqlite): 
    outputting solution values    

* indexeg*.mos (data files: indexeg.mdb, indexeg.xls, indexeg.csv,
  indexeg.sqlite): 
    dense vs. sparse data format    

* multicol*.mos (data files: multicol.dat, multicdd.dat, multicol.mdb,
  multicol.xls, multicol.csv, multicol.sqlite): 
    reading several arrays from a single table    
* multiout*.mos (data files: multiout.dat, multicol.mdb, multicol.xls, 
  multicol.csv, multicol.sqlite): 
    writing several arrays into a single table    
* multitab*.mos (data files: multitab.mdb, multitab.xls, multitab.csv,
  multitab.sqlite): 
    reading an array from several tables; array output split into several
    tables     

* datesinout*.mos (data files: datetime.mdb, datetime.xls, datetime.csv,
  datetime.sqlite):
    reading and writing dates and times
* listinout*.mos (data files: listdata.dat, listdata.mdb, listdata.xls, 
  listdata.csv, listdata.sqlite):
    reading and writing lists
* recordin*.mos / recordout*.mos 
  (data files: recorddata.dat, recorddata.mdb, recorddata.xls, recorddata.csv,
  recorddataout.csv, recorddata.sqlite):
    reading and writing data with record structure
* unioninout*.mos (data files: uniondatain.dat, uniondata.xls, uniondata.xlsx, 
  uniondata.csv, uniondata2.csv, uniondata.sqlite):
    reading and writing data of union types

* threedimarr*.mos
  (data files threedim.csv, threedim.xls, threedim.xlsx, threedim.mdb,
  threedim.sqlite):
    reading a 3-dimensional array from a 2-dim. (rectangular) table 
    (where first N-1 indices correspond to rows, and the last index 
    is given in the columns) 


Advanced SQL functionality:

* odbcinv.mos (data file: multicol.mdb, multicol.sqlite), 
  ociinv.mos (data file: ocidata.dat): 
    selecting columns/fields
* odbcselfunc.mos (data files: odbcsel.mdb, odbcsel.xls, odbcsel.sqlite), 
  ociselfunc.mos: 
    SQL selection statements
* odbcinspectdb.mos:
    retrieving structural information about the database and its tables
    
