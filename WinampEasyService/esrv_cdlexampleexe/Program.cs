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
            const uint CAP_DEFAULT = 0;
            const uint CAP_CUSTOMDIALOG = 1;

            if (args[0] == "GetNodeDesc")
            {
                Console.WriteLine("UI Examples");
                Console.WriteLine("Example UI EXE Plugin");
                Console.WriteLine("NONE");
                Console.WriteLine(CAP_CUSTOMDIALOG);
            }
            else if (args[0] == "GetCustomDialog")
            {
                ApplicationConfiguration.Initialize();
                Application.Run(new Form1(IntPtr.Parse(args[1]), IntPtr.Parse(args[2]), IntPtr.Parse(args[3]), args[4], args[5], args[6], args[7], IntPtr.Parse(args[8])));
            }
        }
    }
}