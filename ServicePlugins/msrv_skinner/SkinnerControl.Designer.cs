namespace msrv_skinner
{
    partial class SkinnerControl
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
            processComboBox = new ComboBox();
            spawnButton = new Button();
            SuspendLayout();
            // 
            // processComboBox
            // 
            processComboBox.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            processComboBox.DropDownStyle = ComboBoxStyle.DropDownList;
            processComboBox.FormattingEnabled = true;
            processComboBox.Location = new Point(3, 3);
            processComboBox.Name = "processComboBox";
            processComboBox.Size = new Size(475, 23);
            processComboBox.TabIndex = 0;
            // 
            // spawnButton
            // 
            spawnButton.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            spawnButton.Location = new Point(3, 32);
            spawnButton.Name = "spawnButton";
            spawnButton.Size = new Size(475, 23);
            spawnButton.TabIndex = 1;
            spawnButton.Text = "Spawn overlay";
            spawnButton.UseVisualStyleBackColor = true;
            spawnButton.Click += spawnButton_Click;
            // 
            // SkinnerControl
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            Controls.Add(spawnButton);
            Controls.Add(processComboBox);
            Name = "SkinnerControl";
            Size = new Size(481, 150);
            Load += SkinnerControl_Load;
            ResumeLayout(false);
        }

        #endregion

        private ComboBox processComboBox;
        private Button spawnButton;
    }
}
