//
// Created by Romain Nosenzo on 15/11/2025.
//

#include <benchmark/benchmark.h>
#include <array>
#include <algorithm>
#include <random>
#include "../include/static_sort.h"

// Générateur de données aléatoires
template <size_t N>
std::array<double, N> generate_random_array() {
    static std::mt19937 gen(42);
    static std::uniform_real_distribution<double> dis(-1000., 1000.);
    std::array<double, N> arr;
    for (auto& val : arr) {
        val = dis(gen);
    }
    return arr;
}

// Générateur de données triées
template <size_t N>
std::array<double, N> generate_sorted_array() {
    std::array<double, N> arr;
    for (size_t i = 0; i < N; ++i) {
        arr[i] = static_cast<double>(i);
    }
    return arr;
}

// Générateur de données inversées
template <size_t N>
std::array<double, N> generate_reversed_array() {
    std::array<double, N> arr;
    for (size_t i = 0; i < N; ++i) {
        arr[i] = static_cast<double>(N - 1 - i);
    }
    return arr;
}

// Générateur de données partiellement triées (pipe organ)
template <size_t N>
std::array<double, N> generate_pipe_organ_array() {
    std::array<double, N> arr;
    size_t mid = N / 2;
    for (size_t i = 0; i < mid; ++i) {
        arr[i] = static_cast<double>(i);
    }
    for (size_t i = mid; i < N; ++i) {
        arr[i] = static_cast<double>(N - i);
    }
    return arr;
}

// Benchmark std::sort - Random
template <size_t N>
static void BM_StdSort_Random(benchmark::State& state) {
    for (auto _ : state) {
        auto arr = generate_random_array<N>();
        benchmark::DoNotOptimize(arr);
        std::sort(arr.begin(), arr.end());
        benchmark::DoNotOptimize(arr);
        benchmark::ClobberMemory();
    }
}

// Benchmark StaticSort - Random
template <size_t N>
static void BM_StaticSort_Random(benchmark::State& state) {
    StaticSort<N> sorter;
    for (auto _ : state) {
        auto arr = generate_random_array<N>();
        benchmark::DoNotOptimize(arr);
        sorter(arr);
        benchmark::DoNotOptimize(arr);
        benchmark::ClobberMemory();
    }
}

// Benchmark StaticTimSort - Random
template <size_t N>
static void BM_StaticTimSort_Random(benchmark::State& state) {
    StaticTimSort<N> sorter;
    for (auto _ : state) {
        auto arr = generate_random_array<N>();
        benchmark::DoNotOptimize(arr);
        sorter(arr);
        benchmark::DoNotOptimize(arr);
        benchmark::ClobberMemory();
    }
}

// Benchmark StaticTimSort - Sorted (best case)
template <size_t N>
static void BM_StaticTimSort_Sorted(benchmark::State& state) {
    StaticTimSort<N> sorter;
    for (auto _ : state) {
        auto arr = generate_sorted_array<N>();
        benchmark::DoNotOptimize(arr);
        sorter(arr);
        benchmark::DoNotOptimize(arr);
        benchmark::ClobberMemory();
    }
}

// Benchmark StaticTimSort - Reversed
template <size_t N>
static void BM_StaticTimSort_Reversed(benchmark::State& state) {
    StaticTimSort<N> sorter;
    for (auto _ : state) {
        auto arr = generate_reversed_array<N>();
        benchmark::DoNotOptimize(arr);
        sorter(arr);
        benchmark::DoNotOptimize(arr);
        benchmark::ClobberMemory();
    }
}

// Enregistrement des benchmarks pour N = 2 à 8 - Random data
BENCHMARK(BM_StdSort_Random<2>);
BENCHMARK(BM_StaticSort_Random<2>);
BENCHMARK(BM_StaticTimSort_Random<2>);

BENCHMARK(BM_StdSort_Random<3>);
BENCHMARK(BM_StaticSort_Random<3>);
BENCHMARK(BM_StaticTimSort_Random<3>);

BENCHMARK(BM_StdSort_Random<4>);
BENCHMARK(BM_StaticSort_Random<4>);
BENCHMARK(BM_StaticTimSort_Random<4>);

BENCHMARK(BM_StdSort_Random<5>);
BENCHMARK(BM_StaticSort_Random<5>);
BENCHMARK(BM_StaticTimSort_Random<5>);

BENCHMARK(BM_StdSort_Random<6>);
BENCHMARK(BM_StaticSort_Random<6>);
BENCHMARK(BM_StaticTimSort_Random<6>);

BENCHMARK(BM_StdSort_Random<7>);
BENCHMARK(BM_StaticSort_Random<7>);
BENCHMARK(BM_StaticTimSort_Random<7>);

BENCHMARK(BM_StdSort_Random<8>);
BENCHMARK(BM_StaticSort_Random<8>);
BENCHMARK(BM_StaticTimSort_Random<8>);

// Benchmarks for sorted/reversed data (showing TimSort benefits)
BENCHMARK(BM_StaticTimSort_Sorted<8>);
BENCHMARK(BM_StaticTimSort_Reversed<8>);
BENCHMARK(BM_StaticTimSort_Sorted<16>);
BENCHMARK(BM_StaticTimSort_Reversed<16>);
BENCHMARK(BM_StaticSort_Random<16>);
BENCHMARK(BM_StaticTimSort_Random<16>);

BENCHMARK_MAIN();
