Imports System.IO
Imports Mosel

' Example for use of Mosel libraries (using 'dotnetstream' IOdriver for data exchange)

Module ExDrvsRaw
    ' Defines the Mosel source of our model 
    Private source_of_model As String = _
      "model drivers" & vbCrLf & _
 _
      "parameters" & vbCrLf & _
      " DATA=''" & vbCrLf & _
      " SOL=''" & vbCrLf & _
      "end-parameters" & vbCrLf & _
 _
      "public declarations" & vbCrLf & _
      " S:set of string" & vbCrLf & _
      " R:range" & vbCrLf & _
      " data:array(S,R) of real" & vbCrLf & _
      " sol:array(1..10) of real" & vbCrLf & _
      "end-declarations" & vbCrLf & _
 _
      "initialisations from 'dotnetraw:'" & vbCrLf & _
      " data as 'DATA(s,r,v)'" & vbCrLf & _
      "end-initialisations" & vbCrLf & _
 _
      "writeln('set   S=',S)" & vbCrLf & _
      "writeln('range R=',R)" & vbCrLf & _
      "writeln('array data=',data)" & vbCrLf & _
 _
      "forall(i in 1..10) sol(i):=i^2" & vbCrLf & _
 _
      "initialisations to 'dotnetraw:noindex'" & vbCrLf & _
      " sol as 'SOL'" & vbCrLf & _
      "end-initialisations" & vbCrLf & _
 _
      "end-model"

    ' Define a structure to store initial values for the array 'data'
    Public Class ModelDataElement
        Public s As String
        Public r As Integer
        Public v As Double

        Public Sub New(ByVal s As String, ByVal r As Integer, ByVal v As Double)
            Me.s = s
            Me.r = r
            Me.v = v
        End Sub

    End Class

    Public Sub RunExDrvsRaw(ByVal Log As TextWriter)
        ' Initialize Mosel
        Dim moselRT As XPRM = XPRM.Init

        ' Use a StringReader to compile and load the Mosel model directly from a .NET string
        Log.WriteLine("Compiling from memory...")
        Dim modelSourceReader As New StringReader(source_of_model)
        Dim model As XPRMModel = moselRT.CompileAndLoad(modelSourceReader)

        ' Send the model's output directly to the log
        model.SetDefaultStream(XPRMStreamType.F_OUTPUT_LINEBUF, Log)

        ' Create an array containing the initialization data
        Dim ModelData(3) As ModelDataElement
        ModelData(0) = New ModelDataElement("one", 2, 12.5)
        ModelData(1) = New ModelDataElement("two", 1, 15)
        ModelData(2) = New ModelDataElement("three", 16, 9)
        ModelData(3) = New ModelDataElement("hundred", 2, 17)
        ' Bind this array to the name "DATA"
        moselRT.Bind("DATA", ModelData)

        ' Create an array to receive solution data and bind it to the name 'SOL'
        Dim Solution(9) As Double
        moselRT.Bind("SOL", Solution)

        ' Run the model
        model.Run()

        ' Print the solution
        Log.WriteLine()
        Log.WriteLine()
        Log.Write("Solution values: ")
        Dim i As Integer
        For i = 0 To 9
            Log.Write(" {0}", Solution(i))
        Next
        Log.WriteLine()
        Log.WriteLine()

    End Sub

End Module
