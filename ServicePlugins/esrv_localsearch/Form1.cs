using System.Runtime.InteropServices;

namespace esrv_localsearch
{
    public partial class Form1 : Form
    {
        [DllImport("user32.dll")]
        static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

        const int SW_HIDE = 0;
        const int SW_SHOW = 5;

        public Form1()
        {
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
            foreach (string fileName in Directory.EnumerateFiles(directoryBox.Text, patternBox.Text, recursiveBox.Checked ? SearchOption.AllDirectories : SearchOption.TopDirectoryOnly))
            {
                Console.WriteLine(System.IO.Path.GetFileName(fileName));
                Console.WriteLine(System.IO.Path.GetFileName(fileName));
                Console.WriteLine(fileName);
                Console.WriteLine(fileName);
            }

            Application.Exit();
        }
    }
}