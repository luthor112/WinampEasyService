namespace esrv_localsearch
{
    partial class Form1
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
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

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            label1 = new Label();
            directoryBox = new TextBox();
            selectButton = new Button();
            label2 = new Label();
            patternBox = new TextBox();
            recursiveBox = new CheckBox();
            searchButton = new Button();
            folderBrowserDialog1 = new FolderBrowserDialog();
            SuspendLayout();
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new Point(12, 9);
            label1.Name = "label1";
            label1.Size = new Size(58, 15);
            label1.TabIndex = 0;
            label1.Text = "Directory:";
            // 
            // directoryBox
            // 
            directoryBox.Location = new Point(76, 6);
            directoryBox.Name = "directoryBox";
            directoryBox.Size = new Size(354, 23);
            directoryBox.TabIndex = 1;
            // 
            // selectButton
            // 
            selectButton.Location = new Point(436, 6);
            selectButton.Name = "selectButton";
            selectButton.Size = new Size(75, 23);
            selectButton.TabIndex = 2;
            selectButton.Text = "Select...";
            selectButton.UseVisualStyleBackColor = true;
            selectButton.Click += selectButton_Click;
            // 
            // label2
            // 
            label2.AutoSize = true;
            label2.Location = new Point(12, 38);
            label2.Name = "label2";
            label2.Size = new Size(48, 15);
            label2.TabIndex = 3;
            label2.Text = "Pattern:";
            // 
            // patternBox
            // 
            patternBox.Location = new Point(76, 35);
            patternBox.Name = "patternBox";
            patternBox.Size = new Size(354, 23);
            patternBox.TabIndex = 4;
            // 
            // recursiveBox
            // 
            recursiveBox.AutoSize = true;
            recursiveBox.Checked = true;
            recursiveBox.CheckState = CheckState.Checked;
            recursiveBox.Location = new Point(436, 38);
            recursiveBox.Name = "recursiveBox";
            recursiveBox.Size = new Size(76, 19);
            recursiveBox.TabIndex = 5;
            recursiveBox.Text = "Recursive";
            recursiveBox.UseVisualStyleBackColor = true;
            // 
            // searchButton
            // 
            searchButton.Location = new Point(12, 64);
            searchButton.Name = "searchButton";
            searchButton.Size = new Size(499, 23);
            searchButton.TabIndex = 6;
            searchButton.Text = "Search";
            searchButton.UseVisualStyleBackColor = true;
            searchButton.Click += searchButton_Click;
            // 
            // Form1
            // 
            AcceptButton = searchButton;
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(521, 98);
            Controls.Add(searchButton);
            Controls.Add(recursiveBox);
            Controls.Add(patternBox);
            Controls.Add(label2);
            Controls.Add(selectButton);
            Controls.Add(directoryBox);
            Controls.Add(label1);
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "Form1";
            StartPosition = FormStartPosition.CenterScreen;
            Text = "Local File Search";
            Load += Form1_Load;
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private Label label1;
        private TextBox directoryBox;
        private Button selectButton;
        private Label label2;
        private TextBox patternBox;
        private CheckBox recursiveBox;
        private Button searchButton;
        private FolderBrowserDialog folderBrowserDialog1;
    }
}