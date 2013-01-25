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
            this.directoryTreeView = new System.Windows.Forms.TreeView();
            this.filesListView = new System.Windows.Forms.ListView();
            this.refreshBackgroundWorker = new System.ComponentModel.BackgroundWorker();
            this.fileListImageList = new System.Windows.Forms.ImageList(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.mainSplitContainer)).BeginInit();
            this.mainSplitContainer.Panel1.SuspendLayout();
            this.mainSplitContainer.Panel2.SuspendLayout();
            this.mainSplitContainer.SuspendLayout();
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
            this.mainSplitContainer.Panel1.Controls.Add(this.directoryTreeView);
            // 
            // mainSplitContainer.Panel2
            // 
            this.mainSplitContainer.Panel2.Controls.Add(this.filesListView);
            this.mainSplitContainer.Size = new System.Drawing.Size(784, 562);
            this.mainSplitContainer.SplitterDistance = 196;
            this.mainSplitContainer.TabIndex = 1;
            // 
            // directoryTreeView
            // 
            this.directoryTreeView.AllowDrop = true;
            this.directoryTreeView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.directoryTreeView.HideSelection = false;
            this.directoryTreeView.Location = new System.Drawing.Point(0, 0);
            this.directoryTreeView.Name = "directoryTreeView";
            this.directoryTreeView.PathSeparator = "/";
            this.directoryTreeView.Size = new System.Drawing.Size(196, 562);
            this.directoryTreeView.TabIndex = 1;
            this.directoryTreeView.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.directoryTreeView_AfterSelect);
            this.directoryTreeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.directoryTreeView_DragDrop);
            this.directoryTreeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.directoryTreeView_DragEnter);
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
            this.filesListView.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.filesListView_ItemDrag);
            this.filesListView.DoubleClick += new System.EventHandler(this.filesListView_DoubleClick);
            // 
            // refreshBackgroundWorker
            // 
            this.refreshBackgroundWorker.DoWork += new System.ComponentModel.DoWorkEventHandler(this.refreshBackgroundWorker_DoWork);
            this.refreshBackgroundWorker.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.refreshBackgroundWorker_RunWorkerCompleted);
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
            this.mainSplitContainer.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.mainSplitContainer)).EndInit();
            this.mainSplitContainer.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer mainSplitContainer;
        private System.Windows.Forms.TreeView directoryTreeView;
        private System.Windows.Forms.ListView filesListView;
        private System.ComponentModel.BackgroundWorker refreshBackgroundWorker;
        private System.Windows.Forms.ImageList fileListImageList;
    }
}