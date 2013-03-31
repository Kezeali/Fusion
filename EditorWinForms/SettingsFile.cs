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
        string serverAddress = "localhost";
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

        public string ServerAddress
        {
            get { return serverAddress; }
            set
            {
                serverAddress = value;
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

        private void WriteValue(StreamWriter writer, string key, string value)
        {
            writer.WriteLine(key + "=" + value);
        }

        public SettingsFile()
        {
            if (File.Exists("config.txt"))
            {
                using (var reader = new StreamReader("config.txt"))
                {
                    while (!reader.EndOfStream)
                    {
                        try
                        {
                            string key, value;
                            ReadValue(reader, out key, out value);
                            if (key == "serverPath")
                                serverPath = value;
                            else if (key == "serverAddress")
                                serverAddress = value;
                            else if (key == "maxConnectionRetries")
                                maxConnectionRetries = int.Parse(value);
                        }
                        catch
                        {
                        }
                    }
                }
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
                    WriteValue(writer, "serverPath", serverPath);
                    WriteValue(writer, "serverAddress", serverAddress);
                    WriteValue(writer, "maxConnectionRetries", maxConnectionRetries.ToString());
                }
            }
            catch
            {
            }
        }
    }
}
