using ConsoleAppCompletion;
using ConsoleAppFramework;

AppDomain.CurrentDomain.UnhandledException += (sender, e) =>
{
    Console.Error.WriteLine(e);
    Environment.Exit(1);
};

if (args.Length != 1)
{
    Console.Error.WriteLine("Wrong arguments");
    Environment.Exit(1);
}

string targetAssemblyPath = args[0];
string baseProgramName = Path.GetFileNameWithoutExtension(targetAssemblyPath);

CommandHelpDefinition[] defs = CommandInfoLoader.GetHelpDefinitions(targetAssemblyPath);

string script = CompletionScriptBuilder.BuildCompletionScript(baseProgramName, defs);

await File.WriteAllTextAsync("testing/generated_completion.sh", script);
