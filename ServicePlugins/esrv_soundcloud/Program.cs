using SoundCloudExplode;
using System.Runtime.InteropServices;
using System.Text;

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
            const uint CAP_DEFAULT = 0;
            const uint CAP_CUSTOMDIALOG = 1;

            if (args[0] == "GetNodeDesc")
            {
                Console.WriteLine("Media");
                Console.WriteLine("SoundCloud");
                Console.WriteLine("Author\tTitle\tPage URL");
                Console.WriteLine(CAP_DEFAULT);
            }
            else if (args[0] == "InvokeService")
            {
                ApplicationConfiguration.Initialize();
                Application.Run(new Form1(true));
            }
            else if (args[0] == "GetFileName")
            {
                string trackID = args[1].Substring(4);
                string cachePath = System.IO.Path.GetTempPath();

                /*string outputFile = System.IO.Path.Join(cachePath, $"sc_{trackID}.mp3");
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
                }*/

                var soundcloud = new SoundCloudClient();
                var track = await soundcloud.Tracks.GetByIdAsync(long.Parse(trackID));
                var downloadURL = await soundcloud.Tracks.GetDownloadUrlAsync(track);
                Console.WriteLine(downloadURL);
            }
        }
    }
}