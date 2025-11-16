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

template<typename T>
concept Swappable = requires(T a, T b)
{
  { a = b } -> std::convertible_to<T&>;
};

template<typename C, typename T>
concept Comparator = requires(C c, T a, T b)
{
  { c(a, b) } -> std::convertible_to<bool>;
};

namespace detail
{
  struct DefaultLess
  {
    template<class A, class B>
    constexpr bool operator()(const A& a, const B& b) const noexcept(noexcept(a < b)) { return a < b; }
  };
}

template<class T, class C>
[[gnu::always_inline]] inline constexpr void swap_if(T& a, T& b, C c)
  noexcept(noexcept(c(a, b)) && std::is_nothrow_move_constructible_v<T>) { if (c(b, a)) std::ranges::swap(a, b); }

template<unsigned NumElements>
class StaticSort
{
  using LT = detail::DefaultLess;

  template<class A, class C, int I0, int I1>
  struct Swap
  {
    [[gnu::always_inline]] constexpr Swap(A& a, C c) noexcept(noexcept(swap_if(a[I0], a[I1], c))) { swap_if(a[I0], a[I1], c); }
  };

  template<class A, class C, int I, int J, int X, int Y>
  struct PB
  {
    constexpr PB(A& a, C c)
    {
      enum { L = X >> 1, M = (X & 1 ? Y : Y + 1) >> 1, IAddL = I + L, XSubL = X - L };
      PB<A, C, I, J, L, M> p0(a, c);
      PB<A, C, IAddL, J + M, XSubL, Y - M> p1(a, c);
      PB<A, C, IAddL, J, XSubL, M> p2(a, c);
    }
  };

  template<class A, class C, int I, int J>
  struct PB<A, C, I, J, 1, 1>
  {
    constexpr PB(A& a, C c) { Swap<A, C, I - 1, J - 1> s(a, c); }
  };

  template<class A, class C, int I, int J>
  struct PB<A, C, I, J, 1, 2>
  {
    constexpr PB(A& a, C c)
    {
      Swap<A, C, I - 1, J> s0(a, c);
      Swap<A, C, I - 1, J - 1> s1(a, c);
    }
  };

  template<class A, class C, int I, int J>
  struct PB<A, C, I, J, 2, 1>
  {
    constexpr PB(A& a, C c)
    {
      Swap<A, C, I - 1, J - 1> s0(a, c);
      Swap<A, C, I, J - 1> s1(a, c);
    }
  };

  template<class A, class C, int I, int M, int Stop>
  struct PS
  {
    constexpr PS(A& a, C c)
    {
      enum { L = M >> 1, IAddL = I + L, MSubL = M - L };
      PS<A, C, I, L, (L <= 1)> ps0(a, c);
      PS<A, C, IAddL, MSubL, (MSubL <= 1)> ps1(a, c);
      PB<A, C, I, IAddL, L, MSubL> pb(a, c);
    }
  };

  template<class A, class C, int I, int M>
  struct PS<A, C, I, M, 1>
  {
    constexpr PS([[maybe_unused]] A& a, [[maybe_unused]] C c) {}
  };

public:
  // Conteneur indexable par operator[]
  template<class Container>
  constexpr void operator()(Container& arr) const { PS<Container, LT, 1, NumElements, (NumElements <= 1)> ps(arr, LT()); }

  // Itérateurs aléatoires (suppose last - first valide)
  template<std::random_access_iterator Iterator>
  constexpr void operator()(Iterator first, Iterator last) const
  {
    auto size = static_cast<unsigned>(last - first);
    if (size != NumElements) return;
    struct IteratorAdapter
    {
      Iterator base;
      constexpr decltype(auto) operator[](size_t i) const noexcept { return *(base + i); }
    };
    IteratorAdapter adapted{first};
    PS<IteratorAdapter, LT, 1, NumElements, (NumElements <= 1)> ps(adapted, LT());
  }

  // Itérateurs + comparateur
  template<std::random_access_iterator Iterator, class Compare>
  constexpr void operator()(Iterator first, Iterator last, Compare lt) const
  {
    auto size = static_cast<unsigned>(last - first);
    if (size != NumElements) return;
    struct IteratorAdapter
    {
      Iterator base;
      constexpr decltype(auto) operator[](size_t i) noexcept { return *(base + i); }
    };
    IteratorAdapter adapted{first};
    using C = Compare&;
    PS<IteratorAdapter, C, 1, NumElements, (NumElements <= 1)> ps(adapted, lt);
  }

  // Ranges
  template<std::ranges::random_access_range R>
  constexpr void operator()(R&& range) const { (*this)(std::ranges::begin(range), std::ranges::end(range)); }

  template<std::ranges::random_access_range R, class Compare>
  constexpr void operator()(R&& range, Compare lt) const { (*this)(std::ranges::begin(range), std::ranges::end(range), lt); }
};

template<>
class StaticSort<2>
{
  using LT = detail::DefaultLess;

public:
  template<class Container>
  constexpr void operator()(Container& arr) const { swap_if(arr[0], arr[1], LT()); }

  template<class Container, class Compare>
  constexpr void operator()(Container& arr, Compare lt) const { swap_if(arr[0], arr[1], lt); }

  template<std::random_access_iterator Iterator>
  constexpr void operator()(Iterator first, Iterator last) const
  {
    if (last - first != 2) return;
    swap_if(*first, *(first + 1), LT());
  }

  template<std::random_access_iterator Iterator, class Compare>
  constexpr void operator()(Iterator first, Iterator last, Compare lt) const
  {
    if (last - first != 2) return;
    swap_if(*first, *(first + 1), lt);
  }

  template<std::ranges::random_access_range R>
  constexpr void operator()(R&& range) const { (*this)(std::ranges::begin(range), std::ranges::end(range)); }

  template<std::ranges::random_access_range R, class Compare>
  constexpr void operator()(R&& range, Compare lt) const { (*this)(std::ranges::begin(range), std::ranges::end(range), lt); }
};


// StaticSort<3> : 3 comparaisons (optimal)
template<>
class StaticSort<3>
{
  using LT = detail::DefaultLess;

public:
  template<class C> constexpr void operator()(C& a) const noexcept(noexcept(swap_if(a[0], a[1], detail::DefaultLess())))
  {
    swap_if(a[1], a[2], LT());
    swap_if(a[0], a[2], LT());
    swap_if(a[0], a[1], LT());
  }

  template<class C, class Compare> constexpr void operator()(C& a, Compare c) const
  {
    swap_if(a[1], a[2], c);
    swap_if(a[0], a[2], c);
    swap_if(a[0], a[1], c);
  }

  template<std::random_access_iterator It> constexpr void operator()(It f, It l) const
  {
    if (l - f != 3) return;
    auto &a = *f, &b = *(f + 1), &c = *(f + 2);
    swap_if(b, c, LT());
    swap_if(a, c, LT());
    swap_if(a, b, LT());
  }

  template<std::random_access_iterator It, class Compare> constexpr void operator()(It f, It l, Compare cmp) const
  {
    if (l - f != 3) return;
    auto &a = *f, &b = *(f + 1), &c = *(f + 2);
    swap_if(b, c, cmp);
    swap_if(a, c, cmp);
    swap_if(a, b, cmp);
  }

  template<std::ranges::random_access_range R> constexpr void operator()(R&& r) const { (*this)(std::ranges::begin(r), std::ranges::end(r)); }
  template<std::ranges::random_access_range R, class Compare> constexpr void operator()(R&& r, Compare c) const { (*this)(std::ranges::begin(r), std::ranges::end(r), c); }
};

// StaticSort<4> : 5 comparaisons (optimal)
template<>
class StaticSort<4>
{
  using LT = detail::DefaultLess;

public:
  template<class C> constexpr void operator()(C& a) const noexcept(noexcept(swap_if(a[0], a[1], detail::DefaultLess())))
  {
    swap_if(a[0], a[1], LT());
    swap_if(a[2], a[3], LT());
    swap_if(a[0], a[2], LT());
    swap_if(a[1], a[3], LT());
    swap_if(a[1], a[2], LT());
  }

  template<class C, class Compare> constexpr void operator()(C& a, Compare c) const
  {
    swap_if(a[0], a[1], c);
    swap_if(a[2], a[3], c);
    swap_if(a[0], a[2], c);
    swap_if(a[1], a[3], c);
    swap_if(a[1], a[2], c);
  }

  template<std::random_access_iterator It> constexpr void operator()(It f, It l) const
  {
    if (l - f != 4) return;
    swap_if(*f, *(f + 1), LT());
    swap_if(*(f + 2), *(f + 3), LT());
    swap_if(*f, *(f + 2), LT());
    swap_if(*(f + 1), *(f + 3), LT());
    swap_if(*(f + 1), *(f + 2), LT());
  }

  template<std::random_access_iterator It, class Compare> constexpr void operator()(It f, It l, Compare c) const
  {
    if (l - f != 4) return;
    swap_if(*f, *(f + 1), c);
    swap_if(*(f + 2), *(f + 3), c);
    swap_if(*f, *(f + 2), c);
    swap_if(*(f + 1), *(f + 3), c);
    swap_if(*(f + 1), *(f + 2), c);
  }

  template<std::ranges::random_access_range R> constexpr void operator()(R&& r) const { (*this)(std::ranges::begin(r), std::ranges::end(r)); }
  template<std::ranges::random_access_range R, class Compare> constexpr void operator()(R&& r, Compare c) const { (*this)(std::ranges::begin(r), std::ranges::end(r), c); }
};

// StaticSort<5> : 9 comparaisons (optimal)
template<>
class StaticSort<5>
{
  using LT = detail::DefaultLess;

public:
  template<class C> constexpr void operator()(C& a) const noexcept(noexcept(swap_if(a[0], a[1], detail::DefaultLess())))
  {
    swap_if(a[0], a[1], LT());
    swap_if(a[3], a[4], LT());
    swap_if(a[2], a[4], LT());
    swap_if(a[2], a[3], LT());
    swap_if(a[0], a[3], LT());
    swap_if(a[0], a[2], LT());
    swap_if(a[1], a[4], LT());
    swap_if(a[1], a[3], LT());
    swap_if(a[1], a[2], LT());
  }

  template<class C, class Compare> constexpr void operator()(C& a, Compare c) const
  {
    swap_if(a[0], a[1], c);
    swap_if(a[3], a[4], c);
    swap_if(a[2], a[4], c);
    swap_if(a[2], a[3], c);
    swap_if(a[0], a[3], c);
    swap_if(a[0], a[2], c);
    swap_if(a[1], a[4], c);
    swap_if(a[1], a[3], c);
    swap_if(a[1], a[2], c);
  }

  template<std::random_access_iterator It> constexpr void operator()(It f, It l) const
  {
    if (l - f != 5) return;
    swap_if(*f, *(f + 1), LT());
    swap_if(*(f + 3), *(f + 4), LT());
    swap_if(*(f + 2), *(f + 4), LT());
    swap_if(*(f + 2), *(f + 3), LT());
    swap_if(*f, *(f + 3), LT());
    swap_if(*f, *(f + 2), LT());
    swap_if(*(f + 1), *(f + 4), LT());
    swap_if(*(f + 1), *(f + 3), LT());
    swap_if(*(f + 1), *(f + 2), LT());
  }

  template<std::random_access_iterator It, class Compare> constexpr void operator()(It f, It l, Compare c) const
  {
    if (l - f != 5) return;
    swap_if(*f, *(f + 1), c);
    swap_if(*(f + 3), *(f + 4), c);
    swap_if(*(f + 2), *(f + 4), c);
    swap_if(*(f + 2), *(f + 3), c);
    swap_if(*f, *(f + 3), c);
    swap_if(*f, *(f + 2), c);
    swap_if(*(f + 1), *(f + 4), c);
    swap_if(*(f + 1), *(f + 3), c);
    swap_if(*(f + 1), *(f + 2), c);
  }

  template<std::ranges::random_access_range R> constexpr void operator()(R&& r) const { (*this)(std::ranges::begin(r), std::ranges::end(r)); }
  template<std::ranges::random_access_range R, class Compare> constexpr void operator()(R&& r, Compare c) const { (*this)(std::ranges::begin(r), std::ranges::end(r), c); }
};

// StaticSort<6> : 12 comparaisons (optimal)
template<>
class StaticSort<6>
{
  using LT = detail::DefaultLess;

public:
  template<class C> constexpr void operator()(C& a) const noexcept(noexcept(swap_if(a[0], a[1], detail::DefaultLess())))
  {
    swap_if(a[1], a[2], LT());
    swap_if(a[4], a[5], LT());
    swap_if(a[0], a[2], LT());
    swap_if(a[3], a[5], LT());
    swap_if(a[0], a[1], LT());
    swap_if(a[3], a[4], LT());
    swap_if(a[1], a[4], LT());
    swap_if(a[0], a[3], LT());
    swap_if(a[2], a[5], LT());
    swap_if(a[1], a[3], LT());
    swap_if(a[2], a[4], LT());
    swap_if(a[2], a[3], LT());
  }

  template<class C, class Compare> constexpr void operator()(C& a, Compare c) const
  {
    swap_if(a[1], a[2], c);
    swap_if(a[4], a[5], c);
    swap_if(a[0], a[2], c);
    swap_if(a[3], a[5], c);
    swap_if(a[0], a[1], c);
    swap_if(a[3], a[4], c);
    swap_if(a[1], a[4], c);
    swap_if(a[0], a[3], c);
    swap_if(a[2], a[5], c);
    swap_if(a[1], a[3], c);
    swap_if(a[2], a[4], c);
    swap_if(a[2], a[3], c);
  }

  template<std::random_access_iterator It> constexpr void operator()(It f, It l) const
  {
    if (l - f != 6) return;
    swap_if(*(f + 1), *(f + 2), LT());
    swap_if(*(f + 4), *(f + 5), LT());
    swap_if(*f, *(f + 2), LT());
    swap_if(*(f + 3), *(f + 5), LT());
    swap_if(*f, *(f + 1), LT());
    swap_if(*(f + 3), *(f + 4), LT());
    swap_if(*(f + 1), *(f + 4), LT());
    swap_if(*f, *(f + 3), LT());
    swap_if(*(f + 2), *(f + 5), LT());
    swap_if(*(f + 1), *(f + 3), LT());
    swap_if(*(f + 2), *(f + 4), LT());
    swap_if(*(f + 2), *(f + 3), LT());
  }

  template<std::random_access_iterator It, class Compare> constexpr void operator()(It f, It l, Compare c) const
  {
    if (l - f != 6) return;
    swap_if(*(f + 1), *(f + 2), c);
    swap_if(*(f + 4), *(f + 5), c);
    swap_if(*f, *(f + 2), c);
    swap_if(*(f + 3), *(f + 5), c);
    swap_if(*f, *(f + 1), c);
    swap_if(*(f + 3), *(f + 4), c);
    swap_if(*(f + 1), *(f + 4), c);
    swap_if(*f, *(f + 3), c);
    swap_if(*(f + 2), *(f + 5), c);
    swap_if(*(f + 1), *(f + 3), c);
    swap_if(*(f + 2), *(f + 4), c);
    swap_if(*(f + 2), *(f + 3), c);
  }

  template<std::ranges::random_access_range R> constexpr void operator()(R&& r) const { (*this)(std::ranges::begin(r), std::ranges::end(r)); }
  template<std::ranges::random_access_range R, class Compare> constexpr void operator()(R&& r, Compare c) const { (*this)(std::ranges::begin(r), std::ranges::end(r), c); }
};

// StaticSort<7> : 16 comparaisons (optimal)
template<>
class StaticSort<7>
{
  using LT = detail::DefaultLess;

public:
  template<class C> constexpr void operator()(C& a) const noexcept(noexcept(swap_if(a[0], a[1], detail::DefaultLess())))
  {
    swap_if(a[1], a[2], LT());
    swap_if(a[3], a[4], LT());
    swap_if(a[5], a[6], LT());
    swap_if(a[0], a[2], LT());
    swap_if(a[3], a[5], LT());
    swap_if(a[4], a[6], LT());
    swap_if(a[0], a[1], LT());
    swap_if(a[4], a[5], LT());
    swap_if(a[2], a[6], LT());
    swap_if(a[0], a[4], LT());
    swap_if(a[1], a[5], LT());
    swap_if(a[0], a[3], LT());
    swap_if(a[2], a[5], LT());
    swap_if(a[1], a[3], LT());
    swap_if(a[2], a[4], LT());
    swap_if(a[2], a[3], LT());
  }

  template<class C, class Compare> constexpr void operator()(C& a, Compare c) const
  {
    swap_if(a[1], a[2], c);
    swap_if(a[3], a[4], c);
    swap_if(a[5], a[6], c);
    swap_if(a[0], a[2], c);
    swap_if(a[3], a[5], c);
    swap_if(a[4], a[6], c);
    swap_if(a[0], a[1], c);
    swap_if(a[4], a[5], c);
    swap_if(a[2], a[6], c);
    swap_if(a[0], a[4], c);
    swap_if(a[1], a[5], c);
    swap_if(a[0], a[3], c);
    swap_if(a[2], a[5], c);
    swap_if(a[1], a[3], c);
    swap_if(a[2], a[4], c);
    swap_if(a[2], a[3], c);
  }

  template<std::random_access_iterator It> constexpr void operator()(It f, It l) const
  {
    if (l - f != 7) return;
    swap_if(*(f + 1), *(f + 2), LT());
    swap_if(*(f + 3), *(f + 4), LT());
    swap_if(*(f + 5), *(f + 6), LT());
    swap_if(*f, *(f + 2), LT());
    swap_if(*(f + 3), *(f + 5), LT());
    swap_if(*(f + 4), *(f + 6), LT());
    swap_if(*f, *(f + 1), LT());
    swap_if(*(f + 4), *(f + 5), LT());
    swap_if(*(f + 2), *(f + 6), LT());
    swap_if(*f, *(f + 4), LT());
    swap_if(*(f + 1), *(f + 5), LT());
    swap_if(*f, *(f + 3), LT());
    swap_if(*(f + 2), *(f + 5), LT());
    swap_if(*(f + 1), *(f + 3), LT());
    swap_if(*(f + 2), *(f + 4), LT());
    swap_if(*(f + 2), *(f + 3), LT());
  }

  template<std::random_access_iterator It, class Compare> constexpr void operator()(It f, It l, Compare c) const
  {
    if (l - f != 7) return;
    swap_if(*(f + 1), *(f + 2), c);
    swap_if(*(f + 3), *(f + 4), c);
    swap_if(*(f + 5), *(f + 6), c);
    swap_if(*f, *(f + 2), c);
    swap_if(*(f + 3), *(f + 5), c);
    swap_if(*(f + 4), *(f + 6), c);
    swap_if(*f, *(f + 1), c);
    swap_if(*(f + 4), *(f + 5), c);
    swap_if(*(f + 2), *(f + 6), c);
    swap_if(*f, *(f + 4), c);
    swap_if(*(f + 1), *(f + 5), c);
    swap_if(*f, *(f + 3), c);
    swap_if(*(f + 2), *(f + 5), c);
    swap_if(*(f + 1), *(f + 3), c);
    swap_if(*(f + 2), *(f + 4), c);
    swap_if(*(f + 2), *(f + 3), c);
  }

  template<std::ranges::random_access_range R> constexpr void operator()(R&& r) const { (*this)(std::ranges::begin(r), std::ranges::end(r)); }
  template<std::ranges::random_access_range R, class Compare> constexpr void operator()(R&& r, Compare c) const { (*this)(std::ranges::begin(r), std::ranges::end(r), c); }
};

// StaticSort<8> : 19 comparaisons (optimal)
template<>
class StaticSort<8>
{
  using LT = detail::DefaultLess;

public:
  template<class C> constexpr void operator()(C& a) const noexcept(noexcept(swap_if(a[0], a[1], detail::DefaultLess())))
  {
    swap_if(a[0], a[1], LT());
    swap_if(a[2], a[3], LT());
    swap_if(a[4], a[5], LT());
    swap_if(a[6], a[7], LT());
    swap_if(a[0], a[2], LT());
    swap_if(a[1], a[3], LT());
    swap_if(a[4], a[6], LT());
    swap_if(a[5], a[7], LT());
    swap_if(a[1], a[2], LT());
    swap_if(a[5], a[6], LT());
    swap_if(a[0], a[4], LT());
    swap_if(a[3], a[7], LT());
    swap_if(a[1], a[5], LT());
    swap_if(a[2], a[6], LT());
    swap_if(a[1], a[4], LT());
    swap_if(a[3], a[6], LT());
    swap_if(a[2], a[4], LT());
    swap_if(a[3], a[5], LT());
    swap_if(a[3], a[4], LT());
  }

  template<class C, class Compare> constexpr void operator()(C& a, Compare c) const
  {
    swap_if(a[0], a[1], c);
    swap_if(a[2], a[3], c);
    swap_if(a[4], a[5], c);
    swap_if(a[6], a[7], c);
    swap_if(a[0], a[2], c);
    swap_if(a[1], a[3], c);
    swap_if(a[4], a[6], c);
    swap_if(a[5], a[7], c);
    swap_if(a[1], a[2], c);
    swap_if(a[5], a[6], c);
    swap_if(a[0], a[4], c);
    swap_if(a[3], a[7], c);
    swap_if(a[1], a[5], c);
    swap_if(a[2], a[6], c);
    swap_if(a[1], a[4], c);
    swap_if(a[3], a[6], c);
    swap_if(a[2], a[4], c);
    swap_if(a[3], a[5], c);
    swap_if(a[3], a[4], c);
  }

  template<std::random_access_iterator It> constexpr void operator()(It f, It l) const
  {
    if (l - f != 8) return;
    swap_if(*f, *(f + 1), LT());
    swap_if(*(f + 2), *(f + 3), LT());
    swap_if(*(f + 4), *(f + 5), LT());
    swap_if(*(f + 6), *(f + 7), LT());
    swap_if(*f, *(f + 2), LT());
    swap_if(*(f + 1), *(f + 3), LT());
    swap_if(*(f + 4), *(f + 6), LT());
    swap_if(*(f + 5), *(f + 7), LT());
    swap_if(*(f + 1), *(f + 2), LT());
    swap_if(*(f + 5), *(f + 6), LT());
    swap_if(*f, *(f + 4), LT());
    swap_if(*(f + 3), *(f + 7), LT());
    swap_if(*(f + 1), *(f + 5), LT());
    swap_if(*(f + 2), *(f + 6), LT());
    swap_if(*(f + 1), *(f + 4), LT());
    swap_if(*(f + 3), *(f + 6), LT());
    swap_if(*(f + 2), *(f + 4), LT());
    swap_if(*(f + 3), *(f + 5), LT());
    swap_if(*(f + 3), *(f + 4), LT());
  }

  template<std::random_access_iterator It, class Compare> constexpr void operator()(It f, It l, Compare c) const
  {
    if (l - f != 8) return;
    swap_if(*f, *(f + 1), c);
    swap_if(*(f + 2), *(f + 3), c);
    swap_if(*(f + 4), *(f + 5), c);
    swap_if(*(f + 6), *(f + 7), c);
    swap_if(*f, *(f + 2), c);
    swap_if(*(f + 1), *(f + 3), c);
    swap_if(*(f + 4), *(f + 6), c);
    swap_if(*(f + 5), *(f + 7), c);
    swap_if(*(f + 1), *(f + 2), c);
    swap_if(*(f + 5), *(f + 6), c);
    swap_if(*f, *(f + 4), c);
    swap_if(*(f + 3), *(f + 7), c);
    swap_if(*(f + 1), *(f + 5), c);
    swap_if(*(f + 2), *(f + 6), c);
    swap_if(*(f + 1), *(f + 4), c);
    swap_if(*(f + 3), *(f + 6), c);
    swap_if(*(f + 2), *(f + 4), c);
    swap_if(*(f + 3), *(f + 5), c);
    swap_if(*(f + 3), *(f + 4), c);
  }

  template<std::ranges::random_access_range R> constexpr void operator()(R&& r) const { (*this)(std::ranges::begin(r), std::ranges::end(r)); }
  template<std::ranges::random_access_range R, class Compare> constexpr void operator()(R&& r, Compare c) const { (*this)(std::ranges::begin(r), std::ranges::end(r), c); }
};


//==================================================================
//                  StaticTimSort
//==================================================================

/**
 * A Functor class to create a sort for fixed sized arrays/containers with a
 * compile time generated Bose-Nelson sorting network.
 * Inspired by TimSort, this scans through the array first.
 * It skips the sorting-network if it is strictly increasing or decreasing. ;)
 * \tparam NumElements  The number of elements in the array or container to sort.
 */
template<unsigned NumElements>
class StaticTimSort
{
  using LT = detail::DefaultLess;

  template<class A, class C> struct Intro
  {
    template<class T>
    static constexpr void reverse([[maybe_unused]] T, A& a)
      noexcept(noexcept(std::ranges::swap(a[0], a[0])) && std::is_nothrow_move_constructible_v<T>)
    {
      if constexpr (NumElements > 1)
      {
        if constexpr (NumElements <= 16)
        {
          constexpr unsigned half = NumElements / 2;
          [&]<std::size_t... I>(std::index_sequence<I...>) { (std::ranges::swap(a[I], a[NumElements - 1 - I]), ...); }(std::make_index_sequence<half>{});
        }
        else { for (unsigned i = 0; i < NumElements / 2; ++i) { std::ranges::swap(a[i], a[NumElements - 1 - i]); } }
      }
    }

    template<class T>
    static constexpr bool sorted(T prev, A& a, C c)
      noexcept(noexcept(c(a[1], a[0])) && noexcept(c(a[0], a[1])))
    {
      if constexpr (NumElements < 8) return false;

      bool hasDec = false;
      bool hasInc = false;

      if constexpr (NumElements <= 22)
      {
        // Scan complet sans early exit pour petites tailles
        for (unsigned i = 1; i < NumElements; ++i)
        {
          T curr = a[i];
          if (c(curr, prev)) hasDec = true;
          if (c(prev, curr)) hasInc = true;
          prev = curr;
        }
      }
      else
      {
        // Early exit pour grandes tailles
        for (unsigned i = 1; i < NumElements; ++i)
        {
          T curr = a[i];
          if (c(curr, prev)) hasDec = true;
          if (c(prev, curr)) hasInc = true;
          prev = curr;
          if (hasInc && hasDec) return false;
        }
      }

      if (!hasDec) return true;
      if (!hasInc)
      {
        reverse(a[0], a);
        return true;
      }
      return false;
    }
  };

public:
  // Conteneur sans comparateur
  template<class Container>
  constexpr void operator()(Container& arr) const
  {
    if constexpr (NumElements == 0) return; // garde pour éviter arr[0] si taille 0
    // Correction: passer arr[0] comme premier argument à sorted, et non arr
    if (!Intro<Container, LT>::sorted(arr[0], arr, LT())) [[likely]]
    {
      StaticSort<NumElements>()(arr);
    }
  }

  // Conteneur avec comparateur
  template<class Container, class Compare>
  constexpr void operator()(Container& arr, Compare&& cmp) const
  noexcept(noexcept(StaticSort<NumElements>()(arr, std::forward<Compare>(cmp))))
  {
    if constexpr (NumElements == 0) return;
    if (!Intro<Container, Compare&>::sorted(arr[0], arr, cmp)) [[likely]]
      StaticSort<NumElements>()(arr, std::forward<Compare>(cmp));
  }

  // Itérateurs
  template<std::random_access_iterator Iterator>
  constexpr void operator()(Iterator first, Iterator last) const
  {
    auto size = static_cast<unsigned>(last - first);
    if (size != NumElements) return;

    struct IteratorAdapter
    {
      Iterator base;
      constexpr decltype(auto) operator[](size_t i) noexcept { return *(base + i); }
    };

    IteratorAdapter adapted{first};
    if (!Intro<IteratorAdapter, LT>::sorted(adapted[0], adapted, LT())) StaticSort<NumElements>()(first, last);
  }

  // Itérateurs + comparateur
  template<std::random_access_iterator Iterator, class Compare>
  constexpr void operator()(Iterator first, Iterator last, Compare lt) const
  {
    auto size = static_cast<unsigned>(last - first);
    if (size != NumElements) return;

    struct IteratorAdapter
    {
      Iterator base;
      constexpr decltype(auto) operator[](size_t i) noexcept { return *(base + i); }
    };

    IteratorAdapter adapted{first};
    using C = Compare&;
    if (!Intro<IteratorAdapter, C>::sorted(adapted[0], adapted, lt)) StaticSort<NumElements>()(first, last, lt);
  }

  // Ranges
  template<std::ranges::random_access_range R>
  constexpr void operator()(R&& range) const { (*this)(std::ranges::begin(range), std::ranges::end(range)); }

  template<std::ranges::random_access_range R, class Compare>
  constexpr void operator()(R&& range, Compare lt) const { (*this)(std::ranges::begin(range), std::ranges::end(range), lt); }
};


// StaticTimSort<2> - délégation directe
// StaticTimSort<2..8> - délégation directe vers StaticSort
template<unsigned N> requires(N >= 2 && N <= 8)
class StaticTimSort<N>
{
  using LT = detail::DefaultLess;

public:
  template<class C> constexpr void operator()(C& a) const { StaticSort<N>()(a); }
  template<class C, class Compare> constexpr void operator()(C& a, Compare c) const { StaticSort<N>()(a, c); }
  template<std::random_access_iterator It> constexpr void operator()(It f, It l) const { StaticSort<N>()(f, l); }
  template<std::random_access_iterator It, class Compare> constexpr void operator()(It f, It l, Compare c) const { StaticSort<N>()(f, l, c); }
  template<std::ranges::random_access_range R> constexpr void operator()(R&& r) const { (*this)(std::ranges::begin(r), std::ranges::end(r)); }
  template<std::ranges::random_access_range R, class Compare> constexpr void operator()(R&& r, Compare c) const { (*this)(std::ranges::begin(r), std::ranges::end(r), c); }
};


#endif

