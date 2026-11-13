Attribute VB_Name = "ModuleUGVB"
'*******************************************************
' Mosel Visual Basic Interface
' Example program
' (c) 2016 Fair Isaac Corporation
'*******************************************************

Option Explicit
Option Base 1


Private Sub burglar_Click()
'*******************************************************
'  Mosel User Guide Example Problems
'  =================================
'  Compiling a model into a BIM file,
'  then load and run it.
'
'  (c) 2008 Fair Isaac Corporation
'      author: S.Heipcke, 2002
'*******************************************************
  Dim model
  Dim ret As Long
  Dim result As Long
  Dim outfile As String, moselfile As String
  
  ret = XPRMinit
  If ret Then
    MsgBox "Mosel initialization error (" & ret & ")"
    Exit Sub
  End If
    
  XPRMsetdefworkdir GetFullPath()
  moselfile = GetFullPath() & "\" & "burglar5"
  outfile = GetFullPath() & "\" & "vbout.txt"
'Compile burglar5.mos
  ret = XPRMcompmod(vbNullString, moselfile & ".mos", vbNullString, "Burglar problem")

  If ret <> 0 Then
    MsgBox "Compile error (" & ret & ")"
    GoTo done
  End If
  
'Load burglar5.bim
  model = XPRMloadmod(moselfile & ".bim", vbNullString)

  If model = 0 Then
    MsgBox "Error loading model"
    GoTo done
  End If
  
'Run the model
  ret = XPRMrunmod(model, result, "OUTFILE=""" & Replace(outfile, "\", "\\") & """")

  If ret <> 0 Then
    MsgBox "Execution error (" & ret & ")"
    GoTo done
  Else
    ShowFile outfile
  End If
  MsgBox vbNewLine & "model Burglar returned: " & result
  
done:
  XPRMfree
End Sub


Private Sub prime_Click()
'*******************************************************
'  Mosel User Guide Example Problems
'  =================================
'  Passing parameters to a Mosel program.
'
'  (c) 2008 Fair Isaac Corporation
'      author: S.Heipcke, 2002
'*******************************************************
  Dim model
  Dim ret As Long
  Dim result As Long
  Dim outfile As String, moselfile As String
    
  ret = XPRMinit
  If ret Then
    MsgBox "Mosel initialization error (" & ret & ")"
    Exit Sub
  End If
    
  XPRMsetdefworkdir GetFullPath()
  moselfile = GetFullPath() & "\" & "prime4"
  outfile = GetFullPath() & "\" & "vbout.txt"
'Compile prime4.mos
  ret = XPRMcompmod(vbNullString, moselfile & ".mos", vbNullString, "Prime numbers")

  If ret <> 0 Then
    MsgBox "Compile error (" & ret & ")"
    GoTo done
  End If
  
'Load prime4.bim
  model = XPRMloadmod(moselfile & ".bim", vbNullString)

  If model = 0 Then
    MsgBox "Error loading model"
    GoTo done
  End If
  
'Run with new parameter settings
  ret = XPRMrunmod(model, result, "LIMIT=500,OUTFILE=""" & Replace(outfile, "\", "\\") & """")

  If ret <> 0 Then
    MsgBox "Execution error (" & ret & ")"
    GoTo done
  Else
    ShowFile outfile
  End If
  MsgBox vbNewLine & "model Prime returned: " & result
  
done:
  XPRMfree
End Sub


Private Sub ShowFile(fn As String)
  Dim vs As String
  vs = CreateObject("Scripting.FileSystemObject").OpenTextFile(fn).ReadAll
  MsgBox vs
End Sub

Private Function GetFullPath() As String
  Dim path As String
  path = ThisWorkbook.path
  If Right(path, 1) = "\" Then path = Left(path, Len(path) - 1)
  GetFullPath = path
End Function

