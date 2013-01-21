namespace EditorWinForms
{
    partial class Console
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
            this.components = new System.ComponentModel.Container();
            this.consoleTextBox = new System.Windows.Forms.TextBox();
            this.commandTextBox = new System.Windows.Forms.TextBox();
            this.autocompleteContextMenuStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.SuspendLayout();
            // 
            // consoleTextBox
            // 
            this.consoleTextBox.BackColor = System.Drawing.Color.Tan;
            this.consoleTextBox.ForeColor = System.Drawing.Color.White;
            this.consoleTextBox.Location = new System.Drawing.Point(0, 1);
            this.consoleTextBox.Multiline = true;
            this.consoleTextBox.Name = "consoleTextBox";
            this.consoleTextBox.ReadOnly = true;
            this.consoleTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.consoleTextBox.Size = new System.Drawing.Size(284, 235);
            this.consoleTextBox.TabIndex = 0;
            // 
            // commandTextBox
            // 
            this.commandTextBox.Location = new System.Drawing.Point(1, 241);
            this.commandTextBox.Name = "commandTextBox";
            this.commandTextBox.Size = new System.Drawing.Size(282, 20);
            this.commandTextBox.TabIndex = 1;
            this.commandTextBox.TextChanged += new System.EventHandler(this.commandTextBox_TextChanged);
            // 
            // autocompleteContextMenuStrip
            // 
            this.autocompleteContextMenuStrip.Name = "autocompleteContextMenuStrip";
            this.autocompleteContextMenuStrip.ShowImageMargin = false;
            this.autocompleteContextMenuStrip.Size = new System.Drawing.Size(128, 26);
            this.autocompleteContextMenuStrip.ItemClicked += new System.Windows.Forms.ToolStripItemClickedEventHandler(this.autocompleteContextMenuStrip_ItemClicked);
            // 
            // Console
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 262);
            this.Controls.Add(this.commandTextBox);
            this.Controls.Add(this.consoleTextBox);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.Name = "Console";
            this.Text = "Console";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox consoleTextBox;
        private System.Windows.Forms.TextBox commandTextBox;
        private System.Windows.Forms.ContextMenuStrip autocompleteContextMenuStrip;

    }
}