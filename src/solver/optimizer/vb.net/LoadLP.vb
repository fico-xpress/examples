Imports System
Imports Microsoft.VisualBasic
Imports System.IO
Imports Optimizer

Module LoadLP
    Public Sub RunLoadLP(ByVal Log As TextWriter)
        ' Store the problem
        ' Row data
        Dim nRow As Integer = 4
        Dim sRowType() As Char = {"L", "L", "L", "L"}
        Dim dRHS() As Double = {24.0, 5.0, 20.0, 9.0}
        Dim sRowName() As String = {"c1", "c2", "c3", "c4"}

        ' Column data
        Dim nCol As Integer = 2
        Dim dObj() As Double = {2.0, 1.0}
        Dim dLowerBd() As Double = {0, 0}
        Dim dUpperBd() As Double = {XPRS.PLUSINFINITY, XPRS.PLUSINFINITY}

        Dim sColName() As String = {"x", "y"}

        ' Matrix data
        Dim nColStart() As Integer = {0, 3, 7}
        Dim nRowInd() As Integer = {0, 2, 3, 0, 1, 2, 3}
        Dim dMatElem() As Double = {1, 3, 1, 4, 1, 1, 1}

        ' Store extra constraint
        Dim nNewRow As Integer = 1
        Dim nNewElem As Integer = 2
        Dim sNewRowType() As Char = {"L"}
        Dim sNewRowName() As String = {"c5"}

        Dim dNewRHS() As Double = {20}
        Dim dNewRowElem() As Double = {6, 1}
        Dim nNewRowStart() As Integer = {0, 2}
        Dim nNewColInd() As Integer = {0, 1}

        Dim dObjValue As Double

        Const sLogFile As String = "loadlp.log"
        Const sProblem1 As String = "lp"
        Const sProblem2 As String = "revised"

        Dim prob As XPRSprob
        prob = Nothing

        Try
            XPRS.Init("")

            prob = New XPRSprob
            prob.SetLogFile(sLogFile)

            ' Tell the Optimizer to call OptimizerMsg whenever a message is output
            prob.AddMessageCallback(New Optimizer.MessageCallback(AddressOf HandleOptimizerMessage), Log)

            prob.LoadLP( _
                sProblem1, _
                nCol, _
                nRow, _
                sRowType, _
                dRHS, _
                Nothing, _
                dObj, _
                nColStart, _
                Nothing, _
                nRowInd, _
                dMatElem, _
                dLowerBd, _
                dUpperBd _
            )

            ' Add row names
            prob.AddNames(1, sRowName, 0, nRow - 1)

            ' Add column names
            prob.AddNames(2, sColName, 0, nCol - 1)

            ' Output the matrix
            prob.WriteProb(sProblem1, "")
            Log.WriteLine("Matrix file {0}.mat has been created", sProblem1)

            ' Solve the LP problem
            prob.LpOptimize()

            ' Get and display the value of the objective function
            dObjValue = prob.LPObjVal
            Log.WriteLine("The optimal objective value is {0}", dObjValue)

            ' Add the extra constraint and solve again

            ' Add new row
            prob.AddRows( _
                nNewRow, _
                nNewElem, _
                sNewRowType, _
                dNewRHS, _
                Nothing, _
                nNewRowStart, _
                nNewColInd, _
                dNewRowElem _
            )

            ' Add new row name
            prob.AddNames(1, sNewRowName, nRow, nRow)

            ' Output the revised matrix
            prob.WriteProb(sProblem2, "")
            Log.WriteLine("Matrix file {0}.mat has been created", sProblem2)

            ' Solve with dual - since the revised problem inherits dual feasibility from the
            ' original
            prob.LpOptimize("d")

            ' Get and display the value of the objective function
            dObjValue = prob.LPObjVal
            Log.WriteLine("The revised optimal objective value is {0}", dObjValue)

        Catch ex As Exception
            Log.WriteLine(ex.ToString)

        Finally
            If (Not prob Is Nothing) Then
                prob.Destroy()
            End If
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

End Module
