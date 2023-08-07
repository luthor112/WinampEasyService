using System.Net;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using System.Web;

namespace esrv_bandcamp
{
    public partial class Form1 : Form
    {
        [DllImport("user32.dll")]
        static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

        const int SW_HIDE = 0;
        const int SW_SHOW = 5;

        string saveFileName = System.IO.Path.GetTempPath() + "\\spotify_saved_login";

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            ShowWindow(this.Handle, SW_SHOW);

            if (File.Exists(saveFileName))
            {
                string[] lines = File.ReadAllLines(saveFileName);
                usernameBox.Text = lines[0];
                identityBox.Text = lines[1];
                saveCheckBox.Checked = true;
            }
        }

        private async void button1_Click(object sender, EventArgs e)
        {
            usernameBox.Enabled = false;
            identityBox.Enabled = false;
            saveCheckBox.Enabled = false;
            button1.Enabled = false;
            button1.Text = "Please wait...";

            if (saveCheckBox.Checked)
            {
                File.WriteAllLines(saveFileName, new string[] { usernameBox.Text, identityBox.Text });
            }

            using (var client = new HttpClient())
            {
                client.DefaultRequestHeaders.Add("Cookie", "identity=" + identityBox.Text);
                string purchasesPageURL = @"https://bandcamp.com/" + usernameBox.Text + @"/purchases";
                string purchasesPage = await client.GetStringAsync(purchasesPageURL);

                List<string> artists = new List<string>();
                List<string> titles = new List<string>();

                var titleRegex = new Regex(@"&quot;item_title&quot;:&quot;.*?&quot;");
                var titleMatches = titleRegex.Matches(purchasesPage);
                foreach (Match match in titleMatches)
                {
                    string matchValue = match.Value;
                    string title = match.Value.Substring(29, matchValue.Length - 35);
                    titles.Add(title);
                }

                var artistRegex = new Regex(@"&quot;artist_name&quot;:&quot;.*?&quot;");
                var artistMatches = artistRegex.Matches(purchasesPage);
                foreach (Match match in artistMatches)
                {
                    string matchValue = match.Value;
                    string artist = match.Value.Substring(30, matchValue.Length - 36);
                    artists.Add(artist);
                }

                for (int i = 0; i < titles.Count; i++)
                {
                    Console.WriteLine(artists[i]);
                    Console.WriteLine(titles[i]);
                    Console.WriteLine("Purchased by " + usernameBox.Text);
                    Console.WriteLine($"ref_{i}_{usernameBox.Text}_{identityBox.Text}");
                }

                Application.Exit();
            }
        }
    }
}