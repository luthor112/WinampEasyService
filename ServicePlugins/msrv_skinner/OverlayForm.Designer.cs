namespace msrv_skinner
{
    partial class OverlayForm
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

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            button1 = new Button();
            leftPicture = new PictureBox();
            pictureBox2 = new PictureBox();
            rightPicture = new PictureBox();
            ((System.ComponentModel.ISupportInitialize)leftPicture).BeginInit();
            ((System.ComponentModel.ISupportInitialize)pictureBox2).BeginInit();
            ((System.ComponentModel.ISupportInitialize)rightPicture).BeginInit();
            SuspendLayout();
            // 
            // button1
            // 
            button1.Location = new Point(58, 12);
            button1.Name = "button1";
            button1.Size = new Size(243, 47);
            button1.TabIndex = 0;
            button1.Text = "BUTTON!!!";
            button1.UseVisualStyleBackColor = true;
            // 
            // leftPicture
            // 
            leftPicture.Location = new Point(0, 0);
            leftPicture.Name = "leftPicture";
            leftPicture.Size = new Size(25, 20);
            leftPicture.TabIndex = 1;
            leftPicture.TabStop = false;
            // 
            // pictureBox2
            // 
            pictureBox2.Location = new Point(25, 0);
            pictureBox2.Name = "pictureBox2";
            pictureBox2.Size = new Size(590, 20);
            pictureBox2.TabIndex = 2;
            pictureBox2.TabStop = false;
            // 
            // rightPicture
            // 
            rightPicture.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            rightPicture.Location = new Point(615, 0);
            rightPicture.Name = "rightPicture";
            rightPicture.Size = new Size(25, 20);
            rightPicture.TabIndex = 3;
            rightPicture.TabStop = false;
            // 
            // OverlayForm
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(640, 240);
            Controls.Add(rightPicture);
            Controls.Add(pictureBox2);
            Controls.Add(leftPicture);
            Controls.Add(button1);
            FormBorderStyle = FormBorderStyle.None;
            Name = "OverlayForm";
            Text = "OverlayForm";
            TransparencyKey = SystemColors.Control;
            FormClosing += Form1_FormClosing;
            Load += OverlayForm_Load;
            ((System.ComponentModel.ISupportInitialize)leftPicture).EndInit();
            ((System.ComponentModel.ISupportInitialize)pictureBox2).EndInit();
            ((System.ComponentModel.ISupportInitialize)rightPicture).EndInit();
            ResumeLayout(false);
        }

        #endregion

        private Button button1;
        private PictureBox leftPicture;
        private PictureBox pictureBox2;
        private PictureBox rightPicture;
    }
}