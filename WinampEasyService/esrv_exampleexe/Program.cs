namespace srv_exampleexe
{
    internal class Program
    {
        static void Main(string[] args)
        {
            if (args[0] == "GetNodeName")
            {
                Console.WriteLine("Example EXE Service Plugin");
            }
            else if (args[0] == "InvokeService")
            {
                Console.WriteLine("Example Author");
                Console.WriteLine("Example Title");
                Console.WriteLine("Hello from CS EXE - This is a direct filename");
                Console.WriteLine("e:\\example.mp3");

                Console.WriteLine("Example Author");
                Console.WriteLine("Example Title 2");
                Console.WriteLine("Hello from CS EXE - This will use a reference");
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