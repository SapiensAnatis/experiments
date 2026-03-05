find_package(grpc CONFIG REQUIRED)

add_library(grpc_coro_lib include/grpc_coro_lib/coro_server_write_reactor.h)

target_link_libraries(grpc_coro_lib PUBLIC gRPC::grpc gRPC::grpc++)

set_target_properties(grpc_coro_lib PROPERTIES LINKER_LANGUAGE CXX)

target_compile_options(grpc_coro_lib PRIVATE -fsanitize=address)
target_link_options(grpc_coro_lib PRIVATE -fsanitize=address)

target_compile_options(grpc_coro_lib PRIVATE -Og -g)
target_compile_options(grpc_coro_lib PRIVATE -Wall -Wextra -Werror -Wtrampolines)

target_include_directories(grpc_coro_lib INTERFACE include)