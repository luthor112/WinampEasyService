using System.Runtime.InteropServices;

namespace esrv_cdlexampleexe
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
        const int WH_CALLWNDPROC = 4;
        const uint EVENT_SYSTEM_MOVESIZEEND = 0x000B;
        const uint EVENT_OBJECT_LOCATIONCHANGE = 0x800B;
        const uint WINEVENT_OUTOFCONTEXT = 0x0000;

        IntPtr hwndWinampParent;
        IntPtr hwndLibraryParent;
        IntPtr hwndParentControl;

        IntPtr hook;

        public Form1(IntPtr _hwndWinampParent, IntPtr _hwndLibraryParent, IntPtr _hwndParentControl)
        {
            hwndWinampParent = _hwndWinampParent;
            hwndLibraryParent = _hwndLibraryParent;
            hwndParentControl = _hwndParentControl;

            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            ShowWindow(this.Handle, SW_SHOW);
            RelocateSelf();

            uint winampProcessID;
            uint libraryThreadID = GetWindowThreadProcessId(hwndLibraryParent, out winampProcessID);
            hook = SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE, IntPtr.Zero, WindowChangeHook, winampProcessID, libraryThreadID, WINEVENT_OUTOFCONTEXT);
        }

        private void WindowChangeHook(IntPtr hWinEventHook, uint eventType,
            IntPtr hwnd, int idObject, int idChild, uint dwEventThread, uint dwmsEventTime)
        {
            RelocateSelf();
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
            UnhookWinEvent(hook);
        }
    }
}