namespace CsvParser.SourceGenerator.Tests;

public static class GeneratorTestHelper
{
    public static GeneratorDriverRunResult RunGenerator<TGenerator>(string source)
        where TGenerator : IIncrementalGenerator, new()
    {
        var syntaxTree = CSharpSyntaxTree.ParseText(source);

        var compilation = CSharpCompilation.Create(
            assemblyName: "Tests",
            syntaxTrees: [syntaxTree],
            references:
            [
                .. Basic.Reference.Assemblies.Net80.References.All,
                MetadataReference.CreateFromFile("./CsvParser.Library.dll"), 
                MetadataReference.CreateFromFile("./Sep.dll"), 
            ],
            options: new(
                outputKind: OutputKind.DynamicallyLinkedLibrary
            )
        );

        var generator = new TGenerator();

        var driver = CSharpGeneratorDriver
            .Create(generator)
            .RunGeneratorsAndUpdateCompilation(
                compilation,
                out var outputCompilation,
                out var diagnostics
            );

        var result = driver.GetRunResult();
        
        Assert.Empty(
            outputCompilation
                .GetDiagnostics()
                .Where(d => d.Severity is DiagnosticSeverity.Error or DiagnosticSeverity.Warning)
        );
        
        Assert.Empty(diagnostics);
        return result;
    }
}