Not compatible with version of Mosel < 1.4.0.

This example demonstrates the following

- How to start and stop a mosel model from Excel/VBA
- How to stop the XPRMrunmod call from locking up the Excel interface
- How to capture the Mosel output and print to a worksheet and the Immediate window


The IO driver functionality is used with XPRMsetdefstream to redirect the Mosel output streams to 
a VB callback function. 

Inside this callback function we do 3 tasks:

1. Print the message to our sheet and the VB editor's Immediate window
2. Check to see if our stop flag has been set and stop Mosel if it has
3. Call DoEvents to tell Excel to process its Windows message queue and thus service its GUI interface



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
