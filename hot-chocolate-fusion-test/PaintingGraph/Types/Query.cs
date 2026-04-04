namespace PaintingGraph.Types;

[QueryType]
public static class Query
{
    private static readonly List<Painting> Paintings =
    [
        new()
        {
            Id = 1,
            Title = "Vitruvian Man",
            Year = 1487,
            Artist = "Leonardo da Vinci"
        },
        new()
        {
            Id = 2,
            Title = "The Night Watch",
            Year = 1642,
            Artist = "Rembrandt van Rijn"
        },
        new()
        {
            Id = 3,
            Title = "The Blue Boy",
            Year = 1770,
            Artist = "Thomas Gainsborough"
        }
    ];
    
    public static Painting GetPaintingById([ID] int id)
    {
        return Paintings.First(x => x.Id == id);
    }

    public static IEnumerable<Painting> GetPaintingsById([ID] List<int> ids)
    {
        return Paintings.Where(x => ids.Contains(x.Id));
    }
}