using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Thrift.Protocol;
using Thrift.Transport;
using FusionEngine.Interprocess;
using System.IO;

namespace EditorWinForms
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();

            transport = new TSocket("localhost", 9090, 60000);
            TProtocol protocol = new TBinaryProtocol(transport);
            client = new Editor.Client(protocol);
        }

        TTransport transport;
        Editor.Client client;

        SettingsFile settings = new SettingsFile();

        ResourceBrowser resourceBrowser = new ResourceBrowser();

        private void MainForm_Load(object sender, EventArgs e)
        {
            try
            {
                var process = System.Diagnostics.Process.Start(settings.ServerPath);

                // Attempt to connect if the process was started successfully
                if (process != null)
                {
                    // TODO: use timer control to fire this
                    int retries = 3;
                    while (!transport.IsOpen && --retries > 0)
                    {
                        transport.Open();
                        var result = MessageBox.Show("Failed to connect to Fusion, retry?", "Failed to connect", MessageBoxButtons.RetryCancel);
                        if (result == System.Windows.Forms.DialogResult.Cancel)
                            break;
                    }
                    if (!transport.IsOpen)
                        MessageBox.Show("Failed to connect to Fusion");
                }
                else
                {
                    MessageBox.Show("Failed to start Fusion at (path): " + settings.ServerPath);
                }
            }
            catch
            {
                MessageBox.Show("Failed to find Fusion server at (path): " + settings.ServerPath);
            }

            resourceBrowser.Client = client;
            resourceBrowser.Show();
        }

        private void openFileDialog1_FileOk(object sender, CancelEventArgs e)
        {
            settings.ServerPath = openFileDialog1.FileName;
        }

        private void settingsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            openFileDialog1.FileName = settings.ServerPath;
            openFileDialog1.ShowDialog();
        }

        private void quitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            client.Stop();
            this.Close();
        }
    }
}
