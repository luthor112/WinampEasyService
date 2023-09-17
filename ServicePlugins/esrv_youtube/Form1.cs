using System.Runtime.InteropServices;
using YoutubeExplode;
using YoutubeExplode.Common;

namespace esrv_youtube
{
    public partial class Form1 : Form
    {
        [DllImport("user32.dll")]
        static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

        const int SW_HIDE = 0;
        const int SW_SHOW = 5;

        bool _directLink;

        public Form1(bool directLink)
        {
            _directLink = directLink;
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            ShowWindow(this.Handle, SW_SHOW);
        }

        private async void button1_ClickAsync(object sender, EventArgs e)
        {
            textBox1.Enabled = false;
            button1.Enabled = false;
            button1.Text = "Please wait...";

            var youtube = new YoutubeClient();
            var videos = await youtube.Search.GetVideosAsync(textBox1.Text);

            foreach (var item in videos)
            {
                Console.WriteLine(item.Author);
                Console.WriteLine(item.Title);
                Console.WriteLine(item.Url);
                if (_directLink)
                    Console.WriteLine(item.Url);
                else
                    Console.WriteLine("ref_" + item.Id);
            }

            Application.Exit();
        }
    }
}