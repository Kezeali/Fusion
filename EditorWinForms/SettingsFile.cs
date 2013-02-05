using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EditorWinForms
{
    public class SettingsFile
    {
        string serverPath = "EditorServer.exe";
        int maxConnectionRetries = 20;

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

        private bool ReadValue(StreamReader reader, out string key, out string value)
        {
            var line = reader.ReadLine();
            var elements = line.Split('=');
            if (elements.Length >= 2)
            {
                key = elements[0];
                value = elements.Skip(1).Aggregate((a, b) => a + b).Trim();
                return true;
            }
            else
            {
                key = line;
                value = line;
                return false;
            }
        }

        public SettingsFile()
        {
            try
            {
                using (var reader = new StreamReader("config.txt"))
                {
                    while (!reader.EndOfStream)
                    {
                        string key, value;
                        ReadValue(reader, out key, out value);
                        if (key == "serverPath")
                            serverPath = value;
                        else if (key == "maxConnectionRetries")
                            maxConnectionRetries = int.Parse(value);
                    }
                    //serverPath = reader.ReadLine();
                    //string line = reader.ReadLine();
                    //if (line.Length > 0)
                    //    int.TryParse(line, out maxConnectionRetries);
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
                    foreach (var field in GetType().GetFields())
                    {
                        writer.Write(string.Format("{0}={1}", field.Name, field.GetValue(this)));
                    }
                    //writer.WriteLine(serverPath);
                    //writer.WriteLine(maxConnectionRetries);
                }
            }
            catch
            {
            }
        }
    }
}
