Attribute VB_Name = "Module1"
Option Explicit
' ********************************************************
'  Mosel Library Example Problems
'  ==============================
'  file foliorun.bas
'  `````````````````
'  Loading and running a BIM file.
'
'  (c) 2008 Fair Isaac Corporation
'      author: S.Heipcke, Aug. 2003
' ********************************************************

Sub Main()
 Dim result As Long
 Dim model

' Initialize Mosel
 XPRMinit

' Load compiled model
 model = XPRMloadmod("foliodata.bim", "")

 XPRMrunmod model, result, ""
 XPRMunloadmod (model)
 model = XPRM_NULL
End Sub
