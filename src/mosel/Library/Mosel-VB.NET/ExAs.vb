Imports System.IO
Imports Mosel

' Example of using arrays with index sets; different ways of enumerating arrays

Module ExAs
    Public Sub RunExAs(ByVal Log As TextWriter)
        Dim mosel As XPRM
        Dim model As XPRMModel
        Dim varr As XPRMArray
        Dim sets() As XPRMSet
        Dim vindex() As XPRMValue
        Dim indices() As Integer
        Dim dimensions As Integer
        Dim i As Integer

        ' Initialise Mosel
        mosel = XPRM.Init
        ' Set Mosel work directory to folder containing our example source code
        mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly.Location).FullName

        ' Compile and load a MOS file
        model = mosel.CompileAndLoad("Models/trans.mos")
        ' Run the model
        model.Run()

        ' Get the model object named 'x'
        ' It must be an array
        varr = model.FindIdentifier("x")
        ' Get the number of dimensions of the array
        dimensions = varr.Dim
        ' Get the indexing sets
        sets = varr.IndexSets

        ' We could use varr.ToString to obtain a summary of the array contents,
        ' but instead we'll demonstrate how to interate over the array's content
        ' directly
        Log.WriteLine("1. Logic entries:")
        ' Get the first entry of varr
        For Each indices In varr.Indices
            Log.Write("x(")
            ' Get the values for this index
            vindex = varr.DereferenceIndex(indices)
            ' Now, output them.  Note that we could call the utility method
            ' varr.IndexToString(indices) instead of doing all this, but again
            ' this wouldn't be such a useful demonstration
            If (dimensions > 1) Then
                For i = 0 To dimensions - 2
                    Log.Write(vindex(i).AsString & ",")
                Next
            End If
            Log.Write(vindex(dimensions - 1).AsString & "), ")
        Next
        Log.WriteLine()

        ' Now enumerate over the true entries - in a dense array this would
        ' be no different from the previous demonstration, so only do it for
        ' a dynamic array
        If (varr.IsDynamic) Then
            Log.WriteLine("2. True entries:")
            For Each indices In varr.TEIndices
                ' Functionally the same as above, but this time we'll demonstrate
                ' using IndexToStringrasExAs
                Log.Write("x" & varr.IndexToString(indices) & ", ")
            Next
        End If
        Log.WriteLine()

        model.Reset()
    End Sub

End Module
