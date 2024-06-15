using SpotifyExplode;
using System.Net;
using System.Runtime.InteropServices;
using System.Text;
using YoutubeExplode;
using YoutubeExplode.Common;
using YoutubeExplode.Videos;
using YoutubeExplode.Videos.Streams;

namespace esrv_spotify
{
    internal static class Program
    {
        /// <summary>
        ///  The main entry point for the application.
        /// </summary>
        [STAThread]
        static async Task Main(string[] args)
        {
            const uint CAP_DEFAULT = 0;
            const uint CAP_CUSTOMDIALOG = 1;
            const uint CAP_MULTISERVICE = 2;
            const uint CAP_CUSTOMREFID = 4;

            if (args[0] == "GetNodeDesc")
            {
                Console.WriteLine("Media");
                Console.WriteLine("Spotify");
                Console.WriteLine("Title\tLink");
                Console.WriteLine(CAP_CUSTOMREFID);
            }
            else if (args[0] == "GetCustomRefId")
            {
                Console.WriteLine("spotify");
            }
            else if (args[0] == "InvokeService")
            {
                ApplicationConfiguration.Initialize();
                Application.Run(new Form1(false));
            }
            else if (args[0] == "GetFileName")
            {
                string trackID = args[1].Substring(4);
                string cachePath = System.IO.Path.GetTempPath();

                var spotify = new SpotifyClient();
                string trackURL = "https://open.spotify.com/track/" + trackID;
                var downloadUrl = await spotify.Tracks.GetDownloadUrlAsync(trackURL);

                if (!string.IsNullOrEmpty(downloadUrl))
                {
                    Console.WriteLine(downloadUrl);
                }
                else
                {
                    string videoID = await spotify.Tracks.GetYoutubeIdAsync(trackURL);
                    Console.WriteLine("https://www.youtube.com/watch?v=" + videoID);
                }
            }
        }
    }
}