using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace EditorWinForms
{
    internal static class ControlUtils
    {
        public static void InvokeLambda(this TreeView treeView, Action<TreeNode> action, TreeNode param)
        {
            treeView.Invoke(action, param);
        }

        public static void InvokeLambda(this TreeView treeView, Action action)
        {
            treeView.Invoke(action);
        }
    }
}
