using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace msrv_soundcloud_plus
{
    public partial class SoundCloudControl : UserControl
    {
        IntPtr hwndWinampParent;

        public SoundCloudControl(IntPtr _hwndWinampParent)
        {
            hwndWinampParent = _hwndWinampParent;
            InitializeComponent();
        }

        private void SoundCloudControl_Load(object sender, EventArgs e)
        {
            FillResultList();
        }

        private void FillResultList()
        {

        }
    }
}
