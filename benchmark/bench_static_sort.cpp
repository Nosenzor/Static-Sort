//
// Created by Romain Nosenzo on 15/11/2025.
//

#include <benchmark/benchmark.h>
#include <array>
#include "../include/static_sort.h"

//Benchmark for StaticSort and StaticTimSort with 2 to 9 elements against std::sort
// Example usage of StaticTimSort for 3 elements (similar for StaticSort):
// StaticTimSort<3> timBoseNelsonSort;
// timBoseNelsonSort(first, last, fCompareEdge);



#include <benchmark/benchmark.h>
#include <array>
#include <algorithm>
#include <random>
#include "../include/static_sort.h"

// Générateur de données aléatoires
template <size_t N>
std::array<int, N> generate_random_array() {
    static std::mt19937 gen(42);
    static std::uniform_int_distribution<int> dis(0, 1000);
    std::array<int, N> arr;
    for (auto& val : arr) {
        val = dis(gen);
    }
    return arr;
}

// Benchmark std::sort
template <size_t N>
static void BM_StdSort(benchmark::State& state) {
    for (auto _ : state) {
        auto arr = generate_random_array<N>();
        benchmark::DoNotOptimize(arr);
        std::sort(arr.begin(), arr.end());
        benchmark::DoNotOptimize(arr);
        benchmark::ClobberMemory();
    }
}

// Benchmark StaticSort
template <size_t N>
static void BM_StaticSort(benchmark::State& state) {
    StaticSort<N> sorter;
    for (auto _ : state) {
        auto arr = generate_random_array<N>();
        benchmark::DoNotOptimize(arr);
        sorter(arr);
        benchmark::DoNotOptimize(arr);
        benchmark::ClobberMemory();
    }
}

// Benchmark StaticTimSort
template <size_t N>
static void BM_StaticTimSort(benchmark::State& state) {
    StaticTimSort<N> sorter;
    for (auto _ : state) {
        auto arr = generate_random_array<N>();
        benchmark::DoNotOptimize(arr);
        sorter(arr);
        benchmark::DoNotOptimize(arr);
        benchmark::ClobberMemory();
    }
}

// Enregistrement des benchmarks pour N = 2 à 8
BENCHMARK(BM_StdSort<2>);
BENCHMARK(BM_StaticSort<2>);
BENCHMARK(BM_StaticTimSort<2>);

BENCHMARK(BM_StdSort<3>);
BENCHMARK(BM_StaticSort<3>);
BENCHMARK(BM_StaticTimSort<3>);

BENCHMARK(BM_StdSort<4>);
BENCHMARK(BM_StaticSort<4>);
BENCHMARK(BM_StaticTimSort<4>);

BENCHMARK(BM_StdSort<5>);
BENCHMARK(BM_StaticSort<5>);
BENCHMARK(BM_StaticTimSort<5>);

BENCHMARK(BM_StdSort<6>);
BENCHMARK(BM_StaticSort<6>);
BENCHMARK(BM_StaticTimSort<6>);

BENCHMARK(BM_StdSort<7>);
BENCHMARK(BM_StaticSort<7>);
BENCHMARK(BM_StaticTimSort<7>);

BENCHMARK(BM_StdSort<8>);
BENCHMARK(BM_StaticSort<8>);
BENCHMARK(BM_StaticTimSort<8>);

BENCHMARK_MAIN();
