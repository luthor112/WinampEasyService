using System.Reflection;

namespace isrv_managed
{
    internal static class Program
    {
        [STAThread]
        static void Main(string[] args)
        {
            var DLL = Assembly.LoadFile(args[0]);
            var theType = DLL.GetType("msrv.EasyService");
            var c = Activator.CreateInstance(theType);

            if (args[1] == "GetNodeDesc")
            {
                var method = theType.GetMethod("GetNodeDesc");
                Tuple<string, string, string, uint> nodeDesc = (Tuple<string, string, string, uint>)method.Invoke(c, null);
                Console.WriteLine(nodeDesc.Item1);
                Console.WriteLine(nodeDesc.Item2);
                Console.WriteLine(nodeDesc.Item3);
                Console.WriteLine(nodeDesc.Item4);
            }
            else if (args[1] == "InvokeService")
            {
                IntPtr hwndWinampParent = IntPtr.Parse(args[2]);
                IntPtr hwndLibraryParent = IntPtr.Parse(args[3]);
                IntPtr hwndParentControl = IntPtr.Parse(args[4]);
                string pluginDir = args[5];
                string skinPath = args[6];
                uint serviceID = uint.Parse(args[7]);

                Dictionary<string, object> functionDict = new Dictionary<string, object>();

                var initMethod = theType.GetMethod("InitService");
                var method = theType.GetMethod("InvokeService");

                initMethod.Invoke(c, new object[] { functionDict, pluginDir, serviceID });
                List<List<string>> fList = (List<List<string>>)method.Invoke(c, new object[] { hwndWinampParent, hwndLibraryParent, hwndParentControl, skinPath });
                foreach (List<string> item in fList)
                {
                    foreach (string attr in item)
                    {
                        Console.WriteLine(attr);
                    }
                }
            }
            else if (args[1] == "GetFileName")
            {
                var method = theType.GetMethod("GetFileName");
                string fName = (string)method.Invoke(c, new object[] { args[2] });
                Console.WriteLine(fName);
            }
            else if (args[1] == "GetCustomDialog")
            {
                IntPtr hwndWinampParent = IntPtr.Parse(args[2]);
                IntPtr hwndLibraryParent = IntPtr.Parse(args[3]);
                IntPtr hwndParentControl = IntPtr.Parse(args[4]);
                string pluginDir = args[5];
                string skinPath = args[6];
                uint serviceID = uint.Parse(args[7]);

                Dictionary<string, object> functionDict = new Dictionary<string, object>();

                var initMethod = theType.GetMethod("InitService");
                var method = theType.GetMethod("GetCustomDialog");

                ApplicationConfiguration.Initialize();
                initMethod.Invoke(c, new object[] { functionDict, pluginDir, serviceID });
                UserControl customUI = (UserControl)method.Invoke(c, new object[] { hwndWinampParent, hwndLibraryParent, hwndParentControl, skinPath });
                Application.Run(new Form1(hwndWinampParent, hwndLibraryParent, hwndParentControl, pluginDir, skinPath, serviceID, customUI));
            }
        }
    }
}