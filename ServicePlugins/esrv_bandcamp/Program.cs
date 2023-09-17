using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.TrackBar;

namespace esrv_bandcamp
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
            GetPrivateProfileString("bandcamp", configKey, configSBGlobal.ToString(), configSB, (uint)configSB.Capacity, configFileName);

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
                Console.WriteLine("Bandcamp");
            }
            else if (args[0] == "InvokeService")
            {
                string cachePath = GetConfigString("cachedir", System.IO.Path.GetTempPath());
                string saveFileName = System.IO.Path.Join(cachePath, "spotify_saved_login");

                ApplicationConfiguration.Initialize();
                Application.Run(new Form1(saveFileName));
            }
            else if (args[0] == "GetFileName")
            {
                // args[1]: ref_{i}_{username}_{identity}
                string[] parts = args[1].Split('_');

                int index = int.Parse(parts[1]);
                string username = parts[2];
                string identity = parts[3];
                string cachePath = GetConfigString("cachedir", System.IO.Path.GetTempPath());

                using (var client = new HttpClient())
                {
                    client.DefaultRequestHeaders.Add("Cookie", "identity=" + identity);
                    string purchasesPageURL = @"https://bandcamp.com/" + username + @"/purchases";
                    string purchasesPage = await client.GetStringAsync(purchasesPageURL);

                    var linkRegex = new Regex(@"&quot;https://bandcamp.com/download\?.*?&quot;");
                    var matches = linkRegex.Matches(purchasesPage);
                    string matchValue = matches[index].Value;
                    string downloadPageURL = matchValue.Substring(6, matchValue.Length - 12).Replace(@"&amp;", "&");
                    string downloadPage = await client.GetStringAsync(downloadPageURL);

                    var downloadRegex = new Regex(@"&quot;https://.{1,20}\.bandcamp.com/download/.*?&quot;");
                    var downloadMatchValue = downloadRegex.Match(downloadPage).Value;
                    string downloadURL = downloadMatchValue.Substring(6, matchValue.Length - 11).Replace(@"&amp;", "&");

                    string fileType = "mp3";
                    if (downloadURL.Contains("/album"))
                        fileType = "zip";
                    string outputFile = System.IO.Path.Join(cachePath, $"bandcamp_{index}.{fileType}");

                    if (File.Exists(outputFile))
                    {
                        Console.WriteLine(outputFile);
                    }
                    else
                    {
                        var response = await client.GetAsync(downloadURL);
                        using (var fs = new FileStream(outputFile, FileMode.Create))
                        {
                            await response.Content.CopyToAsync(fs);
                        }
                        Console.WriteLine(outputFile);
                    }
                }
            }
        }
    }
}