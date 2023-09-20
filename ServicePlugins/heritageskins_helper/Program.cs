using System.Security.Cryptography;
using System.Drawing;
using System.Drawing.Imaging;

namespace heritageskins_helper
{
    internal class Program
    {
        static async Task Main(string[] args)
        {
            if (args.Length < 3)
            {
                Console.WriteLine("Oops...");
            }
            else if (args[0] == "page")
            {
                // page <CategoryLink> <PageNum> <CacheDir>
                await CreatePageCache(args[1], int.Parse(args[2]), args[3]);
            }
            else if (args[0] == "download")
            {
                // download <Link> <FileName>
                await DownloadFileAsync(args[1], args[2]);
            }
        }

        static async Task CreatePageCache(string catLink, int pageNum, string cacheDir)
        {
            Directory.CreateDirectory(cacheDir);

            using (var client = new HttpClient())
            {
                string pageURL = "https://winampheritage.com/skins";
                if (catLink != "_FEATURED_")
                    pageURL += "/" + catLink;
                if (pageNum != 0)
                    pageURL += "/page-" + pageNum.ToString();

                string listPage = await client.GetStringAsync(pageURL);

                
                using (StreamWriter writer = new StreamWriter(System.IO.Path.Join(cacheDir, "page.txt")))
                {
                    int dlPageIndex = listPage.IndexOf("<a rel=\"tab\" href=\"/skin/");
                    while (dlPageIndex != -1)
                    {
                        int dlPageEndIndex = listPage.IndexOf("\"", dlPageIndex + 25);
                        int imageIndex = listPage.IndexOf("<img src=\"", dlPageEndIndex);
                        int imageEndIndex = listPage.IndexOf("\"", imageIndex + 10);
                        int titleIndex = listPage.IndexOf(">", imageEndIndex);
                        int titleEndIndex = listPage.IndexOf("<", titleIndex + 1);

                        string dlPageURL = listPage.Substring(dlPageIndex + 19, dlPageEndIndex - (dlPageIndex + 19));
                        string imageURL = listPage.Substring(imageIndex + 10, imageEndIndex - (imageIndex + 10));
                        string title = listPage.Substring(titleIndex + 1, titleEndIndex - (titleIndex + 1));

                        // Get download link and filename
                        string link = await GetDownloadLink(dlPageURL);
                        string filename = link.Substring(link.LastIndexOf("/") + 1);

                        // Download thumbnail
                        await DownloadThumbnailAsync(imageURL, System.IO.Path.Join(cacheDir, $"{filename}.bmp"));

                        writer.WriteLine(link);
                        writer.WriteLine(title);
                        writer.WriteLine(filename);

                        dlPageIndex = listPage.IndexOf("<a rel=\"tab\" href=\"/skin/", titleEndIndex);
                    }
                }
            }
        }

        static async Task<string> GetDownloadLink(string dlPageURL)
        {
            using (var client = new HttpClient())
            {
                string dlPage = await client.GetStringAsync($"https://winampheritage.com{dlPageURL}");
                int dlButtonIndex = dlPage.IndexOf("<a class=\"downloadbutton\"");
                int dlLinkIndex = dlPage.IndexOf("'/skin/", dlButtonIndex);
                int dlLinkEndIndex = dlPage.IndexOf("'", dlLinkIndex + 7);

                if (dlButtonIndex == -1 || dlLinkIndex == -1 || dlLinkEndIndex == -1)
                    return "";
                else
                    return dlPage.Substring(dlLinkIndex + 1, dlLinkEndIndex - (dlLinkIndex + 1));
            }
        }

        static async Task DownloadThumbnailAsync(string relLink, string outputFile)
        {
            if (!File.Exists(outputFile))
            {
                using (var client = new HttpClient())
                {
                    var response = await client.GetAsync($"https://winampheritage.com{relLink}");
                    Image imageData = Image.FromStream(response.Content.ReadAsStream());
                    if (imageData.Width == 178 && imageData.Height == 75)
                    {
                        imageData.Save(outputFile, ImageFormat.Bmp);
                    }
                    else
                    {
                        Image resizedImage = new Bitmap(178, 75, PixelFormat.Format24bppRgb);
                        using (Graphics grp = Graphics.FromImage(resizedImage))
                        {
                            //grp.FillRectangle(Brushes.DarkGray, 0, 0, 178, 75);
                            grp.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.High;
                            grp.DrawImage(imageData, 0, 0, 178, 75);
                        }
                        resizedImage.Save(outputFile, ImageFormat.Bmp);
                    }
                }
            }
        }

        static async Task DownloadFileAsync(string link, string outputFile)
        {
            if (!File.Exists(outputFile))
            {
                using (var client = new HttpClient())
                {
                    client.DefaultRequestHeaders.Add("Cookie", "downloadsite=winampheritage");

                    var response = await client.GetAsync($"https://winampheritage.com{link}");
                    using (var fs = new FileStream(outputFile, FileMode.Create))
                    {
                        await response.Content.CopyToAsync(fs);
                    }
                }
            }
        }
    }
}