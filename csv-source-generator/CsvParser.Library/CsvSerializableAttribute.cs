namespace CsvParser.Library;

[AttributeUsage(AttributeTargets.Assembly, AllowMultiple = true)]
public sealed class CsvSerializableAttribute(Type type) : Attribute
{
    public Type Type { get; } = type;
}