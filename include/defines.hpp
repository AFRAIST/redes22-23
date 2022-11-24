#pragma once

#define NORETURN      __attribute__((noreturn))
#define ALWAYS_INLINE_LAMBDA __attribute__((always_inline))
#define ALWAYS_INLINE inline __attribute__((always_inline))
#define NOINLINE      __attribute__((noinline))

#define NON_COPYABLE(cls) \
    cls(const cls&) = delete; \
    cls& operator=(const cls&) = delete

#define NON_MOVEABLE(cls) \
    cls(cls&&) = delete; \
    cls& operator=(cls&&) = delete

#define CONCATENATE_IMPL(S1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_IMPL(s1, s2)

#ifdef __COUNTER__
#define ANONYMOUS_VARIABLE(pref) CONCATENATE(pref, __COUNTER__)
#else
#define ANONYMOUS_VARIABLE(pref) CONCATENATE(pref, __LINE__)
#endif

namespace rcwg::impl {
    template<typename... ArgTypes>
    constexpr ALWAYS_INLINE void UnusedImpl(ArgTypes &&... args ) {
        (static_cast<void>(args), ...);
    }
}

#define RCWG_UNUSED(...) ::rcwg::impl::UnusedImpl(__VA_ARGS__)

#define BITSIZEOF(x) (sizeof(x) << 3)

