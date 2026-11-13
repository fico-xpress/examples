Imports System.IO
Imports Mosel

' Example of accessing problems and solution information

Module ExProb
    Public Sub RunExProb(ByVal Log As TextWriter)
        Dim mosel As XPRM
        Dim model As XPRMModel
        Dim varr, darr As XPRMArray
        Dim lgrade As XPRMLinCtr

        ' Initialise Mosel
        mosel = XPRM.Init
        ' Set Mosel work directory to folder containing our example source code
        mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly.Location).FullName
        ' Compile and load model
        model = mosel.CompileAndLoad("Models/blend2.mos")
        ' Run the model (it includes optimization)
        model.Run()

        ' Export the problem to a file in LP format (maximization)
        model.ExportProblem("p", "blend")

        ' Test whether optimal is found
        If (model.ProblemStatus = XPRMProblemStatus.PB_OPTIMAL) Then
            Log.WriteLine("Solution is optimal.")
        End If

        ' Print out the objective function value
        Log.WriteLine("Objective value: {0}", model.ObjectiveValue)

        ' Get the model objects 'x' and 'COST' (both arrays)
        varr = model.FindIdentifier("x")
        darr = model.FindIdentifier("COST")

        ' For each entry in array var, display solution value and corresponding
        ' cost
        Dim indices() As Integer
        For Each indices In varr.Indices
            Log.WriteLine( _
                "x{0}={1} (COST: {2})", _
                varr.IndexToString(indices), _
                varr.Get(indices).AsMPVar.Solution, _
                darr.GetAsReal(indices) _
            )
        Next

        ' Get the model object 'LoGrade'
        ' It must be a reference to a linear constraint
        lgrade = CType(model.FindIdentifier("LoGrade"), XPRMReference).Value.AsLinCtr
        Log.WriteLine( _
            "LoGrade: activity={0}, dual={1}", _
            lgrade.Activity, _
            lgrade.Dual _
        )

        model.Reset()
    End Sub
End Module
