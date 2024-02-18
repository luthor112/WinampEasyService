using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace skinner_helper
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
        static extern bool GetWindowRect(IntPtr hWnd, ref Rect lpRect);

        [DllImport("user32.dll", SetLastError = true)]
        static extern IntPtr FindWindowEx(IntPtr parentHandle, IntPtr hWndChildAfter, string className, string windowTitle);

        [StructLayout(LayoutKind.Sequential)]
        struct Rect
        {
            public readonly int Left;
            public readonly int Top;
            public readonly int Right;
            public readonly int Bottom;
        }

        const uint EVENT_SYSTEM_MINIMIZESTART = 0x0016;
        const uint EVENT_SYSTEM_MINIMIZEEND = 0x0017;
        const uint EVENT_OBJECT_FOCUS = 0x8005;
        const uint EVENT_OBJECT_LOCATIONCHANGE = 0x800B;
        const uint EVENT_SYSTEM_CAPTURESTART = 0x0008;
        const uint EVENT_SYSTEM_CAPTUREEND = 0x0009;
        const uint EVENT_OBJECT_DESTROY = 0x8001;
        const uint WINEVENT_OUTOFCONTEXT = 0x0000;

        const int SW_HIDE = 0;
        const int SW_SHOW = 5;

        IntPtr locationHook;
        IntPtr minimizeHook;
        IntPtr focusHook;
        IntPtr captureHook;
        IntPtr destroyHook;

        IntPtr shadowHwnd;
        string shadowTitle;
        string skinPath;

        public Form1(IntPtr _shadowHwnd, string _shadowTitle, string _skinPath)
        {
            shadowHwnd = _shadowHwnd;
            shadowTitle = _shadowTitle;
            skinPath = _skinPath;
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            string titleFileName = System.IO.Path.Combine(skinPath, "mb.bmp");
            if (!File.Exists(titleFileName))
            {
                titleFileName = System.IO.Path.Combine(skinPath, "mb.png");
                if (!File.Exists(titleFileName))
                {
                    Application.Exit();
                }
            }
            Image fullImage = Image.FromFile(titleFileName);

            Image leftImage = new Bitmap(25, 20, PixelFormat.Format24bppRgb);
            using (Graphics grp = Graphics.FromImage(leftImage))
            {
                grp.DrawImage(fullImage, new Rectangle(0, 0, 25, 20), new Rectangle(0, 0, 25, 20), GraphicsUnit.Pixel);
            }
            leftPicture.Image = leftImage;

            Image midImage = new Bitmap(25, 20, PixelFormat.Format24bppRgb);
            using (Graphics grp = Graphics.FromImage(midImage))
            {
                grp.DrawImage(fullImage, new Rectangle(0, 0, 25, 20), new Rectangle(127, 0, 25, 20), GraphicsUnit.Pixel);
            }
            midPicture.BackgroundImage = midImage;

            Image rightImage = new Bitmap(25, 20, PixelFormat.Format24bppRgb);
            using (Graphics grp = Graphics.FromImage(rightImage))
            {
                grp.DrawImage(fullImage, new Rectangle(0, 0, 25, 20), new Rectangle(153, 0, 25, 20), GraphicsUnit.Pixel);
            }
            rightPicture.Image = rightImage;

            ShowWindow(this.Handle, SW_SHOW);
            RelocateSelf();

            uint shadowProcessID;
            uint shadowThreadID = GetWindowThreadProcessId(shadowHwnd, out shadowProcessID);
            locationHook = SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE, IntPtr.Zero, WindowChangeHook, shadowProcessID, shadowThreadID, WINEVENT_OUTOFCONTEXT);
            minimizeHook = SetWinEventHook(EVENT_SYSTEM_MINIMIZESTART, EVENT_SYSTEM_MINIMIZEEND, IntPtr.Zero, WindowChangeHook, shadowProcessID, shadowThreadID, WINEVENT_OUTOFCONTEXT);
            focusHook = SetWinEventHook(EVENT_OBJECT_FOCUS, EVENT_OBJECT_FOCUS, IntPtr.Zero, WindowChangeHook, shadowProcessID, shadowThreadID, WINEVENT_OUTOFCONTEXT);
            captureHook = SetWinEventHook(EVENT_SYSTEM_CAPTURESTART, EVENT_SYSTEM_CAPTUREEND, IntPtr.Zero, WindowChangeHook, shadowProcessID, shadowThreadID, WINEVENT_OUTOFCONTEXT);
            destroyHook = SetWinEventHook(EVENT_OBJECT_DESTROY, EVENT_OBJECT_DESTROY, IntPtr.Zero, WindowChangeHook, shadowProcessID, shadowThreadID, WINEVENT_OUTOFCONTEXT);
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
            else if (eventType == EVENT_SYSTEM_CAPTURESTART)
            {
                TopMost = true;
            }
            else if (eventType == EVENT_SYSTEM_CAPTUREEND)
            {
                TopMost = false;
            }
            else if (eventType == EVENT_OBJECT_DESTROY && hwnd == shadowHwnd)
            {
                Close();
            }
            else
            {
                RelocateSelf();
            }
        }

        private void RelocateSelf()
        {
            Rect newLocation = new Rect();
            GetWindowRect(shadowHwnd, ref newLocation);
            Point newPoint = new Point(newLocation.Left, newLocation.Top);
            Size newSize = new Size(newLocation.Right - newLocation.Left, 20);

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
            UnhookWinEvent(focusHook);
            UnhookWinEvent(captureHook);
            UnhookWinEvent(destroyHook);
        }

        protected override CreateParams CreateParams
        {
            get
            {
                const int WS_EX_LAYERED = 0x80000;
                const int WS_EX_TRANSPARENT = 0x20;
                CreateParams cp = base.CreateParams;
                cp.ExStyle |= WS_EX_LAYERED;
                cp.ExStyle |= WS_EX_TRANSPARENT;
                return cp;
            }
        }
    }
}
