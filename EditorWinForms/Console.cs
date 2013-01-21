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
            consoleTextBox.Text += line + "\n";
        }

        private void commandTextBox_TextChanged(object sender, EventArgs e)
        {
            var completions = Client.FindConsoleCommandSuggestions(commandTextBox.Text);

            if (completions.Count > 0)
            {
                foreach (var completion in completions)
                {
                    autocompleteContextMenuStrip.Items.Add(completion);
                }

                var position = Point.Empty;
                int lastSpace = commandTextBox.Text.LastIndexOf(' ');
                if (lastSpace > 0)
                    position = commandTextBox.GetPositionFromCharIndex(lastSpace);

                autocompleteContextMenuStrip.Show(commandTextBox, position);
            }
        }

        private void autocompleteContextMenuStrip_ItemClicked(object sender, ToolStripItemClickedEventArgs e)
        {
            commandTextBox.Text = Client.CompleteCommand(commandTextBox.Text, e.ClickedItem.Text);
        }
    }
}
