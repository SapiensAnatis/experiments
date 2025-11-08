// See https://aka.ms/new-console-template for more information

using EventsDemo;

var storage = new PostgresJobStorage();
await storage.InitialiseAsync();

var notifyThread = new Thread(async void () =>
{
    try
    {
        for (int i = 0; i <= 10; i++)
        {
            await storage.EnqueueAsync(new Job(i, $"job_{i}"));
            Console.WriteLine($"[job-writer]: Enqueued job {i}");
            
            Thread.Sleep(250);
        }
    }
    catch (Exception e)
    {
        // async void shenanigans, not ideal but 1) demo, 2) we are perma blocking the main thread
        Console.Error.WriteLine($"[job-writer]: Failed to enqueue job: {e}");
    }
});

notifyThread.Start();

var jobRunner = new JobRunner(storage);

await jobRunner.RunAsync();