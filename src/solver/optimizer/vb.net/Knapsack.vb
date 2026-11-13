Imports System
Imports Microsoft.VisualBasic
Imports System.IO
Imports Optimizer

Module Knapsack
    Const sLogFile As String = "knapsack.log"
    Const sProblem As String = "burglar"

    ' Global variables we'll use
    Private prob As XPRSprob
    Private x() As Double                   ' Nodal LP solution values
    Private gpObjCoef() As Double             ' Objective function coefficients
    Private gdIntTol As Double              ' Integer feasibility tolerance
    Private gnCol As Integer                ' Number of columns


    Public Sub RunKnapsack(ByVal Log As TextWriter)
        prob = Nothing
        Try
            ' Initialise optimizer
            XPRS.Init("")

            prob = New XPRSprob

            ' Set the logfile
            prob.SetLogFile(sLogFile)

            ' Tell Optimizer to call HandleOptimizerMessage whenever a message is output
            prob.AddMessageCallback(New Optimizer.MessageCallback(AddressOf HandleOptimizerMessage), Log)

            ' Turn off presolve and disallow cuts - to slow solution and allow the effect
            ' of the heuristic to be seen
            prob.Presolve = 0
            prob.CutStrategy = 0

            ' Reads the problem file
            prob.ReadProb(frmMain.sDataDirPath & "/" & sProblem, "")

            ' Prepare to apply the heuristic

            ' Get the number of columns
            Dim gnCol As Integer
            gnCol = prob.Cols

            ' Allocate memory to the coefficient and solution arrays
            ReDim gpObjCoef(gnCol)
            ReDim x(gnCol)

            ' Get integer feasibility tolerance
            Dim gdIntTol As Double
            gdIntTol = prob.MIPTol

            ' Tell the optimizer to print global information to the log file at each node
            prob.MIPLog = 3

            ' Tell the optimizer to call truncsol at each node and apply the heuristic
            prob.AddMiplogCallback(New Optimizer.MiplogCallback(AddressOf TruncSol), Log)

            ' Perform the global search - in the course of which the heuristic will be
            ' applied
            Log.WriteLine("Applying a primal heuristic to problem {0}", sProblem)
            prob.MipOptimize()

        Catch ex As Exception
            Log.WriteLine(ex.ToString)
        Finally
            ' Destroy the problem and free the optimizer
            If (Not prob Is Nothing) Then
                prob.Destroy()
            End If
            XPRS.Free()
        End Try
    End Sub

    Private Sub HandleOptimizerMessage(ByVal prob As Optimizer.XPRSprob, ByVal data As Object, _
                                       ByVal message As String, ByVal len As Integer, _
                                       ByVal msglvl As Integer)
        If (Not message Is Nothing) Then
            Dim log As TextWriter
            log = data
            log.WriteLine(message)
        End If
    End Sub

    Public Function TruncSol(ByVal prob As Optimizer.XPRSprob, ByVal data As Object) As Integer
        Dim nNodeNum As Integer             ' Number of nodes solved
        Dim dObjVal As Double               ' Objective value
        Dim dCutoff As Double               ' Cutoff value
        Dim nLPStatus As LPStatus           ' LP solution stgatus
        Dim nIntInf As Integer              ' Number of integer infeasibilities
        Dim i As Integer                    ' Loop counter
        Dim dHeurObj As Double              ' Objective value after the solution values have been truncated
        Dim available as Boolean            ' Whether callback solution is available

        Dim log As TextWriter
        log = data

        ' Get the current node number
        nNodeNum = prob.CurrentNode

        ' Get the objective value at the current node
        dObjVal = prob.LPObjVal

        ' Get the current cutoff value
        dCutoff = prob.MIPAbsCutoff

        ' Get LP solution status and the number of integer infeasibilities
        nLPStatus = prob.LPStatus
        nIntInf = prob.MIPInfeas

        ' Apply heuristic if nodal solution is LP optimal and integer infeasible
        If (nLPStatus = LPStatus.Optimal And nIntInf > 0) Then

            ' Get LP solution
            prob.GetCallbackSolution(available, x, 0, x.Length - 1)

            ' Truncate each solution value = making allowance for the integer
            '  tolerance - and calculate the new "heuristic" objective value
            dHeurObj = 0
            For i = 0 To gnCol - 1
                dHeurObj = dHeurObj + gpObjCoef(i) * CInt(x(i) + gdIntTol)
            Next
            log.WriteLine("  Node {0}: Objective Value: ORIGINAL {1};  HEURISTIC {2}" & vbCrLf, _
                nNodeNum, dObjVal, dHeurObj)

            ' If the "heuristic" objective value is better, update the cutoff -
            '  we assume that all the object coefficients are integers
            If (dHeurObj > dCutoff) Then
                prob.MIPAbsCutoff = dHeurObj + 0.9999
                log.WriteLine("              ** Cutoff updated to {0} **" & vbCrLf, _
                    dHeurObj + 1.0)
            End If


            ' if the nodal solution is not LP optimal do not apply the heuristic
        ElseIf (nLPStatus <> LPStatus.Optimal) Then
            log.WriteLine("   Node {0}: LP solution not optimal, not applying heuristic", nNodeNum)
            log.WriteLine("           ({0})" & vbCrLf, nLPStatus)

            ' If the nodal solution is integer infeasible print the objective value
        ElseIf (nIntInf = 0) Then
            log.WriteLine("   Node {0}: Integer solution found: Objective Value {1}" & _
                          vbCrLf, nNodeNum, dObjVal)
        End If

        Return 0
    End Function

End Module
