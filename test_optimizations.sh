#!/bin/bash

set -e

echo "╔════════════════════════════════════════════════════════════╗"
echo "║     Testing Static-Sort Optimizations                     ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

cd /Users/Romain/CLionProjects/Static-Sort

# Clean build
echo "🧹 Cleaning build directory..."
rm -rf build
mkdir build
cd build

# Configure
echo "⚙️  Configuring CMake..."
cmake -DCMAKE_BUILD_TYPE=Release .. > /dev/null 2>&1

# Build
echo "🔨 Compiling..."
cmake --build . -j$(sysctl -n hw.ncpu) 2>&1 | grep -i "error" && exit 1 || true

echo "✅ Compilation successful!"
echo ""

# Run correctness tests
if [ -f ./test_correctness ]; then
    echo "🧪 Running correctness tests..."
    ./test_correctness
    echo ""
fi

# Run benchmarks
if [ -f ./bench_static_sort ]; then
    echo "📊 Running performance benchmarks..."
    ./bench_static_sort  --benchmark_min_time=0.3s 2>/dev/null | grep "BM_"
fi

echo ""
echo "╔════════════════════════════════════════════════════════════╗"
echo "║     All tests completed successfully! ✅                   ║"
echo "╚════════════════════════════════════════════════════════════╝"

