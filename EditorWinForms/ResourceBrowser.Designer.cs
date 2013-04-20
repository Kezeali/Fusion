namespace EditorWinForms
{
    partial class ResourceBrowser
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ResourceBrowser));
            this.mainSplitContainer = new System.Windows.Forms.SplitContainer();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.directoryTreeView = new System.Windows.Forms.TreeView();
            this.directoryImageList = new System.Windows.Forms.ImageList(this.components);
            this.filesListView = new System.Windows.Forms.ListView();
            this.fileListImageList = new System.Windows.Forms.ImageList(this.components);
            this.refreshBackgroundWorker = new System.ComponentModel.BackgroundWorker();
            this.appFileSystemWatcher = new System.IO.FileSystemWatcher();
            this.writeDirFileSystemWatcher = new System.IO.FileSystemWatcher();
            ((System.ComponentModel.ISupportInitialize)(this.mainSplitContainer)).BeginInit();
            this.mainSplitContainer.Panel1.SuspendLayout();
            this.mainSplitContainer.Panel2.SuspendLayout();
            this.mainSplitContainer.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.appFileSystemWatcher)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.writeDirFileSystemWatcher)).BeginInit();
            this.SuspendLayout();
            // 
            // mainSplitContainer
            // 
            this.mainSplitContainer.BackColor = System.Drawing.SystemColors.ControlDark;
            this.mainSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mainSplitContainer.Location = new System.Drawing.Point(0, 0);
            this.mainSplitContainer.Name = "mainSplitContainer";
            // 
            // mainSplitContainer.Panel1
            // 
            this.mainSplitContainer.Panel1.Controls.Add(this.pictureBox1);
            this.mainSplitContainer.Panel1.Controls.Add(this.directoryTreeView);
            // 
            // mainSplitContainer.Panel2
            // 
            this.mainSplitContainer.Panel2.Controls.Add(this.filesListView);
            this.mainSplitContainer.Size = new System.Drawing.Size(784, 562);
            this.mainSplitContainer.SplitterDistance = 196;
            this.mainSplitContainer.TabIndex = 1;
            // 
            // pictureBox1
            // 
            this.pictureBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.pictureBox1.BackColor = System.Drawing.SystemColors.Window;
            this.pictureBox1.Image = global::EditorWinForms.Properties.Resources.LinkSpinAttackDemoMoving;
            this.pictureBox1.Location = new System.Drawing.Point(143, 3);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(50, 54);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.pictureBox1.TabIndex = 2;
            this.pictureBox1.TabStop = false;
            this.pictureBox1.Visible = false;
            // 
            // directoryTreeView
            // 
            this.directoryTreeView.AllowDrop = true;
            this.directoryTreeView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.directoryTreeView.HideSelection = false;
            this.directoryTreeView.HotTracking = true;
            this.directoryTreeView.ImageIndex = 0;
            this.directoryTreeView.ImageList = this.directoryImageList;
            this.directoryTreeView.Indent = 16;
            this.directoryTreeView.Location = new System.Drawing.Point(0, 0);
            this.directoryTreeView.Name = "directoryTreeView";
            this.directoryTreeView.PathSeparator = "/";
            this.directoryTreeView.SelectedImageIndex = 1;
            this.directoryTreeView.Size = new System.Drawing.Size(196, 562);
            this.directoryTreeView.TabIndex = 1;
            this.directoryTreeView.BeforeExpand += new System.Windows.Forms.TreeViewCancelEventHandler(this.directoryTreeView_BeforeExpand);
            this.directoryTreeView.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.directoryTreeView_AfterSelect);
            this.directoryTreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.directoryTreeView_DragDrop);
            this.directoryTreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.directoryTreeView_DragEnter);
            this.directoryTreeView.DragOver += new System.Windows.Forms.DragEventHandler(this.directoryTreeView_DragOver);
            this.directoryTreeView.DragLeave += new System.EventHandler(this.directoryTreeView_DragLeave);
            // 
            // directoryImageList
            // 
            this.directoryImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("directoryImageList.ImageStream")));
            this.directoryImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.directoryImageList.Images.SetKeyName(0, "Folder");
            this.directoryImageList.Images.SetKeyName(1, "FolderOpen");
            this.directoryImageList.Images.SetKeyName(2, "File");
            this.directoryImageList.Images.SetKeyName(3, "Map");
            // 
            // filesListView
            // 
            this.filesListView.AllowDrop = true;
            this.filesListView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.filesListView.HideSelection = false;
            this.filesListView.LabelEdit = true;
            this.filesListView.LargeImageList = this.fileListImageList;
            this.filesListView.Location = new System.Drawing.Point(0, 0);
            this.filesListView.MultiSelect = false;
            this.filesListView.Name = "filesListView";
            this.filesListView.ShowItemToolTips = true;
            this.filesListView.Size = new System.Drawing.Size(584, 562);
            this.filesListView.TabIndex = 0;
            this.filesListView.UseCompatibleStateImageBehavior = false;
            this.filesListView.View = System.Windows.Forms.View.Tile;
            this.filesListView.AfterLabelEdit += new System.Windows.Forms.LabelEditEventHandler(this.filesListView_AfterLabelEdit);
            this.filesListView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.filesListView_ItemDrag);
            this.filesListView.DoubleClick += new System.EventHandler(this.filesListView_DoubleClick);
            this.filesListView.KeyDown += new System.Windows.Forms.KeyEventHandler(this.filesListView_KeyDown);
            this.filesListView.MouseClick += new System.Windows.Forms.MouseEventHandler(this.filesListView_MouseClick);
            // 
            // fileListImageList
            // 
            this.fileListImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("fileListImageList.ImageStream")));
            this.fileListImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.fileListImageList.Images.SetKeyName(0, "Folder");
            this.fileListImageList.Images.SetKeyName(1, "FolderOpen");
            this.fileListImageList.Images.SetKeyName(2, "File");
            this.fileListImageList.Images.SetKeyName(3, "Map");
            // 
            // refreshBackgroundWorker
            // 
            this.refreshBackgroundWorker.DoWork += new System.ComponentModel.DoWorkEventHandler(this.refreshBackgroundWorker_DoWork);
            this.refreshBackgroundWorker.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.refreshBackgroundWorker_RunWorkerCompleted);
            // 
            // appFileSystemWatcher
            // 
            this.appFileSystemWatcher.EnableRaisingEvents = true;
            this.appFileSystemWatcher.IncludeSubdirectories = true;
            this.appFileSystemWatcher.NotifyFilter = ((System.IO.NotifyFilters)((((System.IO.NotifyFilters.FileName | System.IO.NotifyFilters.DirectoryName) 
            | System.IO.NotifyFilters.Size) 
            | System.IO.NotifyFilters.LastWrite)));
            this.appFileSystemWatcher.SynchronizingObject = this;
            this.appFileSystemWatcher.Changed += new System.IO.FileSystemEventHandler(this.fileSystemWatcher_ChangedCreatedDeleted);
            this.appFileSystemWatcher.Created += new System.IO.FileSystemEventHandler(this.fileSystemWatcher_ChangedCreatedDeleted);
            this.appFileSystemWatcher.Deleted += new System.IO.FileSystemEventHandler(this.fileSystemWatcher_ChangedCreatedDeleted);
            this.appFileSystemWatcher.Renamed += new System.IO.RenamedEventHandler(this.fileSystemWatcher_Renamed);
            // 
            // writeDirFileSystemWatcher
            // 
            this.writeDirFileSystemWatcher.EnableRaisingEvents = true;
            this.writeDirFileSystemWatcher.IncludeSubdirectories = true;
            this.writeDirFileSystemWatcher.NotifyFilter = ((System.IO.NotifyFilters)((((System.IO.NotifyFilters.FileName | System.IO.NotifyFilters.DirectoryName) 
            | System.IO.NotifyFilters.Size) 
            | System.IO.NotifyFilters.LastWrite)));
            this.writeDirFileSystemWatcher.SynchronizingObject = this;
            this.writeDirFileSystemWatcher.Changed += new System.IO.FileSystemEventHandler(this.fileSystemWatcher_ChangedCreatedDeleted);
            this.writeDirFileSystemWatcher.Created += new System.IO.FileSystemEventHandler(this.fileSystemWatcher_ChangedCreatedDeleted);
            this.writeDirFileSystemWatcher.Deleted += new System.IO.FileSystemEventHandler(this.fileSystemWatcher_ChangedCreatedDeleted);
            this.writeDirFileSystemWatcher.Renamed += new System.IO.RenamedEventHandler(this.fileSystemWatcher_Renamed);
            // 
            // ResourceBrowser
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(784, 562);
            this.Controls.Add(this.mainSplitContainer);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.Name = "ResourceBrowser";
            this.Text = "ResourceBrowser";
            this.Shown += new System.EventHandler(this.ResourceBrowser_Shown);
            this.mainSplitContainer.Panel1.ResumeLayout(false);
            this.mainSplitContainer.Panel1.PerformLayout();
            this.mainSplitContainer.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.mainSplitContainer)).EndInit();
            this.mainSplitContainer.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.appFileSystemWatcher)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.writeDirFileSystemWatcher)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer mainSplitContainer;
        private System.Windows.Forms.TreeView directoryTreeView;
        private System.Windows.Forms.ListView filesListView;
        private System.ComponentModel.BackgroundWorker refreshBackgroundWorker;
        private System.Windows.Forms.ImageList fileListImageList;
        private System.IO.FileSystemWatcher appFileSystemWatcher;
        private System.IO.FileSystemWatcher writeDirFileSystemWatcher;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.ImageList directoryImageList;
    }
}