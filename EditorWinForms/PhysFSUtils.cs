using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EditorWinForms
{
    public static class PhysFSUtils
    {
        public static IEnumerable<string> MakeRelative(string path, string basePath, bool dropLastElement = false)
        {
            var baseDirElements = basePath.Split(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
            var pathElements = path.Split(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
            if (pathElements.Length != 0)
            {
                int i = 0;
                for (; i < pathElements.Length && i < baseDirElements.Length; ++i)
                {
                    if (pathElements[i] != baseDirElements[i])
                        break;
                }
                if (dropLastElement)
                    return pathElements.Take(pathElements.Length - 1).Skip(i);
                else
                    return pathElements.Skip(i);
            }
            else
                return pathElements.AsEnumerable();
        }

        public static string MakeRelativeString(string path, string basePath, bool dropLastElement = false)
        {
            return string.Join("/", MakeRelative(path, basePath, dropLastElement));
        }
    }
}
