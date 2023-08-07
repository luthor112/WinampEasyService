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

                var soundcloud = new SoundCloudClient();
                var track = await soundcloud.Tracks.GetByIdAsync(long.Parse(trackID));
                var trackName = string.Join("_", track.Title.Split(Path.GetInvalidFileNameChars()));

                string outputFile = $"{System.IO.Path.GetTempPath()}\\{trackName}.mp3";
                await soundcloud.DownloadAsync(track, outputFile);
                Console.WriteLine(outputFile);
            }
        }
    }
}