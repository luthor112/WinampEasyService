using SoundCloudExplode;
using System.Runtime.InteropServices;
using System.Text;

namespace esrv_soundcloud
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
            GetPrivateProfileString("soundcloud", configKey, configSBGlobal.ToString(), configSB, (uint)configSB.Capacity, configFileName);

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
                Console.WriteLine("SoundCloud");
            }
            else if (args[0] == "InvokeService")
            {
                bool directLink = false;
                string directLinkConfig = GetConfigString("directlink", "false");
                if (directLinkConfig == "true")
                    directLink = true;
                else if (directLinkConfig == "false" || directLinkConfig == "onplay")
                    directLink = false;

                ApplicationConfiguration.Initialize();
                Application.Run(new Form1(directLink));
            }
            else if (args[0] == "GetFileName")
            {
                string trackID = args[1].Substring(4);
                string directLinkConfig = GetConfigString("directlink", "false");
                string cachePath = GetConfigString("cachedir", System.IO.Path.GetTempPath());

                if (directLinkConfig == "false")
                {
                    string outputFile = System.IO.Path.Join(cachePath, $"sc_{trackID}.mp3");
                    if (File.Exists(outputFile))
                    {
                        Console.WriteLine(outputFile);
                    }
                    else
                    {
                        var soundcloud = new SoundCloudClient();
                        var track = await soundcloud.Tracks.GetByIdAsync(long.Parse(trackID));
                        await soundcloud.DownloadAsync(track, outputFile);

                        // Update tags
                        var tfile = TagLib.File.Create(outputFile);
                        tfile.Tag.Performers = new string[] { track?.User?.FullName ?? "" };
                        tfile.Tag.Title = track.Title;
                        tfile.Save();

                        Console.WriteLine(outputFile);
                    }
                }
                else if (directLinkConfig == "onplay")
                {
                    var soundcloud = new SoundCloudClient();
                    var track = await soundcloud.Tracks.GetByIdAsync(long.Parse(trackID));
                    var downloadURL = await soundcloud.Tracks.GetDownloadUrlAsync(track);
                    Console.WriteLine(downloadURL);
                }
            }
        }
    }
}