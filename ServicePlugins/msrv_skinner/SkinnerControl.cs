using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace msrv_skinner
{
    public partial class SkinnerControl : UserControl
    {
        string myDirectory;
        string skinPath;
        List<IntPtr> hwndList = new List<IntPtr>();

        public SkinnerControl(string _myDirectory, string _skinPath)
        {
            myDirectory = _myDirectory;
            skinPath = _skinPath;
            InitializeComponent();
        }

        private void SkinnerControl_Load(object sender, EventArgs e)
        {
            // https://stackoverflow.com/a/7268375
            Process[] processlist = Process.GetProcesses();

            foreach (Process process in processlist)
            {
                if (!String.IsNullOrEmpty(process.MainWindowTitle))
                {
                    processComboBox.Items.Add(process.MainWindowTitle);
                    hwndList.Add(process.MainWindowHandle);
                }
            }
        }

        private void spawnButton_Click(object sender, EventArgs e)
        {
            string selectedTitle = processComboBox.Text;
            IntPtr selectedHWND = hwndList[processComboBox.SelectedIndex];

            //Process.Start(System.IO.Path.Combine(myDirectory, "skinner_helper.exe"), $"{selectedHWND.ToString()} \"{selectedTitle}\" \"{skinPath}\"");
            Process.Start("calc.exe");
        }
    }
}

