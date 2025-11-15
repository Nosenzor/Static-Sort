#!/bin/bash

# Script to build and run benchmarks for Static-Sort

set -e

BUILD_DIR="build"
BUILD_TYPE="${1:-Release}"

echo "Building benchmarks in ${BUILD_TYPE} mode..."

# Create build directory if it doesn't exist
mkdir -p "${BUILD_DIR}"

# Configure CMake
cd "${BUILD_DIR}"
cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" ..

# Build
echo "Compiling..."
cmake --build . --config "${BUILD_TYPE}" -j$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

echo ""
echo "Running benchmarks..."
echo "===================="
./bench_static_sort --benchmark_min_time=0.5s

echo ""
echo "To run with custom options:"
echo "  ./bench_static_sort --benchmark_filter=StaticSort"
echo "  ./bench_static_sort --benchmark_format=json --benchmark_out=results.json"
echo "  ./bench_static_sort --benchmark_repetitions=10"

