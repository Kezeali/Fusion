using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using FusionEngine.Interprocess;

namespace EditorWinForms
{
    public partial class Console : Form
    {
        public Console()
        {
            InitializeComponent();
        }

        public Editor.Client Client { get; set; }

        public void AddToConsole(string line)
        {
            if (consoleTextBox.Text.Length > 0)
                consoleTextBox.Text += "\r\n" + line;
            else
                consoleTextBox.Text = line;
        }

        List<string> currentAutocomplete = new List<string>();

        private void UpdateAutocomplete(string command)
        {
            var completions = Client.FindConsoleCommandSuggestions(command);

            completions.RemoveAll((entry) => string.IsNullOrWhiteSpace(entry));

            if (completions.Count > 0)
            {
                if (!completions.SequenceEqual(currentAutocomplete))
                {
                    currentAutocomplete = new List<string>(completions);
                    commandTextBox.AutoCompleteCustomSource.AddRange(completions.ToArray());
                }
            }
            else
                commandTextBox.AutoCompleteCustomSource.Clear();
        }

        private void commandTextBox_TextChanged(object sender, EventArgs e)
        {
            UpdateAutocomplete(commandTextBox.Text);
        }

        private void commandTextBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Return || e.KeyCode == Keys.Enter)
            {
                e.Handled = true;

                Client.InterpretConsoleCommand(commandTextBox.Text);
                commandTextBox.Clear();
                commandTextBox.AutoCompleteCustomSource.Clear();
            }
        }

        private void commandTextBox_KeyUp(object sender, KeyEventArgs e)
        {
            //UpdateAutocomplete(commandTextBox.Text);
        }
    }
}
