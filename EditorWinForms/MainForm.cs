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
using System.Runtime.InteropServices;
using System.Collections;
using System.Threading;

namespace EditorWinForms
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
        }

        TTransport transport;
        Editor.Client client;

        SettingsFile settings = new SettingsFile();

        ResourceBrowser resourceBrowser = new ResourceBrowser();
        Console console = new Console();

        string mapName;

        bool usingExistingServerProcess = false;

        protected override void WndProc(ref Message m)
        {
            const int WM_PARENTNOTIFY = 0x0210;
            if (!this.Focused && m.Msg == WM_PARENTNOTIFY)
            {
                // Make this form auto-grab the focus when menu/controls are clicked
                this.Activate();
            }
            base.WndProc(ref m);
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            var socket = new TSocket(settings.ServerAddress, 9090);
            transport = new TBufferedTransport(socket, 1024);
            TProtocol protocol = new TBinaryProtocol(transport);
            client = new Editor.Client(protocol);

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

            //waitToConnectTimer.Start();
            connectBackgroundWorker.RunWorkerAsync();
        }

        private void StopServer()
        {
            if (transport.IsOpen)
                client.Stop();
        }

        private void openFileDialog1_FileOk(object sender, CancelEventArgs e)
        {
            settings.ServerPath = locateServerOpenFileDialog.FileName;
        }

        private void settingsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            locateServerOpenFileDialog.FileName = settings.ServerPath;
            locateServerOpenFileDialog.ShowDialog();
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

        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool GetWindowRect(HandleRef hWnd, out RECT lpRect);

        [StructLayout(LayoutKind.Sequential)]
        public struct RECT
        {
            public int Left;        // x position of upper-left corner
            public int Top;         // y position of upper-left corner
            public int Right;       // x position of lower-right corner
            public int Bottom;      // y position of lower-right corner
        }

        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool MoveWindow(HandleRef hWnd, int X, int Y, int nWidth, int nHeight, bool bRepaint);

        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool IsWindowVisible(HandleRef hWnd);

        int connectionAttempts = 0;

        private void connectBackgroundWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            try
            {
                while (!AttemptToConnect())
                {
                    Thread.Sleep(1000);
                    console.AddToConsole(string.Format("Retrying in {0}ms", 1000));
                }
            }
            catch
            { }
        }

        private void connectBackgroundWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (!usingExistingServerProcess)
                editorServerProcess.BeginOutputReadLine();

            ShowResourceBrowser();

            AlignWithEditorWindow();

            BringToFront();

            processEngineMessageTimer.Start();
        }

        private void AlignWithEditorWindow()
        {
            if (IsWindowVisible(new HandleRef(this, editorServerProcess.MainWindowHandle)))
            {
                RECT windowRect;
                if (GetWindowRect(new HandleRef(this, editorServerProcess.MainWindowHandle), out windowRect))
                {
                    //SetDesktopLocation(windowRect.Top - Size.Height, windowRect.Left);
                    SetBounds(windowRect.Left, windowRect.Top - Size.Height, windowRect.Right - windowRect.Left, Height, BoundsSpecified.Location | BoundsSpecified.Width);
                    //SetDesktopBounds(windowRect.Top - Size.Height, windowRect.Left, windowRect.Right - windowRect.Left, Height);
                }
            }
        }

        private bool AttemptToConnect()
        {
            console.AddToConsole("Attempting to connect to engine");
            if (!transport.IsOpen)
            {
                if (connectionAttempts++ < settings.MaxConnectionRetries)
                {
                    if (connectionAttempts > 1)
                        console.AddToConsole(string.Format("Retry {0} of {1}", connectionAttempts - 1, settings.MaxConnectionRetries));
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
                    console.AddToConsole("Failed to connect!");
                    console.AddToConsole("vvvvvv");
                    return true;
                }
            }

            if (transport.IsOpen)
            {
                console.AddToConsole("Connected!");
                console.AddToConsole("vvvvvv");

                return true;
            }

            return false;
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
            {
                resourceBrowser.Client = client;
                resourceBrowser.Show();
            }
            else
            {
                resourceBrowser = new ResourceBrowser();
                resourceBrowser.Client = client;
                resourceBrowser.Show();
            }
        }

        private void clearResourceDatabaseToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //client.InterpretConsoleCommand();
        }

        private void refreshConsoleToolStripMenuItem_Click(object sender, EventArgs e)
        {
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

        private void websiteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Process.Start(@"https://github.com/Kezeali/Fusion");
        }

        private void compileScriptsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            client.RefreshResources();
        }

        FilenameDialog dialog = new FilenameDialog();

        bool ShowMapNameDialog(string title = "Map Name")
        {
            dialog.Text = title;
            if (mapName != null)
                dialog.FileName = mapName;
            dialog.ShowDialog();
            mapName = dialog.FileName;
            return dialog.Result;
        }

        bool ShowMapNameDialogIfNecessary(string title = "Map Name")
        {
            if (mapName == null)
                return ShowMapNameDialog(title);
            return true;
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (ShowMapNameDialogIfNecessary("Save: Set Map Name"))
                client.SaveMap(mapName);
        }

        private void saveMapAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (ShowMapNameDialog("Save As: Set Map Name"))
                client.SaveMap(mapName);
        }

        private void loadToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (ShowMapNameDialog("Load: Set Map Name"))
                client.LoadMap(mapName);
        }

        private void compileMapToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (ShowMapNameDialogIfNecessary("Build: Set Map Name"))
                client.CompileMap(mapName);
        }

        private void MainForm_Move(object sender, EventArgs e)
        {
            //RECT windowRect;
            //if (GetWindowRect(new HandleRef(this, editorServerProcess.MainWindowHandle), out windowRect))
            //{
            //    MoveWindow(new HandleRef(this, editorServerProcess.MainWindowHandle), this.Left, this.Bottom, windowRect.Right - windowRect.Left, windowRect.Bottom - windowRect.Top, true);
            //}
        }

        EngineConnection engineConnection;

        bool editorMissing;
        int originalCheckEnginePositionTimerInterval;

        private void checkEnginePositionTimer_Tick(object sender, EventArgs e)
        {
            try
            {
                AlignWithEditorWindow();
                if (editorMissing)
                {
                    checkEnginePositionTimer.Interval = originalCheckEnginePositionTimerInterval;
                    editorMissing = false;
                }
            }
            catch (InvalidOperationException)
            {
                originalCheckEnginePositionTimerInterval = checkEnginePositionTimer.Interval;
                checkEnginePositionTimer.Interval = 1000;
                editorMissing = true;
            }
        }

        private void processEngineMessageTimer_Tick(object sender, EventArgs e)
        {
            //try
            //{
            //    var dialogRequest = client.PopDialogRequest();
            //    if (dialogRequest.Id > 0)
            //    {
            //        var physfsBasePath = client.GetUserDataDirectory();

            //        if (dialogRequest.Type == DialogType.Open)
            //        {
            //            openFileDialog.FileName = physfsBasePath + dialogRequest.Path;
            //            openFileDialog.Title = dialogRequest.Title;
            //            openFileDialog.Tag = dialogRequest.Id;

            //            var result = openFileDialog.ShowDialog();

            //            dialogRequest.Path = PhysFSUtils.MakeRelativeString(openFileDialog.FileName, physfsBasePath);
            //            client.CompleteDialogRequest(dialogRequest, result == System.Windows.Forms.DialogResult.OK && dialogRequest.Path != string.Empty);
            //        }
            //    }
            //}
            //catch (Exception ex)
            //{
            //    console.AddToConsole(ex.Message);
            //}
        }

    }
}
