Imports System.IO
Imports System.Windows.Forms.TextBox

' Simple class that logs output in a TextBox field on a form

Public Class OptimizerLog
    Inherits System.IO.StringWriter

    Private Field As System.Windows.Forms.TextBox

    Sub New(ByRef tb As System.Windows.Forms.TextBox)
        MyBase.New()
        Field = tb
        Field.Text = ""
        System.Windows.Forms.Application.DoEvents()
    End Sub

    Public Overloads Overrides Sub Flush()
        MyBase.Flush()
        Field.Text = Me.ToString

        ' We want to automatically scroll the textbox to show the last line.
        ' No easy way to scroll the textbox or set the caret position - so we have to
        ' select the last character, which will set the caret position, then scroll to
        ' that.
        Field.SelectionStart = Field.Text.Length
        Field.SelectionLength = 0
        Field.ScrollToCaret()

        System.Windows.Forms.Application.DoEvents()
    End Sub


    Public Overloads Overrides Sub WriteLine(ByVal val As String)
        If (Field.Text.Length = Me.ToString.Length) Then
            MyBase.WriteLine(val)
            Field.AppendText(val)
        Else
            MyBase.WriteLine(val)
            Field.Text = Me.ToString
        End If

        ' We want to automatically scroll the textbox to show the last line.
        ' No easy way to scroll the textbox or set the caret position - so we have to
        ' select the last character, which will set the caret position, then scroll to
        ' that.
        Field.SelectionStart = Field.Text.Length
        Field.SelectionLength = 0
        Field.ScrollToCaret()

        System.Windows.Forms.Application.DoEvents()
    End Sub


    Public Overloads Overrides Sub WriteLine(ByVal fmt As String, ByVal ParamArray arg() As Object)
        Me.WriteLine(String.Format(fmt, arg))
    End Sub

End Class
