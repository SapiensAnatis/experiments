// See https://aka.ms/new-console-template for more information

using CsvHelper.Generated;
using CsvParser.Library;
using CsvParser.Models;

[assembly: CsvSerializable(typeof(CsvParser.Models.MyClass2))]
[assembly: CsvSerializable(typeof(CsvParser.Models.MyClass3))]
[assembly: CsvSerializable(typeof(CsvParser.Models.MyClass4))]

string csv = CsvSerializer.Serialize([
    new MyClass2()
    {
        String = "b",
        Integer = 4,
        SubProperty = new()
        {
            Long = 4,
            SubSubProperty = new()
            {
                String = "Tada"
            }
        }
    }
]);

Console.WriteLine(csv);

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

        public MyClass4 SubSubProperty { get; init; }
    }

    public class MyClass4
    {
        public string String { get; init; }
    }
}