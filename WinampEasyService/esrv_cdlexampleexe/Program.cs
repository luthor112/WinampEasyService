namespace esrv_cdlexampleexe
{
    internal static class Program
    {
        /// <summary>
        ///  The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            if (args[0] == "GetNodeName")
            {
                Console.WriteLine("Example EXE Plugin with Custom Dialog");
            }
            else if (args[0] == "CanGetCustomDialog")
            {
                Console.WriteLine("1");
            }
            else if (args[0] == "GetCustomDialog")
            {
                ApplicationConfiguration.Initialize();
                Application.Run(new Form1(IntPtr.Parse(args[1]), IntPtr.Parse(args[2]), IntPtr.Parse(args[3]), args[4]));
            }
        }
    }
}