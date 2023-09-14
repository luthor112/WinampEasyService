using YoutubeExplode;
using YoutubeExplode.Common;
using YoutubeExplode.Videos;
using YoutubeExplode.Videos.Streams;

namespace esrv_youtube
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
                Console.WriteLine("YouTube");
            }
            else if (args[0] == "InvokeService")
            {
                ApplicationConfiguration.Initialize();
                Application.Run(new Form1(args[1] == "PLAYERTYPE_WACUP"));
            }
            else if (args[0] == "GetFileName")
            {
                string videoID = args[1].Substring(4);

                string[] cachedFiles = Directory.GetFiles(System.IO.Path.GetTempPath(), $"yt_{videoID}.*", SearchOption.TopDirectoryOnly);
                if (cachedFiles.Length == 0)
                {
                    var youtube = new YoutubeClient();
                    var streamManifest = await youtube.Videos.Streams.GetManifestAsync("https://www.youtube.com/watch?v=" + videoID);
                    var streamInfo = streamManifest.GetAudioOnlyStreams().GetWithHighestBitrate();

                    string outputFile = $"{System.IO.Path.GetTempPath()}\\yt_{videoID}.{streamInfo.Container}";
                    await youtube.Videos.Streams.DownloadAsync(streamInfo, outputFile);

                    var videoInfo = await youtube.Videos.GetAsync("https://www.youtube.com/watch?v=" + videoID);
                    var tfile = TagLib.File.Create(outputFile);
                    tfile.Tag.Performers = new string[] { videoInfo.Author.ChannelTitle };
                    tfile.Tag.Title = videoInfo.Title;
                    tfile.Save();

                    Console.WriteLine(outputFile);
                }
                else
                {
                    Console.WriteLine(cachedFiles[0]);
                }
            }
        }
    }
}