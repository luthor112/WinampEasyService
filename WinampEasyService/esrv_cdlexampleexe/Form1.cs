using System.Drawing;
using System.Drawing.Imaging;
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
        const uint EVENT_SYSTEM_FOREGROUND = 0x0003;
        const uint EVENT_SYSTEM_MINIMIZESTART = 0x0016;
        const uint EVENT_SYSTEM_MINIMIZEEND = 0x0017;
        const uint EVENT_OBJECT_FOCUS = 0x8005;
        const uint EVENT_OBJECT_LOCATIONCHANGE = 0x800B;
        const uint WINEVENT_OUTOFCONTEXT = 0x0000;

        IntPtr hwndWinampParent;
        IntPtr hwndLibraryParent;
        IntPtr hwndParentControl;
        string pluginDir;
        string skinPath;
        string configFile;
        string shortName;
        IntPtr serviceID;

        IntPtr locationHook;
        IntPtr minimizeHook;
        IntPtr focusHook;

        Bitmap buttonBgImage;
        Bitmap buttonDownImage;

        public Form1(IntPtr _hwndWinampParent, IntPtr _hwndLibraryParent, IntPtr _hwndParentControl, string _pluginDir, string _skinPath, string _configFile, string _shortName, IntPtr _serviceID)
        {
            hwndWinampParent = _hwndWinampParent;
            hwndLibraryParent = _hwndLibraryParent;
            hwndParentControl = _hwndParentControl;
            pluginDir = _pluginDir;
            skinPath = _skinPath;
            configFile = _configFile;
            shortName = _shortName;
            serviceID = _serviceID;

            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            ShowWindow(this.Handle, SW_SHOW);
            RelocateSelf();

            uint winampProcessID;
            uint libraryThreadID = GetWindowThreadProcessId(hwndLibraryParent, out winampProcessID);
            locationHook = SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE, IntPtr.Zero, WindowChangeHook, winampProcessID, libraryThreadID, WINEVENT_OUTOFCONTEXT);
            minimizeHook = SetWinEventHook(EVENT_SYSTEM_MINIMIZESTART, EVENT_SYSTEM_MINIMIZEEND, IntPtr.Zero, WindowChangeHook, winampProcessID, libraryThreadID, WINEVENT_OUTOFCONTEXT);
            focusHook = SetWinEventHook(EVENT_OBJECT_FOCUS, EVENT_OBJECT_FOCUS, IntPtr.Zero, WindowChangeHook, winampProcessID, libraryThreadID, WINEVENT_OUTOFCONTEXT);

            string genexFilename = System.IO.Path.Join(skinPath, "genex.bmp");
            if (File.Exists(genexFilename))
            {
                Bitmap genex = new Bitmap(genexFilename);
                Color itemBgColor = genex.GetPixel(48, 0);
                Color itemFgColor = genex.GetPixel(50, 0);
                Color windowBackgroundColor = genex.GetPixel(52, 0);
                Color buttonTextColor = genex.GetPixel(54, 0);
                Color windowTextColor = genex.GetPixel(56, 0);

                // 47x15 in genex, but 1px has been removed from the border
                buttonBgImage = new Bitmap(45, 13);
                using (Graphics gr = Graphics.FromImage(buttonBgImage))
                {
                    gr.DrawImage(genex, new Rectangle(0, 0, 45, 13), new Rectangle(1, 1, 45, 13), GraphicsUnit.Pixel);
                }

                buttonDownImage = new Bitmap(45, 13);
                using (Graphics gr = Graphics.FromImage(buttonDownImage))
                {
                    gr.DrawImage(genex, new Rectangle(0, 0, 45, 13), new Rectangle(1, 16, 45, 13), GraphicsUnit.Pixel);
                }

                BackColor = windowBackgroundColor;
                label1.ForeColor = windowTextColor;
                button1.BackColor = Color.Transparent;
                button1.BackgroundImage = buttonBgImage;
                button1.BackgroundImageLayout = ImageLayout.Stretch;
                button1.FlatAppearance.BorderSize = 0;
                button1.FlatAppearance.MouseOverBackColor = Color.Transparent;
                button1.FlatStyle = FlatStyle.Flat;
                button1.ForeColor = buttonTextColor;
                button1.UseVisualStyleBackColor = false;
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

        private void button1_Click(object sender, EventArgs e)
        {
            MessageBox.Show("Button Pressed!", "Example UI EXE Plugin");
        }

        private void button1_MouseDown(object sender, MouseEventArgs e)
        {
            button1.BackgroundImage = buttonDownImage;
        }

        private void button1_MouseUp(object sender, MouseEventArgs e)
        {
            button1.BackgroundImage = buttonBgImage;
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