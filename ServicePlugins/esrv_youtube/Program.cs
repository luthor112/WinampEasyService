using System.Runtime.InteropServices;
using System.Text;
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
            const uint CAP_DEFAULT = 0;
            const uint CAP_CUSTOMDIALOG = 1;

            if (args[0] == "GetNodeDesc")
            {
                Console.WriteLine("Media");
                Console.WriteLine("YouTube");
                Console.WriteLine("Author\tTitle\tPage URL");
                Console.WriteLine(CAP_DEFAULT);
            }
            else if (args[0] == "InvokeService")
            {
                ApplicationConfiguration.Initialize();
                Application.Run(new Form1());
            }
            /*else if (args[0] == "GetFileName")
            {
                string directLinkConfig = GetConfigString("directlink", "false");
                string videoID = args[1].Substring(4);
                string cachePath = GetConfigString("cachedir", System.IO.Path.GetTempPath());

                if (directLinkConfig == "false")
                {
                    string[] cachedFiles = Directory.GetFiles(cachePath, $"yt_{videoID}.*", SearchOption.TopDirectoryOnly);
                    if (cachedFiles.Length == 0)
                    {
                        var youtube = new YoutubeClient();
                        var streamManifest = await youtube.Videos.Streams.GetManifestAsync("https://www.youtube.com/watch?v=" + videoID);
                        var streamInfo = streamManifest.GetAudioOnlyStreams().GetWithHighestBitrate();

                        string outputFile = System.IO.Path.Join(cachePath, $"yt_{videoID}.{streamInfo.Container}");
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
                else if (directLinkConfig == "onplay")
                {
                    var youtube = new YoutubeClient();
                    var streamManifest = await youtube.Videos.Streams.GetManifestAsync("https://www.youtube.com/watch?v=" + videoID);
                    var streamInfo = streamManifest.GetAudioOnlyStreams().GetWithHighestBitrate();
                    Console.WriteLine(streamInfo.Url);
                }
            }*/
        }
    }
}