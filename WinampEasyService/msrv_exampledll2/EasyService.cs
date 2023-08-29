namespace msrv
{
    public class EasyService
    {
        public string GetNodeName()
        {
            return "Example CS DLL Service Plugin v2";
        }

        // #define PLAYERTYPE_WINAMP 0
        // #define PLAYERTYPE_WACUP 1
        public List<List<string>> InvokeService(int PlayerType)
        {
            List<List<string>> retItems = new List<List<string>>();

            retItems.Add(new List<string> { "Example Author", "Example Title", "Hello from CS DLL - This is a direct filename", "e:\\example.mp3" });
            retItems.Add(new List<string> { "Example Author", "Example Title 2", "Hello from CS DLL - This will use a reference", "ref_examplefile" });

            return retItems;
        }

        public string GetFileName(string fileID)
        {
            if (fileID == "ref_examplefile")
                return "e:\\example.mp3";
            else
                return "This really shouldn't happen...";
        }

        public string GetColumnNames()
        {
            return "Title\tType of item";
        }

        public List<List<string>> InvokeServiceCustom(int PlayerType)
        {
            List<List<string>> retItems = new List<List<string>>();

            retItems.Add(new List<string> { "Example Title\tHello from CS DLL - This is a direct filename", "Example Playlist Title", "e:\\example.mp3" });
            retItems.Add(new List<string> { "Example Title 2\tHello from CS DLL - This will use a reference", "Example Playlist Title 2", "ref_examplefile" });

            return retItems;
        }
    }
}
