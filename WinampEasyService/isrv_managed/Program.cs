using System.Reflection;

namespace isrv_managed
{
    internal class Program
    {
        static void Main(string[] args)
        {
            var DLL = Assembly.LoadFile(args[0]);
            var theType = DLL.GetType("msrv.EasyService");
            var c = Activator.CreateInstance(theType);

            if (args[1] == "GetNodeName")
            {
                var method = theType.GetMethod("GetNodeName");
                string nodeName = (string)method.Invoke(c, null);
                Console.WriteLine(nodeName);
            }
            else if (args[1] == "InvokeService")
            {
                var method = theType.GetMethod("InvokeService");
                List<List<string>> fList = (List<List<string>>)method.Invoke(c, new object[] { args[2] == "PLAYERTYPE_WACUP" ? 1 : 0 });
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
            else if (args[1] == "GetColumnNames")
            {
                var method = theType.GetMethod("GetColumnNames");
                if (method != null)
                {
                    string columnNames = (string)method.Invoke(c, null);
                    Console.WriteLine(columnNames);
                }
            }
            else if (args[1] == "InvokeServiceCustom")
            {
                var method = theType.GetMethod("InvokeServiceCustom");
                if (method != null)
                {
                    List<List<string>> fList = (List<List<string>>)method.Invoke(c, new object[] { args[2] == "PLAYERTYPE_WACUP" ? 1 : 0 });
                    foreach (List<string> item in fList)
                    {
                        foreach (string attr in item)
                        {
                            Console.WriteLine(attr);
                        }
                    }
                }
            }
        }
    }
}