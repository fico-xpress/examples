Imports System.IO
Imports Mosel

' Displays information about a Mosel DSO module

Module DispDso
    Public Sub RunDispDso(ByVal ModName As String, ByVal log As TextWriter)
        log.WriteLine("Will interrogate module '{0}'", ModName)

        Dim mosel As XPRM
        Dim ourModule As XPRMModule

        ' Initialise Mosel
        mosel = XPRM.Init

        ' Load the module
        ourModule = mosel.LoadModule(ModName)

        ' Output basic information about the module
        log.WriteLine("Module '{0}', version {1}", ourModule.Name, ourModule.Version)
        If ((Not ourModule.Certificate Is Nothing) And (ourModule.Certificate.Length > 0)) Then
            log.WriteLine(" ({0})", ourModule.Certificate)
        Else
            log.WriteLine("")
        End If
        log.WriteLine()

        ' Output a list of types defined within the module
        log.WriteLine("Types:")
        Dim nt As XPRMNativeType
        For Each nt In ourModule.Types
            log.WriteLine(" {0} (", nt.Name)
            If (nt.HasCreate) Then
                log.Write("create")
            End If
            If (nt.HasDelete) Then
                log.Write(",delete")
            End If
            If (nt.HasToString) Then
                log.Write(",tostring")
            End If
            If (nt.HasPRTBL) Then
                log.Write("+")
            End If
            If (nt.HasFromString) Then
                log.Write(",fromstring")
            End If
            log.WriteLine()
        Next
        log.WriteLine()

        ' List of control parameters
        log.WriteLine("Control Parameters:")
        Dim p As XPRMParameter
        For Each p In ourModule.Parameters
            log.Write(" {0}: {1} (", p.Name, p.TypeName)
            If (Not p.Description Is Nothing And p.Description.Length > 0) Then
                log.Write("{0},", p.Description)
            End If
            log.WriteLine(rwstatus(p) + ")")
        Next
        log.WriteLine()

        ' List of subroutines
        log.WriteLine("Procedures and Functions:")
        Dim proc As XPRMProcedure
        For Each proc In ourModule.Procedures
            dispProcFct(proc, log)
        Next
        log.WriteLine()

        ' List of IO drivers
        log.WriteLine("I/O drivers:")
        Dim iod As XPRMIODriver
        For Each iod In ourModule.IODrivers
            log.WriteLine(" {0}:{1}", iod.Name, IIf(Not iod.Info Is Nothing, iod.Info, ""))
        Next

    End Sub

    ' Return the r/w status of a control parameter
    Private Function rwstatus(ByVal p As XPRMParameter) As String
        If (p.IsReadable) Then
            If (p.IsWriteable) Then
                Return "r/w"
            Else
                Return "r"
            End If
        Else
            If (p.IsWriteable) Then
                Return "w"
            Else
                Return "?"
            End If
        End If
    End Function


    ' Display a prototype from a signature
    Private Function dispProcFct(ByVal proc As XPRMProcedure, ByVal log As TextWriter)
        Dim parms() As Char
        Dim i As Integer

        If (proc.TypeCode <> XPRMVarType.NOT) Then
            log.Write(" function {0}", proc.Name)
        Else
            log.Write(" procedure {0}", proc.Name)
        End If

        If (proc.NbParameters > 0) Then
            log.Write("(")
            parms = proc.ParameterTypes.ToCharArray
            i = 0
            Do While (i < parms.Length)
                If (i > 0) Then
                    log.Write(",")
                End If
                i = dispType(i, parms, log) + 1
            Loop
            log.Write(")")
        End If

        If (proc.TypeCode <> XPRMVarType.NOT) Then
            log.Write(":{0}", proc.TypeName)
        End If
        log.WriteLine()
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
