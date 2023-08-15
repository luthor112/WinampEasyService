using SoundCloudExplode;

namespace esrv_soundcloud
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
                Console.WriteLine("SoundCloud");
            }
            else if (args[0] == "InvokeService")
            {
                ApplicationConfiguration.Initialize();
                Application.Run(new Form1());
            }
            else if (args[0] == "GetFileName")
            {
                string trackID = args[1].Substring(4);

                string outputFile = $"{System.IO.Path.GetTempPath()}\\sc_{trackID}.mp3";
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
        }
    }
}