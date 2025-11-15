#!/bin/bash

# Comprehensive benchmark script for Static-Sort
# Generates JSON output for analysis

set -e

BUILD_DIR="build"
OUTPUT_DIR="benchmark_results"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

echo "═══════════════════════════════════════════════"
echo "   Static-Sort Comprehensive Benchmark Suite"
echo "═══════════════════════════════════════════════"
echo ""

# Create output directory
mkdir -p "${OUTPUT_DIR}"

# Build in Release mode
echo "Building in Release mode..."
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"
cmake -DCMAKE_BUILD_TYPE=Release .. > /dev/null 2>&1
cmake --build . --config Release -j$(sysctl -n hw.ncpu 2>/dev/null || echo 4) > /dev/null 2>&1
cd ..

echo "✓ Build complete"
echo ""

# Run benchmarks
echo "Running benchmarks..."
echo ""

# 1. Random data benchmark
echo "1️⃣  Random data (worst case for sorted algorithms)"
echo "   ────────────────────────────────────────────"
"${BUILD_DIR}/bench_static_sort" \
    --benchmark_filter="Random" \
    --benchmark_min_time=1s \
    --benchmark_format=json \
    --benchmark_out="${OUTPUT_DIR}/random_${TIMESTAMP}.json" \
    2>/dev/null | grep -E "BM_.*<[0-9]+>" | head -21

echo ""

# 2. Sorted data benchmark (TimSort best case)
echo "2️⃣  Sorted data (best case for TimSort)"
echo "   ────────────────────────────────────────────"
"${BUILD_DIR}/bench_static_sort" \
    --benchmark_filter="Sorted" \
    --benchmark_min_time=1s \
    --benchmark_format=json \
    --benchmark_out="${OUTPUT_DIR}/sorted_${TIMESTAMP}.json" \
    2>/dev/null | grep -E "BM_.*<[0-9]+>"

echo ""

# 3. Reversed data benchmark
echo "3️⃣  Reversed data"
echo "   ────────────────────────────────────────────"
"${BUILD_DIR}/bench_static_sort" \
    --benchmark_filter="Reversed" \
    --benchmark_min_time=1s \
    --benchmark_format=json \
    --benchmark_out="${OUTPUT_DIR}/reversed_${TIMESTAMP}.json" \
    2>/dev/null | grep -E "BM_.*<[0-9]+>"

echo ""
echo "═══════════════════════════════════════════════"
echo "✓ Benchmarks complete!"
echo ""
echo "Results saved to: ${OUTPUT_DIR}/"
echo ""
echo "Summary:"
echo "• random_${TIMESTAMP}.json    - Random data results"
echo "• sorted_${TIMESTAMP}.json    - Sorted data results"
echo "• reversed_${TIMESTAMP}.json  - Reversed data results"
echo ""
echo "To view full results with statistics:"
echo "  ${BUILD_DIR}/bench_static_sort --benchmark_repetitions=10"
echo ""

