using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EditorWinForms
{
    public class SettingsFile
    {
        string serverPath = "EditorServer.exe";

        public string ServerPath
        {
            get { return serverPath; }
            set
            {
                serverPath = value;
                Save();
            }
        }

        public SettingsFile()
        {
            try
            {
                using (var reader = new System.IO.StreamReader("config.txt"))
                {
                    serverPath = reader.ReadLine();
                }
            }
            catch
            {
            }
        }

        public void Save()
        {
            // This is separate so it can be called async
            //  TODO: call this async :P
            DoSave();
        }

        void DoSave()
        {
            try
            {
                using (var writer = new System.IO.StreamWriter("config.txt"))
                {
                    writer.WriteLine(serverPath);
                }
            }
            catch
            {
            }
        }
    }
}
