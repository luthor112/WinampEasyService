namespace msrv_soundcloud_plus
{
    partial class SoundCloudControl
    {
        /// <summary> 
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary> 
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            searchBox = new TextBox();
            searchMediaBtn = new Button();
            searchListsBtn = new Button();
            resultListView = new ListView();
            authorHeader = new ColumnHeader();
            titleHeader = new ColumnHeader();
            typeHeader = new ColumnHeader();
            urlHeader = new ColumnHeader();
            SuspendLayout();
            // 
            // searchBox
            // 
            searchBox.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            searchBox.Location = new Point(13, 12);
            searchBox.Name = "searchBox";
            searchBox.PlaceholderText = "Search...";
            searchBox.Size = new Size(368, 23);
            searchBox.TabIndex = 0;
            // 
            // searchMediaBtn
            // 
            searchMediaBtn.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            searchMediaBtn.Location = new Point(387, 11);
            searchMediaBtn.Name = "searchMediaBtn";
            searchMediaBtn.Size = new Size(104, 23);
            searchMediaBtn.TabIndex = 1;
            searchMediaBtn.Text = "Search Media";
            searchMediaBtn.UseVisualStyleBackColor = true;
            // 
            // searchListsBtn
            // 
            searchListsBtn.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            searchListsBtn.Location = new Point(497, 12);
            searchListsBtn.Name = "searchListsBtn";
            searchListsBtn.Size = new Size(104, 23);
            searchListsBtn.TabIndex = 2;
            searchListsBtn.Text = "Search Lists";
            searchListsBtn.UseVisualStyleBackColor = true;
            // 
            // resultListView
            // 
            resultListView.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            resultListView.Columns.AddRange(new ColumnHeader[] { authorHeader, titleHeader, typeHeader, urlHeader });
            resultListView.FullRowSelect = true;
            resultListView.HeaderStyle = ColumnHeaderStyle.Nonclickable;
            resultListView.Location = new Point(13, 41);
            resultListView.MultiSelect = false;
            resultListView.Name = "resultListView";
            resultListView.Size = new Size(588, 352);
            resultListView.TabIndex = 3;
            resultListView.UseCompatibleStateImageBehavior = false;
            resultListView.View = View.Details;
            // 
            // authorHeader
            // 
            authorHeader.Text = "Author";
            authorHeader.Width = 150;
            // 
            // titleHeader
            // 
            titleHeader.Text = "Title";
            titleHeader.Width = 150;
            // 
            // typeHeader
            // 
            typeHeader.Text = "Type";
            // 
            // urlHeader
            // 
            urlHeader.Text = "Page URL";
            urlHeader.Width = 150;
            // 
            // SoundCloudControl
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            Controls.Add(resultListView);
            Controls.Add(searchListsBtn);
            Controls.Add(searchMediaBtn);
            Controls.Add(searchBox);
            Name = "SoundCloudControl";
            Size = new Size(612, 405);
            Load += SoundCloudControl_Load;
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private TextBox searchBox;
        private Button searchMediaBtn;
        private Button searchListsBtn;
        private ListView resultListView;
        private ColumnHeader authorHeader;
        private ColumnHeader titleHeader;
        private ColumnHeader typeHeader;
        private ColumnHeader urlHeader;
    }
}
