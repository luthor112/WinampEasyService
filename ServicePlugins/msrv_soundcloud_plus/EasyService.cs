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
            return new Tuple<string, string, string, uint>("Media", "SoundCloud +", "NONE", CAP_CUSTOMDIALOG);
        }

        public UserControl GetCustomDialog(IntPtr hwndWinampParent, IntPtr hwndLibraryParent, IntPtr hwndParentControl, string skinPath)
        {
            return new msrv_soundcloud_plus.SoundCloudControl(hwndWinampParent);
        }
    }
}
