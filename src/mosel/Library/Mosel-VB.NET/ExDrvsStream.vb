Imports System.IO
Imports Mosel

' Example for use of Mosel libraries (using 'dotnetstream' IOdriver for data exchange)

Module ExDrvsStream
    ' Defines the Mosel source of our model 
    Private source_of_model As String = _
      "model Blend" & vbCrLf & _
      "uses ""mmxprs""" & vbCrLf & _
      "public declarations" & vbCrLf & _
      " ROres = 1..2" & vbCrLf & _
      " REV = 125" & vbCrLf & _
      " MINGRADE = 4" & vbCrLf & _
      " MAXGRADE = 5" & vbCrLf & _
      " COST: array(ROres) of real" & vbCrLf & _
      " AVAIL: array(ROres) of real" & vbCrLf & _
      " GRADE: array(ROres) of real" & vbCrLf & _
      " x: array(ROres) of mpvar" & vbCrLf & _
      "end-declarations" & vbCrLf & _
      "" & vbCrLf & _
      "initializations from 'dotnetstream:BlendIni'" & vbCrLf & _
      " COST" & vbCrLf & _
      " AVAIL" & vbCrLf & _
      " GRADE" & vbCrLf & _
      "end-initializations" & vbCrLf & _
      vbCrLf & _
      "Profit:= sum(o in ROres) (REV-COST(o))*x(o)" & vbCrLf & _
      "LoGrade:= sum(o in ROres) (GRADE(o)-MINGRADE) * x(o) >= 0" & vbCrLf & _
      "UpGrade:= sum(o in ROres) (MAXGRADE-GRADE(o)) * x(o) >= 0" & vbCrLf & _
      "" & vbCrLf & _
      "forall(o in ROres) x(o)<=AVAIL(o)" & vbCrLf & _
      "" & vbCrLf & _
      "maximize(Profit)" & vbCrLf & _
      "writeln(""Objective:"", getobjval)" & vbCrLf & _
      "end-model"

    ' Defines the initialisation data for our model
    Dim init_data As String = _
      "COST: [85 93]" & vbCrLf & _
      "AVAIL: [60 45]" & vbCrLf & _
      "GRADE: [2.1 6.3]" & vbCrLf


    Public Sub RunExDrvsStream(ByVal Log As TextWriter)
        ' Initialize Mosel
        Dim moselRT As XPRM = XPRM.Init

        ' Use a StringReader to compile and load the Mosel model directly from a .NET string
        Log.WriteLine("Compiling from memory...")
        Dim modelSourceReader As New StringReader(source_of_model)
        Dim model As XPRMModel = moselRT.CompileAndLoad(modelSourceReader)

        ' Bind a stream to read the initialization data to the name 'BlendIni'
        model.Bind("BlendIni", New StringReader(init_data))

        ' Send the model's output directly to the log
        model.SetDefaultStream(XPRMStreamType.F_OUTPUT_LINEBUF, Log)

        ' And finally run the model
        model.Run()
    End Sub

End Module
