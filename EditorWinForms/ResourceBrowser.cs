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

            public DirectoryTreeProgress(string path, List<ResourceFile> resources, int i = 0)
            {
                var filename = Path.GetDirectoryName(path);

                this.path = path;
                this.resources = resources;
                this.i = i;

                this.node = new TreeNode(filename);
            }
        }

        public void RefreshBrowser()
        {
            if (Client != null)
            {
                string rootPath = "/";

                directoryTreeView.BeginUpdate();

                directoryTreeView.Nodes.Clear();

                var rootProgress = new DirectoryTreeProgress(rootPath, Client.GetResources(rootPath));

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

                            var childProgress = new DirectoryTreeProgress(path, Client.GetResources(path));
                            progress.node.Nodes.Add(childProgress.node);

                            stack.Push(progress);

                            progress = childProgress;
                        }
                    }
                }

                directoryTreeView.Nodes.Add(rootProgress.node);

                directoryTreeView.EndUpdate();
            }
        }

        private void ResourceBrowser_Shown(object sender, EventArgs e)
        {
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
    }
}
