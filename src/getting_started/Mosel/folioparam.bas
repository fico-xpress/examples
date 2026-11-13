Attribute VB_Name = "Module1"
Option Explicit
' ********************************************************
'  Mosel Library Example Problems
'  ==============================
'  file folioparam.bas
'  ```````````````````
'  Parameterized model execution.
'
'  (c) 2008 Fair Isaac Corporation
'      author: S.Heipcke, Aug. 2003
' ********************************************************

Sub Main()
 Dim result As Long
 Dim model

' Initialize Mosel
 XPRMinit

' Execute = compile/load/run a model
 XPRMexecmod "", "foliodata.mos", "OUTFILE=result2.dat,MAXRISK=0.4,MAXVAL=0.25", result, model

 XPRMunloadmod (model)
 model = XPRM_NULL
End Sub
