cmake_minimum_required(VERSION 3.14)

project(libfby VERSION 1.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if(MSVC)
    add_compile_options(/permissive- /W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(spsc_queue_benchmark bench/spsc_queue_bench.cpp)
target_include_directories(spsc_queue_benchmark PRIVATE include)