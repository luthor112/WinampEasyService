namespace esrv_bandcamp
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
            usernameBox = new TextBox();
            label2 = new Label();
            identityBox = new TextBox();
            saveCheckBox = new CheckBox();
            button1 = new Button();
            SuspendLayout();
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new Point(12, 9);
            label1.Name = "label1";
            label1.Size = new Size(63, 15);
            label1.TabIndex = 0;
            label1.Text = "Username:";
            // 
            // usernameBox
            // 
            usernameBox.Location = new Point(81, 6);
            usernameBox.Name = "usernameBox";
            usernameBox.Size = new Size(613, 23);
            usernameBox.TabIndex = 1;
            // 
            // label2
            // 
            label2.AutoSize = true;
            label2.Location = new Point(12, 37);
            label2.Name = "label2";
            label2.Size = new Size(50, 15);
            label2.TabIndex = 2;
            label2.Text = "Identity:";
            // 
            // identityBox
            // 
            identityBox.Location = new Point(81, 34);
            identityBox.Name = "identityBox";
            identityBox.Size = new Size(613, 23);
            identityBox.TabIndex = 3;
            // 
            // saveCheckBox
            // 
            saveCheckBox.AutoSize = true;
            saveCheckBox.Location = new Point(12, 66);
            saveCheckBox.Name = "saveCheckBox";
            saveCheckBox.Size = new Size(50, 19);
            saveCheckBox.TabIndex = 4;
            saveCheckBox.Text = "Save";
            saveCheckBox.UseVisualStyleBackColor = true;
            // 
            // button1
            // 
            button1.Location = new Point(81, 63);
            button1.Name = "button1";
            button1.Size = new Size(613, 23);
            button1.TabIndex = 5;
            button1.Text = "Load Purchases";
            button1.UseVisualStyleBackColor = true;
            button1.Click += button1_Click;
            // 
            // Form1
            // 
            AcceptButton = button1;
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(706, 97);
            Controls.Add(button1);
            Controls.Add(saveCheckBox);
            Controls.Add(identityBox);
            Controls.Add(label2);
            Controls.Add(usernameBox);
            Controls.Add(label1);
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "Form1";
            StartPosition = FormStartPosition.CenterScreen;
            Text = "Bandcamp Authentication";
            Load += Form1_Load;
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private Label label1;
        private TextBox usernameBox;
        private Label label2;
        private TextBox identityBox;
        private CheckBox saveCheckBox;
        private Button button1;
    }
}