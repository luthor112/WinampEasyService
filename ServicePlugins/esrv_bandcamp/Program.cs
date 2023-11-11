using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;

namespace esrv_bandcamp
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
                Console.WriteLine("Bandcamp");
                Console.WriteLine("Author\tTitle\tPurchase status");
                Console.WriteLine(CAP_DEFAULT);
            }
            else if (args[0] == "InvokeService")
            {
                string cachePath = System.IO.Path.GetTempPath();
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
                string cachePath = System.IO.Path.GetTempPath();

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