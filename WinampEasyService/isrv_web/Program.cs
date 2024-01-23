using CefSharp;
using CefSharp.WinForms;
using System.Security.AccessControl;
using System.Windows.Forms;

namespace isrv_web
{
    internal static class Program
    {
        [STAThread]
        static void Main(string[] args)
        {
            ApplicationConfiguration.Initialize();
            Application.Run(new Form1(args));
        }
    }
}