

#ifndef static_sort_h
#define static_sort_h

/*
 Adapted from the Bose-Nelson Sorting network code from:
 https://github.com/atinm/bose-nelson/blob/master/bose-nelson.c
 */

/**
 * A Functor class to create a sort for fixed sized arrays/containers with a
 * compile time generated Bose-Nelson sorting network.
 * \tparam NumElements  The number of elements in the array or container to sort.
 */
template <unsigned NumElements> class StaticSort
{
	// Default less than comparator
	struct LT
	{
		template <class A, class B>
		inline bool operator () (const A &a, const B &b) const
		{
			return a < b;
		}
	};
	
	template <class A, class C, int I0, int I1> struct Swap
	{
		template <class T> inline void s(T &v0, T &v1, C c)
		{
			// Explicitly code out the Min and Max to nudge the compiler
			// to generate branchless code where applicable.
			T t = c(v0, v1) ? v0 : v1; // Min
			v1 = c(v0, v1) ? v1 : v0; // Max
			v0 = t;
		}
		
		inline Swap(A &a, C c) { s(a[I0], a[I1], c); }
	};
	
	template <class A, class C, int I, int J, int X, int Y> struct PB
	{
		inline PB(A &a, C c)
		{
			enum { L = X >> 1, M = (X & 1 ? Y : Y + 1) >> 1, IAddL = I + L, XSubL = X - L };
			PB<A, C, I, J, L, M> p0(a, c);
			PB<A, C, IAddL, J + M, XSubL, Y - M> p1(a, c);
			PB<A, C, IAddL, J, XSubL, M> p2(a, c);
		}
	};
	
	template <class A, class C, int I, int J> struct PB <A, C, I, J, 1, 1>
	{
		inline PB(A &a, C c) { Swap<A, C, I - 1, J - 1> s(a, c); }
	};
	
	template <class A, class C, int I, int J> struct PB <A, C, I, J, 1, 2>
	{
		inline PB(A &a, C c) { Swap<A, C, I - 1, J> s0(a, c); Swap<A, C, I - 1, J - 1> s1(a, c); }
	};
	
	template <class A, class C, int I, int J> struct PB <A, C, I, J, 2, 1>
	{
		inline PB(A &a, C c) { Swap<A, C, I - 1, J - 1> s0(a, c); Swap<A, C, I, J - 1> s1(a, c); }
	};
	
	template <class A, class C, int I, int M, int Stop> struct PS
	{
		inline PS(A &a, C c)
		{
			enum { L = M >> 1, IAddL = I + L, MSubL = M - L};
			PS<A, C, I, L, (L <= 1)> ps0(a, c);
			PS<A, C, IAddL, MSubL, (MSubL <= 1)> ps1(a, c);
			PB<A, C, I, IAddL, L, MSubL> pb(a, c);
		}
	};
	
	template <class A, class C, int I, int M> struct PS <A, C, I, M, 1>
	{
		inline PS([[maybe_unused]] A &a, [[maybe_unused]] C c) {}
	};
	
public:
  // Version conteneur (comportement existant)
  template <class Container>
  inline void operator() (Container &arr) const
  {
    PS<Container, LT, 1, NumElements, (NumElements <= 1)> ps(arr, LT());
  }

  // Version itérateurs
  template <std::random_access_iterator Iterator>
  inline void operator() (Iterator first, Iterator last) const
  {
    auto size = std::distance(first, last);
    if (size != NumElements) return; // Sécurité

    // Adapter pour utiliser les itérateurs comme un tableau
    auto adapter = [first](size_t i) -> decltype(auto) { return *(first + i); };

    struct IteratorAdapter {
      Iterator base;
      decltype(auto) operator[](size_t i) { return *(base + i); }
    };

    IteratorAdapter adapted{first};
    PS<IteratorAdapter, LT, 1, NumElements, (NumElements <= 1)> ps(adapted, LT());
  }

  // Version avec comparateur personnalisé
  template <std::random_access_iterator Iterator, class Compare>
  inline void operator() (Iterator first, Iterator last, Compare lt) const
  {
    auto size = std::distance(first, last);
    if (size != NumElements) return;

    struct IteratorAdapter {
      Iterator base;
      decltype(auto) operator[](size_t i) { return *(base + i); }
    };

    IteratorAdapter adapted{first};
    typedef Compare & C;
    PS<IteratorAdapter, C, 1, NumElements, (NumElements <= 1)> ps(adapted, lt);
  }

  // Version C++20 ranges
  template <std::ranges::random_access_range R>
  inline void operator() (R&& range) const
  {
    (*this)(std::ranges::begin(range), std::ranges::end(range));
  }

  template <std::ranges::random_access_range R, class Compare>
  inline void operator() (R&& range, Compare lt) const
  {
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
template <unsigned NumElements> class StaticTimSort
{
	// Default less than comparator
	struct LT
	{
		template <class A, class B>
		inline bool operator () (const A &a, const B &b) const
		{
			return a < b;
		}
	};
	
	template <class A, class C> struct Intro
	{
		template <class T>
		static inline void reverse([[maybe_unused]] T _, A &a)
		{
			if constexpr( NumElements > 1) {
				unsigned left = 0, right = NumElements - 1;
				while (left < right) {
					T temp = a[left];
					a[left++] = a[right];
					a[right--] = temp;
				}
			}
		}
		
		template <class T>
		static inline bool sorted(T prev, A &a, C c)
		{
			if constexpr (NumElements < 8) return false;
			
			bool hasDecreasing = false;
			bool hasIncreasing = false;
			
			for (unsigned i = 1; i < NumElements; ++i) {
				T curr = a[i];
				if (c(curr, prev)) {
					hasDecreasing = true;
				}
				if (c(prev, curr)) {
					hasIncreasing = true;
				}
				prev = curr;
        if constexpr( NumElements > 22)
        {
          if (hasIncreasing && hasDecreasing)
          {
            return false;
          }
        }
      }
			if (!hasDecreasing) {
				return true;
			}
			if (!hasIncreasing) {
				reverse(a[0], a);
				return true;
			}
			return false;
		}
	};
	
	
	
public:
public:
  // Version conteneur (comportement existant)
  template <class Container>
  inline void operator() (Container &arr) const
  {
    if (!Intro<Container, LT>::sorted(arr[0], arr, LT()))
      StaticSort<NumElements>()(arr);
  }

  // Version itérateurs
  template <std::random_access_iterator Iterator>
  inline void operator() (Iterator first, Iterator last) const
  {
    auto size = std::distance(first, last);
    if (size != NumElements) return;

    struct IteratorAdapter {
      Iterator base;
      decltype(auto) operator[](size_t i) { return *(base + i); }
    };

    IteratorAdapter adapted{first};
    if (!Intro<IteratorAdapter, LT>::sorted(adapted[0], adapted, LT()))
      StaticSort<NumElements>()(first, last);
  }

  // Version avec comparateur
  template <std::random_access_iterator Iterator, class Compare>
  inline void operator() (Iterator first, Iterator last, Compare lt) const
  {
    auto size = std::distance(first, last);
    if (size != NumElements) return;

    struct IteratorAdapter {
      Iterator base;
      decltype(auto) operator[](size_t i) { return *(base + i); }
    };

    IteratorAdapter adapted{first};
    typedef Compare & C;
    if (!Intro<IteratorAdapter, C>::sorted(adapted[0], adapted, lt))
      StaticSort<NumElements>()(first, last, lt);
  }

  // Version C++20 ranges
  template <std::ranges::random_access_range R>
  inline void operator() (R&& range) const
  {
    (*this)(std::ranges::begin(range), std::ranges::end(range));
  }

  template <std::ranges::random_access_range R, class Compare>
  inline void operator() (R&& range, Compare lt) const
  {
    (*this)(std::ranges::begin(range), std::ranges::end(range), lt);
  }
};
#endif
