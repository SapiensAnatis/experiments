cmake_minimum_required(VERSION 3.31)
project(cpp_udp_server)

set(CMAKE_CXX_STANDARD 23)

add_executable(cpp_udp_server main.cpp
)

include(FetchContent)
FetchContent_Declare(
    libcoro
    GIT_REPOSITORY https://github.com/jbaldwin/libcoro.git
    GIT_TAG 030e27c590d0d7fd9e24c2f40c7463dbdbba3857
)
FetchContent_MakeAvailable(libcoro)

target_link_libraries(cpp_udp_server PRIVATE libcoro)
