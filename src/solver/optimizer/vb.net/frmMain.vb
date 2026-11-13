Imports System
Imports System.IO
Imports System.Threading
Imports Microsoft.VisualBasic

Public Class frmMain
    Inherits System.Windows.Forms.Form
    
    Public Shared sDataDirPath As String = Directory.GetParent(System.Reflection.Assembly.GetExecutingAssembly.Location()).FullName + "/../data"

#Region " Windows Form Designer generated code "

    Public Sub New()
        MyBase.New()

        'This call is required by the Windows Form Designer.
        InitializeComponent()

        'Add any initialization after the InitializeComponent() call

    End Sub

    'Form overrides dispose to clean up the component list.
    Protected Overloads Overrides Sub Dispose(ByVal disposing As Boolean)
        If disposing Then
            If Not (components Is Nothing) Then
                components.Dispose()
            End If
        End If
        MyBase.Dispose(disposing)
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    Friend WithEvents radFixBV As System.Windows.Forms.RadioButton
    Friend WithEvents radKnapsack As System.Windows.Forms.RadioButton
    Friend WithEvents radLoadLp As System.Windows.Forms.RadioButton
    Friend WithEvents radSaveSol As System.Windows.Forms.RadioButton
    Friend WithEvents tbxLog As System.Windows.Forms.TextBox
    Friend WithEvents cmdRun As System.Windows.Forms.Button
    Friend WithEvents lblInstructions As System.Windows.Forms.Label
    Friend WithEvents radGetBreakers As System.Windows.Forms.RadioButton
    <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
        Me.radFixBV = New System.Windows.Forms.RadioButton
        Me.radKnapsack = New System.Windows.Forms.RadioButton
        Me.radLoadLp = New System.Windows.Forms.RadioButton
        Me.radSaveSol = New System.Windows.Forms.RadioButton
        Me.tbxLog = New System.Windows.Forms.TextBox
        Me.cmdRun = New System.Windows.Forms.Button
        Me.lblInstructions = New System.Windows.Forms.Label
        Me.radGetBreakers = New System.Windows.Forms.RadioButton
        Me.SuspendLayout()
        '
        'radFixBV
        '
        Me.radFixBV.Location = New System.Drawing.Point(16, 32)
        Me.radFixBV.Name = "radFixBV"
        Me.radFixBV.Size = New System.Drawing.Size(136, 16)
        Me.radFixBV.TabIndex = 0
        Me.radFixBV.Text = "FixBV"
        '
        'radKnapsack
        '
        Me.radKnapsack.Location = New System.Drawing.Point(16, 56)
        Me.radKnapsack.Name = "radKnapsack"
        Me.radKnapsack.Size = New System.Drawing.Size(136, 16)
        Me.radKnapsack.TabIndex = 1
        Me.radKnapsack.Text = "Knapsack"
        '
        'radLoadLp
        '
        Me.radLoadLp.Location = New System.Drawing.Point(160, 32)
        Me.radLoadLp.Name = "radLoadLp"
        Me.radLoadLp.Size = New System.Drawing.Size(136, 16)
        Me.radLoadLp.TabIndex = 2
        Me.radLoadLp.Text = "LoadLP"
        '
        'radSaveSol
        '
        Me.radSaveSol.Location = New System.Drawing.Point(304, 32)
        Me.radSaveSol.Name = "radSaveSol"
        Me.radSaveSol.Size = New System.Drawing.Size(136, 16)
        Me.radSaveSol.TabIndex = 3
        Me.radSaveSol.Text = "SaveSol"
        '
        'tbxLog
        '
        Me.tbxLog.Location = New System.Drawing.Point(12, 100)
        Me.tbxLog.Multiline = True
        Me.tbxLog.Name = "tbxLog"
        Me.tbxLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical
        Me.tbxLog.Size = New System.Drawing.Size(520, 273)
        Me.tbxLog.TabIndex = 5
        Me.tbxLog.Text = ""
        '
        'cmdRun
        '
        Me.cmdRun.Location = New System.Drawing.Point(437, 27)
        Me.cmdRun.Name = "cmdRun"
        Me.cmdRun.Size = New System.Drawing.Size(99, 61)
        Me.cmdRun.TabIndex = 6
        Me.cmdRun.Text = "Run"
        '
        'lblInstructions
        '
        Me.lblInstructions.Location = New System.Drawing.Point(16, 8)
        Me.lblInstructions.Name = "lblInstructions"
        Me.lblInstructions.Size = New System.Drawing.Size(512, 16)
        Me.lblInstructions.TabIndex = 7
        Me.lblInstructions.Text = "Select the example to run..."
        Me.lblInstructions.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
        '
        'radGetBreakers
        '
        Me.radGetBreakers.Location = New System.Drawing.Point(160, 80)
        Me.radGetBreakers.Name = "radGetBreakers"
        Me.radGetBreakers.Size = New System.Drawing.Size(160, 16)
        Me.radGetBreakers.TabIndex = 10
        Me.radGetBreakers.Text = "RepairInfeas GetBreakers"
        '
        'frmMain
        '
        Me.AutoScaleBaseSize = New System.Drawing.Size(5, 13)
        Me.ClientSize = New System.Drawing.Size(552, 385)
        Me.Controls.Add(Me.radGetBreakers)
        Me.Controls.Add(Me.lblInstructions)
        Me.Controls.Add(Me.cmdRun)
        Me.Controls.Add(Me.tbxLog)
        Me.Controls.Add(Me.radSaveSol)
        Me.Controls.Add(Me.radLoadLp)
        Me.Controls.Add(Me.radKnapsack)
        Me.Controls.Add(Me.radFixBV)
        Me.Name = "frmMain"
        Me.Text = "Optimizer VB.NET Examples"
        Me.ResumeLayout(False)

    End Sub

#End Region


    Delegate Sub SolveFinishCallback()

    Private Sub SolveFinish()
        If Me.InvokeRequired Then
            Dim d As New SolveFinishCallback(AddressOf SolveFinish)
            Me.Invoke(d)
        Else
            cmdRun.Enabled = True
            Me.Cursor = System.Windows.Forms.Cursors.Default
        End If
    End Sub

    Private Sub SolveFixBV()
        Dim Log As OptimizerLog
        Log = New OptimizerLog(Me.tbxLog)
        Log.WriteLine("Running FixBV...")
        Log.WriteLine("")
        FixBV.RunFixBV(Log)
        SolveFinish()
    End Sub

    Private Sub SolveKnapsack()
        Dim Log As OptimizerLog
        Log = New OptimizerLog(Me.tbxLog)
        Log.WriteLine("Running Knapsack...")
        Log.WriteLine("")
        Knapsack.RunKnapsack(Log)
        SolveFinish()
    End Sub

    Private Sub SolveLoadLP()
        Dim Log As OptimizerLog
        Log = New OptimizerLog(Me.tbxLog)
        Log.WriteLine("Running LoadLP...")
        Log.WriteLine("")
        LoadLP.RunLoadLP(Log)
        SolveFinish()
    End Sub

  Private Sub SolveSaveSol()
    Dim Log As OptimizerLog
    Log = New OptimizerLog(Me.tbxLog)
    Log.WriteLine("Running SaveSol...")
    Log.WriteLine("")
    SaveSol.RunSaveSol(Log)
    SolveFinish()
  End Sub

    Private Sub SolveGetBreakers()
        Dim Log As OptimizerLog
        Log = New OptimizerLog(Me.tbxLog)
        Log.WriteLine("Running RepairInfeas - GetBreakers...")
        Log.WriteLine("")
        RepairInfeas_GetBreakers.GetBreakers(Log)
        SolveFinish()
    End Sub

    Private Sub cmdRun_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles cmdRun.Click
        Dim Log As OptimizerLog
        Log = New OptimizerLog(Me.tbxLog)

        Me.Cursor = System.Windows.Forms.Cursors.WaitCursor
        cmdRun.Enabled = False

        If Me.radFixBV.Checked Then
            Dim thread As New Thread(AddressOf SolveFixBV)
            thread.Start()
        ElseIf Me.radKnapsack.Checked Then
            Dim thread As New Thread(AddressOf SolveKnapsack)
            thread.Start()
        ElseIf Me.radLoadLp.Checked Then
            Dim thread As New Thread(AddressOf SolveLoadLP)
            thread.Start()
        ElseIf Me.radSaveSol.Checked Then
            Dim thread As New Thread(AddressOf SolveSaveSol)
            thread.Start()
        ElseIf Me.radGetBreakers.Checked Then
            Dim thread As New Thread(AddressOf SolveGetBreakers)
            thread.Start()
        Else
            Log.WriteLine("ERROR: No example selected.")
            MsgBox("Please select which example you wish to execute")
        End If
    End Sub

End Class
