namespace UserGraph.Types;

[ExtendObjectType<User>]
public static class UserExtensions
{
    [BindMember(nameof(User.OwnedPaintingIds))]
    public static IEnumerable<Painting> GetPaintings([Parent] User user)
    {
        return user.OwnedPaintingIds.Select(x =>
            new Painting() { Id = x });
    }
}