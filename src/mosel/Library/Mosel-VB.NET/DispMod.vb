Imports System.IO
Imports Mosel

' Displays the contents of a model

Module DispMod
    Public Sub RunDispMod(ByVal ModName As String, ByVal Log As TextWriter)
        Dim mosel As XPRM
        Dim model As XPRMModel
        Dim proc As XPRMProcedure

        ' Initialize Mosel and load the model
        mosel = XPRM.Init
        ' Set Mosel work directory to folder containing our example source code
        mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly.Location).FullName

        Log.WriteLine("Compiling model {0}", ModName)
        model = mosel.CompileAndLoad(ModName)
        Log.WriteLine()

        ' List model parameters
        Dim p As XPRMParameter
        Log.WriteLine("Model parameters:")
        For Each p In model.Parameters
            Log.WriteLine(" {0}", p.Name)
        Next
        Log.WriteLine()

        ' List symbols
        Log.WriteLine("Symbols:")
        Dim symb As XPRMIdentifier
        For Each symb In model.Identifiers
            Select Case symb.StructCode

                Case XPRMVarStruct.CONST ' Constant: display value
                    Log.WriteLine(" {0}={1}", symb.Name, symb)

                Case XPRMVarStruct.REF ' Reference: display type
                    Log.WriteLine(" {0}: {1}", symb.Name, symb.TypeName)

                Case XPRMVarStruct.ARRAY ' Array: display type
                    Log.WriteLine(" {0}: array of {1}", symb.Name, symb.TypeName)

                Case XPRMVarStruct.SET ' Set: display type
                    Log.WriteLine(" {0}: set of {1}", symb.Name, symb.TypeName)

                Case XPRMVarStruct.PROC ' Subroutine
                    proc = CType(symb, XPRMProcedure)
                    Do ' look for all overloading procedures/functions
                        dispProcFct(proc, Log) ' display the prototype
                        proc = proc.Next
                    Loop While (Not proc Is Nothing)

                Case Else ' Unknown
                    Log.WriteLine(" {0}: ?", symb.Name)

            End Select
        Next
    End Sub

    ' Display a prototype from a signature
    Private Function dispProcFct(ByVal proc As XPRMProcedure, ByVal log As TextWriter)
        Dim parms() As Char
        Dim i As Integer

        If (proc.TypeCode <> XPRMVarType.NOT) Then
            Log.Write(" function {0}", proc.Name)
        Else
            Log.Write(" procedure {0}", proc.Name)
        End If

        If (proc.NbParameters > 0) Then
            Log.Write("(")
            parms = proc.ParameterTypes.ToCharArray
            i = 0
            Do While (i < parms.Length)
                If (i > 0) Then
                    Log.Write(",")
                End If
                i = dispType(i, parms, Log) + 1
            Loop
            Log.Write(")")
        End If

        If (proc.TypeCode <> XPRMVarType.NOT) Then
            Log.Write(":{0}", proc.TypeName)
        End If
        Log.WriteLine()
    End Function


    ' Display a type name from a signature
    Private Function dispType(ByVal i As Integer, ByVal parms As Char(), ByVal log As TextWriter)
        Dim j As Integer
        Select Case parms(i)
            Case "i"
                log.Write("integer")
            Case "r"
                log.Write("real")
            Case "s"
                log.Write("string")
            Case "b"
                log.Write("boolean")
            Case "v"
                log.Write("mpvar")
            Case "c"
                log.Write("linctr")
            Case "I"
                log.Write("range")
            Case "a"
                log.Write("array")
            Case "e"
                log.Write("set")
            Case "|"
                i = i + 1
                Do
                    log.Write(parms(i))
                    i = i + 1
                Loop While (parms(i) <> "|")
            Case "!"
                i = i + 1
                Do
                    log.Write(parms(i))
                    i = i + 1
                Loop While (parms(i) <> "!")
            Case "A"
                log.Write("array (")
                i = i + 1
                j = i
                Do While (parms(i) <> ".")
                    If (j <> i) Then
                        log.Write(",")
                    End If
                    i = dispType(i, parms, log) + 1
                Loop
                log.Write(") of ")
                i = dispType(i + 1, parms, log)
            Case "E"
                log.Write("set of ")
                i = i + 1
                i = dispType(i, parms, log)
            Case Else
                log.Write("?")
        End Select
        Return i
    End Function
End Module
