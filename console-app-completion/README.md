# Console app completion

Proof-of-concept for auto-generating Bash completions from apps written with [ConsoleAppFramework](https://github.com/Cysharp/ConsoleAppFramework).

In its current form, it loads a `.dll` assembly for a console app and invokes the `GetCliSchema` via reflection to introspect the commands.

## Usage

1. If you haven't already, add the `ConsoleAppFramework.CliSchema` package to your project; this causes ConsoleAppFramework to generate the `GetCliSchema` method that this tool relies on.
2. Invoke the tool with a path to the assembly and pipe the output to a script

```
dotnet run src/ -- /path/to/your/project/bin/Debug/net10.0/YourProject.dll > completions.sh
```

3. Source the completions manually or place them with other bash completions so that they are automatically sourced.

## Notes

The generated completions will invoke `complete` like this if run against `YourProject.dll`:

```
complete -o default -F __start_YourProject YourProject
```

If your tool's binary has a different name, or you add it to your `PATH` under an alias like `your-project`, then you will need to manually edit the completions to reflect this.

