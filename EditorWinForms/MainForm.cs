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
using System.Diagnostics;

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

        bool usingExistingServerProcess = false;

        private void MainForm_Load(object sender, EventArgs e)
        {
            var serverProcessName = Path.GetFileNameWithoutExtension(settings.ServerPath);
            var existingProcesses = Process.GetProcessesByName(serverProcessName);
            if (existingProcesses.Length > 0)
            {
                editorServerProcess = existingProcesses[0];
                usingExistingServerProcess = true;
            }

            //editorServerProcess.OutputDataReceived += editorServerProcess_OutputDataReceived;
            console.Client = client;
            console.Show();

            if (!usingExistingServerProcess)
            {
                editorServerProcess.StartInfo.FileName = settings.ServerPath;
                try
                {
                    if (editorServerProcess.Start())
                    {
                        console.AddToConsole("Server started");
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

            waitToConnectTimer.Start();
        }

        private void StopServer()
        {
            if (transport.IsOpen)
                client.Stop();
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
            this.Close();
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (!usingExistingServerProcess)
                StopServer();
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

                ShowResourceBrowser();
            }
        }

        void editorServerProcess_OutputDataReceived(object sender, System.Diagnostics.DataReceivedEventArgs e)
        {
            console.AddToConsole(e.Data);
        }

        private void openResourceBrowserToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowResourceBrowser();
        }

        private void ShowResourceBrowser()
        {
            if (!resourceBrowser.Visible && !resourceBrowser.IsDisposed)
                resourceBrowser.Show();
            else
            {
                resourceBrowser = new ResourceBrowser();
                resourceBrowser.Client = client;
                resourceBrowser.Show();
            }
        }

        private void refreshConsoleToolStripMenuItem_Click(object sender, EventArgs e)
        {
            console.AddToConsole(editorServerProcess.StandardOutput.ReadToEnd());
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            
        }

        private void printLotsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            for (int i = 0; i < 100; ++i)
                console.AddToConsole("Text");
        }

        bool tick = true;

        private void testConsoleTimer_Tick(object sender, EventArgs e)
        {
            if (tick)
                console.AddToConsole("Tick");
            else
                console.AddToConsole("Tock");
            tick = !tick;
        }

        private void startToolStripMenuItem_Click(object sender, EventArgs e)
        {
            testConsoleTimer.Start();
        }

        private void stopToolStripMenuItem_Click(object sender, EventArgs e)
        {
            testConsoleTimer.Stop();
        }

        private void loremToolStripMenuItem_Click(object sender, EventArgs e)
        {
            console.AddToConsole(
@"The first couple of responses I got were related to how to track a memory leak, so I thought I should share my methodology. I used a combination of WinDbg and perfmon to track the memory use over time (from a couple hours to days). The total number of bytes on all CLR heaps does not increase by more than I expect, but the total number of private bytes steadily increases as more messages are logged. This makes WinDbg less useful, as it's tools (sos) and commands (dumpheap, gcroot, etc.) are based on .NET's managed memory."
                );
        }

    }
}
