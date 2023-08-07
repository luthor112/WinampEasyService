using System.Text.RegularExpressions;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.TrackBar;

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
            if (args[0] == "GetNodeName")
            {
                Console.WriteLine("Bandcamp");
            }
            else if (args[0] == "InvokeService")
            {
                ApplicationConfiguration.Initialize();
                Application.Run(new Form1());
            }
            else if (args[0] == "GetFileName")
            {
                // args[1]: ref_{i}_{username}_{identity}
                string[] parts = args[1].Split('_');

                int index = int.Parse(parts[1]);
                string username = parts[2];
                string identity = parts[3];

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

                    string outputFile = $"{System.IO.Path.GetTempPath()}\\bandcamp_{index}.mp3";
                    var response = await client.GetAsync(downloadURL);
                    using (var fs = new FileStream(outputFile, FileMode.Create))
                    {
                        await response.Content.CopyToAsync(fs);
                    }
                    Console.WriteLine(outputFile);

                    // Debugging
                    File.WriteAllLines("e:\\bandcamp_debug.txt", new string[] { index.ToString(), username, identity, purchasesPageURL, downloadPageURL, downloadURL });
                }
            }
        }
    }
}