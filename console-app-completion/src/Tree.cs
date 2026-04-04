using System.Runtime.InteropServices;

namespace ConsoleAppCompletion;

public class TreeNode
{
    public Dictionary<string, TreeNode> Children { get; } = new();

    public TreeNode GetOrCreateNode(string name)
    {
        ref TreeNode? node = ref CollectionsMarshal.GetValueRefOrAddDefault(this.Children, name, out bool exists);

        if (!exists)
        {
            node = new TreeNode();
        }

        if (node is null)
        {
            throw new InvalidOperationException("Got null node!");
        }

        return node;
    }
}
