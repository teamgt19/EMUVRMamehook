Imports System
Imports System.Collections.Generic
Imports System.Text
Imports System.Runtime.InteropServices
Imports System.Windows.Forms
Imports System.Collections

Class MameInterop
    Implements IDisposable
    <DllImport("Mame.dll")> _
    Private Shared Function init_mame(ByVal clientid As Integer, ByVal name As String, <MarshalAs(UnmanagedType.FunctionPtr)> ByVal start As DMAME_START, <MarshalAs(UnmanagedType.FunctionPtr)> ByVal mstop As DMAME_STOP, <MarshalAs(UnmanagedType.FunctionPtr)> ByVal copydata As DMAME_COPYDATA, <MarshalAs(UnmanagedType.FunctionPtr)> ByVal updatestate As DMAME_UPDATESTATE) As Integer
    End Function

    <DllImport("Mame.dll")> _
    Private Shared Function map_id_to_outname(ByVal id As Integer) As IntPtr
    End Function

    <DllImport("Mame.dll")> _
    Private Shared Function close_mame() As Integer
    End Function

    Private Delegate Function DMAME_START() As Integer
    Private Delegate Function DMAME_STOP() As Integer
    Private Delegate Function DMAME_COPYDATA(ByVal id As Integer, ByVal name As IntPtr) As Integer
    Private Delegate Function DMAME_UPDATESTATE(ByVal id As Integer, ByVal state As Integer) As Integer

    <MarshalAs(UnmanagedType.FunctionPtr)> _
    Dim start_ptr As DMAME_START = Nothing

    <MarshalAs(UnmanagedType.FunctionPtr)> _
    Dim stop_ptr As DMAME_STOP = Nothing

    <MarshalAs(UnmanagedType.FunctionPtr)> _
    Dim copydata_ptr As DMAME_COPYDATA = Nothing

    <MarshalAs(UnmanagedType.FunctionPtr)> _
    Dim updatestate_ptr As DMAME_UPDATESTATE = Nothing

    Dim OutputHash As Hashtable = New Hashtable()
    Dim Text1 As TextBox = Nothing

    Public Sub New(ByVal Text As TextBox)
        Text1 = Text
    End Sub

    Public Sub New(ByVal clientid As Integer, ByVal name As String)
        Initialize(clientid, name)
    End Sub

    Public Sub Initialize(ByVal clientid As Integer, ByVal name As String)
        start_ptr = AddressOf mame_start
        stop_ptr = AddressOf mame_stop
        copydata_ptr = AddressOf mame_copydata
        updatestate_ptr = AddressOf mame_updatestate

        init_mame(clientid, name, start_ptr, stop_ptr, copydata_ptr, updatestate_ptr)
    End Sub

    Function mame_start() As Integer
	OutputHash.Clear()

        Text1.Text += "mame_start" + Environment.NewLine

        Return 1
    End Function

    Function mame_stop() As Integer
        Text1.Text += "mame_stop" + Environment.NewLine

        Return 1
    End Function

    Function mame_copydata(ByVal id As Integer, ByVal nm As IntPtr) As Integer
        Dim name As String = Marshal.PtrToStringAnsi(nm)
        Text1.Text += "id " + id.ToString() + " = '" + name + "'" + Environment.NewLine

        Return 1
    End Function

    Function mame_updatestate(ByVal id As Integer, ByVal state As Integer) As Integer
        Dim name As String = Nothing

        If OutputHash(id) Is Nothing Then
            name = Marshal.PtrToStringAnsi(map_id_to_outname(id))

            OutputHash.Add(id, name)
        Else
            name = CType(OutputHash(id), String)
        End If

        Text1.Text += "update_state: id=" + id.ToString() + " (" + name + ") " + " state=" + state.ToString() + Environment.NewLine

        Return 1
    End Function

    Private disposedValue As Boolean = False        ' To detect redundant calls

    ' IDisposable
    Protected Overridable Sub Dispose(ByVal disposing As Boolean)
        If Not Me.disposedValue Then
            If disposing Then
                ' TODO: free unmanaged resources when explicitly called
                close_mame()
            End If

            ' TODO: free shared unmanaged resources
        End If
        Me.disposedValue = True
    End Sub

#Region " IDisposable Support "
    ' This code added by Visual Basic to correctly implement the disposable pattern.
    Public Sub Dispose() Implements IDisposable.Dispose
        ' Do not change this code.  Put cleanup code in Dispose(ByVal disposing As Boolean) above.
        Dispose(True)
        GC.SuppressFinalize(Me)
    End Sub
#End Region

End Class