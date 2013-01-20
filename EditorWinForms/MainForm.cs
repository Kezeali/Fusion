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
        Console console = new Console();

        private void MainForm_Load(object sender, EventArgs e)
        {
            try
            {
                editorServerProcess.StartInfo.FileName = settings.ServerPath;

                editorServerProcess.OutputDataReceived += editorServerProcess_OutputDataReceived;
                console.Show();

                // Attempt to connect if the process was started successfully
                if (editorServerProcess.Start())
                {
                    waitToConnectTimer.Start();
                }
                else
                {
                    MessageBox.Show("Failed to start Fusion server at (path): " + settings.ServerPath);
                }
            }
            catch (FileNotFoundException)
            {
                MessageBox.Show("Failed to find Fusion server at (path): " + settings.ServerPath);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Failed to start Fusion server at (path): " + settings.ServerPath + ".\n\n" + ex.Message);
            }
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
            if (transport.IsOpen)
                client.Stop();
            this.Close();
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (transport.IsOpen)
                client.Stop();
        }

        private void waitToConnectTimer_Tick(object sender, EventArgs e)
        {
            AttemptToConnect();
        }

        int connectionAttempts = 0;

        private void AttemptToConnect()
        {
            console.AddToConsole(string.Format("Attempting to connect. Max {0} tries", settings.MaxConnectionRetries));
            if (!transport.IsOpen)
            {
                if (connectionAttempts++ < settings.MaxConnectionRetries)
                {
                    try
                    {
                        transport.Open();
                    }
                    catch (System.Net.Sockets.SocketException ex)
                    {
                        console.AddToConsole(ex.Message);
                    }
                }
                else
                {
                    console.AddToConsole("Failed to connect");
                    waitToConnectTimer.Stop();
                }
            }

            if (transport.IsOpen)
            {
                console.AddToConsole("Connected");
                waitToConnectTimer.Stop();

                resourceBrowser.Client = client;
                resourceBrowser.Show();
            }
        }

        void editorServerProcess_OutputDataReceived(object sender, System.Diagnostics.DataReceivedEventArgs e)
        {
            console.AddToConsole(e.Data);
        }

    }
}
