Public Class frmMain
    Inherits System.Windows.Forms.Form

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
    Friend WithEvents tbxLog As System.Windows.Forms.TextBox
    Friend WithEvents cmdRun As System.Windows.Forms.Button
    Friend WithEvents lblInstructions As System.Windows.Forms.Label
    Friend WithEvents radDispDso As System.Windows.Forms.RadioButton
    Friend WithEvents radDispMod As System.Windows.Forms.RadioButton
    Friend WithEvents radExAs As System.Windows.Forms.RadioButton
    Friend WithEvents radExLib As System.Windows.Forms.RadioButton
    Friend WithEvents radExProb As System.Windows.Forms.RadioButton
    Friend WithEvents radExSet As System.Windows.Forms.RadioButton
    Friend WithEvents radExDrvsCallback As System.Windows.Forms.RadioButton
    Friend WithEvents radExDrvsRaw As System.Windows.Forms.RadioButton
    Friend WithEvents radExDrvsStream As System.Windows.Forms.RadioButton
    <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
        Me.radDispDso = New System.Windows.Forms.RadioButton
        Me.radDispMod = New System.Windows.Forms.RadioButton
        Me.radExAs = New System.Windows.Forms.RadioButton
        Me.radExLib = New System.Windows.Forms.RadioButton
        Me.radExDrvsCallback = New System.Windows.Forms.RadioButton
        Me.tbxLog = New System.Windows.Forms.TextBox
        Me.cmdRun = New System.Windows.Forms.Button
        Me.lblInstructions = New System.Windows.Forms.Label
        Me.radExProb = New System.Windows.Forms.RadioButton
        Me.radExSet = New System.Windows.Forms.RadioButton
        Me.radExDrvsRaw = New System.Windows.Forms.RadioButton
        Me.radExDrvsStream = New System.Windows.Forms.RadioButton
        Me.SuspendLayout()
        '
        'radDispDso
        '
        Me.radDispDso.Location = New System.Drawing.Point(16, 32)
        Me.radDispDso.Name = "radDispDso"
        Me.radDispDso.Size = New System.Drawing.Size(136, 16)
        Me.radDispDso.TabIndex = 0
        Me.radDispDso.Text = "DispDSO"
        '
        'radDispMod
        '
        Me.radDispMod.Location = New System.Drawing.Point(16, 56)
        Me.radDispMod.Name = "radDispMod"
        Me.radDispMod.Size = New System.Drawing.Size(136, 16)
        Me.radDispMod.TabIndex = 1
        Me.radDispMod.Text = "DispMod"
        '
        'radExAs
        '
        Me.radExAs.Location = New System.Drawing.Point(160, 32)
        Me.radExAs.Name = "radExAs"
        Me.radExAs.Size = New System.Drawing.Size(136, 16)
        Me.radExAs.TabIndex = 2
        Me.radExAs.Text = "ExAs"
        '
        'radExLib
        '
        Me.radExLib.Location = New System.Drawing.Point(304, 32)
        Me.radExLib.Name = "radExLib"
        Me.radExLib.Size = New System.Drawing.Size(136, 16)
        Me.radExLib.TabIndex = 3
        Me.radExLib.Text = "ExLib"
        '
        'radExDrvsCallback
        '
        Me.radExDrvsCallback.Location = New System.Drawing.Point(448, 56)
        Me.radExDrvsCallback.Name = "radExDrvsCallback"
        Me.radExDrvsCallback.Size = New System.Drawing.Size(136, 16)
        Me.radExDrvsCallback.TabIndex = 4
        Me.radExDrvsCallback.Text = "ExDrvsCallback"
        '
        'tbxLog
        '
        Me.tbxLog.Location = New System.Drawing.Point(16, 120)
        Me.tbxLog.Multiline = True
        Me.tbxLog.Name = "tbxLog"
        Me.tbxLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical
        Me.tbxLog.Size = New System.Drawing.Size(520, 224)
        Me.tbxLog.TabIndex = 5
        Me.tbxLog.Text = ""
        '
        'cmdRun
        '
        Me.cmdRun.Location = New System.Drawing.Point(416, 88)
        Me.cmdRun.Name = "cmdRun"
        Me.cmdRun.Size = New System.Drawing.Size(120, 24)
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
        'radExProb
        '
        Me.radExProb.Location = New System.Drawing.Point(16, 80)
        Me.radExProb.Name = "radExProb"
        Me.radExProb.Size = New System.Drawing.Size(136, 16)
        Me.radExProb.TabIndex = 8
        Me.radExProb.Text = "ExProb"
        '
        'radExSet
        '
        Me.radExSet.Location = New System.Drawing.Point(448, 32)
        Me.radExSet.Name = "radExSet"
        Me.radExSet.Size = New System.Drawing.Size(136, 16)
        Me.radExSet.TabIndex = 9
        Me.radExSet.Text = "ExSet"
        '
        'radExDrvsRaw
        '
        Me.radExDrvsRaw.Location = New System.Drawing.Point(304, 56)
        Me.radExDrvsRaw.Name = "radExDrvsRaw"
        Me.radExDrvsRaw.Size = New System.Drawing.Size(136, 16)
        Me.radExDrvsRaw.TabIndex = 10
        Me.radExDrvsRaw.Text = "ExDrvsRaw"
        '
        'radExDrvsStream
        '
        Me.radExDrvsStream.Location = New System.Drawing.Point(160, 56)
        Me.radExDrvsStream.Name = "radExDrvsStream"
        Me.radExDrvsStream.Size = New System.Drawing.Size(136, 16)
        Me.radExDrvsStream.TabIndex = 11
        Me.radExDrvsStream.Text = "ExDrvsStream"
        '
        'frmMain
        '
        Me.AutoScaleBaseSize = New System.Drawing.Size(5, 13)
        Me.ClientSize = New System.Drawing.Size(552, 349)
        Me.Controls.Add(Me.radExDrvsStream)
        Me.Controls.Add(Me.radExDrvsRaw)
        Me.Controls.Add(Me.radExSet)
        Me.Controls.Add(Me.radExProb)
        Me.Controls.Add(Me.lblInstructions)
        Me.Controls.Add(Me.cmdRun)
        Me.Controls.Add(Me.tbxLog)
        Me.Controls.Add(Me.radExDrvsCallback)
        Me.Controls.Add(Me.radExLib)
        Me.Controls.Add(Me.radExAs)
        Me.Controls.Add(Me.radDispMod)
        Me.Controls.Add(Me.radDispDso)
        Me.Name = "frmMain"
        Me.Text = "Mosel VB.NET Examples"
        Me.ResumeLayout(False)

    End Sub

#End Region

    Private Sub cmdRun_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles cmdRun.Click
        Dim Log As OptimizerLog
        Log = New OptimizerLog(Me.tbxLog)
        Me.Cursor = Cursors.WaitCursor
        If Me.radDispDso.Checked Then
            Log.WriteLine("Running DispDSO...")
            Log.WriteLine("")
            DispDso.RunDispDso("mmxprs", Log)
        ElseIf Me.radDispMod.Checked Then
            Log.WriteLine("Running DispMod...")
            Log.WriteLine("")
            DispMod.RunDispMod("Models\blend2.mos", Log)
        ElseIf Me.radExAs.Checked Then
            Log.WriteLine("Running ExAs...")
            Log.WriteLine("")
            ExAs.RunExAs(Log)
        ElseIf Me.radExLib.Checked Then
            Log.WriteLine("Running ExLib...")
            Log.WriteLine()
            ExLib.RunExLib(Log)
        ElseIf Me.radExSet.Checked Then
            Log.WriteLine("Running ExSet...")
            Log.WriteLine()
            ExSet.RunExSet(Log)
        ElseIf Me.radExProb.Checked Then
            Log.WriteLine("Running ExProb...")
            Log.WriteLine()
            ExProb.RunExProb(Log)
        ElseIf Me.radExDrvsStream.Checked Then
            Log.WriteLine("Running ExDrvsStream...")
            Log.WriteLine()
            ExDrvsStream.RunExDrvsStream(Log)
        ElseIf Me.radExDrvsCallback.Checked Then
            Log.WriteLine("Running ExDrvsCallback...")
            Log.WriteLine()
            ExDrvsCallback.RunExDrvsCallback(Log)
        ElseIf Me.radExDrvsRaw.Checked Then
            Log.WriteLine("Running ExDrvsRaw")
            Log.WriteLine()
            ExDrvsRaw.RunExDrvsRaw(Log)
        Else
            Log.WriteLine("ERROR: No example selected.")
            MsgBox("Please select which example you wish to execute")
        End If
        Log.WriteLine()
        Log.WriteLine("Example ended.")
        Me.Cursor = Cursors.Default
    End Sub
End Class
