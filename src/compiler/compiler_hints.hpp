#pragma once

#ifdef NDEBUG
#ifndef forceinline
#define forceinline inline __attribute__((always_inline))
#endif
#else
#ifndef forceinline
#define forceinline
#endif
#endif

#ifdef NDEBUG
#ifndef unroll_loops
#define unroll_loops __attribute__((optimize("unroll-loops")))
#endif
#else
#ifndef unroll_loops
#define unroll_loops
#endif
#endif

#ifndef likely
#define likely(expr) __builtin_expect((expr), 1)
#endif

#ifndef unlikely
#define unlikely(expr) __builtin_expect((expr), 0)
#endif

template<class T>
struct [[gnu::packed]] unaligned {
	    T value;
};

template<class T>
forceinline T unaligned_load(const void *ptr) noexcept {
	    return reinterpret_cast<const unaligned<T> *>(ptr)->value;
}

template<class T>
forceinline void unaligned_store(void *ptr, T value) noexcept {
	    reinterpret_cast<unaligned<T> *>(ptr)->value = value;
}

