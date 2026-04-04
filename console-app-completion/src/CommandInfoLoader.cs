using System.Runtime.Loader;
using ConsoleAppFramework;

namespace ConsoleAppCompletion;

public static class CommandInfoLoader
{
    public static CommandHelpDefinition[] GetHelpDefinitions(string assemblyPath)
    {
        var resolver = new AssemblyDependencyResolver(assemblyPath);

        var loadContext = new AssemblyLoadContext("Analysis", isCollectible: true);

        loadContext.Resolving += (ctx, name) =>
        {
            var path = resolver.ResolveAssemblyToPath(name);
            return path != null ? ctx.LoadFromAssemblyPath(path) : null;
        };

        var assembly = loadContext.LoadFromAssemblyPath(assemblyPath);

        var type = assembly.GetType("ConsoleAppFramework.ConsoleApp+ConsoleAppBuilder");

        if (type is null)
        {
            throw new CommandInfoLoaderException("The provided assembly does not contain a type called ConsoleAppFramework.ConsoleApp+ConsoleAppBuilder. Is it really a console app?");
        }

        var instance = Activator.CreateInstance(type);

        var method = type.GetMethod("GetCliSchema");

        if (method is null)
        {
            throw new CommandInfoLoaderException("Failed to find method GetCliSchema(). ConsoleAppFramework does not generate this unless ConsoleAppFramework.CliSchema is installed. Try installing this package and rebuilding the assembly.");
        }

        CommandHelpDefinition[] defs = (CommandHelpDefinition[])method.Invoke(instance, [])!;

        return defs;
    }
}
