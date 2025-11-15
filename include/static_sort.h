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
    static constexpr void s(T &v0, T &v1, C c) noexcept(noexcept(c(v0, v1)) && std::is_nothrow_move_constructible_v<T>)
    {
      //use move semantics to avoid unnecessary copies
      if (c(v1, v0))
      {
        T tmp = std::move(v0);
        v0 = std::move(v1);
        v1 = std::move(tmp);
      }
    }
    constexpr Swap(A &a, C c) noexcept(noexcept(s(a[I0], a[I1], c))) {
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

