using SoundCloudExplode;
using SoundCloudExplode.Search;
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

                var soundcloud = new SoundCloudClient();
                var track = await soundcloud.Tracks.GetByIdAsync(long.Parse(trackID));
                var downloadURL = await soundcloud.Tracks.GetDownloadUrlAsync(track);
                Console.WriteLine(downloadURL);
            }
            else if (args[0] == "internal")
            {
                // Extra functionality to support the SoundCloud+ plugin
                string pageCacheName = System.IO.Path.Join(System.IO.Path.GetTempPath(), "sc_plus_page.txt");
                using (StreamWriter writer = new StreamWriter(pageCacheName))
                {
                    if (args[1] == "search")
                    {
                        var soundcloud = new SoundCloudClient();
                        var results = await soundcloud.Search.GetResultsAsync(args[3]);
                        foreach (var result in results)
                        {
                            switch (result)
                            {
                                case TrackSearchResult track:
                                    {
                                        if (args[2] == "media")
                                        {
                                            string author = track?.User?.FullName;
                                            if (string.IsNullOrEmpty(author))
                                                author = "Unknown";

                                            var downloadURL = await soundcloud.Tracks.GetDownloadUrlAsync(track);
                                            writer.WriteLine($"{author}\t{track.Title}\tMedia\t{track.Url}\t{downloadURL}");
                                        }
                                        break;
                                    }
                                case PlaylistSearchResult playlist:
                                    {
                                        if (args[2] == "list")
                                        {
                                            string creator = playlist?.User?.FullName;
                                            if (string.IsNullOrEmpty(creator))
                                                creator = "Unknown";

                                            writer.WriteLine($"{creator}\t{playlist.Title}\tList\t{playlist.Url}\t{playlist.Url}");
                                        }
                                        break;
                                    }
                            }
                        }
                    }
                    else if (args[1] == "openlist")
                    {
                        var soundcloud = new SoundCloudClient();
                        var tracks = await soundcloud.Playlists.GetTracksAsync(args[2]);

                        foreach (var track in tracks)
                        {
                            string author = track?.User?.FullName;
                            if (string.IsNullOrEmpty(author))
                                author = "Unknown";

                            var downloadURL = await soundcloud.Tracks.GetDownloadUrlAsync(track);
                            writer.WriteLine($"{author}\t{track.Title}\tMedia\t{track.Uri}\t{downloadURL}");
                        }
                    }
                }
            }
        }
    }
}