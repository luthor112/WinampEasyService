using SpotifyExplode;
using SpotifyExplode.Search;
using System.Runtime.InteropServices;

namespace esrv_spotify
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

        private async void button1_Click(object sender, EventArgs e)
        {
            textBox1.Enabled = false;
            button1.Enabled = false;
            button1.Text = "Please wait...";

            var spotify = new SpotifyClient();
            var results = await spotify.Search.GetResultsAsync(textBox1.Text);

            foreach (var item in results)
            {
                switch (item)
                {
                    case TrackSearchResult track:
                        {
                            if (useCustomColumns)
                            {
                                Console.WriteLine(track.Title + "\t" + track.Url);
                                Console.WriteLine(track.Title);
                                Console.WriteLine("ref_" + track.Id);
                            }
                            else
                            {
                                Console.WriteLine(track.Title);
                                Console.WriteLine(track.Title);
                                Console.WriteLine(track.Url);
                                Console.WriteLine("ref_" + track.Id);
                            }
                            break;
                        }
                }
            }

            Application.Exit();
        }
    }
}