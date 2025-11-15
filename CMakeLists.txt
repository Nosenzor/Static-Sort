cmake_minimum_required(VERSION 3.15)
project(StaticSort VERSION 1.0.0 LANGUAGES CXX)

# Définir le standard C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Bibliothèque header-only
add_library(static_sort INTERFACE)
target_include_directories(static_sort INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Options de compilation pour les exemples
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(-O3 -Wall -Wextra)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/O2 /W4)
endif()

# Exemple principal
add_executable(example_main examples/main.cpp)
target_link_libraries(example_main PRIVATE static_sort)

# Benchmarks (optionnels)
option(BUILD_BENCHMARKS "Build benchmark executables" OFF)

if(BUILD_BENCHMARKS)
    add_executable(bench_6 examples/main.cpp)
    target_compile_definitions(bench_6 PRIVATE BENCH_6)
    target_link_libraries(bench_6 PRIVATE static_sort)

    add_executable(bench_10 examples/main.cpp)
    target_compile_definitions(bench_10 PRIVATE BENCH_10)
    target_link_libraries(bench_10 PRIVATE static_sort)

    add_executable(bench_n examples/main.cpp)
    target_compile_definitions(bench_n PRIVATE BENCH_N)
    target_link_libraries(bench_n PRIVATE static_sort)

    add_executable(bench_pair_sort examples/main.cpp)
    target_compile_definitions(bench_pair_sort PRIVATE BENCH_PAIR_SORT)
    target_link_libraries(bench_pair_sort PRIVATE static_sort)
endif()

# Installation (optionnelle)
option(INSTALL_STATIC_SORT "Install StaticSort library" OFF)

if(INSTALL_STATIC_SORT)
    include(GNUInstallDirs)

    install(TARGETS static_sort
            EXPORT StaticSortTargets
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(FILES include/static_sort.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(EXPORT StaticSortTargets
            FILE StaticSortTargets.cmake
            NAMESPACE StaticSort::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/StaticSort
    )
endif()

