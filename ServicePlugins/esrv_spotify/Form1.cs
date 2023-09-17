using SpotifyExplode;
using SpotifyExplode.Search;
using System.Runtime.InteropServices;
using YoutubeExplode;
using YoutubeExplode.Common;
using YoutubeExplode.Videos;
using YoutubeExplode.Videos.Streams;

namespace esrv_spotify
{
    public partial class Form1 : Form
    {
        [DllImport("user32.dll")]
        static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

        const int SW_HIDE = 0;
        const int SW_SHOW = 5;

        bool useCustomColumns;
        bool _directLink;

        public Form1(bool _useCustomColumns, bool directLink)
        {
            useCustomColumns = _useCustomColumns;
            _directLink = directLink;

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
                            }
                            else
                            {
                                Console.WriteLine(track.Title);
                                Console.WriteLine(track.Title);
                                Console.WriteLine(track.Url);
                            }

                            if (_directLink)
                            {
                                var downloadUrl = await spotify.Tracks.GetDownloadUrlAsync(track.Url);
                                if (!string.IsNullOrEmpty(downloadUrl))
                                {
                                    Console.WriteLine(downloadUrl);
                                }
                                else
                                {
                                    string videoID = await spotify.Tracks.GetYoutubeIdAsync(track.Url);
                                    var youtube = new YoutubeClient();
                                    var streamManifest = await youtube.Videos.Streams.GetManifestAsync("https://www.youtube.com/watch?v=" + videoID);
                                    var streamInfo = streamManifest.GetAudioOnlyStreams().GetWithHighestBitrate();
                                    Console.WriteLine(streamInfo.Url);
                                }
                            }
                            else
                            {
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