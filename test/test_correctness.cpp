#include <iostream>
#include <array>
#include <cassert>
#include "../include/static_sort.h"

// Test helper
template<typename T, size_t N>
bool is_sorted(const std::array<T, N>& arr) {
    for (size_t i = 1; i < N; ++i) {
        if (arr[i] < arr[i-1]) return false;
    }
    return true;
}

// Test avec différents types
void test_integers() {
    std::array<int, 6> data = {5, 2, 8, 1, 9, 3};
    StaticSort<6>()(data);
    assert(is_sorted(data));
    assert(data[0] == 1 && data[5] == 9);
    std::cout << "✓ Test integers passed\n";
}

void test_doubles() {
    std::array<double, 5> data = {3.14, 1.41, 2.71, 0.57, 1.73};
    StaticSort<5>()(data);
    assert(is_sorted(data));
    std::cout << "✓ Test doubles passed\n";
}

void test_strings() {
    std::array<std::string, 4> data = {"delta", "alpha", "charlie", "bravo"};
    StaticSort<4>()(data);
    assert(is_sorted(data));
    assert(data[0] == "alpha");
    std::cout << "✓ Test strings passed\n";
}

void test_custom_comparator() {
    std::array<int, 5> data = {1, 2, 3, 4, 5};
    StaticSort<5>()(data, [](int a, int b) { return a > b; });
    assert(data[0] == 5 && data[4] == 1);
    std::cout << "✓ Test custom comparator passed\n";
}

void test_timsort_sorted() {
    std::array<int, 8> data = {1, 2, 3, 4, 5, 6, 7, 8};
    StaticTimSort<8>()(data);
    assert(is_sorted(data));
    std::cout << "✓ Test TimSort sorted passed\n";
}

void test_timsort_reversed() {
    std::array<int, 8> data = {8, 7, 6, 5, 4, 3, 2, 1};
    StaticTimSort<8>()(data);
    assert(is_sorted(data));
    std::cout << "✓ Test TimSort reversed passed\n";
}

void test_all_sizes() {
    for (size_t n = 2; n <= 8; ++n) {
        std::cout << "  Testing size " << n << "...";

        if (n == 2) {
            std::array<int, 2> arr = {2, 1};
            StaticSort<2>()(arr);
            assert(is_sorted(arr));
        } else if (n == 3) {
            std::array<int, 3> arr = {3, 1, 2};
            StaticSort<3>()(arr);
            assert(is_sorted(arr));
        } else if (n == 4) {
            std::array<int, 4> arr = {4, 2, 3, 1};
            StaticSort<4>()(arr);
            assert(is_sorted(arr));
        } else if (n == 5) {
            std::array<int, 5> arr = {5, 3, 1, 4, 2};
            StaticSort<5>()(arr);
            assert(is_sorted(arr));
        } else if (n == 6) {
            std::array<int, 6> arr = {6, 4, 2, 5, 3, 1};
            StaticSort<6>()(arr);
            assert(is_sorted(arr));
        } else if (n == 7) {
            std::array<int, 7> arr = {7, 5, 3, 1, 6, 4, 2};
            StaticSort<7>()(arr);
            assert(is_sorted(arr));
        } else if (n == 8) {
            std::array<int, 8> arr = {8, 6, 4, 2, 7, 5, 3, 1};
            StaticSort<8>()(arr);
            assert(is_sorted(arr));
        }

        std::cout << " ✓\n";
    }
    std::cout << "✓ All sizes tested successfully\n";
}

int main() {
    std::cout << "Running StaticSort correctness tests...\n";
    std::cout << "=======================================\n\n";

    test_integers();
    test_doubles();
    test_strings();
    test_custom_comparator();
    test_timsort_sorted();
    test_timsort_reversed();
    test_all_sizes();

    std::cout << "\n=======================================\n";
    std::cout << "✅ All tests passed successfully!\n";

    return 0;
}

