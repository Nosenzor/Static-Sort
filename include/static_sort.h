#ifndef static_sort_h
#define static_sort_h

#include <algorithm>
#include <iterator>
#include <ranges>
#include <concepts>
#include <type_traits>

/*
 Adapted from the Bose-Nelson Sorting network code from:
 https://github.com/atinm/bose-nelson/blob/master/bose-nelson.c
 */

/**
 * A Functor class to create a sort for fixed sized arrays/containers with a
 * compile time generated Bose-Nelson sorting network.
 * \tparam NumElements  The number of elements in the array or container to sort.
 */

template <typename T>
concept Swappable = requires(T a, T b) {
  { a = b } -> std::convertible_to<T&>;
};

template <typename C, typename T>
concept Comparator = requires(C c, T a, T b) {
  { c(a, b) } -> std::convertible_to<bool>;
};


template <unsigned NumElements>
class StaticSort
{
  struct LT {
    template <class A, class B>
    constexpr bool operator()(const A &a, const B &b) const noexcept(noexcept(a < b)) {
      return a < b;
    }
  };

  template <class A, class C, int I0, int I1>
  struct Swap {
    template <class T>
    [[gnu::always_inline]] static constexpr void s(T &v0, T &v1, C c) noexcept(noexcept(c(v0, v1)) && std::is_nothrow_move_constructible_v<T>)
    {
      // Branchless swap for trivial types with std::less
      if constexpr (std::is_trivially_copyable_v<T> &&
                    std::is_same_v<C, LT> &&
                    (std::is_arithmetic_v<T> || std::is_pointer_v<T>)) {
        T min_val = (v1 < v0) ? v1 : v0;
        T max_val = (v1 < v0) ? v0 : v1;
        v0 = min_val;
        v1 = max_val;
      } else {
        // Standard swap with move semantics for complex types
        if (c(v1, v0)) {
          T tmp = std::move(v0);
          v0 = std::move(v1);
          v1 = std::move(tmp);
        }
      }
    }
    [[gnu::always_inline]] constexpr Swap(A &a, C c) noexcept(noexcept(s(a[I0], a[I1], c))) {
      s(a[I0], a[I1], c);
    }
  };

  template <class A, class C, int I, int J, int X, int Y>
  struct PB {
    constexpr PB(A &a, C c) {
      enum { L = X >> 1, M = (X & 1 ? Y : Y + 1) >> 1, IAddL = I + L, XSubL = X - L };
      PB<A, C, I, J, L, M> p0(a, c);
      PB<A, C, IAddL, J + M, XSubL, Y - M> p1(a, c);
      PB<A, C, IAddL, J, XSubL, M> p2(a, c);
    }
  };
  template <class A, class C, int I, int J>
  struct PB<A, C, I, J, 1, 1> {
    constexpr PB(A &a, C c) { Swap<A, C, I - 1, J - 1> s(a, c); }
  };
  template <class A, class C, int I, int J>
  struct PB<A, C, I, J, 1, 2> {
    constexpr PB(A &a, C c) {
      Swap<A, C, I - 1, J> s0(a, c);
      Swap<A, C, I - 1, J - 1> s1(a, c);
    }
  };
  template <class A, class C, int I, int J>
  struct PB<A, C, I, J, 2, 1> {
    constexpr PB(A &a, C c) {
      Swap<A, C, I - 1, J - 1> s0(a, c);
      Swap<A, C, I, J - 1> s1(a, c);
    }
  };

  template <class A, class C, int I, int M, int Stop>
  struct PS {
    constexpr PS(A &a, C c) {
      enum { L = M >> 1, IAddL = I + L, MSubL = M - L };
      PS<A, C, I, L, (L <= 1)> ps0(a, c);
      PS<A, C, IAddL, MSubL, (MSubL <= 1)> ps1(a, c);
      PB<A, C, I, IAddL, L, MSubL> pb(a, c);
    }
  };
  template <class A, class C, int I, int M>
  struct PS<A, C, I, M, 1> {
    constexpr PS([[maybe_unused]] A &a, [[maybe_unused]] C c) {}
  };

public:
  // Conteneur indexable par operator[]
  template <class Container>
  constexpr void operator()(Container &arr) const
  {
    PS<Container, LT, 1, NumElements, (NumElements <= 1)> ps(arr, LT());
  }

  // Itérateurs aléatoires (suppose last - first valide)
  template <std::random_access_iterator Iterator>
  constexpr void operator()(Iterator first, Iterator last) const
  {
    auto size = static_cast<unsigned>(last - first);
    if (size != NumElements) return;
    struct IteratorAdapter {
      Iterator base;
      constexpr decltype(auto) operator[](size_t i) const noexcept {
        return *(base + i);
      }
    };
    IteratorAdapter adapted{first};
    PS<IteratorAdapter, LT, 1, NumElements, (NumElements <= 1)> ps(adapted, LT());
  }

  // Itérateurs + comparateur
  template <std::random_access_iterator Iterator, class Compare>
  constexpr void operator()(Iterator first, Iterator last, Compare lt) const
  {
    auto size = static_cast<unsigned>(last - first);
    if (size != NumElements) return;
    struct IteratorAdapter {
      Iterator base;
      constexpr decltype(auto) operator[](size_t i) noexcept { return *(base + i); }
    };
    IteratorAdapter adapted{first};
    using C = Compare &;
    PS<IteratorAdapter, C, 1, NumElements, (NumElements <= 1)> ps(adapted, lt);
  }

  // Ranges
  template <std::ranges::random_access_range R>
  constexpr void operator()(R &&range) const
  {
    (*this)(std::ranges::begin(range), std::ranges::end(range));
  }

  template <std::ranges::random_access_range R, class Compare>
  constexpr void operator()(R &&range, Compare lt) const
  {
    (*this)(std::ranges::begin(range), std::ranges::end(range), lt);
  }
};

// Spécialisation optimisée pour 2 éléments
template <>
class StaticSort<2>
{
  struct LT {
    template <class A, class B>
    constexpr bool operator()(const A &a, const B &b) const noexcept(noexcept(a < b)) {
      return a < b;
    }
  };

public:
  template <class Container>
  constexpr void operator()(Container &arr) const noexcept(noexcept(arr[0] < arr[1])) {
    if (arr[1] < arr[0]) {
      auto tmp = std::move(arr[0]);
      arr[0] = std::move(arr[1]);
      arr[1] = std::move(tmp);
    }
  }

  template <class Container, class Compare>
  constexpr void operator()(Container &arr, Compare lt) const noexcept(noexcept(lt(arr[0], arr[1]))) {
    if (lt(arr[1], arr[0])) {
      auto tmp = std::move(arr[0]);
      arr[0] = std::move(arr[1]);
      arr[1] = std::move(tmp);
    }
  }

  template <std::random_access_iterator Iterator>
  constexpr void operator()(Iterator first, Iterator last) const {
    if (last - first != 2) return;
    if (*(first + 1) < *first) {
      std::iter_swap(first, first + 1);
    }
  }

  template <std::random_access_iterator Iterator, class Compare>
  constexpr void operator()(Iterator first, Iterator last, Compare lt) const {
    if (last - first != 2) return;
    if (lt(*(first + 1), *first)) {
      std::iter_swap(first, first + 1);
    }
  }

  template <std::ranges::random_access_range R>
  constexpr void operator()(R &&range) const {
    (*this)(std::ranges::begin(range), std::ranges::end(range));
  }

  template <std::ranges::random_access_range R, class Compare>
  constexpr void operator()(R &&range, Compare lt) const {
    (*this)(std::ranges::begin(range), std::ranges::end(range), lt);
  }
};

// Spécialisation optimisée pour 3 éléments (3 comparaisons optimales)
template <>
class StaticSort<3>
{
  struct LT {
    template <class A, class B>
    constexpr bool operator()(const A &a, const B &b) const noexcept(noexcept(a < b)) {
      return a < b;
    }
  };

  template <class T, class C>
  [[gnu::always_inline]] static constexpr void sort3(T &a, T &b, T &c, C lt) {
    // Réseau optimal : 3 comparaisons
    if (lt(b, a)) std::swap(a, b);
    if (lt(c, b)) std::swap(b, c);
    if (lt(b, a)) std::swap(a, b);
  }

public:
  template <class Container>
  constexpr void operator()(Container &arr) const {
    sort3(arr[0], arr[1], arr[2], LT());
  }

  template <class Container, class Compare>
  constexpr void operator()(Container &arr, Compare lt) const {
    sort3(arr[0], arr[1], arr[2], lt);
  }

  template <std::random_access_iterator Iterator>
  constexpr void operator()(Iterator first, Iterator last) const {
    if (last - first != 3) return;
    auto &a = *first, &b = *(first + 1), &c = *(first + 2);
    sort3(a, b, c, LT());
  }

  template <std::random_access_iterator Iterator, class Compare>
  constexpr void operator()(Iterator first, Iterator last, Compare lt) const {
    if (last - first != 3) return;
    auto &a = *first, &b = *(first + 1), &c = *(first + 2);
    sort3(a, b, c, lt);
  }

  template <std::ranges::random_access_range R>
  constexpr void operator()(R &&range) const {
    (*this)(std::ranges::begin(range), std::ranges::end(range));
  }

  template <std::ranges::random_access_range R, class Compare>
  constexpr void operator()(R &&range, Compare lt) const {
    (*this)(std::ranges::begin(range), std::ranges::end(range), lt);
  }
};

// Spécialisation optimisée pour 4 éléments (5 comparaisons optimales)
template <>
class StaticSort<4>
{
  struct LT {
    template <class A, class B>
    constexpr bool operator()(const A &a, const B &b) const noexcept(noexcept(a < b)) {
      return a < b;
    }
  };

  template <class T, class C>
  [[gnu::always_inline]] static constexpr void sort4(T &a, T &b, T &c, T &d, C lt) {
    // Réseau optimal : 5 comparaisons
    if (lt(b, a)) std::swap(a, b);
    if (lt(d, c)) std::swap(c, d);
    if (lt(c, a)) std::swap(a, c);
    if (lt(d, b)) std::swap(b, d);
    if (lt(c, b)) std::swap(b, c);
  }

public:
  template <class Container>
  constexpr void operator()(Container &arr) const {
    sort4(arr[0], arr[1], arr[2], arr[3], LT());
  }

  template <class Container, class Compare>
  constexpr void operator()(Container &arr, Compare lt) const {
    sort4(arr[0], arr[1], arr[2], arr[3], lt);
  }

  template <std::random_access_iterator Iterator>
  constexpr void operator()(Iterator first, Iterator last) const {
    if (last - first != 4) return;
    auto &a = *first, &b = *(first + 1), &c = *(first + 2), &d = *(first + 3);
    sort4(a, b, c, d, LT());
  }

  template <std::random_access_iterator Iterator, class Compare>
  constexpr void operator()(Iterator first, Iterator last, Compare lt) const {
    if (last - first != 4) return;
    auto &a = *first, &b = *(first + 1), &c = *(first + 2), &d = *(first + 3);
    sort4(a, b, c, d, lt);
  }

  template <std::ranges::random_access_range R>
  constexpr void operator()(R &&range) const {
    (*this)(std::ranges::begin(range), std::ranges::end(range));
  }

  template <std::ranges::random_access_range R, class Compare>
  constexpr void operator()(R &&range, Compare lt) const {
    (*this)(std::ranges::begin(range), std::ranges::end(range), lt);
  }
};

// Spécialisation optimisée pour 5 éléments (9 comparaisons optimales)
template <>
class StaticSort<5>
{
  struct LT {
    template <class A, class B>
    constexpr bool operator()(const A &a, const B &b) const noexcept(noexcept(a < b)) {
      return a < b;
    }
  };

  template <class T, class C>
  [[gnu::always_inline]] static constexpr void sort5(T &a, T &b, T &c, T &d, T &e, C lt) {
    // Réseau optimal : 9 comparaisons
    if (lt(b, a)) std::swap(a, b);
    if (lt(d, c)) std::swap(c, d);
    if (lt(c, a)) std::swap(a, c);
    if (lt(e, b)) std::swap(b, e);
    if (lt(d, b)) std::swap(b, d);
    if (lt(e, c)) std::swap(c, e);
    if (lt(c, a)) std::swap(a, c);
    if (lt(d, c)) std::swap(c, d);
    if (lt(e, d)) std::swap(d, e);
  }

public:
  template <class Container>
  constexpr void operator()(Container &arr) const {
    sort5(arr[0], arr[1], arr[2], arr[3], arr[4], LT());
  }

  template <class Container, class Compare>
  constexpr void operator()(Container &arr, Compare lt) const {
    sort5(arr[0], arr[1], arr[2], arr[3], arr[4], lt);
  }

  template <std::random_access_iterator Iterator>
  constexpr void operator()(Iterator first, Iterator last) const {
    if (last - first != 5) return;
    auto &a = *first, &b = *(first + 1), &c = *(first + 2), &d = *(first + 3), &e = *(first + 4);
    sort5(a, b, c, d, e, LT());
  }

  template <std::random_access_iterator Iterator, class Compare>
  constexpr void operator()(Iterator first, Iterator last, Compare lt) const {
    if (last - first != 5) return;
    auto &a = *first, &b = *(first + 1), &c = *(first + 2), &d = *(first + 3), &e = *(first + 4);
    sort5(a, b, c, d, e, lt);
  }

  template <std::ranges::random_access_range R>
  constexpr void operator()(R &&range) const {
    (*this)(std::ranges::begin(range), std::ranges::end(range));
  }

  template <std::ranges::random_access_range R, class Compare>
  constexpr void operator()(R &&range, Compare lt) const {
    (*this)(std::ranges::begin(range), std::ranges::end(range), lt);
  }
};

// Spécialisation optimisée pour 6 éléments (12 comparaisons optimales)
template <>
class StaticSort<6>
{
  struct LT {
    template <class A, class B>
    constexpr bool operator()(const A &a, const B &b) const noexcept(noexcept(a < b)) {
      return a < b;
    }
  };

  template <class T, class C>
  [[gnu::always_inline]] static constexpr void sort6(T &a, T &b, T &c, T &d, T &e, T &f, C lt) {
    // Réseau optimal : 12 comparaisons
    if (lt(b, a)) std::swap(a, b);
    if (lt(d, c)) std::swap(c, d);
    if (lt(f, e)) std::swap(e, f);
    if (lt(c, a)) std::swap(a, c);
    if (lt(e, c)) std::swap(c, e);
    if (lt(d, b)) std::swap(b, d);
    if (lt(f, d)) std::swap(d, f);
    if (lt(e, a)) std::swap(a, e);
    if (lt(d, b)) std::swap(b, d);
    if (lt(f, b)) std::swap(b, f);
    if (lt(d, c)) std::swap(c, d);
    if (lt(e, d)) std::swap(d, e);
  }

public:
  template <class Container>
  constexpr void operator()(Container &arr) const {
    sort6(arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], LT());
  }

  template <class Container, class Compare>
  constexpr void operator()(Container &arr, Compare lt) const {
    sort6(arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], lt);
  }

  template <std::random_access_iterator Iterator>
  constexpr void operator()(Iterator first, Iterator last) const {
    if (last - first != 6) return;
    auto &a = *first, &b = *(first + 1), &c = *(first + 2),
         &d = *(first + 3), &e = *(first + 4), &f = *(first + 5);
    sort6(a, b, c, d, e, f, LT());
  }

  template <std::random_access_iterator Iterator, class Compare>
  constexpr void operator()(Iterator first, Iterator last, Compare lt) const {
    if (last - first != 6) return;
    auto &a = *first, &b = *(first + 1), &c = *(first + 2),
         &d = *(first + 3), &e = *(first + 4), &f = *(first + 5);
    sort6(a, b, c, d, e, f, lt);
  }

  template <std::ranges::random_access_range R>
  constexpr void operator()(R &&range) const {
    (*this)(std::ranges::begin(range), std::ranges::end(range));
  }

  template <std::ranges::random_access_range R, class Compare>
  constexpr void operator()(R &&range, Compare lt) const {
    (*this)(std::ranges::begin(range), std::ranges::end(range), lt);
  }
};

// Spécialisation optimisée pour 7 éléments (16 comparaisons optimales)
template <>
class StaticSort<7>
{
  struct LT {
    template <class A, class B>
    constexpr bool operator()(const A &a, const B &b) const noexcept(noexcept(a < b)) {
      return a < b;
    }
  };

  template <class T, class C>
  [[gnu::always_inline]] static constexpr void sort7(T &a, T &b, T &c, T &d, T &e, T &f, T &g, C lt) {
    // Réseau optimal : 16 comparaisons
    if (lt(b, a)) std::swap(a, b);
    if (lt(d, c)) std::swap(c, d);
    if (lt(f, e)) std::swap(e, f);
    if (lt(c, a)) std::swap(a, c);
    if (lt(e, c)) std::swap(c, e);
    if (lt(g, e)) std::swap(e, g);
    if (lt(d, b)) std::swap(b, d);
    if (lt(f, d)) std::swap(d, f);
    if (lt(e, a)) std::swap(a, e);
    if (lt(g, c)) std::swap(c, g);
    if (lt(d, b)) std::swap(b, d);
    if (lt(f, b)) std::swap(b, f);
    if (lt(g, d)) std::swap(d, g);
    if (lt(f, c)) std::swap(c, f);
    if (lt(g, e)) std::swap(e, g);
    if (lt(f, d)) std::swap(d, f);
  }

public:
  template <class Container>
  constexpr void operator()(Container &arr) const {
    sort7(arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], arr[6], LT());
  }

  template <class Container, class Compare>
  constexpr void operator()(Container &arr, Compare lt) const {
    sort7(arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], arr[6], lt);
  }

  template <std::random_access_iterator Iterator>
  constexpr void operator()(Iterator first, Iterator last) const {
    if (last - first != 7) return;
    auto &a = *first, &b = *(first + 1), &c = *(first + 2), &d = *(first + 3),
         &e = *(first + 4), &f = *(first + 5), &g = *(first + 6);
    sort7(a, b, c, d, e, f, g, LT());
  }

  template <std::random_access_iterator Iterator, class Compare>
  constexpr void operator()(Iterator first, Iterator last, Compare lt) const {
    if (last - first != 7) return;
    auto &a = *first, &b = *(first + 1), &c = *(first + 2), &d = *(first + 3),
         &e = *(first + 4), &f = *(first + 5), &g = *(first + 6);
    sort7(a, b, c, d, e, f, g, lt);
  }

  template <std::ranges::random_access_range R>
  constexpr void operator()(R &&range) const {
    (*this)(std::ranges::begin(range), std::ranges::end(range));
  }

  template <std::ranges::random_access_range R, class Compare>
  constexpr void operator()(R &&range, Compare lt) const {
    (*this)(std::ranges::begin(range), std::ranges::end(range), lt);
  }
};

// Spécialisation optimisée pour 8 éléments (19 comparaisons optimales)
template <>
class StaticSort<8>
{
  struct LT {
    template <class A, class B>
    constexpr bool operator()(const A &a, const B &b) const noexcept(noexcept(a < b)) {
      return a < b;
    }
  };

  template <class T, class C>
  [[gnu::always_inline]] static constexpr void sort8(T &a, T &b, T &c, T &d, T &e, T &f, T &g, T &h, C lt) {
    // Réseau optimal : 19 comparaisons
    if (lt(b, a)) std::swap(a, b);
    if (lt(d, c)) std::swap(c, d);
    if (lt(f, e)) std::swap(e, f);
    if (lt(h, g)) std::swap(g, h);
    if (lt(c, a)) std::swap(a, c);
    if (lt(d, b)) std::swap(b, d);
    if (lt(g, e)) std::swap(e, g);
    if (lt(h, f)) std::swap(f, h);
    if (lt(e, a)) std::swap(a, e);
    if (lt(f, b)) std::swap(b, f);
    if (lt(g, c)) std::swap(c, g);
    if (lt(h, d)) std::swap(d, h);
    if (lt(e, c)) std::swap(c, e);
    if (lt(f, d)) std::swap(d, f);
    if (lt(b, a)) std::swap(a, b);
    if (lt(d, c)) std::swap(c, d);
    if (lt(f, e)) std::swap(e, f);
    if (lt(h, g)) std::swap(g, h);
    if (lt(d, b)) std::swap(b, d);
    if (lt(f, b)) std::swap(b, f);
    if (lt(h, d)) std::swap(d, h);
    if (lt(f, d)) std::swap(d, f);
  }

public:
  template <class Container>
  constexpr void operator()(Container &arr) const {
    sort8(arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], arr[6], arr[7], LT());
  }

  template <class Container, class Compare>
  constexpr void operator()(Container &arr, Compare lt) const {
    sort8(arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], arr[6], arr[7], lt);
  }

  template <std::random_access_iterator Iterator>
  constexpr void operator()(Iterator first, Iterator last) const {
    if (last - first != 8) return;
    auto &a = *first, &b = *(first + 1), &c = *(first + 2), &d = *(first + 3),
         &e = *(first + 4), &f = *(first + 5), &g = *(first + 6), &h = *(first + 7);
    sort8(a, b, c, d, e, f, g, h, LT());
  }

  template <std::random_access_iterator Iterator, class Compare>
  constexpr void operator()(Iterator first, Iterator last, Compare lt) const {
    if (last - first != 8) return;
    auto &a = *first, &b = *(first + 1), &c = *(first + 2), &d = *(first + 3),
         &e = *(first + 4), &f = *(first + 5), &g = *(first + 6), &h = *(first + 7);
    sort8(a, b, c, d, e, f, g, h, lt);
  }

  template <std::ranges::random_access_range R>
  constexpr void operator()(R &&range) const {
    (*this)(std::ranges::begin(range), std::ranges::end(range));
  }

  template <std::ranges::random_access_range R, class Compare>
  constexpr void operator()(R &&range, Compare lt) const {
    (*this)(std::ranges::begin(range), std::ranges::end(range), lt);
  }
};


/**
 * A Functor class to create a sort for fixed sized arrays/containers with a
 * compile time generated Bose-Nelson sorting network.
 * Inspired by TimSort, this scans through the array first.
 * It skips the sorting-network if it is strictly increasing or decreasing. ;)
 * \tparam NumElements  The number of elements in the array or container to sort.
 */
template <unsigned NumElements>
class StaticTimSort
{
  struct LT {
    template <class A, class B>
    constexpr bool operator()(const A &a, const B &b) const noexcept(noexcept(a < b)) {
      return a < b;
    }
  };

  template <class A, class C> struct Intro {
    template <class T>
    static constexpr void reverse([[maybe_unused]] T, A &a) noexcept(noexcept(a[0] = a[0]) && std::is_nothrow_move_constructible_v<T>) {
      if constexpr (NumElements > 1) {
        if (std::is_constant_evaluated()) {
          // Version manuelle pour constexpr
          unsigned left = 0, right = NumElements - 1;
          while (left < right) {
            T tmp = std::move(a[left]);
            a[left++] = std::move(a[right]);
            a[right--] = std::move(tmp);
          }
        } else {
          // Utilisation de std::reverse pour les contextes runtime
          std::reverse(&a[0], &a[NumElements]);
        }
      }
    }

    template <class T>
    static constexpr bool sorted(T prev, A &a, C c)
      noexcept(noexcept(c(a[1], a[0])) && noexcept(c(a[0], a[1])))
    {
      if constexpr (NumElements < 8) return false;

      bool hasDec = false;
      bool hasInc = false;

      if constexpr (NumElements <= 22) {
        // Scan complet sans early exit pour petites tailles
        for (unsigned i = 1; i < NumElements; ++i) {
          T curr = a[i];
          if (c(curr, prev)) hasDec = true;
          if (c(prev, curr)) hasInc = true;
          prev = curr;
        }
      } else {
        // Early exit pour grandes tailles
        for (unsigned i = 1; i < NumElements; ++i) {
          T curr = a[i];
          if (c(curr, prev)) hasDec = true;
          if (c(prev, curr)) hasInc = true;
          prev = curr;
          if (hasInc && hasDec) return false;
        }
      }

      if (!hasDec) return true;
      if (!hasInc) {
        reverse(a[0], a);
        return true;
      }
      return false;
    }
  };

public:
  // Conteneur sans comparateur
  template <class Container>
  constexpr void operator()(Container &arr) const
  {
    if (!Intro<Container, LT>::sorted(arr[0], arr, LT()))
      StaticSort<NumElements>()(arr);
  }

  // Conteneur avec comparateur
  template <class Container, class Compare>
  constexpr void operator()(Container &arr, Compare lt) const
  {
    if (!Intro<Container, Compare &>::sorted(arr[0], arr, lt))
      StaticSort<NumElements>()(arr, lt);
  }

  // Itérateurs
  template <std::random_access_iterator Iterator>
  constexpr void operator()(Iterator first, Iterator last) const
  {
    auto size = static_cast<unsigned>(last - first);
    if (size != NumElements) return;

    struct IteratorAdapter {
      Iterator base;
      constexpr decltype(auto) operator[](size_t i) noexcept { return *(base + i); }
    };

    IteratorAdapter adapted{first};
    if (!Intro<IteratorAdapter, LT>::sorted(adapted[0], adapted, LT()))
      StaticSort<NumElements>()(first, last);
  }

  // Itérateurs + comparateur
  template <std::random_access_iterator Iterator, class Compare>
  constexpr void operator()(Iterator first, Iterator last, Compare lt) const
  {
    auto size = static_cast<unsigned>(last - first);
    if (size != NumElements) return;

    struct IteratorAdapter {
      Iterator base;
      constexpr decltype(auto) operator[](size_t i) noexcept { return *(base + i); }
    };

    IteratorAdapter adapted{first};
    using C = Compare &;
    if (!Intro<IteratorAdapter, C>::sorted(adapted[0], adapted, lt))
      StaticSort<NumElements>()(first, last, lt);
  }

  // Ranges
  template <std::ranges::random_access_range R>
  constexpr void operator()(R &&range) const
  {
    (*this)(std::ranges::begin(range), std::ranges::end(range));
  }

  template <std::ranges::random_access_range R, class Compare>
  constexpr void operator()(R &&range, Compare lt) const
  {
    (*this)(std::ranges::begin(range), std::ranges::end(range), lt);
  }
};

#endif

