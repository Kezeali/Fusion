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
    public partial class ModelessMessageBox : Form
    {
        public ModelessMessageBox()
        {
            InitializeComponent();
        }

        public string Message
        {
            get
            {
                return messageLabel.Text;
            }
            set
            {
                messageLabel.Text = value;
            }
        }

        public string Caption
        {
            get
            {
                return base.Text;
            }
            set
            {
                base.Text = value;
            }
        }

        public Image Image
        {
            get
            {
                return messagePictureBox.Image;
            }
            set
            {
                messagePictureBox.Image = value;
            }
        }

        public string OkText
        {
            get
            {
                return okButton.Text;
            }
            set
            {
                okButton.Text = value;
            }
        }

        public string CancelText
        {
            get
            {
                return cancelButton.Text;
            }
            set
            {
                cancelButton.Text = value;
            }
        }

        public Action Ok { get; set; }

        public Action Cancel { get; set; }

        void CenterIn(Form parent)
        {
            Location = new Point(
                (int)(parent.Left + parent.Width * 0.5 - Width * 0.5),
                (int)(parent.Top + parent.Height * 0.5 - Height * 0.5));
        }

        public static ModelessMessageBox Show(Form parent, string text, string caption, Action ok, Action cancel = null)
        {
            var messageBox = new ModelessMessageBox();
            //messageBox.Parent = parent;
            messageBox.Message = text;
            messageBox.Text = caption;
            messageBox.Image = messageBox.defaultImageList.Images[0];
            messageBox.Ok = ok;
            messageBox.Cancel = cancel;

            messageBox.CenterIn(parent);

            messageBox.Show();
            return messageBox;
        }

        public static ModelessMessageBox Show(Form parent, string text, string caption, Image image, Action ok, Action cancel = null)
        {
            var messageBox = new ModelessMessageBox();
            //messageBox.Parent = parent;
            messageBox.Message = text;
            messageBox.Text = caption;
            messageBox.Image = image; 
            messageBox.Ok = ok;
            messageBox.Cancel = cancel;

            messageBox.CenterIn(parent);

            messageBox.Show();
            return messageBox;
        }

        public static ModelessMessageBox Show(Form parent, string text, string caption, Image image, string okText, string cancelText, Action ok, Action cancel = null)
        {
            var messageBox = new ModelessMessageBox();
            //messageBox.Parent = parent;
            messageBox.Message = text;
            messageBox.Text = caption;
            messageBox.Image = image;
            messageBox.okButton.Text = okText;
            messageBox.cancelButton.Text = cancelText;
            messageBox.Ok = ok;
            messageBox.Cancel = cancel;

            messageBox.CenterIn(parent);

            messageBox.Show();
            return messageBox;
        }

        private void ModelessMessageBox_FormClosed(object sender, FormClosedEventArgs e)
        {
            if (e.CloseReason == CloseReason.UserClosing)
            {
                if (Cancel != null)
                {
                    Cancel();
                    Cancel = null;
                }
            }
        }

        private void okButton_Click(object sender, EventArgs e)
        {
            if (Ok != null)
            {
                Ok();
                Ok = null;
            }
            Close();
        }

        private void cancelButton_Click(object sender, EventArgs e)
        {
            if (Cancel != null)
            {
                Cancel();
                Cancel = null;
            }
            Close();
        }
    }
}
