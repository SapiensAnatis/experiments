namespace CsvParser.SourceGenerator.Tests;

public class CsvSerializationGeneratorTests
{
    [Fact]
    public void GenerateSerializationMethod()
    {
        var result = GeneratorTestHelper.RunGenerator<CsvSerializationGenerator>(
            """
            using CsvParser.Library;
            
            [assembly: CsvSerializable(typeof(CsvParser.Models.MyClass2))]
            [assembly: CsvSerializable(typeof(CsvParser.Models.MyClass3))]
            
            namespace CsvParser.Models
            {
                public class MyClass2
                {
                    public string String { get; init; }
            
                    public int Integer { get; init; }
            
                    public MyClass3 SubProperty { get; init; }
                }
            
                public class MyClass3
                {
                    public long Long { get; init; }
                }
            }
            """);
    }
}