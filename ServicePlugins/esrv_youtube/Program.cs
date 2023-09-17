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
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
        static extern uint GetPrivateProfileString(
            string lpAppName,
            string lpKeyName,
            string lpDefault,
            StringBuilder lpReturnedString,
            uint nSize,
            string lpFileName);

        static string GetConfigString(string configKey, string defaultValue)
        {
            string configFileName = System.IO.Path.Join(System.IO.Path.GetDirectoryName(Application.ExecutablePath), "easysrv.ini");

            StringBuilder configSBGlobal = new StringBuilder(256);
            GetPrivateProfileString("global", configKey, defaultValue, configSBGlobal, (uint)configSBGlobal.Capacity, configFileName);

            StringBuilder configSB = new StringBuilder(256);
            GetPrivateProfileString("youtube", configKey, configSBGlobal.ToString(), configSB, (uint)configSB.Capacity, configFileName);

            return configSB.ToString();
        }

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
                bool directLink = (args[1] == "PLAYERTYPE_WACUP");
                string directLinkConfig = GetConfigString("directlink", "default");
                if (directLinkConfig == "true")
                    directLink = true;
                else if (directLinkConfig == "false")
                    directLink = false;

                ApplicationConfiguration.Initialize();
                Application.Run(new Form1(directLink));
            }
            else if (args[0] == "GetFileName")
            {
                string videoID = args[1].Substring(4);
                string cachePath = GetConfigString("cachedir", System.IO.Path.GetTempPath());

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
        }
    }
}