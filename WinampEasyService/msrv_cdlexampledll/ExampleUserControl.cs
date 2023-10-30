using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace msrv_cdlexampledll
{
    public partial class ExampleUserControl : UserControl
    {
        public ExampleUserControl()
        {
            InitializeComponent();
        }

        private void ExampleUserControl_Load(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            MessageBox.Show("Button Pressed!", "Example UI CS DLL Plugin");
        }
    }
}
