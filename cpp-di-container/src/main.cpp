#include "CakeProtocol.hpp"
#include "CakeService.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "ServiceContainer.hpp"


#include <iostream>

int main() {
    ServiceCollection collection;

    std::cout << "Registering services...\n";

    collection.RegisterService(CakeProtocol::Create);
    collection.RegisterService(CakeService::Create);
    collection.RegisterService(Server::Create);
    collection.RegisterService<Logger>();

    std::cout << "Building service provider...\n";

    auto prov = collection.BuildServiceProvider();

    const auto *server = prov.GetService<Server>();
    server->DoThing();

    std::cout << "Destroying service provider...\n";
}
