Attribute VB_Name = "Module1"
Option Explicit

Private output_array() As String

Public Declare Function init_mame Lib "Mame" (ByVal id As Long, ByVal name As String, ByVal MameStart As Long, ByVal MameStop As Long, ByVal MameCopyData As Long, ByVal UpdateState As Long) As Long
Public Declare Function close_mame Lib "Mame" () As Long
Public Declare Function map_id_to_outname Lib "Mame" (ByVal id As Long) As String

Public Function mame_start() As Long
    ReDim output_array(1, 0) As String
       
    Form1.Text1.Text = ""
    Form1.Text1.Text = Form1.Text1.Text & "mame_start" & vbNewLine
End Function

Public Function mame_stop() As Long
    Form1.Text1.Text = Form1.Text1.Text & "mame_stop" & vbNewLine
End Function

Public Function mame_copydata(ByVal id As Long, ByVal name As String) As Long
    Form1.Text1.Text = Form1.Text1.Text & "id " & id & " = " & "'" & name & "'" & vbNewLine
End Function

Public Function mame_updatestate(ByVal id As Long, ByVal state As Long) As Long
    Dim name As String
    
    name = get_name_from_id(id)
    
    Form1.Text1.Text = Form1.Text1.Text & "update_state: id=" & id & " (" & name & ") state=" & state & vbNewLine
End Function

Public Function get_name_from_id(id As Long) As String
    Dim i As Integer
    Dim idStr As String
    
    idStr = ""
    
    For i = 1 To UBound(output_array, 2)
        If output_array(0, i) = id Then
            idStr = output_array(1, i)
        End If
    Next i
    
    If idStr = "" Then
        idStr = map_id_to_outname(id)
        ReDim Preserve output_array(1, UBound(output_array, 2) + 1) As String
        output_array(0, UBound(output_array, 2)) = id
        output_array(1, UBound(output_array, 2)) = idStr
    End If
    
    get_name_from_id = idStr
End Function

