using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace EditorWinForms
{
    public partial class Console : Form
    {
        public Console()
        {
            InitializeComponent();
        }

        public void AddToConsole(string line)
        {
            consoleTextBox.Text += line + "\n";
        }
    }
}
