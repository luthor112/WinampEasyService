namespace msrv
{
    public class EasyService
    {
        public string GetNodeName()
        {
            return "Example CS DLL with Custom UI";
        }

        public UserControl GetCustomDialog(IntPtr WinampHWND)
        {
            return new msrv_cdlexampledll.ExampleUserControl();
        }
    }
}