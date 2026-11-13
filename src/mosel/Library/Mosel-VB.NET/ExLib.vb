Imports System.IO
Imports Mosel

' Example of working with models and accessing Mosel dynamic libraries

Module ExLib
    Public Sub RunExLib(ByVal Log As TextWriter)
        Dim mosel As XPRM
        Dim models(2) As XPRMModel

        ' Initialise Mosel
        mosel = XPRM.Init
        ' Set Mosel work directory to folder containing our example source code
        mosel.WorkDir = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly.Location).FullName
        ' Compile and Load the BIM files
        models(0) = mosel.CompileAndLoad("Models/burglari.mos")
        models(1) = mosel.CompileAndLoad("Models/chess2.mos")
        models(2) = mosel.CompileAndLoad("Models/trans.mos")
        Log.WriteLine("Models loaded")

        ' Display basic information about the models
        Dim model As XPRMModel
        For Each model In models
            Log.WriteLine("  {0}: {1} ({2}, '{3}' size:{4})", _
                model.Number, model.Name, model.SysComment, _
                model.UserComment, model.Size)
        Next
        Log.WriteLine()

        ' Enumerate all loaded modules and display information
        Log.WriteLine("Additional libraries loaded:")
        Dim mo As XPRMModule
        For Each mo In mosel.Modules
            Log.WriteLine( _
                "  {0} (version {1}) used by {2} model(s)", _
                mo.Name, mo.Version, mo.NumberOfReferences)
        Next
    End Sub
End Module
