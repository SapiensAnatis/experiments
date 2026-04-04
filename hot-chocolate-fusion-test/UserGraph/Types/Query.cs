namespace UserGraph.Types;

[QueryType]
public static class Query
{
    private static readonly List<User> Users =
    [
        new()
        {
            Id = 1,
            Name = "Steven Ericsson",
            OwnedPaintingIds = [1, 3],
        },
        new()
        {
            Id = 2,
            Name = "David Preadly",
            OwnedPaintingIds = [2]
        }
    ];
    
    public static IEnumerable<User> GetUsers()
        => Users;
}