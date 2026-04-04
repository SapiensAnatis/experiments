namespace UserGraph.Types;

public class User
{
    [ID]
    public int Id { get; set; }
    
    public required string Name { get; set; }
    
    public required List<int> OwnedPaintingIds { get; set; }
}