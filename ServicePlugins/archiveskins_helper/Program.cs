using Newtonsoft.Json.Linq;
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Security.Cryptography;

namespace archiveskins_helper
{
    internal class Program
    {
        static async Task Main(string[] args)
        {
            if (args[0] == "page")
            {
                // page <PageNum> <PageLength> <CacheDir>
                await CreatePageCache(int.Parse(args[1]), int.Parse(args[2]), args[3]);
            }
            else if (args[0] == "download")
            {
                // download <ID> <FileName> <FullPath>
                await DownloadFileAsync(args[1], args[2], args[3]);
            }
            else
            {
                Console.WriteLine("Oops...");
            }
        }

        static async Task CreatePageCache(int pageNum, int pageLength, string cacheDir)
        {
            Directory.CreateDirectory(cacheDir);

            using (var client = new HttpClient())
            {
                string pageURL = $"https://archive.org/advancedsearch.php?q=collection%3A(winampskins)&fl[]=identifier&fl[]=name&fl[]=source&fl[]=title&rows={pageLength}&page={pageNum}&output=json";
                string resultString = await client.GetStringAsync(pageURL);
                JObject resultJSON = JObject.Parse(resultString);

                using (StreamWriter writer = new StreamWriter($"{cacheDir}\\page.txt"))
                {
                    foreach (var node in resultJSON["response"]["docs"])
                    {
                        string objectID = node["identifier"].ToString();
                        string title = node["title"].ToString();

                        // Get metadata for this specific object
                        string metaURL = $"https://archive.org/metadata/{objectID}";
                        string metaResultString = await client.GetStringAsync(metaURL);
                        JObject metaResultJSON = JObject.Parse(metaResultString);

                        string[] validExts = { ".wsz", ".wal", ".zip" };
                        bool found = false;
                        string filename = "NOT FOUND";
                        foreach (var fileDesc in metaResultJSON["files"])
                        {
                            string currentFilename = fileDesc["name"].ToString();
                            if (validExts.Contains(currentFilename.Substring(currentFilename.Length - 4)))
                            {
                                found = true;
                                filename = currentFilename;
                                break;
                            }
                        }
                        
                        if (found)
                        {
                            writer.WriteLine(objectID);
                            writer.WriteLine(title);
                            writer.WriteLine(filename);

                            await DownloadThumbnailAsync(objectID, cacheDir + "\\" + objectID + ".bmp");
                        }
                    }

                    writer.WriteLine("");
                }
            }
        }

        static async Task DownloadThumbnailAsync(string objectID, string outputFile)
        {
            if (!File.Exists(outputFile))
            {
                using (var client = new HttpClient())
                {
                    var response = await client.GetAsync($"https://archive.org/download/{objectID}/__ia_thumb.jpg");
                    Image imageData = Image.FromStream(response.Content.ReadAsStream());
                    imageData.Save(outputFile, ImageFormat.Bmp);
                }
            }
        }

        static async Task DownloadFileAsync(string objectID, string filename, string outputFile)
        {
            if (!File.Exists(outputFile))
            {
                using (var client = new HttpClient())
                {
                    var response = await client.GetAsync($"https://archive.org/download/{objectID}/{filename}");
                    using (var fs = new FileStream(outputFile, FileMode.Create))
                    {
                        await response.Content.CopyToAsync(fs);
                    }
                }
            }
        }
    }
}
