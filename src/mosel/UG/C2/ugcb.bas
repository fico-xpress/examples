Attribute VB_Name = "ModuleUGcb"
'*******************************************************
'  Mosel User Guide Example Problems
'  =================================
'  Retrieving the output of a model run via a callback.
'
'  (c) 2008 Fair Isaac Corporation, rev. Apr. 2016
'*******************************************************
Option Explicit

Private ROWNUM As Long
Public Sub example()
  Dim nReturn As Long
  Dim result As Long
  Dim module
    
  ClearColumn
' Initialize Mosel. Must be called first
  nReturn = XPRMinit
  If nReturn <> 0 Then
    PrintLn ("Failed to initialize Mosel")
    Exit Sub
  End If

' Redirect the output and error streams to the callback
  nReturn = XPRMsetdefstream(0, XPRM_F_WRITE, XPRM_IO_CB(AddressOf OutputCB))
  nReturn = XPRMsetdefstream(0, XPRM_F_ERROR, XPRM_IO_CB(AddressOf OutputCB))

  PrintLn "Starting model..."
 
' Run the model
  nReturn = XPRMexecmod("", GetFullPath() & "\" & "burglar10.mos", _
                       "FULLPATH='" & GetFullPath() & "'", result, module)
  If nReturn <> 0 Then
    PrintLn ("Failed to execute model")
    GoTo done
  Else
    PrintLn "Finished model"
  End If
    
 done:
   XPRMfree
End Sub

#If VBA7 Then
Private Sub OutputCB(ByVal model As LongPtr, ByVal ref As LongPtr, _
                    ByVal msg As String, ByVal size As Long)
  ' Output to the spreadsheet
  Call PrintLn(msg)
End Sub
#Else
Private Sub OutputCB(ByVal model As Long, ByVal ref As Long, _
                    ByVal msg As String, ByVal size As Long)
  ' Output to the spreadsheet
  Call PrintLn(msg)
End Sub
#End If

Public Sub PrintLn(ByVal msg As String)
  ' strip any trailing newlines first
  If Right(msg, Len(vbLf)) = vbLf Then msg = Left(msg, Len(msg) - Len(vbLf))
  If Right(msg, Len(vbCr)) = vbCr Then msg = Left(msg, Len(msg) - Len(vbCr))
  Worksheets("Run Model").Cells(ROWNUM, 2) = Trim(msg)
  ROWNUM = ROWNUM + 1
End Sub

Sub ClearColumn()
  Worksheets("Run Model").Columns(2).ClearContents
  ROWNUM = 1
End Sub

Function GetFullPath() As String
  Dim path As String
  path = ThisWorkbook.path
  If Right(path, 1) = "\" Then path = Left(path, Len(path) - 1)
  GetFullPath = path
End Function
