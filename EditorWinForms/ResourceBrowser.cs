using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Windows.Forms;
using FusionEngine.Interprocess;

namespace EditorWinForms
{
    public partial class ResourceBrowser : Form
    {
        public ResourceBrowser()
        {
            InitializeComponent();
        }

        public Editor.Client Client { get; set; }

        class DirectoryTreeProgress
        {
            public string path;
            public TreeNode node;
            //public TreeNode[] children;
            public List<ResourceFile> resources;
            public int i;

            public DirectoryTreeProgress(string title, string path, List<ResourceFile> resources, int i = 0)
            {
                this.path = path;
                this.resources = resources;
                this.i = i;

                this.node = new TreeNode(title);
                this.node.Name = title;
                this.node.Tag = path;
            }
        }

        public void RefreshBrowser()
        {
            if (Client != null)
            {
                string rootPath = "/";

                var rootProgress = new DirectoryTreeProgress("/", rootPath, Client.GetResources(rootPath));

                Stack<DirectoryTreeProgress> stack = new Stack<DirectoryTreeProgress>();
                stack.Push(rootProgress);

                while (stack.Count > 0)
                {
                    var progress = stack.Pop();

                    for (; progress.i < progress.resources.Count; ++progress.i)
                    {
                        if (progress.resources[progress.i].Directory)
                        {
                            string path = progress.resources[progress.i].Filename;

                            string title;
                            if (path != "/")
                            {
                                var end = path.LastIndexOfAny(new char[] { Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar });
                                if (end != -1 && end + 1 < path.Length)
                                    title = path.Substring(end + 1);
                                else
                                    title = path;
                            }
                            else
                                title = path;

                            var childProgress = new DirectoryTreeProgress(title, path, Client.GetResources(path));
                            progress.node.Nodes.Add(childProgress.node);

                            ++progress.i;
                            stack.Push(progress);

                            progress = childProgress;
                        }
                    }
                }

                directoryTreeView.Invoke(ClearDirectoryTree);
                directoryTreeView.Invoke(AddToDirectoryTree, rootProgress.node);
                directoryTreeView.Invoke(RedrawDirectoryTree);
            }
        }

        private delegate void ClearDirectoryTreeFn();
        private delegate void AddToDirectoryTreeFn(TreeNode node);
        private delegate void RedrawDirectoryTreeFn();
        private delegate void BeginUpdateDirectoryTreeFn();
        private delegate void EndUpdateDirectoryTreeFn();
        ClearDirectoryTreeFn ClearDirectoryTree;
        AddToDirectoryTreeFn AddToDirectoryTree;
        RedrawDirectoryTreeFn RedrawDirectoryTree;
        BeginUpdateDirectoryTreeFn BeginUpdateDirectoryTree;
        EndUpdateDirectoryTreeFn EndUpdateDirectoryTree;

        private void ResourceBrowser_Shown(object sender, EventArgs e)
        {
            ClearDirectoryTree = () => directoryTreeView.Nodes.Clear();
            AddToDirectoryTree = (node) => directoryTreeView.Nodes.Add(node);
            RedrawDirectoryTree = () => directoryTreeView.Update();
            BeginUpdateDirectoryTree = () => directoryTreeView.BeginUpdate();
            EndUpdateDirectoryTree = () => { directoryTreeView.EndUpdate(); directoryTreeView.Update(); };

            refreshBackgroundWorker.RunWorkerAsync();
        }

        private void refreshBackgroundWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            try
            {
                RefreshBrowser();
            }
            catch (Exception)
            {
            }
        }

        private void refreshBackgroundWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (directoryTreeView.Nodes.Count > 0)
                directoryTreeView.Nodes[0].Expand();
        }

        private void NavigateToPath(string path)
        {
            var resources = Client.GetResources(path);

            filesListView.BeginUpdate();

            filesListView.Clear();

            foreach (var resource in resources)
            {
                var item = new ListViewItem(Path.GetFileName(resource.Filename), resource.Directory ? "Folder" : "File");
                item.Tag = resource.Filename;
                filesListView.Items.Add(item);
            }

            filesListView.EndUpdate();

            // Make sure the navigated-to node is expanded
            TreeNode node = directoryTreeView.Nodes[0];
            var currentCollection = node.Nodes;

            var pathElements = path.Split(new char[] { Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar }, StringSplitOptions.RemoveEmptyEntries);
            foreach (var element in pathElements)
            {
                node = currentCollection[element];
                if (node != null)
                    currentCollection = node.Nodes;
                else
                    break;
            }

            if (node != null && directoryTreeView.SelectedNode != node)
            {
                directoryTreeView.SelectedNode = node;
                node.Expand();
            }
        }

        private void directoryTreeView_AfterSelect(object sender, TreeViewEventArgs e)
        {
            // Substring to skip the root node name (which is bogus)
            NavigateToPath(e.Node.FullPath.Substring(1));
        }

        private void filesListView_DoubleClick(object sender, EventArgs e)
        {
            if (filesListView.SelectedItems.Count > 0)
            {
                var path =  (string)filesListView.SelectedItems[0].Tag;
                NavigateToPath(path);
            }
        }

        private void filesListView_ItemDrag(object sender, ItemDragEventArgs e)
        {
            try
            {
                var item = (ListViewItem)e.Item;
                filesListView.DoDragDrop(item.Tag, DragDropEffects.Move | DragDropEffects.Copy | DragDropEffects.Scroll);
            }
            catch
            {
            }
        }

        private void directoryTreeView_DragEnter(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.Text))
            {
                if (((e.KeyState & 4) == 4) && ((e.AllowedEffect & DragDropEffects.Move) == DragDropEffects.Move))
                    e.Effect = DragDropEffects.Move;
                else
                    e.Effect = DragDropEffects.Copy;
            }
            else if (e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                e.Effect = DragDropEffects.Copy;
            }
            else
                e.Effect = DragDropEffects.None;
        }

        private void directoryTreeView_DragDrop(object sender, DragEventArgs e)
        {
            var node = directoryTreeView.GetNodeAt(e.X, e.Y);
            if (node != null)
            {
                if (e.Data.GetDataPresent(DataFormats.Text))
                {
                    try
                    {
                        var targetPath = node.FullPath.Substring(1);
                        var sourcePath = (string)e.Data.GetData(DataFormats.Text);
                        Client.MoveResource(sourcePath, targetPath);
                    }
                    catch
                    { }
                }
            }
        }
        
    }
}
