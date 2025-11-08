namespace EventsDemo;

public interface IJobStorage
{
    public Task InitialiseAsync();

    public Task EnqueueAsync(Job job);

    public Task<Job> WaitForJobAsync();
}