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
        int maxConnectionRetries = 3;

        public string ServerPath
        {
            get { return serverPath; }
            set
            {
                serverPath = value;
                Save();
            }
        }

        public int MaxConnectionRetries
        {
            get { return maxConnectionRetries; }
            set
            {
                maxConnectionRetries = value;
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
                    string line = reader.ReadLine();
                    if (line.Length > 0)
                        int.TryParse(line, out maxConnectionRetries);
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
                    writer.WriteLine(maxConnectionRetries);
                }
            }
            catch
            {
            }
        }
    }
}
