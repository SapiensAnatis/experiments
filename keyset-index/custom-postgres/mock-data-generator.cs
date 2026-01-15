/*

Vibe coded with this prompt:

I have this database schema:

CREATE TABLE logs(
    id PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    time TIMESTAMP WITH TIME ZONE NOT NULL,
    message VARCHAR(255) NOT NULL
);

Write me a C# single-file program to generate a CSV file with mock data for this, which accepts as its first command-line argument the number of rows to generate. It should output to mock_data.csv 

*/

using System;
using System.IO;
using System.Globalization;

if (args.Length == 0 || !int.TryParse(args[0], out int rowCount))
{
    Console.WriteLine("Usage: dotnet run -- <number_of_rows>");
    return;
}

string filePath = "mock_data.csv";
string[] messages = { "User logged in", "API request failed", "System update started", "Database backup completed", "Cache cleared" };
Random rng = new Random();

try
{
    using (StreamWriter writer = new StreamWriter(filePath))
    {
        // Writing the CSV Header
        writer.WriteLine("id,time,message");

        for (int i = 1; i <= rowCount; i++)
        {
            // Generating mock data
            int id = i;
            // Generates a timestamp within the last 30 days
            DateTimeOffset timestamp = DateTimeOffset.UtcNow.AddSeconds(-rng.Next(0, 2592000));
            string message = messages[rng.Next(messages.Length)];

            // Formatting time as ISO 8601 for PostgreSQL/Standard SQL compatibility
            string formattedTime = timestamp.ToString("yyyy-MM-dd HH:mm:ss.fffK", CultureInfo.InvariantCulture);

            // Write the line
            writer.WriteLine($"{id},{formattedTime},\"{message}\"");
            
            if (i % 10000 == 0) Console.WriteLine($"Generated {i} rows...");
        }
    }

    Console.WriteLine($"\nSuccess! {rowCount} rows written to {filePath}");
}
catch (Exception ex)
{
    Console.WriteLine($"An error occurred: {ex.Message}");
}