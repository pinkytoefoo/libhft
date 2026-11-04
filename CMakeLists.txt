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


# --- BENCHMARKS ---
add_executable(spsc_queue_benchmark bench/spsc_queue_bench.cpp)
target_include_directories(spsc_queue_benchmark PRIVATE include)


# --- TESTS ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(tests test/all.cpp)
target_include_directories(tests PRIVATE include GoogleTest)
target_link_libraries(tests PRIVATE  GTest::gtest_main)
