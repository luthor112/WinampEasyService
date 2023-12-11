using System.Drawing.Drawing2D;
using System.Globalization;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;
using Button = System.Windows.Forms.Button;
using ListView = System.Windows.Forms.ListView;

namespace isrv_managed
{
    internal static class Program
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

        #region Skinning
        static Bitmap? buttonBgImage = null;
        static Bitmap? buttonDownImage = null;

        static Color listviewHeaderBgColor = Color.Transparent;
        static Color listviewHeaderTextColor = Color.Transparent;
        static Color listviewHeaderFrameMiddleColor = Color.Transparent;
        static Color listviewActiveSelectionBgColor = Color.Transparent;

        public static void SkinControl(Control ctrl, string skinPath)
        {
            string genexFilename = System.IO.Path.Join(skinPath, "genex.bmp");
            if (File.Exists(genexFilename))
            {
                // Needs more colours
                Bitmap genex = new Bitmap(genexFilename);
                Color itemBgColor = genex.GetPixel(48, 0);
                Color itemFgColor = genex.GetPixel(50, 0);
                Color windowBackgroundColor = genex.GetPixel(52, 0);
                Color buttonTextColor = genex.GetPixel(54, 0);
                Color windowTextColor = genex.GetPixel(56, 0);
                Color dividerBorderColor = genex.GetPixel(58, 0);
                Color playlistSelectionColor = genex.GetPixel(60, 0);
                listviewHeaderBgColor = genex.GetPixel(62, 0);
                listviewHeaderTextColor = genex.GetPixel(64, 0);
                Color listviewHeaderFrameTopColor = genex.GetPixel(66, 0);
                listviewHeaderFrameMiddleColor = genex.GetPixel(68, 0);
                Color listviewHeaderFrameBottomColor = genex.GetPixel(70, 0);
                Color listviewHeaderEmptyColor = genex.GetPixel(72, 0);
                Color scrollbarFgColor = genex.GetPixel(74, 0);
                Color scrollbarBgColor = genex.GetPixel(76, 0);
                Color scrollbarInvFgColor = genex.GetPixel(78, 0);
                Color scrollbarInvBgColor = genex.GetPixel(80, 0);
                Color scrollbarDeadAreaColor = genex.GetPixel(82, 0);
                Color listviewActiveSelectionTextColor = genex.GetPixel(84, 0);
                listviewActiveSelectionBgColor = genex.GetPixel(86, 0);
                Color listviewInactiveSelectionTextColor = genex.GetPixel(88, 0);
                Color listviewInactiveSelectionBgColor = genex.GetPixel(90, 0);
                Color itemBgColorAlter = genex.GetPixel(92, 0);
                Color itemFgColorAlter = genex.GetPixel(94, 0);

                if (buttonBgImage == null)
                {
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
                }

                ctrl.BackColor = windowBackgroundColor;

                foreach (Control control in ctrl.Controls)
                {
                    if (control is Label)
                    {
                        Label label = (Label)control;
                        label.ForeColor = windowTextColor;
                    }
                    else if (control is Button)
                    {
                        Button button = (Button)control;
                        button.BackColor = Color.Transparent;
                        button.BackgroundImage = buttonBgImage;
                        button.BackgroundImageLayout = ImageLayout.Stretch;
                        button.FlatAppearance.BorderSize = 0;
                        button.FlatAppearance.MouseOverBackColor = Color.Transparent;
                        button.FlatStyle = FlatStyle.Flat;
                        button.ForeColor = buttonTextColor;
                        button.UseVisualStyleBackColor = false;

                        button.MouseDown += ButtonMouseDown;
                        button.MouseUp += ButtonMouseUp;
                    }
                    else if (control is ListView)
                    {
                        ListView list = (ListView)control;
                        list.BackColor = itemBgColor;
                        list.ForeColor = itemFgColor;

                        // Configure the ListView control for owner-draw and add 
                        // handlers for the owner-draw events.
                        list.OwnerDraw = true;
                        list.DrawColumnHeader += ListView_DrawColumnHeader;
                        list.DrawItem += ListView_DrawItem;
                        list.DrawSubItem += ListView_DrawSubItem;

                        // Add a handler for the MouseUp event so an item can be 
                        // selected by clicking anywhere along its width.
                        list.MouseUp += ListView_MouseUp;

                        // Add handlers for various events to compensate for an 
                        // extra DrawItem event that occurs the first time the mouse 
                        // moves over each row. 
                        list.MouseMove += ListView_MouseMove;
                        list.ColumnWidthChanged += ListView_ColumnWidthChanged;
                        list.Invalidated += ListView_Invalidated;
                    }
                    else
                    {
                        control.BackColor = itemBgColor;
                        control.ForeColor = itemFgColor;
                    }
                }
            }
        }
        #endregion

        #region Button Events
        static void ButtonMouseDown(object sender, MouseEventArgs e)
        {
            Button button = (Button)sender;
            button.BackgroundImage = buttonDownImage;
        }

        static void ButtonMouseUp(object sender, MouseEventArgs e)
        {
            Button button = (Button)sender;
            button.BackgroundImage = buttonBgImage;
        }
        #endregion

        #region ListView Events
        // https://stackoverflow.com/a/75716080
        static void ListView_DrawColumnHeader(object sender, DrawListViewColumnHeaderEventArgs e)
        {
            //Fills one solid background for each cell.
            using (SolidBrush backBrush = new SolidBrush(listviewHeaderBgColor))
            {
                e.Graphics.FillRectangle(backBrush, e.Bounds);
            }
            //Draw the borders for the header around each cell.
            using (Pen backBrush = new Pen(listviewHeaderFrameMiddleColor))
            {
                e.Graphics.DrawRectangle(backBrush, e.Bounds);
            }
            using (SolidBrush foreBrush = new SolidBrush(listviewHeaderTextColor))
            {
                //Since e.Header.TextAlign returns 'HorizontalAlignment' with values of (Right, Center, Left).  
                //DrawString uses 'StringAlignment' with values of (Near, Center, Far). 
                //We must translate these and setup a vertical alignment that doesn't exist in DrawListViewColumnHeaderEventArgs.
                StringFormat stringFormat = GetStringFormat(e.Header.TextAlign);

                //Do some padding, since these draws right up next to the border for Left/Near.  Will need to change this if you use Right/Far
                Rectangle rect = e.Bounds; rect.X += 2;
                e.Graphics.DrawString(e.Header.Text, e.Font, foreBrush, rect, stringFormat);
            }
        }

        static StringFormat GetStringFormat(HorizontalAlignment ha)
        {
            StringAlignment align;

            switch (ha)
            {
                case HorizontalAlignment.Right:
                    align = StringAlignment.Far;
                    break;
                case HorizontalAlignment.Center:
                    align = StringAlignment.Center;
                    break;
                default:
                    align = StringAlignment.Near;
                    break;
            }

            return new StringFormat()
            {
                Alignment = align,
                LineAlignment = StringAlignment.Center
            };
        }

        // https://learn.microsoft.com/en-us/dotnet/api/system.windows.forms.drawlistviewsubitemeventargs?view=windowsdesktop-8.0
        // Selects and focuses an item when it is clicked anywhere along 
        // its width. The click must normally be on the parent item text.
        static void ListView_MouseUp(object sender, MouseEventArgs e)
        {
            ListView listView = (ListView)sender;
            ListViewItem clickedItem = listView.GetItemAt(5, e.Y);
            if (clickedItem != null)
            {
                clickedItem.Selected = true;
                clickedItem.Focused = true;
            }
        }

        // Draws the backgrounds for entire ListView items.
        static void ListView_DrawItem(object sender, DrawListViewItemEventArgs e)
        {
            ListView listView = (ListView)sender;

            if ((e.State & ListViewItemStates.Focused) != 0)
            {
                // Draw the background and focus rectangle for a selected item.
                using (SolidBrush backBrush = new SolidBrush(listviewActiveSelectionBgColor))
                {
                    Rectangle rect = e.Bounds; rect.X += 4;
                    e.Graphics.FillRectangle(backBrush, rect);
                }
                e.DrawFocusRectangle();
            }
            else
            {
                // Draw the background for an unselected item.
                e.DrawBackground();
            }

            // Draw the item text for views other than the Details view.
            if (listView.View != View.Details)
            {
                e.DrawText();
            }
        }

        // Draws subitem text and applies formatting.
        static void ListView_DrawSubItem(object sender, DrawListViewSubItemEventArgs e)
        {
            ListView listView = (ListView)sender;
            TextFormatFlags flags = TextFormatFlags.Left;

            // Store the column text alignment, letting it default
            // to Left if it has not been set to Center or Right.
            switch (e.Header.TextAlign)
            {
                case HorizontalAlignment.Center:
                    flags = TextFormatFlags.HorizontalCenter;
                    break;
                case HorizontalAlignment.Right:
                    flags = TextFormatFlags.Right;
                    break;
            }

            e.DrawText(flags);
        }

        // Forces each row to repaint itself the first time the mouse moves over 
        // it, compensating for an extra DrawItem event sent by the wrapped 
        // Win32 control. This issue occurs each time the ListView is invalidated.
        static void ListView_MouseMove(object sender, MouseEventArgs e)
        {
            ListView listView = (ListView)sender;
            ListViewItem item = listView.GetItemAt(e.X, e.Y);
            if (item != null && item.Tag == null)
            {
                listView.Invalidate(item.Bounds);
                item.Tag = "tagged";
            }
        }

        // Resets the item tags. 
        static void ListView_Invalidated(object sender, InvalidateEventArgs e)
        {
            ListView listView = (ListView)sender;
            foreach (ListViewItem item in listView.Items)
            {
                if (item == null) return;
                item.Tag = null;
            }
        }

        // Forces the entire control to repaint if a column width is changed.
        static void ListView_ColumnWidthChanged(object sender, ColumnWidthChangedEventArgs e)
        {
            ListView listView = (ListView)sender;
            listView.Invalidate();
        }
        #endregion

        static Dictionary<string, object> BuildFunctionDict(string configFile, string shortName, string skinPath, IntPtr hwndWinampParent)
        {
            Dictionary<string, object> functionDict = new Dictionary<string, object>();

            functionDict.Add("GetOption", (string optionName, string defaultValue) => {
                StringBuilder sb = new StringBuilder(1024);
                GetPrivateProfileString(shortName, optionName, defaultValue, sb, (uint)sb.Capacity, configFile);
                return sb.ToString();
            });

            functionDict.Add("SetOption", (string optionName, string optionValue) => {
                WritePrivateProfileString(shortName, optionName, optionValue, configFile);
            });

            functionDict.Add("SkinForm", (Form form) => {
                SkinControl(form, skinPath);
            });

            functionDict.Add("AddItem", (string filename, bool enqueue) => {
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
            });

            return functionDict;
        }

        [STAThread]
        static void Main(string[] args)
        {
            var DLL = Assembly.LoadFile(args[0]);
            var theType = DLL.GetType("msrv.EasyService");
            var c = Activator.CreateInstance(theType);

            if (args[1] == "GetNodeDesc")
            {
                var method = theType.GetMethod("GetNodeDesc");
                if (method != null)
                {
                    Tuple<string, string, string, uint> nodeDesc = (Tuple<string, string, string, uint>)method.Invoke(c, null);
                    Console.WriteLine(nodeDesc.Item1);
                    Console.WriteLine(nodeDesc.Item2);
                    Console.WriteLine(nodeDesc.Item3);
                    Console.WriteLine(nodeDesc.Item4);
                }
                else
                {
                    Console.WriteLine("INVALID_SERVICE");
                    Console.WriteLine("INVALID_SERVICE");
                    Console.WriteLine("INVALID_SERVICE");
                    Console.WriteLine(0);
                }
            }
            else if (args[1] == "InvokeService")
            {
                IntPtr hwndWinampParent = IntPtr.Parse(args[2]);
                IntPtr hwndLibraryParent = IntPtr.Parse(args[3]);
                IntPtr hwndParentControl = IntPtr.Parse(args[4]);
                string pluginDir = args[5];
                string skinPath = args[6];
                string configFile = args[7];
                string shortName = args[8];
                uint serviceID = uint.Parse(args[9]);

                Dictionary<string, object> functionDict = BuildFunctionDict(configFile, shortName, skinPath, hwndWinampParent);

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
                string configFile = args[7];
                string shortName = args[8];
                uint serviceID = uint.Parse(args[9]);

                Dictionary<string, object> functionDict = BuildFunctionDict(configFile, shortName, skinPath, hwndWinampParent);

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