using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using System.Collections;

class MameInterop : IDisposable
{
    [DllImport("Mame.dll")]
    static extern int init_mame(int clientid, string name, [MarshalAs(UnmanagedType.FunctionPtr)] MAME_START start, [MarshalAs(UnmanagedType.FunctionPtr)] MAME_STOP stop, [MarshalAs(UnmanagedType.FunctionPtr)] MAME_COPYDATA copydata, [MarshalAs(UnmanagedType.FunctionPtr)] MAME_UPDATESTATE updatestate);

    [DllImport("Mame.dll")]
    static extern IntPtr map_id_to_outname(int id);

    [DllImport("Mame.dll")]
    static extern int close_mame();

    private delegate int MAME_START();
    private delegate int MAME_STOP();
    private delegate int MAME_COPYDATA(int id, IntPtr name);
    private delegate int MAME_UPDATESTATE(int id, int state);

    [MarshalAs(UnmanagedType.FunctionPtr)]
    MAME_START start_ptr = null;

    [MarshalAs(UnmanagedType.FunctionPtr)]
    MAME_STOP stop_ptr = null;

    [MarshalAs(UnmanagedType.FunctionPtr)]
    MAME_COPYDATA copydata_ptr = null;

    [MarshalAs(UnmanagedType.FunctionPtr)]
    MAME_UPDATESTATE updatestate_ptr = null;

    Hashtable OutputHash = new Hashtable();
    TextBox Text1 = null;

    public MameInterop(TextBox Text)
    {
        Text1 = Text;
    }

    public MameInterop(int clientid, string name)
    {
        Initialize(clientid, name);
    }

    public void Initialize(int clientid, string name)
    {
        start_ptr = new MAME_START(mame_start);
        stop_ptr = new MAME_STOP(mame_stop);
        copydata_ptr = new MAME_COPYDATA(mame_copydata);
        updatestate_ptr = new MAME_UPDATESTATE(mame_updatestate);

        init_mame(clientid, name, start_ptr, stop_ptr, copydata_ptr, updatestate_ptr);
    }

    int mame_start()
    {
        OutputHash.Clear();

        Text1.Text += "mame_start" + Environment.NewLine;

        return 1;
    }

    int mame_stop()
    {
        Text1.Text += "mame_stop" + Environment.NewLine;

        return 1;
    }

    int mame_copydata(int id, IntPtr nm)
    {
        string name = Marshal.PtrToStringAnsi(nm);

        Text1.Text += "id " + id.ToString() + " = '" + name + "'" + Environment.NewLine;

        return 1;
    }

    int mame_updatestate(int id, int state)
    {
        string name = null;

        if (OutputHash[id] == null)
        {
            name = Marshal.PtrToStringAnsi(map_id_to_outname(id));

            OutputHash.Add(id, name);
        }
        else
            name = (string) OutputHash[id];

        Text1.Text += "update_state: id=" + id.ToString() + " (" + name + ") " + " state=" + state.ToString() + Environment.NewLine;

        return 1;
    }

    #region IDisposable Members

    public void  Dispose()
    {
 	    close_mame();
    }

    #endregion
}

