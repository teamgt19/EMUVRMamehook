Public Class Form1
    Private Mame As MameInterop = Nothing

    Private Sub Form1_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load
        Mame = New MameInterop(Me.TextBox1)
        Mame.Initialize(2, "VBTest")
    End Sub

    Private Sub Form1_FormClosing(ByVal sender As System.Object, ByVal e As System.Windows.Forms.FormClosingEventArgs) Handles MyBase.FormClosing
        Mame.Dispose()
    End Sub
End Class
