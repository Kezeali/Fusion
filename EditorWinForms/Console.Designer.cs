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
            this.consoleTextBox = new System.Windows.Forms.TextBox();
            this.commandTextBox = new System.Windows.Forms.TextBox();
            this.autocompleteMenu = new AutocompleteMenuNS.AutocompleteMenu();
            this.SuspendLayout();
            // 
            // consoleTextBox
            // 
            this.consoleTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.autocompleteMenu.SetAutocompleteMenu(this.consoleTextBox, null);
            this.consoleTextBox.BackColor = System.Drawing.Color.AliceBlue;
            this.consoleTextBox.CausesValidation = false;
            this.consoleTextBox.Font = new System.Drawing.Font("Consolas", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.consoleTextBox.ForeColor = System.Drawing.Color.DimGray;
            this.consoleTextBox.HideSelection = false;
            this.consoleTextBox.Location = new System.Drawing.Point(1, 1);
            this.consoleTextBox.Multiline = true;
            this.consoleTextBox.Name = "consoleTextBox";
            this.consoleTextBox.ReadOnly = true;
            this.consoleTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.consoleTextBox.Size = new System.Drawing.Size(483, 535);
            this.consoleTextBox.TabIndex = 1;
            // 
            // commandTextBox
            // 
            this.commandTextBox.AcceptsTab = true;
            this.commandTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.autocompleteMenu.SetAutocompleteMenu(this.commandTextBox, this.autocompleteMenu);
            this.commandTextBox.CausesValidation = false;
            this.commandTextBox.Font = new System.Drawing.Font("Consolas", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.commandTextBox.Location = new System.Drawing.Point(1, 541);
            this.commandTextBox.Name = "commandTextBox";
            this.commandTextBox.Size = new System.Drawing.Size(482, 23);
            this.commandTextBox.TabIndex = 0;
            this.commandTextBox.KeyDown += new System.Windows.Forms.KeyEventHandler(this.commandTextBox_KeyDown);
            this.commandTextBox.KeyUp += new System.Windows.Forms.KeyEventHandler(this.commandTextBox_KeyUp);
            // 
            // autocompleteMenu
            // 
            this.autocompleteMenu.AllowsTabKey = true;
            this.autocompleteMenu.AppearInterval = 100;
            this.autocompleteMenu.Font = new System.Drawing.Font("Consolas", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.autocompleteMenu.ImageList = null;
            this.autocompleteMenu.Items = new string[0];
            this.autocompleteMenu.MinFragmentLength = 1;
            this.autocompleteMenu.SearchPattern = "[\\w]";
            this.autocompleteMenu.TargetControlWrapper = null;
            // 
            // Console
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(484, 566);
            this.Controls.Add(this.commandTextBox);
            this.Controls.Add(this.consoleTextBox);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.Name = "Console";
            this.Text = "Console";
            this.Shown += new System.EventHandler(this.Console_Shown);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox consoleTextBox;
        private System.Windows.Forms.TextBox commandTextBox;
        private AutocompleteMenuNS.AutocompleteMenu autocompleteMenu;

    }
}