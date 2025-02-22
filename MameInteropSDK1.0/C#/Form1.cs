using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace MameInteropTest
{
    public partial class Form1 : Form
    {
        MameInterop mame = null;

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            mame = new MameInterop(this.textBox1);
            mame.Initialize(2, "C#Test");
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            mame.Dispose();
        }
    }
}