using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using ConsoleAppFramework;

namespace ConsoleAppCompletion;

public static class CompletionScriptBuilder
{
    public static string BuildCompletionScript(string baseProgramName, CommandHelpDefinition[] defs)
    {
        StringBuilder sb = new StringBuilder();

        Dictionary<string, HashSet<string>> completionDict = BuildCompletionsDict(defs);

        string functionName = $"__start_{baseProgramName}";

        Dictionary<string, List<string>> completionsByCommand = [];

        sb.AppendLine($"function {functionName}() {{");
        sb.AppendLine($"  local cur=\"${{COMP_WORDS[COMP_CWORD]}}\"");
        sb.AppendLine($"  local cmd=\"${{COMP_WORDS[*]:1:COMP_CWORD-1}}\"");
        sb.AppendLine($"  case \"$cmd\" in");

        // Case matches top to bottom - put longest context strings first
        List<KeyValuePair<string, HashSet<string>>> sorted = completionDict.OrderByDescending(x => x.Key.Length).ToList();
        foreach (var (context, options) in sorted)
        {
            string optionsJoined = string.Join(" ", options);
            sb.AppendLine($"    \"{context}\")");
            sb.AppendLine($"      COMPREPLY=($(compgen -W \"{optionsJoined}\" -- \"$cur\"))");
            sb.AppendLine($"      ;;");
        }

        sb.AppendLine($"  esac");
        sb.AppendLine($"}}");

        sb.AppendLine();
        sb.AppendLine($"complete -o default -F {functionName} {baseProgramName}");

        return sb.ToString();
    }

    private static Dictionary<string, HashSet<string>> BuildCompletionsDict(CommandHelpDefinition[] defs)
    {
        Dictionary<string, HashSet<string>> result = [];

        HashSet<string> GetCompletionsSet(string context)
        {
            ref HashSet<string>? list = ref CollectionsMarshal.GetValueRefOrAddDefault(result, context, out bool exists);

            if (!exists)
            {
                list = new();
            }

            if (list == null)
            {
                throw new UnreachableException("exists was true but list null - null list added to dict?");
            }

            return list;
        }

        foreach (var def in defs)
        {
            // Add completions for this command and any subcommands

            string[] tokens = def.CommandName.Split(" ");
            for (int i = 0; i < tokens.Length; i++)
            {
                // TODO: less string.split/join -> fewer allocations

                string opt = tokens[i];
                string context = string.Join(" ", tokens[0..i]);

                // For deeply nested hierarchies e.g. binary subcommand1 subcommand2 & binary subcommand1 subcommand2, this approach will add subcommand1 to the completions for "" twice,
                // use a set to get the unique values (even though compgen doesn't really care).

                HashSet<string> completions = GetCompletionsSet(context);
                completions.Add(opt);
            }

            // Add completions for options

            HashSet<string> fullCommandCompletions = GetCompletionsSet(def.CommandName);

            IEnumerable<string[]> commandOptions = def.Options.Where(x =>
                x.Index == null && x.Options.Length > 0 // Filter out positional arguments & ensure command has options
            ).Select(x => x.Options);

            foreach (var options in commandOptions)
            {
                // Do not add aliases of options - we assume that the longest option name is the 'canonical' one that should be suggested; this
                // assumes that options are only aliased to values shorter than the original name.
                string? longestOption = options.MaxBy(x => x.Length);
                if (longestOption is null)
                {
                    throw new UnreachableException("MaxBy returned null for non-empty array");
                }

                fullCommandCompletions.Add(longestOption);
            }
        }

        return result;
    }
}
