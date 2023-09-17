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
            GetPrivateProfileString("spotify", configKey, configSBGlobal.ToString(), configSB, (uint)configSB.Capacity, configFileName);

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
                Console.WriteLine("Spotify");
            }
            else if (args[0] == "GetColumnNames")
            {
                Console.WriteLine("Title\tLink");
            }
            else if (args[0] == "InvokeService")
            {
                ApplicationConfiguration.Initialize();
                Application.Run(new Form1(false, GetConfigString("directlink", "false") == "true"));
            }
            else if (args[0] == "InvokeServiceCustom")
            {
                ApplicationConfiguration.Initialize();
                Application.Run(new Form1(true, GetConfigString("directlink", "false") == "true"));
            }
            else if (args[0] == "GetFileName")
            {
                string trackID = args[1].Substring(4);
                string directLinkConfig = GetConfigString("directlink", "false");
                string cachePath = GetConfigString("cachedir", System.IO.Path.GetTempPath());

                if (directLinkConfig == "false")
                {
                    string outputFile = System.IO.Path.Join(cachePath, $"s_{trackID}.mp3");
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

                            string[] cachedFiles = Directory.GetFiles(cachePath, $"yt_{videoID}.*", SearchOption.TopDirectoryOnly);
                            if (cachedFiles.Length == 0)
                            {
                                var youtube = new YoutubeClient();
                                var streamManifest = await youtube.Videos.Streams.GetManifestAsync("https://www.youtube.com/watch?v=" + videoID);
                                var streamInfo = streamManifest.GetAudioOnlyStreams().GetWithHighestBitrate();
                                var stream = await youtube.Videos.Streams.GetAsync(streamInfo);

                                string outputFile2 = System.IO.Path.Join(cachePath, $"yt_{videoID}.{streamInfo.Container}");
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
                else if (directLinkConfig == "onplay")
                {
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
                        var youtube = new YoutubeClient();
                        var streamManifest = await youtube.Videos.Streams.GetManifestAsync("https://www.youtube.com/watch?v=" + videoID);
                        var streamInfo = streamManifest.GetAudioOnlyStreams().GetWithHighestBitrate();
                        Console.WriteLine(streamInfo.Url);
                    }
                }
            }
        }
    }
}