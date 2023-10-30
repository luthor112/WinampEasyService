namespace msrv
{
    public class EasyService
    {
        const uint CAP_DEFAULT = 0;
        const uint CAP_CUSTOMDIALOG = 1;

        Dictionary<string, object> functions = new Dictionary<string, object>();
        string myDirectory;
        uint myServiceID;

        public void InitService(Dictionary<string, object> functionDict, string pluginDir, uint serviceID)
        {
            functions = functionDict;
            myDirectory = pluginDir;
            myServiceID = serviceID;
        }

        public Tuple<string, string, string, uint> GetNodeDesc()
        {
            return new Tuple<string, string, string, uint>("Examples", "Example CS DLL Plugin", "Title\tType of item", CAP_DEFAULT);
        }

        public List<List<string>> InvokeService(IntPtr hwndWinampParent, IntPtr hwndLibraryParent, IntPtr hwndParentControl, string skinPath)
        {
            List<List<string>> retItems = new List<List<string>>();

            retItems.Add(new List<string> { "Example Title\tHello from CS DLL - This is a direct filename", "Example Playlist Title", "e:\\example.mp3" });
            retItems.Add(new List<string> { "Example Title 2\tHello from CS DLL - This will use a reference", "Example Playlist Title 2", "ref_examplefile" });

            return retItems;
        }

        public string GetFileName(string fileID)
        {
            if (fileID == "ref_examplefile")
                return "e:\\example.mp3";
            else
                return "This really shouldn't happen...";
        }
    }
}
