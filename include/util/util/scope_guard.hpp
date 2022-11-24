#pragma once
#include "../../content.hpp"

namespace rcwg::util {
    namespace impl {
        template<class F>
        class ScopeGuard {
            NON_COPYABLE(ScopeGuard);
            private:
                F f;
                bool active;
            public:
                constexpr ALWAYS_INLINE ScopeGuard(F f) : f(std::move(f)), active(true) { }
                constexpr ALWAYS_INLINE ~ScopeGuard() { if (active) { f(); } }
                constexpr ALWAYS_INLINE void Cancel() { active = false; }

                constexpr ALWAYS_INLINE ScopeGuard(ScopeGuard&& rhs) : f(std::move(rhs.f)), active(rhs.active) {
                    rhs.Cancel();
                }

                ScopeGuard &operator=(ScopeGuard&& rhs) = delete;
        };

        template<class F>
        constexpr ALWAYS_INLINE ScopeGuard<F> MakeScopeGuard(F f) {
            return ScopeGuard<F>(std::move(f));
        }

        enum class ScopeGuardOnExit {};

        template <typename F>
        constexpr ALWAYS_INLINE ScopeGuard<F> operator+(ScopeGuardOnExit, F&& f) {
            return ScopeGuard<F>(std::forward<F>(f));
        }
    }
}

#define SCOPE_GUARD   ::rcwg::util::impl::ScopeGuardOnExit() + [&]() ALWAYS_INLINE_LAMBDA
#define ON_SCOPE_EXIT auto ANONYMOUS_VARIABLE(SCOPE_EXIT_STATE_) = SCOPE_GUARD

