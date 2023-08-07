using SpotifyExplode;
using System.Net;
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
            if (args[0] == "GetNodeName")
            {
                Console.WriteLine("Spotify");
            }
            else if (args[0] == "InvokeService")
            {
                ApplicationConfiguration.Initialize();
                Application.Run(new Form1());
            }
            else if (args[0] == "GetFileName")
            {
                string trackID = args[1].Substring(4);
                
                var spotify = new SpotifyClient();
                string trackURL = "https://open.spotify.com/track/" + trackID;
                var downloadUrl = await spotify.Tracks.GetDownloadUrlAsync(trackURL);

                try
                {
                    string outputFile = $"{System.IO.Path.GetTempPath()}\\{trackID}.mp3";
                    using (var client = new WebClient())
                    {
                        client.DownloadFile(downloadUrl, outputFile);
                    }

                    Console.WriteLine(outputFile);
                }
                catch (Exception ex)
                {
                    string videoID = await spotify.Tracks.GetYoutubeIdAsync(trackURL);

                    var youtube = new YoutubeClient();
                    var streamManifest = await youtube.Videos.Streams.GetManifestAsync("https://www.youtube.com/watch?v=" + videoID);
                    var streamInfo = streamManifest.GetAudioOnlyStreams().GetWithHighestBitrate();
                    var stream = await youtube.Videos.Streams.GetAsync(streamInfo);

                    string outputFile = $"{System.IO.Path.GetTempPath()}\\{videoID}.{streamInfo.Container}";
                    await youtube.Videos.Streams.DownloadAsync(streamInfo, outputFile);
                    Console.WriteLine(outputFile);
                }
            }
        }
    }
}