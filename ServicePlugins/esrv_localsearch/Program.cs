namespace esrv_localsearch
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
                Console.WriteLine("Local Search");
            }
            else if (args[0] == "GetColumnNames")
            {
                Console.WriteLine("Filename\tFull path");
            }
            else if (args[0] == "InvokeService")
            {
                ApplicationConfiguration.Initialize();
                Application.Run(new Form1(false));
            }
            else if (args[0] == "InvokeServiceCustom")
            {
                ApplicationConfiguration.Initialize();
                Application.Run(new Form1(true));
            }
        }
    }
}