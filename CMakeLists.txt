cmake_minimum_required(VERSION 3.15)
project(StaticSort VERSION 1.0.0 LANGUAGES CXX)

# Définir le standard C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimisations pour les benchmarks
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Bibliothèque header-only
add_library(static_sort INTERFACE)
target_include_directories(static_sort INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Télécharger et compiler Google Benchmark avec FetchContent
include(FetchContent)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Enable testing of the benchmark library." FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "Enable installation of benchmark." FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Enable Google Test unit tests." FORCE)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)
FetchContent_MakeAvailable(benchmark)

# Build benchmark executable
add_executable(bench_static_sort benchmark/bench_static_sort.cpp)
target_link_libraries(bench_static_sort PRIVATE static_sort benchmark::benchmark)
target_compile_options(bench_static_sort PRIVATE -Wall)

# Build test executable
add_executable(test_correctness test/test_correctness.cpp)
target_link_libraries(test_correctness PRIVATE static_sort)
target_compile_options(test_correctness PRIVATE -Wall)

