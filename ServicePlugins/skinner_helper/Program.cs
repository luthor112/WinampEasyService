namespace skinner_helper
{
    internal static class Program
    {
        [STAThread]
        static void Main(string[] args)
        {
            ApplicationConfiguration.Initialize();
            Application.Run(new Form1(IntPtr.Parse(args[0]), args[1], args[2]));
        }
    }
}