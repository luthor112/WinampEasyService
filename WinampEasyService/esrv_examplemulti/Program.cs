namespace esrv_examplemulti
{
    internal class Program
    {
        static void Main(string[] args)
        {
            const uint CAP_DEFAULT = 0;
            const uint CAP_CUSTOMDIALOG = 1;
            const uint CAP_MULTISERVICE = 2;

            int currentMultiID = 0;
            int cmdIndex = 0;
            if (args[0] == "select")
            {
                currentMultiID = int.Parse(args[1]);
                cmdIndex = 2;
            }

            if (args[cmdIndex] == "GetNodeNum")
            {
                Console.WriteLine(2);
            }
            else if (args[cmdIndex] == "GetNodeDesc")
            {
                switch (currentMultiID)
                {
                    case 0:
                        Console.WriteLine("Examples");
                        Console.WriteLine("Example EXE Multi Plugin 1");
                        Console.WriteLine("Title\tType of item");
                        Console.WriteLine(CAP_MULTISERVICE);
                        break;
                    case 1:
                        Console.WriteLine("Examples");
                        Console.WriteLine("Example EXE Multi Plugin 2");
                        Console.WriteLine("Title\tType of item");
                        Console.WriteLine(CAP_MULTISERVICE);
                        break;
                }
            }
            else if (args[cmdIndex] == "InvokeService")
            {
                switch (currentMultiID)
                {
                    case 0:
                        Console.WriteLine("Example Title\tHello from CS EXE Subservice 1!");
                        Console.WriteLine("Example Playlist Title");
                        Console.WriteLine("e:\\example.mp3");

                        Console.WriteLine("Example Title 2\tHello from CS EXE Subservice 1!");
                        Console.WriteLine("Example Playlist Title 2");
                        Console.WriteLine("e:\\example.mp3");

                        break;
                    case 1:
                        Console.WriteLine("Example Title\tHello from CS EXE Subservice 2!");
                        Console.WriteLine("Example Playlist Title");
                        Console.WriteLine("e:\\example.mp3");

                        Console.WriteLine("Example Title 2\tHello from CS EXE Subservice 2!");
                        Console.WriteLine("Example Playlist Title 2");
                        Console.WriteLine("e:\\example.mp3");

                        break;
                }
            }
        }
    }
}
