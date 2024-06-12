namespace msrv
{
    public class EasyService
    {
        const uint CAP_DEFAULT = 0;
        const uint CAP_CUSTOMDIALOG = 1;
        const uint CAP_MULTISERVICE = 2;

        Dictionary<string, object> functions = new Dictionary<string, object>();
        string myDirectory;
        uint myServiceID;
        int currentMultiID = 0;

        public void InitService(Dictionary<string, object> functionDict, string pluginDir, uint serviceID)
        {
            functions = functionDict;
            myDirectory = pluginDir;
            myServiceID = serviceID;
        }

        public int GetNodeNum()
        {
            return 2;
        }

        public void SelectService(int multiID)
        {
            currentMultiID = multiID;
        }

        public Tuple<string, string, string, uint> GetNodeDesc()
        {
            switch (currentMultiID)
            {
                default:
                case 0:
                    return new Tuple<string, string, string, uint>("Examples", "Example CS DLL Multi Plugin 1", "Title\tType of item", CAP_MULTISERVICE);
                case 1:
                    return new Tuple<string, string, string, uint>("Examples", "Example CS DLL Multi Plugin 2", "Title\tType of item", CAP_MULTISERVICE);
            }
        }

        public List<List<string>> InvokeService(IntPtr hwndWinampParent, IntPtr hwndLibraryParent, IntPtr hwndParentControl, string skinPath)
        {
            List<List<string>> retItems = new List<List<string>>();

            switch (currentMultiID)
            {
                case 0:
                    retItems.Add(new List<string> { "Example Title\tHello from CS DLL Subservice 1!", "Example Playlist Title", "e:\\example.mp3" });
                    retItems.Add(new List<string> { "Example Title 2\ttHello from CS DLL Subservice 1!", "Example Playlist Title 2", "e:\\example.mp3" });
                    break;
                case 1:
                    retItems.Add(new List<string> { "Example Title\tHello from CS DLL Subservice 2!", "Example Playlist Title", "e:\\example.mp3" });
                    retItems.Add(new List<string> { "Example Title 2\ttHello from CS DLL Subservice 2!", "Example Playlist Title 2", "e:\\example.mp3" });
                    break;
            }

            return retItems;
        }
    }
}
