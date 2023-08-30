using System.Runtime.InteropServices;

namespace esrv_localsearch
{
    public partial class Form1 : Form
    {
        [DllImport("user32.dll")]
        static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

        const int SW_HIDE = 0;
        const int SW_SHOW = 5;

        bool useCustomColumns;

        public Form1(bool _useCustomColumns)
        {
            useCustomColumns = _useCustomColumns;

            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            ShowWindow(this.Handle, SW_SHOW);
        }

        private void selectButton_Click(object sender, EventArgs e)
        {
            if (folderBrowserDialog1.ShowDialog() == DialogResult.OK)
                directoryBox.Text = folderBrowserDialog1.SelectedPath;
        }

        private void searchButton_Click(object sender, EventArgs e)
        {
            directoryBox.Enabled = false;
            patternBox.Enabled = false;
            recursiveBox.Enabled = false;
            selectButton.Enabled = false;
            searchButton.Enabled = false;
            searchButton.Text = "Please wait...";

            foreach (string fileName in Directory.EnumerateFiles(directoryBox.Text, patternBox.Text, recursiveBox.Checked ? SearchOption.AllDirectories : SearchOption.TopDirectoryOnly))
            {
                if (useCustomColumns)
                {
                    Console.WriteLine(System.IO.Path.GetFileName(fileName) + "\t" + fileName);
                    Console.WriteLine(System.IO.Path.GetFileName(fileName));
                    Console.WriteLine(fileName);
                }
                else
                {
                    Console.WriteLine(System.IO.Path.GetFileName(fileName));
                    Console.WriteLine(System.IO.Path.GetFileName(fileName));
                    Console.WriteLine(fileName);
                    Console.WriteLine(fileName);
                }
            }

            Application.Exit();
        }
    }
}