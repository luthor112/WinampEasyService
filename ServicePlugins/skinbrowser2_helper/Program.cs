using System.Drawing;
using System.Drawing.Imaging;
using System.IO.Compression;

namespace skinbrowser2_helper
{
    internal class Program
    {
        static void Main(string[] args)
        {
            if (args.Length != 3)
            {
                Console.WriteLine("Oops...");
            }
            else if (args[0] == "all")
            {
                // all <SkinsDir> <ThumbsDir>
                ExtractAllThumbnails(args[1], args[2]);
            }
            else if (args[0] == "single")
            {
                // single <SkinFile> <ThumbsDir>
                ExtractThumbnail(args[1], args[2]);
            }
        }

        static void ExtractAllThumbnails(string skinsDir, string thumbsDir)
        {
            Directory.CreateDirectory(thumbsDir);

            foreach (string fsName in Directory.EnumerateFileSystemEntries(skinsDir))
            {
                ExtractThumbnail(fsName, thumbsDir);
            }
        }

        static void ExtractThumbnail(string skinFilename, string thumbsDir)
        {
            string[] validThumbnails = { "main.png", "main.bmp", "mainframe.png", "mainframe.bmp", "screenshot.png", "screenshot.bmp", "wacupscreenshot.png", "wacupscreenshot.bmp" };

            if (Directory.Exists(skinFilename))
            {
                foreach (string checkName in validThumbnails)
                {
                    string checkNameFull = $"{skinFilename}\\{checkName}";
                    if (File.Exists(checkNameFull))
                    {
                        Image imageFile = Image.FromFile(checkNameFull);
                        imageFile.Save($"{thumbsDir}\\{System.IO.Path.GetFileName(skinFilename)}.bmp", ImageFormat.Bmp);
                        return;
                    }
                }
            }
            else if (File.Exists(skinFilename))
            {
                // This is ugly, but it keeps the order of validThumbnails
                ZipArchive archive = ZipFile.OpenRead(skinFilename);
                foreach (string checkName in validThumbnails)
                {
                    foreach (ZipArchiveEntry entry in archive.Entries)
                    {
                        if (entry.Name.ToLower() == checkName)
                        {
                            Image imageFile = Image.FromStream(entry.Open());
                            imageFile.Save($"{thumbsDir}\\{System.IO.Path.GetFileName(skinFilename)}.bmp", ImageFormat.Bmp);
                            return;
                        }
                    }
                }
            }

            // Default image to prevent the desync of the list
            Image dummyImage = new Bitmap(275, 116, PixelFormat.Format24bppRgb);
            using (Graphics grp = Graphics.FromImage(dummyImage))
            {
                grp.FillRectangle(Brushes.DarkGray, 0, 0, 275, 116);
            }
            dummyImage.Save($"{thumbsDir}\\{System.IO.Path.GetFileName(skinFilename)}.bmp", ImageFormat.Bmp);
        }
    }
}