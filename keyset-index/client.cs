// https://github.com/dotnet/sdk/issues/52285
#:property PublishAot=false
#:package Npgsql@10.0.1

using System.Diagnostics;
using Npgsql;
using NpgsqlTypes;

string command = """
SELECT id, time, message FROM logs
WHERE (
    (
        $1 is NULL OR $2 is NULL OR
        (time, id) < ($1, $2)
    ) AND (
        $3 is NULL OR time > $3
    ) AND (
        $4 is NULL OR time < $4
    )
)
ORDER BY time DESC, id DESC
LIMIT $5
""";

NpgsqlParameter[] parameters = [
    // last date
    new() { Value = DateTimeOffset.Parse("2025-06-01T00:00:00Z"), NpgsqlDbType = NpgsqlDbType.TimestampTz },

    // last ID
    new() { Value = 500000, NpgsqlDbType = NpgsqlDbType.Integer  },

    // start date
    new() { Value = DBNull.Value, NpgsqlDbType = NpgsqlDbType.TimestampTz },

    // end date
    new() { Value = DBNull.Value, NpgsqlDbType = NpgsqlDbType.TimestampTz },

    // page size
    new() { Value = 100, NpgsqlDbType = NpgsqlDbType.Integer },
];

try
{
    Console.WriteLine("Connecting to database...");

    await MeasureQueryAsync(command, parameters);

    Console.WriteLine($"Query finished.");
}
catch (Exception ex)
{
    Console.WriteLine($"Error: {ex.Message}");
}


static async Task MeasureQueryAsync(string command, IEnumerable<NpgsqlParameter> parameters)
{
    string connectionString = "Host=localhost;Port=5432;Username=postgres;Password=password;Database=postgres";

    // High-precision timing
    Stopwatch sw = new Stopwatch();

    await using var conn = new NpgsqlConnection(connectionString);
    await using var cmd = new NpgsqlCommand(command, conn);

    // Add parameters dynamically
    foreach (var param in parameters)
    {
        cmd.Parameters.Add(param);
    }

    // Open connection (we don't time the handshake, usually we only want query execution time)
    await conn.OpenAsync();

    await using var explain = new NpgsqlCommand("SET auto_explain.log_min_duration = 0", conn);
    await explain.ExecuteNonQueryAsync();

    Console.WriteLine($"Executing: {command}");

    sw.Start();

    // EXECUTE
    // ExecuteScalarAsync is used here for single values (like COUNT). 
    // Use ExecuteNonQueryAsync() for INSERT/UPDATE/DELETE.
    // Use ExecuteReaderAsync() for retrieving rows.
    object? result = await cmd.ExecuteNonQueryAsync();

    sw.Stop();

    // Report timing
    double ms = sw.Elapsed.TotalMilliseconds;
    Console.WriteLine($"--------------------------------------------------");
    Console.WriteLine($"Execution Time : {ms:F4} ms");
    Console.WriteLine($"Ticks          : {sw.ElapsedTicks}");
    Console.WriteLine($"--------------------------------------------------");
}