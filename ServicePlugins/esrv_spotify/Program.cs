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

                string outputFile = $"{System.IO.Path.GetTempPath()}\\s_{trackID}.mp3";
                if (File.Exists(outputFile))
                {
                    Console.WriteLine(outputFile);
                }
                else
                {
                    var spotify = new SpotifyClient();
                    string trackURL = "https://open.spotify.com/track/" + trackID;
                    var downloadUrl = await spotify.Tracks.GetDownloadUrlAsync(trackURL);

                    try
                    {
                        using (var client = new WebClient())
                        {
                            client.DownloadFile(downloadUrl, outputFile);
                        }

                        Console.WriteLine(outputFile);
                    }
                    catch (Exception)
                    {
                        string videoID = await spotify.Tracks.GetYoutubeIdAsync(trackURL);

                        string[] cachedFiles = Directory.GetFiles(System.IO.Path.GetTempPath(), $"yt_{videoID}.*", SearchOption.TopDirectoryOnly);
                        if (cachedFiles.Length == 0)
                        {
                            var youtube = new YoutubeClient();
                            var streamManifest = await youtube.Videos.Streams.GetManifestAsync("https://www.youtube.com/watch?v=" + videoID);
                            var streamInfo = streamManifest.GetAudioOnlyStreams().GetWithHighestBitrate();
                            var stream = await youtube.Videos.Streams.GetAsync(streamInfo);

                            string outputFile2 = $"{System.IO.Path.GetTempPath()}\\yt_{videoID}.{streamInfo.Container}";
                            await youtube.Videos.Streams.DownloadAsync(streamInfo, outputFile2);

                            var videoInfo = await youtube.Videos.GetAsync("https://www.youtube.com/watch?v=" + videoID);
                            var tfile = TagLib.File.Create(outputFile2);
                            tfile.Tag.Performers = new string[] { videoInfo.Author.ChannelTitle };
                            tfile.Tag.Title = videoInfo.Title;
                            tfile.Save();

                            Console.WriteLine(outputFile2);
                        }
                        else
                        {
                            Console.WriteLine(cachedFiles[0]);
                        }
                    }
                }
            }
        }
    }
}