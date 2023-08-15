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
                Application.Run(new Form1());
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
                    var stream = await youtube.Videos.Streams.GetAsync(streamInfo);

                    string outputFile = $"{System.IO.Path.GetTempPath()}\\yt_{videoID}.{streamInfo.Container}";
                    await youtube.Videos.Streams.DownloadAsync(streamInfo, outputFile);
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