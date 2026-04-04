var builder = WebApplication.CreateBuilder(args);

builder.Services
    .AddGraphQLServer()
    .AddUserGraphTypes();

var app = builder.Build();

app.MapGraphQL();

app.RunWithGraphQLCommands(args);