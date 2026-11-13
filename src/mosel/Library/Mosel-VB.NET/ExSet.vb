Imports System.IO
Imports Mosel

' Example of accessing sets in Mosel
Module ExSet

    Public Sub RunExSet(ByVal Log As TextWriter)
        Dim mosel As XPRM
        Dim model As XPRMModel
        Dim mySet As XPRMSet
        Dim first, last, i As Integer

        ' Initialise Mosel
        mosel = XPRM.Init
        ' Set Mosel work directory to folder containing our example source code
        mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly.Location).FullName
        ' Compile and load model
        model = mosel.CompileAndLoad("Models/burglari.mos")
        ' And run it
        model.Run()

        ' Get the model object named 'ITEMS'
        ' (it must be a set)
        mySet = model.FindIdentifier("ITEMS")

        If (Not mySet.IsEmpty) Then
            ' Items in a set are indexed by numbers
            ' So get the first and last indexes
            first = mySet.FirstIndex
            last = mySet.LastIndex
            Log.WriteLine("Elements of set ITEMS:")
            For i = first To last
                Log.Write(" {0}, ", mySet.GetAsString(i))
            Next
            Log.WriteLine()

            ' We've written this out explicitly to demonstrate set access, but the
            ' set actually knows how to output itself.  Uncomment the following line
            ' to see this in action
            ' Log.WriteLine(mySet.ToString)

            If (mySet.GetIndex("CD player") < 0) Then
                Log.WriteLine("'CD player' is not contained in 'ITEMS'")
            End If

        End If
    End Sub

End Module
