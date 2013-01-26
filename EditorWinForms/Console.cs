using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows.Forms;
using AutocompleteMenuNS;
using FusionEngine.Interprocess;

namespace EditorWinForms
{
    public partial class Console : Form
    {
        public Console()
        {
            InitializeComponent();
        }

        Editor.Client client;

        public Editor.Client Client
        {
            get
            {
                return client;
            }
            set
            {
                client = value;
                autocompleteMenu.SetAutocompleteItems(new ConsoleCommandCollection(client, commandTextBox));
            }
        }

        public void AddToConsole(string line)
        {
            AddToTextBox(consoleTextBox, line);
        }

        void AddToTextBoxCrappy(string line)
        {
            int initialSelectionStart = consoleTextBox.SelectionStart;
            int initialSelectionLength = consoleTextBox.SelectionLength;
            bool scrollToEnd = initialSelectionStart == consoleTextBox.Text.Length;

            if (scrollToEnd)
            {
                if (consoleTextBox.Text.Length > 0)
                    consoleTextBox.AppendText("\r\n" + line);
                else
                    consoleTextBox.AppendText(line);
            }
            else
            {
                if (consoleTextBox.Text.Length > 0)
                    consoleTextBox.Text += "\r\n" + line;
                else
                    consoleTextBox.Text = line;

                consoleTextBox.SelectionStart = initialSelectionStart;
                consoleTextBox.SelectionLength = initialSelectionLength;
                consoleTextBox.ClearUndo();
            }

            const int maxLines = 2000;

            if (consoleTextBox.Lines.Length > maxLines)
            {
                int exceededLines = consoleTextBox.Lines.Length - maxLines;

                int lengthOfTextRemoved = consoleTextBox.Lines.Take(exceededLines).Sum((lineEntry) => lineEntry.Length);

                int adjustedSelectionStart;
                if (scrollToEnd)
                    adjustedSelectionStart = consoleTextBox.Text.Length;
                else
                    adjustedSelectionStart = (initialSelectionStart >= lengthOfTextRemoved ? initialSelectionStart - lengthOfTextRemoved : 0);

                var textboxLines = consoleTextBox.Lines;
                System.Array.Copy(textboxLines, exceededLines, textboxLines, 0, textboxLines.Length - exceededLines);
                System.Array.Resize(ref textboxLines, textboxLines.Length - exceededLines);
                consoleTextBox.Lines = textboxLines;

                consoleTextBox.SelectionStart = adjustedSelectionStart;
            }

            if (!scrollToEnd)
            {
                consoleTextBox.ScrollToCaret();
            }
        }

        // Constants for extern calls to various scrollbar functions
        private const int SB_HORZ = 0x0;
        private const int SB_VERT = 0x1;
        private const int WM_HSCROLL = 0x114;
        private const int WM_VSCROLL = 0x115;
        private const int SB_THUMBPOSITION = 4;
        private const int SB_BOTTOM = 7;
        private const int SB_OFFSET = 13;

        [DllImport("user32.dll")]
        static extern int SetScrollPos(IntPtr hWnd, int nBar, int nPos, bool bRedraw);
        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        private static extern int GetScrollPos(IntPtr hWnd, int nBar);
        [DllImport("user32.dll")]
        private static extern bool PostMessageA(IntPtr hWnd, int nBar, int wParam, int lParam);
        [DllImport("user32.dll")]
        static extern bool GetScrollRange(IntPtr hWnd, int nBar, out int lpMinPos, out int lpMaxPos);

        delegate void AppendToTextboxFn(TextBoxBase textBox, string line);

        // Based on http://stackoverflow.com/questions/1743448/auto-scrolling-text-box-uses-more-memory-than-expected
        private static void AddToTextBox(TextBoxBase consoleTextBox, string line)
        {
            // Make sure this is done in the UI thread
            if (consoleTextBox.InvokeRequired)
            {
                consoleTextBox.Invoke(new AppendToTextboxFn(AddToTextBox), new object[] { consoleTextBox, line });
            }
            else
            {
                int selectionStart = consoleTextBox.SelectionStart;
                int selectionLength = consoleTextBox.SelectionLength;

                bool scrollToBottom = false;

                int vertScrollMin;
                int vertScrollMax;
                int scrollBitHeight;
                int savedScrollPosition;

                // Win32 magic to keep the text box scrolling to the newest append to the text box unless
                // the user has moved the scroll box up
                //scrollBitHeight = (int)((consoleTextBox.ClientSize.Height + consoleTextBox.Font.Height - SystemInformation.HorizontalScrollBarHeight) / (consoleTextBox.Font.Height));
                scrollBitHeight = (int)((consoleTextBox.ClientSize.Height + consoleTextBox.Font.Height) / (consoleTextBox.Font.Height));
                savedScrollPosition = GetScrollPos(consoleTextBox.Handle, SB_VERT);
                GetScrollRange(consoleTextBox.Handle, SB_VERT, out vertScrollMin, out vertScrollMax);
                if (savedScrollPosition >= (vertScrollMax - scrollBitHeight - 1))
                    scrollToBottom = true;

                const int maxLines = 200;

                if (consoleTextBox.Lines.Length > maxLines)
                {
                    int exceededLines = consoleTextBox.Lines.Length - maxLines;

                    int lengthOfTextRemoved = consoleTextBox.Lines.Take(exceededLines).Sum((lineEntry) => lineEntry.Length + Environment.NewLine.Length);

                    if (selectionStart >= lengthOfTextRemoved)
                        selectionStart = selectionStart - lengthOfTextRemoved;
                    else
                    {
                        selectionLength = selectionStart + selectionLength > lengthOfTextRemoved ? selectionLength - (lengthOfTextRemoved - selectionStart) : 0;
                        selectionStart = 0;
                    }

                    var textboxLines = consoleTextBox.Lines;
                    System.Array.Copy(textboxLines, exceededLines, textboxLines, 0, textboxLines.Length - exceededLines);
                    System.Array.Resize(ref textboxLines, textboxLines.Length - exceededLines);
                    consoleTextBox.Lines = textboxLines;
                }

                if (scrollToBottom)
                {
                    if (consoleTextBox.Text.Length > 0)
                        consoleTextBox.AppendText(Environment.NewLine + line);
                    else
                        consoleTextBox.AppendText(line);
                }
                else
                {
                    if (consoleTextBox.Text.Length > 0)
                        consoleTextBox.Text += (Environment.NewLine + line);
                    else
                        consoleTextBox.Text = (line);
                }

                consoleTextBox.SelectionStart = selectionStart;
                consoleTextBox.SelectionLength = selectionLength;

                if (scrollToBottom)
                {
                    GetScrollRange(consoleTextBox.Handle, SB_VERT, out vertScrollMin, out vertScrollMax);
                    savedScrollPosition = vertScrollMax - scrollBitHeight;
                    scrollToBottom = false;
                }
                SetScrollPos(consoleTextBox.Handle, SB_VERT, savedScrollPosition, true);
                PostMessageA(consoleTextBox.Handle, WM_VSCROLL, SB_THUMBPOSITION + 0x10000 * savedScrollPosition, 0);
            }
        }

        List<string> currentAutocomplete = new List<string>();

        internal class ConsoleCommandCollection : IEnumerable<AutocompleteItem>
        {
            private Editor.Client client;
            private TextBoxBase textBox;

            public ConsoleCommandCollection(Editor.Client client, TextBoxBase textBox)
            {
                this.client = client;
                this.textBox = textBox;
            }

            public IEnumerator<AutocompleteItem> GetEnumerator()
            {
                return BuildCompletionList().GetEnumerator();
            }

            System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
            {
                return GetEnumerator();
            }

            private IEnumerable<AutocompleteItem> BuildCompletionList()
            {
                var completions = client.FindConsoleCommandSuggestions(textBox.Text);

                completions.RemoveAll((entry) => string.IsNullOrWhiteSpace(entry));

                foreach (var completion in completions)
                {
                    var commandHelp = client.GetConsoleCommandHelp(completion);
                    string tooltipTitle;
                    if (commandHelp.ArgumentNames != null && commandHelp.ArgumentNames.Count > 0)
                        tooltipTitle = string.Join(", ", commandHelp.ArgumentNames);
                    else
                        tooltipTitle = completion;
                    yield return new AutocompleteItem(completion, 0, completion, tooltipTitle, commandHelp.HelpText);
                }
            }
        }

        private void commandTextBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Return || e.KeyCode == Keys.Enter)
            {
                e.Handled = true;

                Client.InterpretConsoleCommand(commandTextBox.Text);

                commandTextBox.Clear();
            }
            else if (e.Control && e.KeyCode == Keys.Space)
            {
                e.SuppressKeyPress = true;

                autocompleteMenu.Show(commandTextBox, true);
            }
        }

        private void commandTextBox_KeyUp(object sender, KeyEventArgs e)
        {
        }

        protected override bool ProcessTabKey(bool forward)
        {
            return (!commandTextBox.Focused || autocompleteMenu.VisibleItems.Count == 0) && base.ProcessTabKey(forward);
        }

        private void Console_Shown(object sender, EventArgs e)
        {
            commandTextBox.Focus();
        }
    }
}
