namespace PaintingGraph.Types;

public class Painting
{
    [ID]
    public int Id { get; set; }
    
    public required string Title { get; set; }
    
    public required string Artist { get; set; }
    
    public int Year { get; set; }
}