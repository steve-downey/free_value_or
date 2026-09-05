// value_or.hpp                                                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_BEMAN_FREE_VALUE_OR_VALUE_OR
#define INCLUDED_BEMAN_FREE_VALUE_OR_VALUE_OR

#ifndef BEMAN_FREE_VALUE_OR_INCLUDED_FROM_INTERFACE_UNIT
    #include <initializer_list>
    #include <iterator>
    #include <type_traits>
    #include <utility>
#endif

namespace smd {
namespace free_value_or {

template <class T>
concept nullable = requires(const T t) {
    bool(t);
    *(t);
};

namespace detail {
// Polyfill for std::reference_constructs_from_temporary_v (P2255), a C++23
// library trait. Prefer the standard trait when the stdlib provides it;
// otherwise fall back to the __reference_constructs_from_temporary compiler
// builtin, which GCC and Clang expose independently of the standard version
// (so the dangling-reference checks below stay active at C++20 and on C++23
// stdlibs that predate the trait, e.g. libstdc++ < 13).
template <class To, class From>
inline constexpr bool reference_constructs_from_temporary_v =
#if defined(__cpp_lib_reference_from_temporary)
    std::reference_constructs_from_temporary_v<To, From>;
#elif defined(__has_builtin) && __has_builtin(__reference_constructs_from_temporary)
    __reference_constructs_from_temporary(To, From);
#elif defined(_MSC_VER)
    // MSVC has the intrinsic but does not report it through __has_builtin, which
    // covers only a fixed list; its own <type_traits> uses the intrinsic with no
    // version guard, so keying on _MSC_VER matches the STL shipped alongside.
    __reference_constructs_from_temporary(To, From);
#else
    #error "no std::reference_constructs_from_temporary_v and no __reference_constructs_from_temporary builtin"
#endif
} // namespace detail

template <nullable T, class U, class R>
constexpr auto reference_or(T&& m, U&& u) -> R;

template <nullable T, class U, class R>
constexpr auto value_or(T&& m, U&& u) -> R;

template <nullable T, class I, class R>
constexpr auto or_invoke(T&& m, I&& invocable) -> R;

// R precedes the Args pack: MSVC (C3547) rejects a non-deduced template
// parameter that follows a function parameter pack, even when it is defaulted
// on the definition. R depends only on Ret and T, so it is safe before Args.
template <class Ret, nullable T, class R, class... Args>
constexpr R or_construct(T&& m, Args&&... args);

template <class Ret, nullable T, class E, class R, class... Args>
constexpr R or_construct(T&& m, std::initializer_list<E> il, Args&&... args);
} // namespace free_value_or
} // namespace smd

template <smd::free_value_or::nullable T, class U, class R = std::common_reference_t<std::iter_reference_t<T>, U&&>>
constexpr auto smd::free_value_or::reference_or(T&& m, U&& u) -> R {
    // Reject fallbacks/holders that would bind the returned reference to a
    // temporary. Uses the P2255 trait (polyfilled for C++20 above).
    static_assert(!smd::free_value_or::detail::reference_constructs_from_temporary_v<R, U>);
    static_assert(!smd::free_value_or::detail::reference_constructs_from_temporary_v<R, T&>);

    return bool(m) ? static_cast<R>(*m) : static_cast<R>((U&&)u);
}

template <smd::free_value_or::nullable T, class U, class R = std::common_type_t<std::iter_reference_t<T>, U&&>>
constexpr auto smd::free_value_or::value_or(T&& m, U&& u) -> R {
    return bool(m) ? static_cast<R>(*m) : static_cast<R>(std::forward<U>(u));
}

template <smd::free_value_or::nullable T,
          class I,
          class R = std::common_type_t<std::iter_reference_t<T>, std::invoke_result_t<I>>>
constexpr auto smd::free_value_or::or_invoke(T&& m, I&& invocable) -> R {
    return bool(m) ? static_cast<R>(*m) : static_cast<R>(std::forward<I>(invocable)());
}

template <class Ret = void,
          smd::free_value_or::nullable T,
          class R = std::conditional_t<std::is_void_v<Ret>, std::remove_cvref_t<std::iter_reference_t<T>>, Ret>,
          class... Args>
constexpr R smd::free_value_or::or_construct(T&& m, Args&&... args) {
    return bool(m) ? static_cast<R>(*m) : R(std::forward<Args>(args)...);
}

template <class Ret = void,
          smd::free_value_or::nullable T,
          class E,
          class R = std::conditional_t<std::is_void_v<Ret>, std::remove_cvref_t<std::iter_reference_t<T>>, Ret>,
          class... Args>
constexpr R smd::free_value_or::or_construct(T&& m, std::initializer_list<E> il, Args&&... args) {
    return bool(m) ? static_cast<R>(*m) : R(il, std::forward<Args>(args)...);
}

#endif
