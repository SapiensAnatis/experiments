#include "GreeterService.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>

#include <memory>
#include <print>

static constexpr std::string_view ServerAddress = "127.0.0.1:50051";

int main() {
    const auto greeterService = std::make_unique<app::GreeterService>();

    grpc::ServerBuilder builder;
    builder.RegisterService(greeterService.get());
    builder.AddListeningPort(std::string(ServerAddress), grpc::InsecureServerCredentials());

    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    const auto server = builder.BuildAndStart();
    if (!server) {
        return EXIT_FAILURE;
    }

    std::println("Server listening on {}", ServerAddress);

    server->Wait();
}