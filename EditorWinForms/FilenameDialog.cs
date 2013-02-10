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
    public partial class FilenameDialog : Form
    {
        public FilenameDialog()
        {
            InitializeComponent();
        }

        public string FileName
        {
            get
            {
                return filenameTextBox.Text;
            }
            set
            {
                filenameTextBox.Text = value;
            }
        }

        public bool Result
        {
            get;
            set;
        }

        private void okButton_Click(object sender, EventArgs e)
        {
            Result = true;
            this.Close();
        }

        private void cancelButton_Click(object sender, EventArgs e)
        {
            Result = false;
            this.Close();
        }


    }
}
