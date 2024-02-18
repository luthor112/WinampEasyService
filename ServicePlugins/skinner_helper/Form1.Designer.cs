namespace skinner_helper
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
            leftPicture = new PictureBox();
            midPicture = new PictureBox();
            rightPicture = new PictureBox();
            ((System.ComponentModel.ISupportInitialize)leftPicture).BeginInit();
            ((System.ComponentModel.ISupportInitialize)midPicture).BeginInit();
            ((System.ComponentModel.ISupportInitialize)rightPicture).BeginInit();
            SuspendLayout();
            // 
            // leftPicture
            // 
            leftPicture.Location = new Point(0, 0);
            leftPicture.Name = "leftPicture";
            leftPicture.Size = new Size(25, 20);
            leftPicture.TabIndex = 0;
            leftPicture.TabStop = false;
            // 
            // midPicture
            // 
            midPicture.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            midPicture.Location = new Point(25, 0);
            midPicture.Name = "midPicture";
            midPicture.Size = new Size(590, 20);
            midPicture.TabIndex = 1;
            midPicture.TabStop = false;
            // 
            // rightPicture
            // 
            rightPicture.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            rightPicture.Location = new Point(615, 0);
            rightPicture.Name = "rightPicture";
            rightPicture.Size = new Size(25, 20);
            rightPicture.TabIndex = 2;
            rightPicture.TabStop = false;
            // 
            // Form1
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(640, 240);
            Controls.Add(rightPicture);
            Controls.Add(midPicture);
            Controls.Add(leftPicture);
            FormBorderStyle = FormBorderStyle.None;
            Name = "Form1";
            ShowInTaskbar = false;
            Text = "Form1";
            TransparencyKey = SystemColors.Control;
            FormClosing += Form1_FormClosing;
            Load += Form1_Load;
            ((System.ComponentModel.ISupportInitialize)leftPicture).EndInit();
            ((System.ComponentModel.ISupportInitialize)midPicture).EndInit();
            ((System.ComponentModel.ISupportInitialize)rightPicture).EndInit();
            ResumeLayout(false);
        }

        #endregion

        private PictureBox leftPicture;
        private PictureBox midPicture;
        private PictureBox rightPicture;
    }
}
