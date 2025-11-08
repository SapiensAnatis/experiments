using System.Threading.Channels;
using Npgsql;

namespace EventsDemo;

public class PostgresJobStorage : IJobStorage
{
    private readonly NpgsqlConnection _writerConn;
    private readonly NpgsqlConnection _readerConn;

    private readonly SemaphoreSlim _readerConnSemaphore = new SemaphoreSlim(1, 1);

    private readonly Channel<Job> _channel;

    public PostgresJobStorage()
    {
        _writerConn = new NpgsqlConnection("Host=localhost;Username=postgres;Password=postgres");
        _readerConn = new NpgsqlConnection(_writerConn.ConnectionString);

        _channel = Channel.CreateUnbounded<Job>();
    }

    public async Task InitialiseAsync()
    {
        await _writerConn.OpenAsync();
        await _readerConn.OpenAsync();

        // Clear table to avoid PK conflicts
        await using var truncateCommand = new NpgsqlCommand("TRUNCATE TABLE jobs", _writerConn);
        await truncateCommand.ExecuteNonQueryAsync();

        _readerConn.Notification += async (sender, args) => { await OnReceiveJob(args); };

        await using var command = new NpgsqlCommand("LISTEN job_channel", _readerConn);
        await command.ExecuteNonQueryAsync();
    }

    public async Task EnqueueAsync(Job job)
    {
        await using var insertCommand =
            new NpgsqlCommand("INSERT INTO jobs (id, name) VALUES ($1, $2)", _writerConn);
        insertCommand.Parameters.Add(new() { Value = job.Id });
        insertCommand.Parameters.Add(new() { Value = job.Name });

        await insertCommand.ExecuteNonQueryAsync();

        await using var notifyCommand = new NpgsqlCommand("SELECT pg_notify('job_channel', $1)", _writerConn);
        notifyCommand.Parameters.Add(new()
        {
            Value = job.Id.ToString() // argument must be a string, or boom syntax error
        });

        await notifyCommand.ExecuteNonQueryAsync();
    }


    public async Task<Job> WaitForJobAsync()
    {
        Console.WriteLine("[job-runner]: Waiting for next job...");

        // If we have data ready, don't block in WaitAsync, as we'll then be waiting for the next NOTIFY which could
        // happen much later.
        //
        // Apply a short timeout in case of an empty channel so that we can react better to bursts of NOTIFY commands.
        // You can see the need for this if you remove the all of the sleeps -- this thread will probably get stuck 
        // in WaitAsync after running job #9, because it enters the call between when the last NOTIFY is issued and
        // when the last job is placed into the channel, so it waits for a NOTIFY that will never come.

        CancellationTokenSource cts = new CancellationTokenSource(TimeSpan.FromMilliseconds(250));

        try
        {
            if (await _channel.Reader.WaitToReadAsync(cts.Token) && _channel.Reader.TryRead(out var job))
            {
                return job;
            }
        }
        catch (OperationCanceledException)
        {
            // go into WaitAsync
        }

        // Wait for notification. Without calling WaitAsync, Npgsql will only process notifications whenever a command
        // happens to be sent by another part of the app.
        // This is mutexed as I had issues with it being used concurrently by OnReceiveJob and WaitForJobAsync
        await _readerConnSemaphore.WaitAsync();
        try
        {
            await _readerConn.WaitAsync();
        }
        finally
        {
            _readerConnSemaphore.Release();
        }

        // Wait for value from notification.
        return await _channel.Reader.ReadAsync();
    }

    private async Task OnReceiveJob(NpgsqlNotificationEventArgs args)
    {
        int id = int.Parse(args.Payload);
        
        // Use the ID provided in the notification to retrieve the job.
        // If desired, we could send enough information in the notification to avoid this (e.g. JSON payload?)
        
        await _readerConnSemaphore.WaitAsync();
        try
        {
            await using var getJobCommand =
                new NpgsqlCommand("SELECT id, name FROM jobs WHERE id = $1 LIMIT 1", _readerConn)
                {
                    Parameters =
                    {
                        new() { Value = id }
                    }
                };

            await using var reader = await getJobCommand.ExecuteReaderAsync();
            if (!await reader.ReadAsync())
            {
                Console.Error.WriteLine($"[job-runner]: Job {id} was not found");
                return;
            }

            var job = new Job(reader.GetInt32(0), reader.GetString(1));
            await _channel.Writer.WriteAsync(job);
        }
        finally
        {
            _readerConnSemaphore.Release();
        }
    }
}