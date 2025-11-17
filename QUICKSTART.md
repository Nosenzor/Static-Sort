# Quick Start Guide - Optimized Static-Sort

## 🚀 Installation

```bash
# Clone the repository
git clone <your-repo-url>
cd Static-Sort

# Build and run benchmarks
./run_benchmark.sh
```

## 📖 Usage Examples

### Basic Usage

```cpp
#include "static_sort.h"
#include <array>

// Sort a fixed-size array
std::array<int, 6> data = {5, 2, 8, 1, 9, 3};
StaticSort<6> sorter;
sorter(data);
// data is now: {1, 2, 3, 5, 8, 9}
```

### With Custom Comparator

```cpp
#include "static_sort.h"
#include <array>
#include <string>

std::array<std::string, 4> names = {"Charlie", "Alice", "Bob", "David"};
StaticSort<4> sorter;

// Sort in descending order
sorter(names, [](const auto& a, const auto& b) { return a > b; });
// names is now: {"David", "Charlie", "Bob", "Alice"}
```

### Using StaticTimSort for Pre-sorted Data

```cpp
#include "static_sort.h"
#include <array>

// Best for data that might be partially sorted
std::array<double, 10> data = {1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0};
StaticTimSort<10> sorter;
sorter(data);  // Extremely fast on already-sorted data!
```

### With Iterators

```cpp
#include "static_sort.h"
#include <vector>

std::vector<int> vec = {3, 1, 4, 1, 5};
StaticSort<5> sorter;
sorter(vec.begin(), vec.end());
```

### With C++20 Ranges

```cpp
#include "static_sort.h"
#include <array>

std::array<int, 8> data = {8, 7, 6, 5, 4, 3, 2, 1};
StaticSort<8> sorter;
sorter(data);  // Works directly with ranges!
```

## 🎯 When to Use What

### Use `StaticSort<N>`:
```cpp
// ✅ Random data
std::array<int, 7> random = {42, 17, 8, 93, 5, 71, 23};
StaticSort<7>()(random);

// ✅ Small fixed-size arrays (N ≤ 8)
std::array<float, 4> small = {3.14f, 2.71f, 1.41f, 1.73f};
StaticSort<4>()(small);

// ✅ Need predictable performance
StaticSort<6>()(data);  // Always same number of comparisons
```

### Use `StaticTimSort<N>`:
```cpp
// ✅ Potentially sorted data
std::array<int, 10> likely_sorted = {1, 2, 3, 5, 4, 6, 7, 8, 9, 10};
StaticTimSort<10>()(likely_sorted);

// ✅ Real-world data (often has patterns)
std::array<double, 16> sensor_readings = {...};
StaticTimSort<16>()(sensor_readings);

// ✅ Need best-case optimization
if (data_might_be_sorted) {
    StaticTimSort<8>()(data);  // 34x faster if already sorted!
}
```

### Use `std::sort`:
```cpp
// ✅ Large arrays (N > 16)
std::vector<int> large(1000);
std::sort(large.begin(), large.end());

// ✅ Dynamic size
void sort_dynamic(std::vector<int>& vec) {
    std::sort(vec.begin(), vec.end());
}
```

## 🔥 Performance Tips

### 1. Enable Optimizations
```bash
# CMake
cmake -DCMAKE_BUILD_TYPE=Release ..

# Or manually
g++ -O3 -march=native -DNDEBUG your_code.cpp
clang++ -O3 -march=native -DNDEBUG your_code.cpp
```

### 2. Use Appropriate Type
```cpp
// ✅ GOOD: Trivial types benefit from branchless swap
std::array<int, 6> ints;
std::array<double, 6> doubles;
std::array<int*, 6> pointers;

// ⚠️ OK: Non-trivial types use move semantics
std::array<std::string, 6> strings;
std::array<std::vector<int>, 6> vectors;
```

### 3. Inline in Hot Paths
```cpp
// Mark your sorting function as inline if called in tight loops
inline void process_batch(std::array<int, 8>& batch) {
    StaticSort<8>()(batch);  // Will be fully inlined
    // ... process sorted batch
}
```

### 4. Constexpr for Compile-Time Sorting
```cpp
constexpr auto sorted_at_compile_time() {
    std::array<int, 5> data = {5, 2, 8, 1, 3};
    StaticSort<5>()(data);
    return data;
}

// Sorted at compile time! Zero runtime cost!
constexpr auto sorted = sorted_at_compile_time();
```

## 📊 Benchmark Your Code

### Quick Benchmark
```bash
./build/bench_static_sort --benchmark_filter="<6>"
```

### Comprehensive Benchmark
```bash
./benchmark_comprehensive.sh
```

### Custom Benchmark
```bash
# Run 10 iterations with statistics
./build/bench_static_sort --benchmark_repetitions=10

# Export results to JSON
./build/bench_static_sort --benchmark_format=json --benchmark_out=results.json

# Filter specific tests
./build/bench_static_sort --benchmark_filter="StaticSort.*Random"
```

## 🐛 Common Pitfalls

### ❌ Wrong Size
```cpp
std::array<int, 5> data = {1, 2, 3, 4, 5};
StaticSort<6>()(data);  // ❌ WRONG! Size mismatch!
```

### ✅ Correct
```cpp
std::array<int, 5> data = {1, 2, 3, 4, 5};
StaticSort<5>()(data);  // ✅ Correct!
```

### ❌ Dynamic Size
```cpp
void sort_any_size(std::vector<int>& vec) {
    StaticSort<?>()(vec);  // ❌ Can't determine size at compile time!
}
```

### ✅ Correct
```cpp
template <size_t N>
void sort_fixed_size(std::array<int, N>& arr) {
    StaticSort<N>()(arr);  // ✅ Size known at compile time!
}
```

## 🎓 Advanced Usage

### Template Function
```cpp
template <size_t N, typename T>
void sort_and_process(std::array<T, N>& data) {
    StaticSort<N>()(data);
    // Process sorted data...
}
```

### Partial Specialization
```cpp
template <size_t N>
struct MyContainer {
    std::array<int, N> data;
    
    void sort() {
        if constexpr (N <= 8) {
            StaticSort<N>()(data);  // Fast for small N
        } else {
            std::sort(data.begin(), data.end());  // std::sort for large N
        }
    }
};
```

### Compile-Time Selection
```cpp
template <size_t N>
constexpr auto get_sorter() {
    if constexpr (N <= 8) {
        return StaticSort<N>{};
    } else {
        return [](auto& arr) { std::sort(arr.begin(), arr.end()); };
    }
}
```

## 📈 Expected Performance

| Array Size | Speedup vs std::sort | Use Case |
|------------|---------------------|----------|
| 2-3        | 3-7%                | Pairs, RGB colors |
| 4-5        | 7-9%                | RGBA colors, quaternions |
| 6-8        | 8-12%               | Small particle systems |
| 8 (sorted) | **34x** (TimSort)   | Time-series data |
| 16 (sorted)| **24x** (TimSort)   | Small buffers |

## 🎯 Real-World Use Cases

### Graphics: Sort 3D Points
```cpp
struct Point3D { float x, y, z; };
std::array<Point3D, 8> vertices = {...};
StaticSort<8>()(vertices, [](const auto& a, const auto& b) {
    return a.z < b.z;  // Depth sort for rendering
});
```

### Gaming: Sort Nearby Entities
```cpp
std::array<Entity*, 6> nearby_entities = find_nearby();
StaticSort<6>()(nearby_entities, [](auto* a, auto* b) {
    return distance_squared(a) < distance_squared(b);
});
```

### Data Processing: Sort Sensor Data
```cpp
std::array<double, 10> sensor_readings = read_sensors();
StaticTimSort<10>()(sensor_readings);  // Often partially sorted
auto median = sensor_readings[5];
```

---

**For more details, see:**
- [PERFORMANCE.md](PERFORMANCE.md) - Detailed performance analysis
- [README.md](README.md) - Project overview

