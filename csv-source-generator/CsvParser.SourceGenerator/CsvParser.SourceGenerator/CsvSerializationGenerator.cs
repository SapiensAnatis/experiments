using System.Collections.Immutable;

namespace CsvParser.SourceGenerator;

[Generator]
public class CsvSerializationGenerator : IIncrementalGenerator
{
    public void Initialize(IncrementalGeneratorInitializationContext context)
    {
        var values = context.SyntaxProvider
            .ForAttributeWithMetadataName("CsvParser.Library.CsvSerializableAttribute",
                static (_, _) => true,
                static (context, token) => TransformDeclaration(context, token).ToEquatableReadOnlyList()
            )
            .SelectMany(static (x, _) => x)
            .Collect();

        context.RegisterSourceOutput(values, WriteSource);
    }

    private static IEnumerable<CsvSerializableDeclaration> TransformDeclaration(
        GeneratorAttributeSyntaxContext context,
        CancellationToken cancellationToken
    )
    {
        foreach (AttributeData attribute in context.Attributes)
        {
            cancellationToken.ThrowIfCancellationRequested();

            if (attribute.ConstructorArguments is not [{ Value: INamedTypeSymbol type }])
            {
                continue;
            }

            foreach (var member in type.GetTypeMembers())
            {
                cancellationToken.ThrowIfCancellationRequested();
            }

            var props = type
                .GetMembers()
                .OfType<IPropertySymbol>()
                .Where(x => x.DeclaredAccessibility == Accessibility.Public && x.Kind == SymbolKind.Property)
                .Select(Property.FromSymbol)
                .ToEquatableReadOnlyList();

            yield return new(
                TypeName.FromSymbol(type),
                props
            );
        }
    }

    private static void WriteSource(
        SourceProductionContext context,
        ImmutableArray<CsvSerializableDeclaration> declarations
    )
    {
        CodeStringBuilder codeStringBuilder = new();

        codeStringBuilder.AppendLine(
            """
            namespace CsvHelper.Generated;

            public static class CsvSerializer
            {
                private delegate void PopulateRow(object input, global::nietras.SeparatedValues.SepWriter.Row row, string path = ""); 
                
            """);

        codeStringBuilder.IncreaseIndent();

        codeStringBuilder.AppendLine(
            """
            private static readonly global::System.Collections.Frozen.FrozenDictionary<
                global::System.Type, 
                PopulateRow
            > SerializerLookup;

            static CsvSerializer()
            {
                var serializerLookup = new global::System.Collections.Generic.Dictionary<
                    global::System.Type, 
                    PopulateRow
                >()
                {
            """);

        codeStringBuilder.IncreaseIndent();
        codeStringBuilder.IncreaseIndent();

        foreach (CsvSerializableDeclaration decl in declarations)
        {
            codeStringBuilder.AppendLine(
                $"[typeof({decl.TypeName.QualifiedName})] = {decl.TypeName.GetSerializeMethodName()},");
        }

        codeStringBuilder.DecreaseIndent();
        codeStringBuilder.AppendLine("};");
        codeStringBuilder.AppendLine();
        codeStringBuilder.AppendLine(
            "SerializerLookup = global::System.Collections.Frozen.FrozenDictionary.ToFrozenDictionary(serializerLookup);");

        codeStringBuilder.DecreaseIndent();
        codeStringBuilder.AppendLine("}");
        codeStringBuilder.AppendLine();

        foreach (CsvSerializableDeclaration decl in declarations)
        {
            GenerateSerializeMethod(codeStringBuilder, decl);
            codeStringBuilder.AppendLine();
        }

        codeStringBuilder.AppendLine(
            """
            public static string Serialize<T>(global::System.Collections.Generic.IEnumerable<T> input)
            {
                if (!SerializerLookup.TryGetValue(typeof(T), out var serializer))
                {
                    throw new global::System.NotSupportedException($"Cannot serialize type {typeof(T)}");
                }
                
                using var csvWriter = global::nietras.SeparatedValues.SepWriterExtensions.ToText(
                    global::nietras.SeparatedValues.Sep.Writer()
                );
                
                foreach (var item in input)
                {
                    using var writeRow = csvWriter.NewRow();
                    serializer(item, writeRow);
                }
                
                return csvWriter.ToString();
            }
            """);

        codeStringBuilder.DecreaseIndent();
        codeStringBuilder.AppendLine("}");

        context.AddSource("CsvSerializer.g.cs", codeStringBuilder.ToString());
    }

    private static void GenerateSerializeMethod(CodeStringBuilder codeStringBuilder, CsvSerializableDeclaration decl)
    {
        codeStringBuilder.AppendLine(
            $$"""
              private static void {{decl.TypeName.GetSerializeMethodName()}}(object input, global::nietras.SeparatedValues.SepWriter.Row row, string path = "")
              {
              """);
        codeStringBuilder.IncreaseIndent();
        codeStringBuilder.AppendLine(
            $"{decl.TypeName.QualifiedName} castedInput = ({decl.TypeName.QualifiedName})input;");
        codeStringBuilder.AppendLine();
        
        codeStringBuilder.AppendLine(
            """
            string pathPrefix = path.Length > 0 
                ? path + "/"
                : string.Empty;        
                                  
            """);

        foreach (var property in decl.Properties)
        {
            if (property.IsComplex)
            {
                codeStringBuilder.AppendLine(
                    $"{property.TypeName.GetSerializeMethodName()}(castedInput.{property.Name}, row, pathPrefix + nameof({decl.TypeName.QualifiedName}.{property.Name}));");
            }
            else
            {
                string method = property.IsUseSet ? "Set" : "Format";
                codeStringBuilder.AppendLine(
                    $"""
                     row[pathPrefix + "{property.Name}"].{method}(castedInput.{property.Name});
                     """
                );
            }
        }

        codeStringBuilder.DecreaseIndent();
        codeStringBuilder.AppendLine("}");
    }
}