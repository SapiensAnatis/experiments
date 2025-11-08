var builder = WebApplication.CreateBuilder(args);

builder.Services
    .AddGraphQLServer()
    .AddPaintingGraphTypes();

var app = builder.Build();

app.MapGraphQL();

app.RunWithGraphQLCommands(args);