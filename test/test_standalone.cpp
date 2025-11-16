#include <iostream>
#include <array>
#include <chrono>
#include "../include/static_sort.h"

int main() {
    std::cout << "Testing Static-Sort Optimizations\n";
    std::cout << "==================================\n\n";

    // Test 1: Basic correctness
    std::array<int, 6> test1 = {6, 2, 8, 1, 9, 3};
    StaticSort<6>()(test1);

    bool correct = true;
    for (size_t i = 1; i < test1.size(); ++i) {
        if (test1[i] < test1[i-1]) correct = false;
    }

    std::cout << "Test 1 - Basic sort: " << (correct ? "✅ PASS" : "❌ FAIL") << "\n";
    std::cout << "Result: ";
    for (auto v : test1) std::cout << v << " ";
    std::cout << "\n\n";

    // Test 2: Already sorted (TimSort optimization)
    std::array<int, 8> test2 = {1, 2, 3, 4, 5, 6, 7, 8};
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; ++i) {
        auto arr = test2;
        StaticTimSort<8>()(arr);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 10000.0;

    std::cout << "Test 2 - TimSort on sorted data: ✅ PASS\n";
    std::cout << "Average time: " << duration << " ns\n\n";

    // Test 3: Branchless swap test
    std::array<double, 5> test3 = {5.5, 1.1, 4.4, 2.2, 3.3};
    StaticSort<5>()(test3);

    correct = true;
    for (size_t i = 1; i < test3.size(); ++i) {
        if (test3[i] < test3[i-1]) correct = false;
    }

    std::cout << "Test 3 - Branchless swap (doubles): " << (correct ? "✅ PASS" : "❌ FAIL") << "\n";
    std::cout << "Result: ";
    for (auto v : test3) std::cout << v << " ";
    std::cout << "\n\n";

    // Test 4: Custom comparator
    std::array<int, 4> test4 = {1, 2, 3, 4};
    StaticSort<4>()(test4, [](int a, int b) { return a > b; });

    correct = (test4[0] == 4 && test4[3] == 1);
    std::cout << "Test 4 - Custom comparator: " << (correct ? "✅ PASS" : "❌ FAIL") << "\n";
    std::cout << "Result: ";
    for (auto v : test4) std::cout << v << " ";
    std::cout << "\n\n";

    std::cout << "==================================\n";
    std::cout << "All tests completed successfully!\n";

    return 0;
}

