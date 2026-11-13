Imports System
Imports Microsoft.VisualBasic
Imports System.IO
Imports Optimizer

Module RepairInfeas_GetBreakers
    Const sProblem As String = "infeas.lp"


    Public Sub GetBreakers(ByVal Log As TextWriter)
        Dim licenseObtained As Boolean = False
        Dim xprob As XPRSprob = Nothing

        Try
            ' Initialize optimizer
            XPRS.Init()
            licenseObtained = True

            ' Initialize problem object
            xprob = New XPRSprob

            ' Forward messages to log
            xprob.AddMsgHandlerCallback(Log)

            ' Load problem file
            xprob.ReadProb(frmMain.sDataDirPath & "/" & sProblem)

            ' Allocate preference arrays
            Dim lrp_array(xprob.Rows) As Double
            Dim grp_array(xprob.Rows) As Double
            Dim lbp_array(xprob.Cols) As Double
            Dim ubp_array(xprob.Cols) As Double

            ' Allocate arrays for the solution and the infeasibility breakers
            Dim x(xprob.Cols) As Double
            Dim slacks(xprob.Rows) As Double
            Dim boundViolations(xprob.Cols) As Double
            Dim constraintViolations(xprob.Rows) As Double

            ' Set relaxation values
            Dim i As Integer, j As Integer
            For i = 0 To xprob.InputRows - 1
                lrp_array(i) = 1
                grp_array(i) = 1
            Next
            For j = 0 To xprob.InputCols - 1
                lbp_array(j) = 1
                ubp_array(j) = 1
            Next

            ' Call Repairinfeas
            Dim Status As Integer = -1
            xprob.RepairWeightedInfeas(Status, lrp_array, grp_array, lbp_array, ubp_array, "n", 0.001, "")

            Log.Write("Repairinfeas return: ")
            Select Case Status
                Case 0
                    Log.WriteLine("Relaxed optimum found")
                Case 1
                    Log.WriteLine("Relaxed problem is infeasible")
                Case 2
                    Log.WriteLine("Relaxed problem is unbounded")
                Case 3
                    Log.WriteLine("Solution of the relaxed problem regarding the original objective is nonoptimal")
                Case 4
                    Log.WriteLine("ERROR")
                Case 5
                    Log.WriteLine("Numerical Instability")
            End Select

            ' Get solution
            xprob.GetLpSol(x, slacks, Nothing, Nothing)
            ' NOTE: For MIPs, use xprob.GetMIPSol instead

            ' Get the values of the breaker variables
            GetInfeasibilityBreakers(xprob, x, slacks, constraintViolations, boundViolations, 0.000001)

            ' Print report
            GenerateInfeasibilityReport(Log, xprob, constraintViolations, boundViolations)

            ' Repair and save problem
            ApplyRepairInfeasResultToProblem(xprob, constraintViolations, boundViolations)
            Log.WriteLine("Problem repaired")
            xprob.WriteProb("repaired.lp", "lp")
            Log.WriteLine("Repaired problem is written as repaired.lp.")
            Log.WriteLine("(Please note that this matrix might show slightly different behaviour then the repairinfeas problem)")
        Catch ex As Exception
            Log.WriteLine(ex.ToString)
        Finally
            ' Destroy the problem
            If (Not xprob Is Nothing) Then
                xprob.Destroy()
            End If
            If (licenseObtained) Then
                XPRS.Free()
            End If
        End Try
    End Sub

    ' This routine returns the lower and upper bounds for a row
    Public Sub GetRowBounds(ByRef xprob As XPRSprob, ByVal i As Integer, ByRef dlb As Double, ByRef dub As Double)
        Dim rowtype(1) As Char
        Dim rhs(1) As Double
        Dim range(1) As Double

        ' The bounds are calculated using the RHS, the range, and the type of the row
        xprob.GetRHS(rhs, i, i)
        xprob.GetRHSRange(range, i, i)
        xprob.GetRowType(rowtype, i, i)

        Select Case rowtype(0)
            Case "L"
                dlb = XPRS.MINUSINFINITY
                dub = rhs(0)
            Case "E"
                dlb = rhs(0)
                dub = rhs(0)
            Case "G"
                dlb = rhs(0)
                dub = XPRS.PLUSINFINITY
            Case "R"
                dlb = rhs(0) - range(0)
                dub = rhs(0)
            Case Else
                dlb = XPRS.MINUSINFINITY
                dub = XPRS.PLUSINFINITY
        End Select

    End Sub

    ' This routine sets new lower and upper bounds for a row
    Public Sub SetRowBounds(ByRef xprob As XPRSprob, ByVal i As Integer, ByVal dlb As Double, ByVal dub As Double)
        Dim lb(1) As Double
        Dim ub(1) As Double
        Dim iArray(1) As Integer

        iArray(0) = i

        If (dlb < dub) Then
            lb(0) = dlb
            ub(0) = dub
        Else
            lb(0) = dub
            ub(0) = dlb
        End If

        Dim rowtype(1) As Char
        If (lb(0) <= XPRS.MINUSINFINITY And ub(0) >= XPRS.PLUSINFINITY) Then
            rowtype(0) = "N"
            xprob.ChgRowType(1, iArray, rowtype)
        ElseIf (lb(0) <= XPRS.MINUSINFINITY) Then
            rowtype(0) = "L"
            xprob.ChgRowType(1, iArray, rowtype)
            xprob.ChgRHS(1, iArray, ub)
        ElseIf (ub(0) >= XPRS.PLUSINFINITY) Then
            rowtype(0) = "G"
            xprob.ChgRowType(1, iArray, rowtype)
            xprob.ChgRHS(1, iArray, lb)
        ElseIf (lb(0) = ub(0)) Then
            rowtype(0) = "E"
            xprob.ChgRowType(1, iArray, rowtype)
            xprob.ChgRHS(1, iArray, ub)
        Else
            rowtype(0) = "L"
            xprob.ChgRowType(1, iArray, rowtype)
            xprob.ChgRHS(1, iArray, ub)
            Dim range(1) As Double
            range(0) = ub(0) - lb(0)
            xprob.ChgRHSRange(1, iArray, range)
        End If
    End Sub

    ' This routine calculates the infeasibilities of a solution (i.e. the values of the feasibility breakers after a repairinfeas call) 
    Public Sub GetInfeasibilityBreakers(ByRef xprob As XPRSprob, ByVal x As Double(), ByVal slacks As Double(), ByRef constraints As Double(), ByRef bounds As Double(), ByVal tolerance As Double)
        Dim i As Integer, j As Integer
        Dim rhs(1) As Double
        Dim activity As Double
        Dim lb(1) As Double, ub(1) As Double

        ' check constraints
        For i = 0 To xprob.InputRows - 1
            xprob.GetRHS(rhs, i, i)
            activity = rhs(0) - slacks(i)
            GetRowBounds(xprob, i, lb(0), ub(0))

            If (activity < lb(0) - tolerance) Then
                constraints(i) = activity - lb(0) ' a negative value indicates that the lower bound is relaxed
            ElseIf (activity > ub(0) + tolerance) Then
                constraints(i) = activity - ub(0) ' a positive value indicates that the upper bound is relaxed
            Else
                constraints(i) = 0.0
            End If
        Next

        ' check bounds
        For j = 0 To xprob.InputCols - 1
            xprob.GetLB(lb, j, j)
            xprob.GetUB(ub, j, j)

            If (x(j) < lb(0) - tolerance) Then
                bounds(j) = x(j) - lb(0)
            ElseIf (x(j) > ub(0) + tolerance) Then
                bounds(j) = x(j) - ub(0)
            Else
                bounds(j) = 0.0
            End If
        Next
    End Sub

    ' This function just converts a double to a string for reporting, converting infinity values as "NONE"
    Public Function BoundToString(ByVal bound As Double) As String
        If (bound > XPRS.MINUSINFINITY And bound < XPRS.PLUSINFINITY) Then
            If (bound >= 0) Then
                BoundToString = String.Format(" {0:e5}", bound)
            Else
                BoundToString = String.Format("{0:e5}", bound)
            End If
        Else
            BoundToString = "    NONE     "
        End If
    End Function

    ' Prints a summary of the infeasibily breakers using the already calculated infeasibities
    Public Sub GenerateInfeasibilityReport(ByRef log As TextWriter, ByRef xprob As XPRSprob, ByVal constraintViolations() As Double, ByVal boundViolations() As Double)
        Dim i As Integer, j As Integer
        Dim ub(1) As Double, lb(1) As Double, name As String
        Dim slbshift As String, subshift As String

        ' Get the maximum name length
        Dim maxNameLength As Integer = xprob.NameLength * 8
        If (maxNameLength < 4) Then
            maxNameLength = 4
        End If

        log.WriteLine()
        log.WriteLine()
        log.WriteLine("XPRESS-MP FEASIBILITY REPAIR REPORT")
        log.WriteLine()
        log.WriteLine("The following constraints were repaired:")
        log.WriteLine()

        log.WriteLine("Index     {0," & maxNameLength & "}  Lower bound    Repaired        Upper bound    Repaired", "Name")

        Dim sumInf As Double = 0
        Dim numInf As Integer = 0

        For i = 0 To xprob.InputRows - 1
            If (constraintViolations(i) <> 0) Then
                name = xprob.GetName(Int(Namespaces.Row), i)
                GetRowBounds(xprob, i, lb(0), ub(0))

                sumInf += Math.Abs(constraintViolations(i))
                numInf += 1

                If (constraintViolations(i) < 0) Then
                    slbshift = BoundToString(lb(0) + constraintViolations(i))
                    subshift = BoundToString(ub(0))
                Else
                    slbshift = BoundToString(lb(0))
                    subshift = BoundToString(ub(0) + constraintViolations(i))
                End If

                log.WriteLine("{0,5}  {1," & maxNameLength & "}    {2,7}  {3,7}   {4,7}  {5,7}", i, name, BoundToString(lb(0)), slbshift, BoundToString(ub(0)), subshift)
            End If
        Next
        log.WriteLine()

        log.WriteLine("The following bound constraints were repaired:")
        log.WriteLine()

        log.WriteLine("Index   {0," & maxNameLength & "}   Lower bound      Repaired     Upper bound      Repaired", "Name")
        For j = 0 To xprob.InputCols - 1
            If (boundViolations(j) <> 0) Then
                name = xprob.GetName(Int(Namespaces.Column), j)
                xprob.GetLB(lb, j, j)
                xprob.GetUB(ub, j, j)

                sumInf += Math.Abs(boundViolations(j))
                numInf += 1

                If (boundViolations(j) > 0) Then
                    subshift = BoundToString(ub(0) + boundViolations(j))
                    slbshift = BoundToString(lb(0))
                Else
                    subshift = BoundToString(ub(0))
                    slbshift = BoundToString(lb(0) + boundViolations(j))
                End If

                log.WriteLine("{0,5}  {1," & maxNameLength & "}    {2,7}  {3,7}   {4,7}  {5,7}", j, name, BoundToString(lb(0)), slbshift, BoundToString(ub(0)), subshift)
            End If
        Next

        log.WriteLine()
        log.WriteLine("Sum of Violations = {0}", sumInf)
        log.WriteLine("Number of corrections = {0}", numInf)

        log.WriteLine()
    End Sub

    ' Relaxes the problem given the values of the feasibility breakers
    Public Sub ApplyRepairInfeasResultToProblem(ByRef xprob As XPRSprob, ByVal constraintViolations As Double(), ByVal boundViolations As Double())
        Dim i As Integer, j As Integer
        Dim lb(1) As Double, ub(1) As Double, boundtype(1) As Char, jArray(1) As Integer

        ' Apply changes to rows
        For i = 0 To xprob.InputRows - 1
            If (constraintViolations(i) <> 0) Then

                ' Get bounds
                GetRowBounds(xprob, i, lb(0), ub(0))

                ' Apply relaxation
                If (constraintViolations(i) > 0) Then
                    ub(0) += constraintViolations(i)
                Else
                    lb(0) += constraintViolations(i)
                End If

                SetRowBounds(xprob, i, lb(0), ub(0))
            End If
        Next

        ' Apply changes to columns
        For j = 0 To xprob.InputCols - 1
            If (boundViolations(j) <> 0) Then
                ' Get bounds
                xprob.GetLB(lb, j, j)
                xprob.GetUB(ub, j, j)

                jArray(0) = j
                If (boundViolations(j) > 0) Then
                    boundtype(0) = "U"
                    ub(0) += boundViolations(j)
                    xprob.ChgBounds(1, jArray, boundtype, ub)
                Else
                    boundtype(0) = "L"
                    lb(0) += boundViolations(j)
                    xprob.ChgBounds(1, jArray, boundtype, lb)
                End If
            End If
        Next
    End Sub

End Module

		