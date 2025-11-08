cmake_minimum_required(VERSION 3.31.6)

project(grpc-callback-api)

set(CMAKE_CXX_STANDARD 23)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_subdirectory(proto)


add_executable(grpc-callback-api main.cpp
        GreeterService.cpp
        Reactor.cpp
        ReactorPublisher.cpp)

target_link_libraries(grpc-callback-api PRIVATE protos gRPC::grpc++_reflection)
