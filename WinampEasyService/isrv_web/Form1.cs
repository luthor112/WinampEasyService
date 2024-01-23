using CefSharp.WinForms;
using CefSharp;
using System.Security.AccessControl;
using System.Runtime.InteropServices;

namespace isrv_web
{
    public partial class Form1 : Form
    {
        [DllImport("user32.dll")]
        static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

        [DllImport("user32.dll", SetLastError = true)]
        static extern IntPtr SetWindowsHookEx(int hookType, HookProc lpfn, IntPtr hMod, uint dwThreadId);
        delegate IntPtr HookProc(int code, IntPtr wParam, IntPtr lParam);

        [DllImport("user32.dll")]
        static extern IntPtr SetWinEventHook(uint eventMin, uint eventMax,
            IntPtr hmodWinEventProc, WinEventDelegate lpfnWinEventProc, uint idProcess,
            uint idThread, uint dwFlags);
        delegate void WinEventDelegate(IntPtr hWinEventHook, uint eventType,
            IntPtr hwnd, int idObject, int idChild, uint dwEventThread, uint dwmsEventTime);

        [DllImport("user32.dll")]
        static extern bool UnhookWinEvent(IntPtr hWinEventHook);

        [DllImport("user32.dll", SetLastError = true)]
        static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint lpdwProcessId);

        [DllImport("user32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool GetWindowRect(IntPtr hWnd, ref Rect lpRect);

        [DllImport("user32.dll", SetLastError = true)]
        public static extern IntPtr FindWindowEx(IntPtr parentHandle, IntPtr hWndChildAfter, string className, string windowTitle);

        [StructLayout(LayoutKind.Sequential)]
        private struct Rect
        {
            public readonly int Left;
            public readonly int Top;
            public readonly int Right;
            public readonly int Bottom;
        }

        const int SW_HIDE = 0;
        const int SW_SHOW = 5;
        const uint EVENT_SYSTEM_FOREGROUND = 0x0003;
        const uint EVENT_SYSTEM_MINIMIZESTART = 0x0016;
        const uint EVENT_SYSTEM_MINIMIZEEND = 0x0017;
        const uint EVENT_OBJECT_FOCUS = 0x8005;
        const uint EVENT_OBJECT_LOCATIONCHANGE = 0x800B;
        const uint WINEVENT_OUTOFCONTEXT = 0x0000;

        const uint CAP_DEFAULT = 0;
        const uint CAP_CUSTOMDIALOG = 1;

        IntPtr hwndWinampParent;
        IntPtr hwndLibraryParent;
        IntPtr hwndParentControl;
        string pluginDir;
        string skinPath;
        string configFile;
        string shortName;
        uint serviceID;

        IntPtr locationHook;
        IntPtr minimizeHook;
        IntPtr focusHook;

        string[] callArgs;
        ChromiumWebBrowser browser;

        public Form1(string[] args)
        {
            callArgs = args;
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            var settings = new CefSettings()
            {
                //By default CefSharp will use an in-memory cache, you need to specify a Cache Folder to persist data
                CachePath = System.IO.Path.Join(System.IO.Path.GetTempPath(), "wmp_webcache"),
                PersistSessionCookies = true
            };
            settings.CefCommandLineArgs.Add("enable-media-stream");
            settings.CefCommandLineArgs.Add("use-fake-ui-for-media-stream");
            Cef.Initialize(settings, performDependencyCheck: true, browserProcessHandler: null);

            if (callArgs[1] == "GetNodeDesc" || callArgs[1] == "GetFileName")
            {
                browser = new ChromiumWebBrowser(callArgs[0]);
                browser.JavascriptObjectRepository.Settings.LegacyBindingEnabled = true;
                Controls.Add(browser);

                browser.LoadingStateChanged += Browser_LoadingStateChanged;
            }
            else if (callArgs[1] == "GetCustomDialog")
            {
                hwndWinampParent = IntPtr.Parse(callArgs[2]);
                hwndLibraryParent = IntPtr.Parse(callArgs[3]);
                hwndParentControl = IntPtr.Parse(callArgs[4]);
                pluginDir = callArgs[5];
                skinPath = callArgs[6];
                configFile = callArgs[7];
                shortName = callArgs[8];
                serviceID = uint.Parse(callArgs[9]);

                browser = new ChromiumWebBrowser(callArgs[0]);
                browser.JavascriptObjectRepository.Settings.LegacyBindingEnabled = true;
                // TODO
                //browser.JavascriptObjectRepository.Register("yolo", new yolo());
                Controls.Add(browser);

                ShowWindow(this.Handle, SW_SHOW);
                RelocateSelf();

                uint winampProcessID;
                uint libraryThreadID = GetWindowThreadProcessId(hwndLibraryParent, out winampProcessID);
                locationHook = SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE, IntPtr.Zero, WindowChangeHook, winampProcessID, libraryThreadID, WINEVENT_OUTOFCONTEXT);
                minimizeHook = SetWinEventHook(EVENT_SYSTEM_MINIMIZESTART, EVENT_SYSTEM_MINIMIZEEND, IntPtr.Zero, WindowChangeHook, winampProcessID, libraryThreadID, WINEVENT_OUTOFCONTEXT);
                focusHook = SetWinEventHook(EVENT_OBJECT_FOCUS, EVENT_OBJECT_FOCUS, IntPtr.Zero, WindowChangeHook, winampProcessID, libraryThreadID, WINEVENT_OUTOFCONTEXT);
            }
        }

        private async void Browser_LoadingStateChanged(object? sender, LoadingStateChangedEventArgs e)
        {
            if (!e.IsLoading)
            {
                if (callArgs[1] == "GetNodeDesc")
                {
                    var nodeName = await browser.EvaluateScriptAsync<string>("getNodeName();");
                    Console.WriteLine("Web");
                    Console.WriteLine(nodeName);
                    Console.WriteLine("NONE");
                    Console.WriteLine(CAP_CUSTOMDIALOG);
                    this.BeginInvoke(() => { Cef.Shutdown(); });
                }
                else if (callArgs[1] == "GetFileName")
                {
                    var fileName = await browser.EvaluateScriptAsync<string>($"getFileName('{callArgs[2]}');");
                    Console.WriteLine(fileName);
                    this.BeginInvoke(() => { Cef.Shutdown(); });
                }
            }
        }

        private void WindowChangeHook(IntPtr hWinEventHook, uint eventType,
            IntPtr hwnd, int idObject, int idChild, uint dwEventThread, uint dwmsEventTime)
        {
            if (eventType == EVENT_SYSTEM_MINIMIZESTART)
            {
                WindowState = FormWindowState.Minimized;
            }
            else if (eventType == EVENT_SYSTEM_MINIMIZEEND)
            {
                WindowState = FormWindowState.Normal;
            }
            else if (eventType == EVENT_OBJECT_FOCUS)
            {
                TopMost = true;
                TopMost = false;
            }
            else
            {
                RelocateSelf();
            }
        }

        private void RelocateSelf()
        {
            IntPtr clientArea = FindWindowEx(hwndParentControl, IntPtr.Zero, "#32770", null);

            Rect newLocation = new Rect();
            GetWindowRect(clientArea, ref newLocation);
            Point newPoint = new Point(newLocation.Left, newLocation.Top);
            Size newSize = new Size(newLocation.Right - newLocation.Left, newLocation.Bottom - newLocation.Top);

            if (newPoint != Location || newSize != Size)
            {
                Location = newPoint;
                Size = newSize;
                TopMost = true;
                TopMost = false;
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            UnhookWinEvent(locationHook);
            UnhookWinEvent(minimizeHook);
        }

        protected override CreateParams CreateParams
        {
            get
            {
                CreateParams cparam = base.CreateParams;
                // turn on WS_EX_TOOLWINDOW style bit
                cparam.ExStyle |= 0x80;
                return cparam;
            }
        }
    }
}
