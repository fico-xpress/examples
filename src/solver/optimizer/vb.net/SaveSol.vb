Imports System
Imports Microsoft.VisualBasic
Imports System.IO
Imports Optimizer

Module SaveSol
    ' Globals
    Private prob As XPRSprob
    Private gnCol As Integer            ' Numberfile of columns in the problem matrix
    Private gpIntSol() As Double        ' Integer solution values
    Private pOutput As StreamWriter

    Public Sub RunSaveSol(ByVal Log As TextWriter)
        Const sProblem As String = "burglar"         ' Problem name
        Const sOutFile As String = "savesol.out"                ' Output file name

        Dim nSol As Integer                     ' Number of integer solutions found
        Dim dBestObj As Double                  ' Best objective value found
        Dim nNodes As Integer                   ' Number of nodes solved in the global search
        Dim nActNodes As Integer                ' Number of active nodes ignored by the search
        Dim nLastNode As Integer                ' Node at which the last integer solution was found
        Dim dBestBnd As Double                  ' Best bound found in the global search
        Dim nGlStatus As Optimizer.MIPStatus    ' Global search status - complete, incomplete, etc
        Dim i As Integer                        ' Loop counter

        prob = Nothing

        Try
            ' Open the output file, first deleting it if it already exists
            If (File.Exists(sOutFile)) Then
                File.Delete(sOutFile)
            End If
            pOutput = New StreamWriter(sOutFile)

            ' Initialise Optimizer
            XPRS.Init("")

            prob = New XPRSprob

            ' Tell Optimizer to call optimizermsg whenever a message is output
            prob.AddMessageCallback(New Optimizer.MessageCallback(AddressOf HandleOptimizerMessage), Log)

            ' Allow no cuts - so the problme does not solve too quickly
            prob.CutStrategy = 0

            ' Read the problem file
            prob.ReadProb(frmMain.sDataDirPath & "/" & sProblem, "")

            ' Tell Optimizer to presolve every integer solution found, save it to memory and
            ' print the solution values to the output file

            ' Call function printsol whenever an integer solution is found
            prob.AddIntsolCallback(New Optimizer.IntsolCallback(AddressOf PrintSol), Log)

            ' Get the number of columns and allocate space for the solution array
            gnCol = prob.Cols
            ReDim gpIntSol(gnCol)

            ' Search for integer solutions
            Log.WriteLine("Solving problem {0}" & vbCrLf & vbCrLf, sProblem)
            prob.MipOptimize()

            ' Retrieve the results of the global search

            ' Get the number of integer solutions found
            nSol = prob.MIPSols

            ' Get the objective value of the best integer solution found
            dBestObj = prob.MIPObjVal

            ' Get the number of outstanding nodes
            nActNodes = prob.ActiveNodes

            ' Get th enode at which the last feasible integer solution was found
            nLastNode = prob.MIPSolNode

            ' Get the number of nodes solved
            nNodes = prob.Nodes

            ' Get the value of the best bound
            dBestBnd = prob.BestBound

            ' Get the global status
            nGlStatus = prob.MIPStatus

            ' Display the results of the global search
            Select Case nGlStatus
                Case MIPStatus.NoSolutionFound
                    Log.WriteLine("Global search incomplete" & vbCrLf)
                    Log.WriteLine("  No integer solution found")
                    Log.WriteLine("  {0} nodes searched", nNodes)
                    Log.WriteLine("  {0} nodes remaining in search", nActNodes)
                    Log.WriteLine("  Best bound {0}" & vbCrLf, dBestBnd)
                Case MIPStatus.Solution
                    Log.WriteLine("Global search incomplete" & vbCrLf)
                    Log.WriteLine("  {0} integer solution{1} found", nSol, IIf(nSol = 1, "", "s"))
                    Log.WriteLine("  {0} nodes searched", nNodes)
                    Log.WriteLine("  {0} nodes remaining in search", nActNodes)
                    Log.WriteLine("  Best bound {0}" & vbCrLf, dBestBnd)
                    Log.WriteLine("  Best integer solution at node {0}" & vbCrLf, nLastNode)
                    Log.WriteLine("     Objective value {0}" & vbCrLf, dBestObj)
                    Log.WriteLine("     Solution values")
                    For i = 0 To gnCol - 1
                        Log.WriteLine("         x[{0}]={1}", i, gpIntSol(i))
                    Next
                    Log.WriteLine("")
                Case MIPStatus.Infeasible
                    Log.WriteLine("Global search incomplete" & vbCrLf)
                    Log.WriteLine("  No integer solution found")
                    Log.WriteLine("  {0} nodes searched", nNodes)
                    Log.WriteLine("  Best bound {0}" & vbCrLf, dBestBnd)
                Case MIPStatus.Optimal
                    Log.WriteLine("Global search complete" & vbCrLf)
                    Log.WriteLine("  {0} nodes searched", nNodes)
                    Log.WriteLine("  {0} integer solution{1} found", nSol, IIf(nSol = 1, "", "s"))
                    Log.WriteLine("  Best integer solution at node {0}" & vbCrLf, nLastNode)
                    Log.WriteLine("     Objective value {0}" & vbCrLf, dBestObj)
                    Log.WriteLine("     Solution values")
                    For i = 0 To gnCol - 1
                        Log.WriteLine("         x[{0}]={1}", i, gpIntSol(i))
                    Next
                    Log.WriteLine("")
                Case Else
                    Log.WriteLine("Global search did not take place" & vbCrLf)
            End Select

        Catch ex As Exception
            Log.WriteLine(ex.ToString)
        Finally
            If (Not prob Is Nothing) Then
                prob.Destroy()
            End If
            pOutput.Close()
            XPRS.Free()
        End Try

    End Sub


    Private Sub HandleOptimizerMessage(ByVal prob As Optimizer.XPRSprob, ByVal data As Object, _
                                       ByVal message As String, ByVal len As Integer, _
                                       ByVal msglvl As Integer)
        Dim log As TextWriter
        log = data
        If (msglvl = 3 Or msglvl = 4) Then
            log.WriteLine(message)
        End If
    End Sub


    Private Sub PrintSol(ByVal prob As Optimizer.XPRSprob, ByVal data As Object)
        Dim nNodeNum As Integer             ' Current node number
        Dim dObjVal As Double               ' Objective value of the nodal integer solution
        Dim i As Integer                    ' Loop counter
        Dim available as Boolean            ' Whether solution is available

        ' Get the current node number
        nNodeNum = prob.CurrentNode

        ' Get the objective value of the current integer solution
        dObjVal = prob.MIPObjVal

        ' Retrieve the postsolved solution values from memory
        prob.GetCallbackSolution(available, gpIntSol, 0, gpIntSol.Length - 1)

        ' Print the solution to the output file
        pOutput.WriteLine("Node {0}", nNodeNum)
        pOutput.WriteLine("   Integer solution has objective value {0}", dObjVal)
        pOutput.WriteLine("   Postsolved solution values are:")
        For i = 0 To gnCol - 1
            pOutput.WriteLine("      x[{0}]={1}", i, gpIntSol(i))
        Next
        pOutput.WriteLine()
    End Sub
End Module
