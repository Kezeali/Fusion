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

namespace EditorWinForms
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();

            transport = new TSocket("localhost", 9090, 60000);
            TProtocol protocol = new TBinaryProtocol(transport);
            client = new Editor.Client(protocol);
        }

        TTransport transport;
        Editor.Client client;

        SettingsFile settings = new SettingsFile();

        private void Form1_Load(object sender, EventArgs e)
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

        private void buttonListEnts_Click(object sender, EventArgs e)
        {
            client.test(new Test() { Name = "Test" });
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
            client.stop();
            this.Close();
        }
    }
}
