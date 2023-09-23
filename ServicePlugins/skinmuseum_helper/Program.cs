using Newtonsoft.Json.Linq;
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO.Compression;
using System.Reflection;
using System.Runtime.InteropServices;

namespace skinmuseum_helper
{
    internal class Program
    {
        [DllImport("kernel32.dll")]
        static extern uint GetPrivateProfileInt(string lpAppName, string lpKeyName, int nDefault, string lpFileName);

        static async Task Main(string[] args)
        {
            if (args.Length != 3)
            {
                Console.WriteLine("Oops...");
            }
            else if (args[0] == "page")
            {
                // page <PageNum> <CacheDir>
                await CreatePageCache(int.Parse(args[1]), args[2]);
            }
            else if (args[0] == "download")
            {
                // download <MD5> <FileName>
                await DownloadFileAsync(args[1], args[2]);
            }
        }

        static async Task CreatePageCache(int pageNum, string cacheDir)
        {
            Directory.CreateDirectory(cacheDir);

            using (var client = new HttpClient())
            {
                string pageURL = "https://api.webampskins.org/graphql";
                client.DefaultRequestHeaders.Add("Accept", "application/json");

                string configFileName = System.IO.Path.Join(System.IO.Path.GetDirectoryName(Assembly.GetEntryAssembly().Location), "easysrv.ini");
                int pageLength = (int)GetPrivateProfileInt("skinmuseum", "pagelength", 50, configFileName);

                StringContent jsonContent = new StringContent("{\"query\":\"\\n        query MuseumPage($offset: Int, $first: Int) {\\n          skins(offset: $offset, first: $first, sort: MUSEUM) {\\n            count\\n            nodes {\\n              md5\\n              filename\\n              nsfw\\n            }\\n          }\\n        }\\n      \",\"variables\":{\"offset\":" + ((pageNum - 1) * pageLength).ToString() + ",\"first\":" + pageLength.ToString() + "}}",
                                                              System.Text.Encoding.UTF8,
                                                              "application/json");
                var result = await client.PostAsync(pageURL, jsonContent);
                string resultString = await result.Content.ReadAsStringAsync();
                JObject resultJSON = JObject.Parse(resultString);

                using (StreamWriter writer = new StreamWriter($"{cacheDir}\\page.txt"))
                {
                    foreach (var node in resultJSON["data"]["skins"]["nodes"])
                    {
                        await DownloadThumbnailAsync(node["md5"].ToString(), cacheDir + "\\" + node["filename"].ToString() + ".bmp");
                        writer.WriteLine($"{node["md5"]} {node["filename"]}");
                    }

                    writer.WriteLine("");
                }
            }
        }

        static async Task DownloadThumbnailAsync(string md5, string outputFile)
        {
            if (!File.Exists(outputFile))
            {
                using (var client = new HttpClient())
                {
                    var response = await client.GetAsync($"https://r2.webampskins.org/screenshots/{md5}.png");
                    Image imageData = Image.FromStream(response.Content.ReadAsStream());
                    imageData.Save(outputFile, ImageFormat.Bmp);
                }
            }
        }

        static async Task DownloadFileAsync(string md5, string outputFile)
        {
            if (!File.Exists(outputFile))
            {
                using (var client = new HttpClient())
                {
                    var response = await client.GetAsync($"https://r2.webampskins.org/skins/{md5}{outputFile.Substring(outputFile.Length - 4)}");
                    if (!response.IsSuccessStatusCode)
                    {
                        string[] validExts = { ".wsz", ".wal", ".zip" };
                        foreach (string ext in validExts)
                        {
                            response = await client.GetAsync($"https://r2.webampskins.org/skins/{md5}{ext}");
                            if (response.IsSuccessStatusCode)
                                break;
                        }
                    }
                    using (var fs = new FileStream(outputFile, FileMode.Create))
                    {
                        await response.Content.CopyToAsync(fs);
                    }
                }
            }
        }
    }
}