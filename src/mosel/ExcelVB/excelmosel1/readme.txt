This package contains an example of how to launch Xpress-Mosel from a VBA script 
within Microsoft Excel and retrieve the results of running a model.

When a spreadsheet is open within Excel, Excel places a read-only lock on the 
file such that the external (ODBC, XLS/XLSX) drivers cannot write to the 
spreadsheet. This means that if you execute a Mosel model, either from the 
command line console or from Xpress Workbench, the spreadsheet must be closed 
if you wish to export data from the model to the spreadsheet via these drivers. 
(Alternatively, you can use the Excel COM driver mmsheet.excel that allows you 
to keep the spreadsheet open while writing to it from Mosel.)

This example shows how to retrieve the results using the VBA interface 
to Mosel, and then writing the data to the spreadsheet from the VBA macro. 
Mosel's Excel interface is used by the model to read the input data 
from the spreadsheet, but is NOT used to output the results.

Run the macro 'test' in the Excel spreadsheet to run the Mosel model
repeatedly with modified input data, displaying the results in the spreadsheet.
If the model 'excelmosel1.mos' is run directly, input data are read from the
spreadsheet without writing back to it.


Changes to Mosel VBA interfaces for Office 2010

64-bit support
Following the introduction of VBA7 as part of Office 2010, Xpress now has
three versions of VBA to contend with:

VBA 6 32-bit
VBA 7 32-bit
VBA 7 64-bit

It's the last that has brought greatest changes to VBA, as the long-standing
convention of storing pointers and handles in VBA Long types is no longer
tenable.

VBA 7 makes the following changes:

1) Addition of a 64-bit LongLong type, and a VarType() value vbLongLong
(64-bit VBA only)
2) Addition of a LongPtr type that is an alias to Long on 32-bit and to
LongLong on 64-bit
3) Conditional compilation symbols VBA7 and Win64, for where code cannot be
version or platform agnostic
4) A PtrSafe declaration be attached to Declare statements to show
32-vs-64-bit portability has been taken into account (compulsory on 64-bit)

Representing pointers
While most Mosel pointer types in VBA were represented as Long, these should
now be declared as LongPtr on VBA 7. However, this may lead to a difficult
transition period when migrating from VBA 6 (or maintaining a VBA script that
needs to work in both versions), and a workable compromise is to declare them
in your code as Variant. See the example code for details. This only applies
to pointer and handle types: return codes and array indexes, etc should still
be declared as Long (i.e. a C int). See the changes made in XPRM.bas, where
most API functions are declared twice.


Note in VBA callback function signatures, parameters or return values
representing pointer types must still be declared as LongPtr in VBA 7, and
Long elsewhere. There is a new constant, XPRM_PTR_VARTYPE, that is the
VarType() value for pointers on each platform. The ExcelMosel2 example
shows how to write a Callback handler that works in both VBA6 and VBA7.

Passing VBA arrays is not so easily handled: the parameter to XPRMgetarrsets
needs to be passed the right type, so needs to be declared as an array of Long
or LongPtr as appropriate. There is a new helper function,
XPRMgetarrsetstovariant, that taks a ByRef Variant which can contain a
pre-sized array of Long (32-bit only), LongLong (VBA 7 64-bit only), LongPtr
(VBA 7 only), or Variant.

Breaking changes
While fixing the code to be 64-bit-friendly, the following anomalies were
rectified, which required breaking compatibility:
1) XPRMgetnextpbcomp now takes a ref parameter of type ByVal Long[Ptr], rather
than ByRef Long
2) XPRMgetfieldval now has appropriate signature for the Mosel function, and is
routed via xprmvb to handle the XPRMalltypes->VARIANT conversion
3) XPRM_ADDRESSOF_STR, XPRM_ADDRESSOF_LNG, and XPRM_ADDRESSOF_DBL have been removed
4) XPRMautounloaddso is now a Sub, rather than a Function (implicitly returning a Variant)
