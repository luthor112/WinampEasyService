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

namespace msrv_soundcloud_plus
{
    public partial class SoundCloudControl : UserControl
    {
        Action<string, bool> addItem;

        IntPtr hwndWinampParent;
        Dictionary<string, object> functions;
        string myDirectory;

        public SoundCloudControl(IntPtr _hwndWinampParent, Dictionary<string, object> _functions, string _myDirectory)
        {
            hwndWinampParent = _hwndWinampParent;
            functions = _functions;
            myDirectory = _myDirectory;

            addItem = (Action<string, bool>)functions["AddItem"];

            InitializeComponent();
        }

        private void SoundCloudControl_Load(object sender, EventArgs e)
        {
            FillResultList();
        }

        private void FillResultList()
        {
            resultListView.Items.Clear();

            string pageCacheName = System.IO.Path.Join(System.IO.Path.GetTempPath(), "sc_plus_page.txt");
            if (System.IO.File.Exists(pageCacheName))
            {
                foreach (var line in System.IO.File.ReadLines(pageCacheName))
                {
                    if (!string.IsNullOrEmpty(line))
                    {
                        string[] strings = line.Split('\t');
                        ListViewItem lvi = new ListViewItem(new string[] { strings[0], strings[1], strings[2], strings[3], strings[4] });
                        resultListView.Items.Add(lvi);
                    }
                }
            }
        }

        private void InternalRunAndWait(string args)
        {
            ListViewItem lvi = new ListViewItem(new string[] { "Loading..." });
            resultListView.Items.Clear();
            resultListView.Items.Add(lvi);

            ProcessStartInfo processStartInfo = new ProcessStartInfo("esrv_soundcloud.exe");
            processStartInfo.UseShellExecute = false;
            processStartInfo.CreateNoWindow = true;
            processStartInfo.WindowStyle = ProcessWindowStyle.Hidden;
            processStartInfo.WorkingDirectory = myDirectory;
            processStartInfo.Arguments = args;
            Process.Start(processStartInfo).WaitForExit();
        }

        private void searchMediaBtn_Click(object sender, EventArgs e)
        {
            InternalRunAndWait($"internal search media \"{searchBox.Text}\"");
            FillResultList();
        }

        private void searchListsBtn_Click(object sender, EventArgs e)
        {
            InternalRunAndWait($"internal search list \"{searchBox.Text}\"");
            FillResultList();
        }

        private void searchBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == (char)Keys.Enter)
            {
                searchMediaBtn_Click(null, null);
            }
        }

        private void resultListView_ItemActivate(object sender, EventArgs e)
        {
            if (resultListView.SelectedItems[0].SubItems[2].Text == "Media")
            {
                addItem(resultListView.SelectedItems[0].SubItems[4].Text, Control.ModifierKeys == Keys.Alt);
            }
            else
            {
                InternalRunAndWait($"internal openlist \"{resultListView.SelectedItems[0].SubItems[4].Text}\"");
                FillResultList();
            }
        }
    }
}
