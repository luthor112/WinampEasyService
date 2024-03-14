using CefSharp.WinForms;
using CefSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;

namespace isrv_web
{
    public class WinampJSObject
    {
        #region P/Invoke
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
        static extern uint GetPrivateProfileString(
            string lpAppName,
            string lpKeyName,
            string lpDefault,
            StringBuilder lpReturnedString,
            uint nSize,
            string lpFileName);

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
        static extern uint WritePrivateProfileString(
            string lpAppName,
            string lpKeyName,
            string lpString,
            string lpFileName);

        [DllImport("user32.dll", CharSet = CharSet.Unicode)]
        static extern IntPtr SendMessage(
            IntPtr hWnd,
            uint Msg,
            IntPtr wParam,
            IntPtr lParam);

        [StructLayout(LayoutKind.Sequential)]
        struct COPYDATASTRUCT
        {
            public IntPtr dwData;
            public int cbData;
            public IntPtr lpData;
        }

        const uint WM_COMMAND = 0x0111;
        const uint WM_USER = 0x0400;
        const uint WM_WA_IPC = WM_USER;
        const uint WM_COPYDATA = 0x004A;
        const uint WINAMP_BUTTON2 = 40045;
        const uint IPC_DELETE = 101;
        const uint IPC_SETPLAYLISTPOS = 121;
        const uint IPC_ENQUEUEFILE = 100;
        const uint IPC_ENQUEUEFILEW = 1100;
        #endregion

        string shortName;
        string configFile;
        IntPtr hwndWinampParent;
        ChromiumWebBrowser browser;

        public WinampJSObject(string _shortName, string _configFile, IntPtr _hwndWinampParent, ChromiumWebBrowser _browser)
        {
            shortName = _shortName;
            configFile = _configFile;
            hwndWinampParent = _hwndWinampParent;
            browser = _browser;
        }

        public string getoption(string optionName, string defaultValue)
        {
            StringBuilder sb = new StringBuilder(1024);
            GetPrivateProfileString(shortName, optionName, defaultValue, sb, (uint)sb.Capacity, configFile);
            return sb.ToString();
        }

        public void setoption(string optionName, string optionValue)
        {
            WritePrivateProfileString(shortName, optionName, optionValue, configFile);
        }

        public void additem(string filename, bool enqueue)
        {
            COPYDATASTRUCT copyData = new COPYDATASTRUCT();
            copyData.dwData = new IntPtr(IPC_ENQUEUEFILEW);
            copyData.cbData = (filename.Length + 1) * 2;
            copyData.lpData = Marshal.StringToHGlobalUni(filename);

            IntPtr ptrCopyData = Marshal.AllocCoTaskMem(Marshal.SizeOf(copyData));
            Marshal.StructureToPtr(copyData, ptrCopyData, false);

            if (enqueue)
            {
                SendMessage(hwndWinampParent, WM_COPYDATA, new IntPtr(0), ptrCopyData);
            }
            else
            {
                SendMessage(hwndWinampParent, WM_WA_IPC, new IntPtr(0), new IntPtr(IPC_DELETE));
                SendMessage(hwndWinampParent, WM_COPYDATA, new IntPtr(0), ptrCopyData);
                SendMessage(hwndWinampParent, WM_WA_IPC, new IntPtr(1), new IntPtr(IPC_SETPLAYLISTPOS));
                SendMessage(hwndWinampParent, WM_COMMAND, new IntPtr(WINAMP_BUTTON2), new IntPtr(0));
            }

            Marshal.FreeCoTaskMem(ptrCopyData);
        }

        public void inject(string url, string js)
        {
            browser.LoadUrlAsync(url);
            browser.ExecuteScriptAsyncWhenPageLoaded(js, false);
        }
    }
}
