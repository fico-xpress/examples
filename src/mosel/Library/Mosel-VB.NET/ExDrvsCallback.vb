'********************************************************
'*  Mosel Library Examples                              *
'*  ======================                              *
'*                                                      *
'*  file ExDrvsCallback.vb                              *
'*  ```````````````````                                 *
'*  Example for the use of the Mosel libraries          *
'*  (using dotnet: I/O driver for data exchange via     *
'*  callbacks                                           *
'*                                                      *
'*  (c) 2011 Fair Isaac Corporation                     *
'*      author: J.Farmer, Y. Colombani, 2011            *
'********************************************************

Imports System
Imports System.IO
Imports Mosel

Module ExDrvsCallback

    Private OutputLog As TextWriter

    ' Define the Mosel source of our model
    Private source_of_model As String = _
      "model tstcb" & vbCrLf & _
      "uses 'mmsystem'" & vbCrLf & _
      "parameters" & vbCrLf & _
      " ICB_INITFROM=''" & vbCrLf & _
      " ICB_INITTO=''" & vbCrLf & _
      "end-parameters" & vbCrLf & _
 _
      "public declarations" & vbCrLf & _
      " v_i:integer" & vbCrLf & _
      " v_r:real" & vbCrLf & _
      " v_s:string" & vbCrLf & _
      " v_b:boolean" & vbCrLf & _
      " v_d:date" & vbCrLf & _
 _
      " s_i:set of integer" & vbCrLf & _
      " l_i:list of integer" & vbCrLf & _
 _
      " s_d:set of date" & vbCrLf & _
      " l_d:list of date" & vbCrLf & _
 _
      " a_i:array(range) of integer" & vbCrLf & _
      " Rx:range" & vbCrLf & _
      " a_s:array(Rx) of string" & vbCrLf & _
      " a_r:array(Rx) of real" & vbCrLf & _
 _
      " R=record" & vbCrLf & _
      "    public i:integer" & vbCrLf & _
      "    public s:set of integer" & vbCrLf & _
      "   end-record" & vbCrLf & _
      " r:R" & vbCrLf & _
      " a_R:array(range) of R" & vbCrLf & _
      "end-declarations" & vbCrLf & _
 _
      "initialisations from ICB_INITFROM" & vbCrLf & _
      " v_i" & vbCrLf & _
      " v_r" & vbCrLf & _
      " v_s" & vbCrLf & _
      " v_b" & vbCrLf & _
      " v_d" & vbCrLf & _
 _
      " s_i" & vbCrLf & _
      " l_i" & vbCrLf & _
 _
      " s_d" & vbCrLf & _
      " l_d" & vbCrLf & _
 _
      " a_i" & vbCrLf & _
      " [a_s,a_r] as 'ax'" & vbCrLf & _
 _
      " r" & vbCrLf & _
      " a_R" & vbCrLf & _
      "end-initialisations" & vbCrLf & _
 _
      " writeln('v_i=',v_i)" & vbCrLf & _
      " writeln('v_r=',v_r)" & vbCrLf & _
      " writeln('v_s=',v_s)" & vbCrLf & _
      " writeln('v_b=',v_b)" & vbCrLf & _
      " writeln('v_d=',v_d)" & vbCrLf & _
      " writeln('s_i=',s_i)" & vbCrLf & _
      " writeln('l_i=',l_i)" & vbCrLf & _
      " writeln('s_d=',s_d)" & vbCrLf & _
      " writeln('l_d=',l_d)" & vbCrLf & _
      " writeln('a_i=',a_i)" & vbCrLf & _
      " writeln('a_r=',a_r)" & vbCrLf & _
      " writeln('a_s=',a_s)" & vbCrLf & _
      " writeln('r=',r)" & vbCrLf & _
      " writeln('a_R=',a_R)" & vbCrLf & _
 _
      "initialisations to ICB_INITTO" & vbCrLf & _
      " v_i" & vbCrLf & _
      " v_r" & vbCrLf & _
      " v_s" & vbCrLf & _
      " v_b" & vbCrLf & _
      " v_d" & vbCrLf & _
 _
      " s_i" & vbCrLf & _
      " l_i" & vbCrLf & _
 _
      " s_d" & vbCrLf & _
      " l_d" & vbCrLf & _
 _
      " a_i" & vbCrLf & _
 _
      " r" & vbCrLf & _
      " a_R" & vbCrLf & _
      "end-initialisations" & vbCrLf & _
      "end-model"


    ' A function to initialize the Mosel data-structures via callback
    Private Function initializeFrom(ByVal ictx As XPRMInitializeContext, ByVal label As String, ByVal type As XPRMTyped) As Boolean

        Try

            Select Case label
                Case "v_i" ' v_i:999
                    ictx.Send(999)
                    Return True

                Case "v_r" ' v_r:999.99
                    ictx.Send(999.99)
                    Return True

                Case "v_b" ' v_b:false
                    ictx.Send(False)
                    Return True

                Case "v_s" ' v_s:"tralala"
                    ictx.Send("tralala")
                    Return True

                Case "v_d" ' v_d:"2012-12-12"
                    ictx.Send("2012-12-12")
                    Return True

                Case "s_i", "l_i" ' s_d:[10 20 30 ... ]
                    ictx.Send(XPRMInitializeControl.OpenList)
                    For i As Integer = 1 To 10
                        ictx.Send(i * 10)
                    Next
                    ictx.Send(XPRMInitializeControl.CloseList)
                    Return True

                Case "s_d", "l_d" ' s_d:["2001-01-11" "2002-02-21" ... ]
                    ictx.Send(XPRMInitializeControl.OpenList)
                    For i As Integer = 1 To 10
                        ictx.Send(String.Format("{0}-{1}-{2}", 2000 + i, i, i + 1))
                    Next
                    ictx.Send(XPRMInitializeControl.CloseList)
                    Return True

                Case "a_i" '  // a_i:[ (1) 10 (2) 20 ... ]
                    ictx.Send(XPRMInitializeControl.OpenList)
                    For i As Integer = 1 To 10
                        ictx.Send(XPRMInitializeControl.OpenIndices)
                        ictx.Send(i)
                        ictx.Send(XPRMInitializeControl.CloseIndices)
                        ictx.Send(i * 10)
                    Next
                    ictx.Send(XPRMInitializeControl.CloseList)
                    Return True

                Case "ax" '   ax:[ (1) [ "aa1" 1.23 ] (2) [ "aa2" 2.46 ] ... ]
                    ictx.Send(XPRMInitializeControl.OpenList)
                    For i As Integer = 1 To 10
                        ictx.Send(XPRMInitializeControl.OpenIndices)
                        ictx.Send(i)
                        ictx.Send(XPRMInitializeControl.CloseIndices)
                        ictx.Send(XPRMInitializeControl.OpenList)
                        ictx.Send(String.Format("aa{0}", i))
                        ictx.Send(CType(i * 1.23, Double))
                        ictx.Send(XPRMInitializeControl.CloseList)
                    Next
                    ictx.Send(XPRMInitializeControl.CloseList)
                    Return True

                Case "r" '   r:[ 123 [ 10 20 30 ] ]
                    ictx.Send(XPRMInitializeControl.OpenList)
                    ictx.Send(123)
                    ictx.Send(XPRMInitializeControl.OpenList)
                    For i As Integer = 1 To 3
                        ictx.Send(i * 10)
                    Next
                    ictx.Send(XPRMInitializeControl.CloseList)
                    ictx.Send(XPRMInitializeControl.CloseList)
                    Return True

                Case "a_R" '   a_R:[ (1) [10 [10 20 30] ] (1) [20 [20 40 60] ] ... ]
                    ictx.Send(XPRMInitializeControl.OpenList)
                    For i As Integer = 1 To 10
                        ictx.Send(XPRMInitializeControl.OpenIndices)
                        ictx.Send(i)
                        ictx.Send(XPRMInitializeControl.CloseIndices)
                        ictx.Send(XPRMInitializeControl.OpenList)
                        ictx.Send(i * 10)
                        ictx.Send(XPRMInitializeControl.OpenList)
                        For j As Integer = 1 To 3
                            ictx.Send(j * i * 10)
                        Next
                        ictx.Send(XPRMInitializeControl.CloseList)
                        ictx.Send(XPRMInitializeControl.CloseList)
                    Next
                    ictx.Send(XPRMInitializeControl.CloseList)
                    Return True

                Case Else
                    OutputLog.WriteLine("Label '{0}' not found", label)
                    Return False
            End Select

        Catch e As Exception
            OutputLog.WriteLine("Label '{0}' could not be initialized - {1}", label, e.Message)
            Return False
        End Try
    End Function


    ' A method to retrieve data from Mosel
    Private Function initializeTo(ByVal label As String, ByVal val As XPRMValue) As Boolean
        OutputLog.WriteLine(".NET: {0} = {1}", label, val)
        Return True
    End Function




    ' Main function
    Public Sub RunExDrvsCallback(ByVal Log As TextWriter)
        ' Initialize mosel
        Dim mosel As XPRM = XPRM.Init
        ' Set default output stream to stdout
        mosel.SetDefaultStream(XPRMStreamType.F_OUTPUT_LINEBUF, Log)
        mosel.SetDefaultStream(XPRMStreamType.F_ERROR, Log)
        OutputLog = Log

        ' Compile and load the model
        Dim model As XPRMModel = mosel.CompileAndLoad(New StringReader(source_of_model))

        ' Set the execution parameters and bind the variables
        model.SetExecParam("ICB_INITFROM", "dotnet:cbinitfrom")
        model.SetExecParam("ICB_INITTO", "dotnet:cbinitto")
        model.Bind("cbinitfrom", New XPRMInitializationFrom(AddressOf initializeFrom))
        model.Bind("cbinitto", New XPRMInitializationTo(AddressOf initializeTo))

        ' Run the model
        model.Run()

    End Sub

End Module

