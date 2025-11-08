//
// Created by jay on 14/10/25.
//
#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "GreeterService.h"

static const std::string ServerAddress = "0.0.0.0:50051";

int main() {
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();


    grpc::ServerBuilder builder;
    builder.AddListeningPort(ServerAddress, grpc::InsecureServerCredentials());

    GreeterService greeterService;
    builder.RegisterService(&greeterService);

    const std::unique_ptr<grpc::Server> server = builder.BuildAndStart();

    std::cout << "Server listening on: " << ServerAddress << std::endl;

    server->Wait();
}
