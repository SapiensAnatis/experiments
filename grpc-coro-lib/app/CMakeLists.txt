find_package(grpc CONFIG REQUIRED)

add_library(protos helloworld.proto)

target_link_libraries(protos
        PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)
target_include_directories(protos PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET protos LANGUAGE cpp)
protobuf_generate(TARGET protos LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

add_executable(grpc_coroutine_practice main.cpp
        GreeterService.cpp
        GreeterService.h
)

target_link_libraries(grpc_coroutine_practice PRIVATE gRPC::grpc++_reflection protos grpc_coro_lib)

target_compile_options(grpc_coroutine_practice PRIVATE -fsanitize=address)
target_link_options(grpc_coroutine_practice PRIVATE -fsanitize=address)

target_compile_options(grpc_coroutine_practice PRIVATE -Og -g)
target_compile_options(grpc_coroutine_practice PRIVATE -Wall -Wextra -Werror -Wtrampolines)
