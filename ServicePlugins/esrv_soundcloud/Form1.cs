using SoundCloudExplode;
using SoundCloudExplode.Search;
using System.Runtime.InteropServices;

namespace esrv_soundcloud
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

        private async void button1_Click(object sender, EventArgs e)
        {
            textBox1.Enabled = false;
            button1.Enabled = false;
            button1.Text = "Please wait...";

            var soundcloud = new SoundCloudClient();
            var results = await soundcloud.Search.GetResultsAsync(textBox1.Text);
            foreach (var result in results)
            {
                switch (result)
                {
                    case TrackSearchResult track:
                        {
                            Console.WriteLine(track?.User?.FullName ?? "");
                            Console.WriteLine(track.Title);
                            Console.WriteLine(track.Url);
                            Console.WriteLine("ref_" + track.Id);
                            break;
                        }
                }
            }

            Application.Exit();
        }
    }
}