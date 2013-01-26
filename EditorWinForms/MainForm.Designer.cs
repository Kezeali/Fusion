namespace EditorWinForms
{
    partial class MainForm
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
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.menuStripMain = new System.Windows.Forms.MenuStrip();
            this.editorToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
            this.settingsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.quitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.loadToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.compileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.compileScriptsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.compileMapToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.resourcesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openResourceBrowserToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripSeparator();
            this.clearResourceDatabaseToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.loadResourceToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.websiteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.waitToConnectTimer = new System.Windows.Forms.Timer(this.components);
            this.backgroundWorker1 = new System.ComponentModel.BackgroundWorker();
            this.editorServerProcess = new System.Diagnostics.Process();
            this.refreshConsoleToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem4 = new System.Windows.Forms.ToolStripSeparator();
            this.testToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.printLotsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.printAllTheTimeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.testConsoleTimer = new System.Windows.Forms.Timer(this.components);
            this.startToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.stopToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.loremToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.menuStripMain.SuspendLayout();
            this.SuspendLayout();
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.DefaultExt = "exe";
            this.openFileDialog1.FileName = "openFileDialog1";
            this.openFileDialog1.Filter = "Executable files|*.exe";
            this.openFileDialog1.FileOk += new System.ComponentModel.CancelEventHandler(this.openFileDialog1_FileOk);
            // 
            // menuStripMain
            // 
            this.menuStripMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.editorToolStripMenuItem,
            this.fileToolStripMenuItem,
            this.compileToolStripMenuItem,
            this.resourcesToolStripMenuItem,
            this.helpToolStripMenuItem,
            this.testToolStripMenuItem});
            this.menuStripMain.Location = new System.Drawing.Point(0, 0);
            this.menuStripMain.Name = "menuStripMain";
            this.menuStripMain.Size = new System.Drawing.Size(428, 24);
            this.menuStripMain.TabIndex = 2;
            this.menuStripMain.Text = "menuStrip1";
            // 
            // editorToolStripMenuItem
            // 
            this.editorToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.aboutToolStripMenuItem,
            this.toolStripMenuItem2,
            this.refreshConsoleToolStripMenuItem,
            this.toolStripMenuItem4,
            this.settingsToolStripMenuItem,
            this.quitToolStripMenuItem});
            this.editorToolStripMenuItem.Name = "editorToolStripMenuItem";
            this.editorToolStripMenuItem.Size = new System.Drawing.Size(88, 20);
            this.editorToolStripMenuItem.Text = "Fusion Editor";
            // 
            // aboutToolStripMenuItem
            // 
            this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
            this.aboutToolStripMenuItem.Size = new System.Drawing.Size(159, 22);
            this.aboutToolStripMenuItem.Text = "About";
            this.aboutToolStripMenuItem.Click += new System.EventHandler(this.aboutToolStripMenuItem_Click);
            // 
            // toolStripMenuItem2
            // 
            this.toolStripMenuItem2.Name = "toolStripMenuItem2";
            this.toolStripMenuItem2.Size = new System.Drawing.Size(156, 6);
            // 
            // settingsToolStripMenuItem
            // 
            this.settingsToolStripMenuItem.Name = "settingsToolStripMenuItem";
            this.settingsToolStripMenuItem.Size = new System.Drawing.Size(159, 22);
            this.settingsToolStripMenuItem.Text = "Settings...";
            this.settingsToolStripMenuItem.Click += new System.EventHandler(this.settingsToolStripMenuItem_Click);
            // 
            // quitToolStripMenuItem
            // 
            this.quitToolStripMenuItem.Name = "quitToolStripMenuItem";
            this.quitToolStripMenuItem.Size = new System.Drawing.Size(159, 22);
            this.quitToolStripMenuItem.Text = "Quit";
            this.quitToolStripMenuItem.Click += new System.EventHandler(this.quitToolStripMenuItem_Click);
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.saveToolStripMenuItem,
            this.loadToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // saveToolStripMenuItem
            // 
            this.saveToolStripMenuItem.Name = "saveToolStripMenuItem";
            this.saveToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.saveToolStripMenuItem.Text = "Save Map";
            // 
            // loadToolStripMenuItem
            // 
            this.loadToolStripMenuItem.Name = "loadToolStripMenuItem";
            this.loadToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.loadToolStripMenuItem.Text = "Load Map";
            // 
            // compileToolStripMenuItem
            // 
            this.compileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.compileScriptsToolStripMenuItem,
            this.toolStripMenuItem1,
            this.compileMapToolStripMenuItem});
            this.compileToolStripMenuItem.Name = "compileToolStripMenuItem";
            this.compileToolStripMenuItem.Size = new System.Drawing.Size(64, 20);
            this.compileToolStripMenuItem.Text = "Compile";
            // 
            // compileScriptsToolStripMenuItem
            // 
            this.compileScriptsToolStripMenuItem.Name = "compileScriptsToolStripMenuItem";
            this.compileScriptsToolStripMenuItem.Size = new System.Drawing.Size(157, 22);
            this.compileScriptsToolStripMenuItem.Text = "Compile Scripts";
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(154, 6);
            // 
            // compileMapToolStripMenuItem
            // 
            this.compileMapToolStripMenuItem.Name = "compileMapToolStripMenuItem";
            this.compileMapToolStripMenuItem.Size = new System.Drawing.Size(157, 22);
            this.compileMapToolStripMenuItem.Text = "Build Map";
            // 
            // resourcesToolStripMenuItem
            // 
            this.resourcesToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openResourceBrowserToolStripMenuItem,
            this.toolStripMenuItem3,
            this.clearResourceDatabaseToolStripMenuItem,
            this.loadResourceToolStripMenuItem});
            this.resourcesToolStripMenuItem.Name = "resourcesToolStripMenuItem";
            this.resourcesToolStripMenuItem.Size = new System.Drawing.Size(72, 20);
            this.resourcesToolStripMenuItem.Text = "Resources";
            // 
            // openResourceBrowserToolStripMenuItem
            // 
            this.openResourceBrowserToolStripMenuItem.Name = "openResourceBrowserToolStripMenuItem";
            this.openResourceBrowserToolStripMenuItem.Size = new System.Drawing.Size(208, 22);
            this.openResourceBrowserToolStripMenuItem.Text = "Open Resource Browser...";
            this.openResourceBrowserToolStripMenuItem.Click += new System.EventHandler(this.openResourceBrowserToolStripMenuItem_Click);
            // 
            // toolStripMenuItem3
            // 
            this.toolStripMenuItem3.Name = "toolStripMenuItem3";
            this.toolStripMenuItem3.Size = new System.Drawing.Size(205, 6);
            // 
            // clearResourceDatabaseToolStripMenuItem
            // 
            this.clearResourceDatabaseToolStripMenuItem.Name = "clearResourceDatabaseToolStripMenuItem";
            this.clearResourceDatabaseToolStripMenuItem.Size = new System.Drawing.Size(208, 22);
            this.clearResourceDatabaseToolStripMenuItem.Text = "Clear Resource Database";
            // 
            // loadResourceToolStripMenuItem
            // 
            this.loadResourceToolStripMenuItem.Name = "loadResourceToolStripMenuItem";
            this.loadResourceToolStripMenuItem.Size = new System.Drawing.Size(208, 22);
            this.loadResourceToolStripMenuItem.Text = "Load Resource...";
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.Alignment = System.Windows.Forms.ToolStripItemAlignment.Right;
            this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.websiteToolStripMenuItem});
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
            this.helpToolStripMenuItem.Text = "Help";
            // 
            // websiteToolStripMenuItem
            // 
            this.websiteToolStripMenuItem.Name = "websiteToolStripMenuItem";
            this.websiteToolStripMenuItem.Size = new System.Drawing.Size(116, 22);
            this.websiteToolStripMenuItem.Text = "Website";
            // 
            // waitToConnectTimer
            // 
            this.waitToConnectTimer.Interval = 1000;
            this.waitToConnectTimer.Tick += new System.EventHandler(this.waitToConnectTimer_Tick);
            // 
            // editorServerProcess
            // 
            this.editorServerProcess.StartInfo.Domain = "";
            this.editorServerProcess.StartInfo.ErrorDialog = true;
            this.editorServerProcess.StartInfo.LoadUserProfile = false;
            this.editorServerProcess.StartInfo.Password = null;
            this.editorServerProcess.StartInfo.RedirectStandardError = true;
            this.editorServerProcess.StartInfo.RedirectStandardOutput = true;
            this.editorServerProcess.StartInfo.StandardErrorEncoding = null;
            this.editorServerProcess.StartInfo.StandardOutputEncoding = null;
            this.editorServerProcess.StartInfo.UserName = "";
            this.editorServerProcess.StartInfo.UseShellExecute = false;
            this.editorServerProcess.SynchronizingObject = this;
            this.editorServerProcess.OutputDataReceived += new System.Diagnostics.DataReceivedEventHandler(this.editorServerProcess_OutputDataReceived);
            this.editorServerProcess.ErrorDataReceived += new System.Diagnostics.DataReceivedEventHandler(this.editorServerProcess_OutputDataReceived);
            // 
            // refreshConsoleToolStripMenuItem
            // 
            this.refreshConsoleToolStripMenuItem.Name = "refreshConsoleToolStripMenuItem";
            this.refreshConsoleToolStripMenuItem.Size = new System.Drawing.Size(159, 22);
            this.refreshConsoleToolStripMenuItem.Text = "Refresh Console";
            this.refreshConsoleToolStripMenuItem.Click += new System.EventHandler(this.refreshConsoleToolStripMenuItem_Click);
            // 
            // toolStripMenuItem4
            // 
            this.toolStripMenuItem4.Name = "toolStripMenuItem4";
            this.toolStripMenuItem4.Size = new System.Drawing.Size(156, 6);
            // 
            // testToolStripMenuItem
            // 
            this.testToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.printLotsToolStripMenuItem,
            this.printAllTheTimeToolStripMenuItem,
            this.loremToolStripMenuItem});
            this.testToolStripMenuItem.Name = "testToolStripMenuItem";
            this.testToolStripMenuItem.Size = new System.Drawing.Size(41, 20);
            this.testToolStripMenuItem.Text = "Test";
            // 
            // printLotsToolStripMenuItem
            // 
            this.printLotsToolStripMenuItem.Name = "printLotsToolStripMenuItem";
            this.printLotsToolStripMenuItem.Size = new System.Drawing.Size(169, 22);
            this.printLotsToolStripMenuItem.Text = "Print Lots";
            this.printLotsToolStripMenuItem.Click += new System.EventHandler(this.printLotsToolStripMenuItem_Click);
            // 
            // printAllTheTimeToolStripMenuItem
            // 
            this.printAllTheTimeToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.startToolStripMenuItem,
            this.stopToolStripMenuItem});
            this.printAllTheTimeToolStripMenuItem.Name = "printAllTheTimeToolStripMenuItem";
            this.printAllTheTimeToolStripMenuItem.Size = new System.Drawing.Size(169, 22);
            this.printAllTheTimeToolStripMenuItem.Text = "Print All The Time";
            // 
            // testConsoleTimer
            // 
            this.testConsoleTimer.Interval = 500;
            this.testConsoleTimer.Tick += new System.EventHandler(this.testConsoleTimer_Tick);
            // 
            // startToolStripMenuItem
            // 
            this.startToolStripMenuItem.Name = "startToolStripMenuItem";
            this.startToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.startToolStripMenuItem.Text = "Start";
            this.startToolStripMenuItem.Click += new System.EventHandler(this.startToolStripMenuItem_Click);
            // 
            // stopToolStripMenuItem
            // 
            this.stopToolStripMenuItem.Name = "stopToolStripMenuItem";
            this.stopToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.stopToolStripMenuItem.Text = "Stop";
            this.stopToolStripMenuItem.Click += new System.EventHandler(this.stopToolStripMenuItem_Click);
            // 
            // loremToolStripMenuItem
            // 
            this.loremToolStripMenuItem.Name = "loremToolStripMenuItem";
            this.loremToolStripMenuItem.Size = new System.Drawing.Size(169, 22);
            this.loremToolStripMenuItem.Text = "Lorem";
            this.loremToolStripMenuItem.Click += new System.EventHandler(this.loremToolStripMenuItem_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(428, 26);
            this.Controls.Add(this.menuStripMain);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Location = new System.Drawing.Point(0, -800);
            this.MainMenuStrip = this.menuStripMain;
            this.MinimizeBox = false;
            this.Name = "MainForm";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "Fusion Editor";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.menuStripMain.ResumeLayout(false);
            this.menuStripMain.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.MenuStrip menuStripMain;
        private System.Windows.Forms.ToolStripMenuItem editorToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem settingsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem quitToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem loadToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem compileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem compileScriptsToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem compileMapToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem resourcesToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem clearResourceDatabaseToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem loadResourceToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
        private System.Windows.Forms.ToolStripMenuItem websiteToolStripMenuItem;
        private System.Windows.Forms.Timer waitToConnectTimer;
        private System.ComponentModel.BackgroundWorker backgroundWorker1;
        private System.Diagnostics.Process editorServerProcess;
        private System.Windows.Forms.ToolStripMenuItem openResourceBrowserToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem3;
        private System.Windows.Forms.ToolStripMenuItem refreshConsoleToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem4;
        private System.Windows.Forms.ToolStripMenuItem testToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem printLotsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem printAllTheTimeToolStripMenuItem;
        private System.Windows.Forms.Timer testConsoleTimer;
        private System.Windows.Forms.ToolStripMenuItem startToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem stopToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem loremToolStripMenuItem;
    }
}

