namespace EventsDemo;

public class JobRunner(IJobStorage storage)
{
    public async Task RunAsync()
    {
        while (true)
        {
            var job = await GetNextJobAsync();
            Console.WriteLine($"[job-runner]: Running job {job.Id}");
            await Task.Delay(1000);
            // if (await storage.ClaimJob(this, job))
            //     job.Run(); // some deep magic here, but also not important for this discussion
        }
    }

    private async Task<Job> GetNextJobAsync()
    {
        while (true)
        {
            // var jobs = await storage.GetJobs(new() { Status = Scheduled });
            // if (jobs.FirstOrDefault(j => j.ScheduledTime is null || j.ScheduledTime < Now) is { } j)
            //     return j;
            //
            // var nextTimeStamp = jobs.Concat(_scheduledJobs)
            //     .OrderBy(j => j.ScheduledTime)
            //     .FirstOrDefault();

            var job = await storage.WaitForJobAsync();

            // if (task == <wake on newJob>)
            //     continue; // restart, because nw job may be scheduled or unscheduled

            // if (nextTimeStamp.IsScheduled)
            // {
            //     await storage.EnqueueJob(transformToRegularJob);
            //     continue;
            // }

            return job;
        }
    }
}