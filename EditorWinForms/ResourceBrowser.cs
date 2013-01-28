﻿using System;
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
using System.Threading;

namespace EditorWinForms
{
    public partial class ResourceBrowser : Form
    {
        public ResourceBrowser()
        {
            InitializeComponent();
        }

        public Editor.Client Client { get; set; }

        class HoveredNode
        {
            public TreeNode node;
            public bool wasExpanded = false;
            public DateTime startHoverTime = DateTime.Now;
        }

        HoveredNode hoveredNode;

        private void SetHoveredNode(TreeNode node)
        {
            if (hoveredNode == null || node != hoveredNode.node)
            {
                ClearHoveredNode();

                if (node != null)
                {
                    try
                    {
                        hoveredNode = new HoveredNode();

                        hoveredNode.node = node;

                        hoveredNode.wasExpanded = hoveredNode.node.IsExpanded;
                        hoveredNode.node.BackColor = Color.SkyBlue;
                    }
                    catch
                    {
                        hoveredNode = null;
                    }
                }
            }
            else
            {
                if ((DateTime.Now - hoveredNode.startHoverTime).TotalSeconds >= 1.0)
                {
                    hoveredNode.node.Expand();
                }
            }
        }

        bool IsParent(TreeNode expectedParent, TreeNode child)
        {
            var current = child;
            while (current != null)
            {
                if (current == expectedParent)
                    return true;
                current = current.Parent;
            }
            return false;
        }

        private void ClearHoveredNode()
        {
            try
            {
                if (hoveredNode != null)
                {
                    //if (!hoveredNode.wasExpanded)
                    //{
                    //    var node = hoveredNode.node;
                    //    new Thread(() =>
                    //    {
                    //        Thread.Sleep(1000);
                    //        directoryTreeView.Invoke((Action)(() =>
                    //        {
                    //            if (hoveredNode != null && !IsParent(node, hoveredNode.node))
                    //                node.Collapse();
                    //        }));
                    //    }).Start();
                    //}

                    hoveredNode.node.BackColor = Color.Transparent;
                    hoveredNode = null;
                }
            }
            catch
            {
            }
        }

        private static string GetLastPathElement(string path)
        {
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
            return title;
        }

        class NativeDirectoryTreeProgress
        {
            public string path;
            public TreeNode node;
            //public TreeNode[] children;
            public List<ResourceFile> resources;
            public int i;

            public NativeDirectoryTreeProgress(TreeNode node)
            {
            }

            public NativeDirectoryTreeProgress(string title, string path, List<ResourceFile> resources, int i = 0)
            {
                this.path = path;
                this.resources = resources;
                this.i = i;

                this.node = new TreeNode(title);
                this.node.Name = title;
                this.node.Tag = path;
            }
        }

        public void RefreshBrowserWithNativeFS()
        {
            var basePath = Client.GetUserDataDirectory();

            TreeNode rootNode = new TreeNode("/");
            rootNode.Name = "";
            rootNode.Tag = "/";

            PopulateNode(basePath, rootNode);

            directoryTreeView.Invoke((Action<TreeNode>)((nodeToAdd) => {
                directoryTreeView.BeginUpdate();
                directoryTreeView.Nodes.Clear();
                directoryTreeView.Nodes.Add(rootNode);
                directoryTreeView.EndUpdate();
            }));
        }

        class SubdirToProcess
        {
            public string path;
            public TreeNode treeNode;

            public SubdirToProcess(string path, TreeNode treeNode)
            {
                this.path = path;
                this.treeNode = treeNode;
            }
        }

        private static void PopulateNode(string basePath, TreeNode rootNode, System.Action<string, TreeNode> subdirectoryHandler = null)
        {
            rootNode.Nodes.Clear();

            foreach (var entry in Directory.EnumerateDirectories(basePath))
            {
                var foldername = GetLastPathElement(entry);
                TreeNode newChild = new TreeNode(foldername);
                newChild.Name = foldername;
                newChild.Tag = (string)rootNode.Tag + foldername + "/";
                rootNode.Nodes.Add(newChild);

                subdirectoryHandler(entry, newChild);
            }
        }

        private void PopulateNodePhysFS(TreeNode rootNode)
        {
            rootNode.Nodes.Clear();

            foreach (var entry in Client.GetResources((string)rootNode.Tag))
            {
                CreateChild(rootNode, entry);
            }
        }

        private void PopulateNodeRecursivePhysFS(TreeNode rootNode)
        {
            Queue<SubdirToProcess> subdirs = new Queue<SubdirToProcess>();
            subdirs.Enqueue(new SubdirToProcess((string)rootNode.Tag, rootNode));

            while (subdirs.Count > 0)
            {
                var toProcess = subdirs.Dequeue();

                try
                {
                    directoryTreeView.InvokeLambda(() => toProcess.treeNode.Nodes.Clear());

                    foreach (var entry in Client.GetResources(toProcess.path))
                    {
                        if (entry.Directory)
                        {
                            TreeNode newChild = CreateChild(toProcess.treeNode, entry);

                            directoryTreeView.InvokeLambda((nodeToAdd) =>
                            {
                                toProcess.treeNode.Nodes.Add(nodeToAdd);
                            }, newChild);

                            subdirs.Enqueue(new SubdirToProcess(entry.Path, newChild));
                        }
                    }
                }
                catch (Exception ex)
                {
                    directoryTreeView.InvokeLambda(() => toProcess.treeNode.Nodes.Add("Error: " + ex.Message));
                }
            }
        }

        private static TreeNode CreateChild(TreeNode rootNode, ResourceFile entry)
        {
            var foldername = GetLastPathElement(entry.Path);
            TreeNode newChild = new TreeNode(foldername);
            newChild.Name = foldername;
            newChild.Tag = (string)rootNode.Tag + foldername + "/";
            return newChild;
        }

        private static void PopulateNodeRecursive(string basePath, TreeNode rootNode)
        {
            Queue<SubdirToProcess> subdirectoriesToProcess = new Queue<SubdirToProcess>();
            subdirectoriesToProcess.Enqueue(new SubdirToProcess(basePath, rootNode));

            while (subdirectoriesToProcess.Count > 0)
            {
                var toProcess = subdirectoriesToProcess.Dequeue();

                try
                {
                    PopulateNode(toProcess.path, toProcess.treeNode, (subPath, subNode) => subdirectoriesToProcess.Enqueue(new SubdirToProcess(subPath, subNode)));
                }
                catch (Exception ex)
                {
                    toProcess.treeNode.Nodes.Add("Error: " + ex.Message);
                }
            }
        }

        private static void PopulateSubnodes(string basePath, TreeNode rootNode)
        {
            if (Path.DirectorySeparatorChar != '/')
            {
                //var fullPath = basePath + ((string)rootNode.Tag).Replace('/', Path.DirectorySeparatorChar);

                foreach (var entry in rootNode.Nodes)
                {
                    var subnode = (TreeNode)entry;
                    var relPath = ((string)subnode.Tag).Replace('/', Path.DirectorySeparatorChar);
                    PopulateNode(basePath + relPath, subnode);
                }
            }
            else
            {
                //var fullPath = basePath + (string)rootNode.Tag;

                foreach (var entry in rootNode.Nodes)
                {
                    var subnode = (TreeNode)entry;
                    var relPath = (string)subnode.Tag;
                    PopulateNode(basePath + relPath, subnode);
                }
            }
        }

        void RefreshNode(string basePath, TreeNode rootNode)
        {
            directoryTreeView.Invoke(BeginUpdateDirectoryTree);
            directoryTreeView.Invoke((Action)(() => { PopulateNode(basePath, rootNode); }));
            directoryTreeView.Invoke(EndUpdateDirectoryTree);
        }

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

        public void RefreshBrowserWithPhysFS()
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
                            string path = progress.resources[progress.i].Path;

                            string title;
                            title = GetLastPathElement(path);

                            var childProgress = new DirectoryTreeProgress(title, path, Client.GetResources(path));
                            progress.node.Nodes.Add(childProgress.node);

                            ++progress.i;
                            stack.Push(progress);

                            progress = childProgress;
                        }
                    }
                }

                directoryTreeView.Invoke(BeginUpdateDirectoryTree);
                directoryTreeView.Invoke(ClearDirectoryTree);
                directoryTreeView.Invoke(AddToDirectoryTree, rootProgress.node);
                directoryTreeView.Invoke(RedrawDirectoryTree);
                directoryTreeView.Invoke(EndUpdateDirectoryTree);
            }
        }

        private void directoryTreeView_BeforeExpand(object sender, TreeViewCancelEventArgs e)
        {
            //PopulateSubnodes(Client.GetUserDataDirectory(), e.Node);
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
            pictureBox1.Invoke((Action)(() => pictureBox1.Visible = true));
            try
            {
                //RefreshBrowserWithPhysFS();
                TreeNode rootNode = new TreeNode("/");
                rootNode.Name = "";
                rootNode.Tag = "/";

                directoryTreeView.Invoke((Action)(() =>
                {
                    directoryTreeView.Nodes.Add(rootNode);
                }));

                PopulateNodeRecursivePhysFS(rootNode);
            }
            catch (Exception)
            {
            }
            pictureBox1.Invoke((Action)(() => pictureBox1.Visible = false));
        }

        private void refreshBackgroundWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            appFileSystemWatcher.Path = Client.GetDataDirectory();
            appFileSystemWatcher.EnableRaisingEvents = true;
            writeDirFileSystemWatcher.Path = Client.GetUserDataDirectory();
            writeDirFileSystemWatcher.EnableRaisingEvents = true;

            while (postLoadActions.Count > 0)
                postLoadActions.Dequeue()();

            if (directoryTreeView.Nodes.Count > 0)
                directoryTreeView.TopNode.Expand();
        }


        private void refreshNodeBackgroundWorker_DoWork(object sender, DoWorkEventArgs e)
        {
        }


        private void NavigateToPath(string path)
        {
            var resources = Client.GetResources(path);

            filesListView.BeginUpdate();

            filesListView.Clear();

            foreach (var resource in resources)
            {
                var item = new ListViewItem(Path.GetFileName(resource.Path), resource.Directory ? "Folder" : "File");
                item.Tag = resource.Path;
                filesListView.Items.Add(item);
            }

            filesListView.EndUpdate();

            // Make sure the navigated-to node is expanded
            TreeNode node = FindNode(path, directoryTreeView.TopNode);

            if (node != null && directoryTreeView.SelectedNode != node)
            {
                directoryTreeView.SelectedNode = node;
                node.Expand();
            }
        }

        private static TreeNode FindNode(string path, TreeNode node)
        {
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
            return node;
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
            DecideDragEffects(e);
        }

        private static void DecideDragEffects(DragEventArgs e)
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
            ClearHoveredNode();

            var node = directoryTreeView.GetNodeAt(directoryTreeView.PointToClient(new Point(e.X, e.Y)));
            if (node != null)
            {
                if (e.Data.GetDataPresent(DataFormats.Text))
                {
                    try
                    {
                        var sourcePath = (string)e.Data.GetData(DataFormats.Text);
                        var targetPath = node.FullPath.Substring(1) + "/" + Path.GetFileName(sourcePath);
                        if ((e.Effect & DragDropEffects.Copy) != 0)
                            Client.CopyResource(sourcePath, targetPath);
                        else if ((e.Effect & DragDropEffects.Move) != 0)
                            Client.MoveResource(sourcePath, targetPath);
                    }
                    catch
                    { }
                }
            }
        }

        private void directoryTreeView_DragOver(object sender, DragEventArgs e)
        {
            DecideDragEffects(e);
            SetHoveredNode(directoryTreeView.GetNodeAt(directoryTreeView.PointToClient(new Point(e.X, e.Y))));
        }

        private void directoryTreeView_DragLeave(object sender, EventArgs e)
        {
            ClearHoveredNode();
        }

        private void filesListView_KeyDown(object sender, KeyEventArgs e)
        {
            try
            {
                foreach (var item in filesListView.SelectedItems)
                {
                    var path = (string)((ListViewItem)item).Tag;
                    Client.DeleteResource(path);
                }
            }
            catch
            { }
        }

        static IEnumerable<string> MakeRelative(string path, string basePath)
        {
            var baseDirElements = basePath.Split(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
            var pathElements = path.Split(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
            int i = 0;
            for (; i < pathElements.Length && i < baseDirElements.Length; ++i)
            {
                if (pathElements[i] != baseDirElements[i])
                    break;
            }
            return pathElements.Skip(i);
        }

        private void RefreshPath(string absoslutePath, string basePath)
        {
            string navigateTo = null;
            if (directoryTreeView.SelectedNode != null)
                navigateTo = directoryTreeView.SelectedNode.FullPath;

            if (Directory.Exists(absoslutePath))
            {
                var relativePath = string.Join("/", MakeRelative(absoslutePath, basePath));
                var nodeChanged = FindNode(relativePath, directoryTreeView.TopNode);
                if (nodeChanged != null)
                    PopulateNodePhysFS(nodeChanged);
                //refreshBackgroundWorker.RunWorkerAsync();
            }

            if (navigateTo != null)
                NavigateToPath(navigateTo);
        }


        private void RenamePath(string fullPath, string oldFullPath, string basePath)
        {
            var relativePath = string.Join("/", MakeRelative(oldFullPath, basePath));
            var node = FindNode(relativePath, directoryTreeView.TopNode);
            if (node != null)
                node.Remove();

            RefreshPath(fullPath, basePath);
        }

        Queue<Action> postLoadActions = new Queue<Action>();

        private void fileSystemWatcher_ChangedCreatedDeleted(object sender, FileSystemEventArgs e)
        {
            if (!directoryTreeView.IsDisposed)
            {
                string basePath = ((FileSystemWatcher)sender).Path;

                if (!refreshBackgroundWorker.IsBusy)
                    RefreshPath(e.FullPath, basePath);
                else
                    postLoadActions.Enqueue(() => { RefreshPath(e.FullPath, basePath); });
            }
        }

        private void fileSystemWatcher_Renamed(object sender, RenamedEventArgs e)
        {
            if (!directoryTreeView.IsDisposed)
            {
                var basePath = ((FileSystemWatcher)sender).Path;

                if (!refreshBackgroundWorker.IsBusy)
                    RenamePath(e.FullPath, e.OldFullPath, basePath);
                else
                    postLoadActions.Enqueue(() => { RenamePath(e.FullPath, e.OldFullPath, basePath); });
            }
        }

    }
}
