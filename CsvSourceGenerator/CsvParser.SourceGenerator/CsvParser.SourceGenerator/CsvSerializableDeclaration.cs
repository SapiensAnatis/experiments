using System.Collections.Frozen;

namespace CsvParser.SourceGenerator;

internal sealed record CsvSerializableDeclaration(
    TypeName TypeName,
    EquatableReadOnlyList<Property> Properties);

internal sealed record TypeName(string QualifiedName, string ShortTypeName)
{
    private static readonly SymbolDisplayFormat ShortTypeNameFormat = new(
        SymbolDisplayGlobalNamespaceStyle.Omitted,
        SymbolDisplayTypeQualificationStyle.NameAndContainingTypesAndNamespaces
    );

    public static TypeName FromSymbol(ITypeSymbol symbol)
    {
        return new(symbol.ToDisplayString(SymbolDisplayFormat.FullyQualifiedFormat),
            symbol.ToDisplayString(ShortTypeNameFormat).Replace(".", ""));
    }
    
    public string GetSerializeMethodName() => $"Serialize_{this.ShortTypeName}";
    
}

internal sealed record Property(string Name, TypeName TypeName, bool IsUseSet, bool IsComplex)
{
    private static readonly FrozenSet<string> KnownPrimitiveTypes = new List<string>()
    {
        "String",
        "Byte",
        "Int16",
        "Int32",
        "Int64",
        "Guid"
    }.ToFrozenSet();
    
    public static Property FromSymbol(IPropertySymbol symbol)
    {
        bool isUseSet = symbol.Type.SpecialType == SpecialType.System_String;
        bool isComplex = !KnownPrimitiveTypes.Contains(symbol.Type.MetadataName);

        return new(symbol.Name, TypeName.FromSymbol(symbol.Type), isUseSet, isComplex);
    }
}