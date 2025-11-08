
cmake_minimum_required(VERSION 3.31)
project(cpp_di_container)

set(CMAKE_CXX_STANDARD 20)

add_executable(cpp_di_container src/main.cpp
    src/Logger.hpp
    src/CakeService.hpp
    src/Server.hpp
    src/CakeProtocol.hpp
    src/ServiceContainer.hpp
)

target_compile_options(cpp_di_container PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Werror" "-O0" "-g")
