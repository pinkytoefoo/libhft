cmake_minimum_required(VERSION 3.14)

project(libhft VERSION 1.1 LANGUAGES CXX)

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
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(tests test/all.cpp)
target_include_directories(tests PRIVATE include GoogleTest)
target_link_libraries(tests PRIVATE  GTest::gtest_main)
