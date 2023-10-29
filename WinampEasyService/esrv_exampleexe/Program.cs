namespace srv_exampleexe
{
    internal class Program
    {
        static void Main(string[] args)
        {
            const uint CAP_DEFAULT = 0;
            const uint CAP_CUSTOMDIALOG = 1;

            if (args[0] == "GetNodeDesc")
            {
                Console.WriteLine("Examples");
                Console.WriteLine("Example EXE Plugin");
                Console.WriteLine("Title\tType of item");
                Console.WriteLine(CAP_DEFAULT);
            }
            else if (args[0] == "InvokeService")
            {
                Console.WriteLine("Example Title\tHello from CS EXE - This is a direct filename");
                Console.WriteLine("Example Playlist Title");
                Console.WriteLine("e:\\example.mp3");

                Console.WriteLine("Example Title 2\tHello from CS EXE - This will use a reference");
                Console.WriteLine("Example Playlist Title 2");
                Console.WriteLine("ref_examplefile");
            }
            else if (args[0] == "GetFileName")
            {
                if (args[1] == "ref_examplefile")
                    Console.WriteLine("e:\\example.mp3");
                else
                    Console.WriteLine("This really shouldn't happen...");
            }
        }
    }
}