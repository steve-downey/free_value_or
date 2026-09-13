// value_or.hpp                                                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_BEMAN_FREE_VALUE_OR_VALUE_OR
#define INCLUDED_BEMAN_FREE_VALUE_OR_VALUE_OR

#ifndef BEMAN_FREE_VALUE_OR_INCLUDED_FROM_INTERFACE_UNIT
    #include <initializer_list>
    #include <iterator>
    // This monolithic reference implementation supplies specializations
    // normatively declared in the standard <optional> and <expected> headers.
    #include <optional>
    #include <type_traits>
    #include <utility>
    #include <version>
    #if defined(__has_include) && __has_include(<expected>)
        #include <expected>
        #define BEMAN_FREE_VALUE_OR_DETAIL_HAS_STD_EXPECTED_HEADER 1
    #endif
    #if defined(__has_include) && __has_include(<beman/optional/optional.hpp>)
        #include <beman/optional/optional.hpp>
        #define BEMAN_FREE_VALUE_OR_DETAIL_HAS_BEMAN_OPTIONAL_HEADER 1
    #endif
    #if defined(__has_include) && __has_include(<beman/expected/expected.hpp>)
        #include <beman/expected/expected.hpp>
        #define BEMAN_FREE_VALUE_OR_DETAIL_HAS_BEMAN_EXPECTED_HEADER 1
    #endif
#endif

namespace smd {
namespace free_value_or {

template <class T>
concept nullable = requires(const std::remove_reference_t<T>& t) {
    bool(t);
    *(t);
};

// As with borrowed_range, an lvalue is safe to observe regardless of its
// type. An rvalue nullable must opt in to promise that destroying the
// nullable does not invalidate the object produced by dereferencing it.
template <class T>
inline constexpr bool enable_borrowed_nullable = false;

template <class T>
inline constexpr bool enable_borrowed_nullable<T*> = true;

template <class T>
inline constexpr bool enable_borrowed_nullable<std::optional<T&>> = true;

#if defined(BEMAN_FREE_VALUE_OR_DETAIL_HAS_STD_EXPECTED_HEADER) && defined(__cpp_lib_expected) && \
    __cpp_lib_expected >= 202202L
template <class T, class E>
inline constexpr bool enable_borrowed_nullable<std::expected<T&, E>> = true;
#endif

#if defined(BEMAN_FREE_VALUE_OR_DETAIL_HAS_BEMAN_OPTIONAL_HEADER)
template <class T>
inline constexpr bool enable_borrowed_nullable<beman::optional::optional<T&>> = true;
#endif

#if defined(BEMAN_FREE_VALUE_OR_DETAIL_HAS_BEMAN_EXPECTED_HEADER) && defined(BEMAN_EXPECTED_HAS_REFERENCES) && \
    BEMAN_EXPECTED_HAS_REFERENCES
template <class T, class E>
inline constexpr bool enable_borrowed_nullable<beman::expected::expected<T&, E>> = true;
#endif

template <class T>
concept borrowed_nullable =
    nullable<T> && (std::is_lvalue_reference_v<T> || enable_borrowed_nullable<std::remove_cvref_t<T>>);

// The type *m yields, with the value category of the nullable carried
// through.  std::iter_reference_t always dereferences an lvalue; this does
// not, so an expiring owning nullable yields an expiring payload.
//
// For value-producing operations, operator* itself determines whether an
// rvalue nullable permits moving the payload: move only when dereference
// yields T&&. reference_or additionally requires borrowed_nullable because
// a T& result alone does not promise that destroying the wrapper preserves
// the referent (notably for owning smart pointers).
template <class T>
using deref_t = decltype(*std::declval<T>());

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
#elif defined(_MSC_VER)
    // MSVC has the intrinsic but does not report it through __has_builtin, which
    // covers only a fixed list; its own <type_traits> uses the intrinsic with no
    // version guard, so keying on _MSC_VER matches the STL shipped alongside.
    // It precedes the __has_builtin arm because MSVC's preprocessor warns
    // (C4067) on that line.
    __reference_constructs_from_temporary(To, From);
#elif defined(__has_builtin) && __has_builtin(__reference_constructs_from_temporary)
    __reference_constructs_from_temporary(To, From);
#else
    #error "no std::reference_constructs_from_temporary_v and no __reference_constructs_from_temporary builtin"
#endif

template <class To, class From>
inline constexpr bool explicitly_convertible_to_v = requires { static_cast<To>(std::declval<From>()); };
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

template <smd::free_value_or::nullable T,
          class U,
          class R = std::common_reference_t<smd::free_value_or::deref_t<T>, U&&>>
constexpr auto smd::free_value_or::reference_or(T&& m, U&& u) -> R {
    static_assert(smd::free_value_or::borrowed_nullable<T>,
                  "reference_or requires an lvalue or borrowed nullable; name the nullable or use value_or for a "
                  "value result");
    // A non-reference common_reference materializes a new result object and
    // defeats this function's reference semantics.
    static_assert(std::is_reference_v<R>,
                  "reference_or requires common_reference_t to be a reference; use value_or for a value result");
    // Reject fallbacks/holders that would bind the returned reference to a
    // temporary. Uses the P2255 trait (polyfilled for C++20 above).
    static_assert(!smd::free_value_or::detail::reference_constructs_from_temporary_v<R, U>,
                  "reference_or alternative would bind the result to a temporary");
    // Stated on deref_t<T>, the type *std::forward<T>(m) yields. Stated on T&
    // it would name the nullable rather than its payload, and could never fire.
    static_assert(
        !smd::free_value_or::detail::reference_constructs_from_temporary_v<R, smd::free_value_or::deref_t<T>>,
        "reference_or dereferenced nullable would bind the result to a temporary");

    return bool(m) ? static_cast<R>(*std::forward<T>(m)) : static_cast<R>((U&&)u);
}

template <smd::free_value_or::nullable T, class U, class R = std::common_type_t<smd::free_value_or::deref_t<T>, U&&>>
constexpr auto smd::free_value_or::value_or(T&& m, U&& u) -> R {
    return bool(m) ? static_cast<R>(*std::forward<T>(m)) : static_cast<R>(std::forward<U>(u));
}

template <smd::free_value_or::nullable T,
          class I,
          class R = std::common_type_t<smd::free_value_or::deref_t<T>, std::invoke_result_t<I>>>
constexpr auto smd::free_value_or::or_invoke(T&& m, I&& invocable) -> R {
    return bool(m) ? static_cast<R>(*std::forward<T>(m)) : static_cast<R>(std::forward<I>(invocable)());
}

template <class Ret = void,
          smd::free_value_or::nullable T,
          class R = std::conditional_t<std::is_void_v<Ret>, std::remove_cvref_t<smd::free_value_or::deref_t<T>>, Ret>,
          class... Args>
constexpr R smd::free_value_or::or_construct(T&& m, Args&&... args) {
    static_assert(!std::is_reference_v<R>, "or_construct requires a non-reference result type");
    static_assert(std::is_constructible_v<R, Args...>, "or_construct requires is_constructible_v<R, Args...>");
    static_assert(smd::free_value_or::detail::explicitly_convertible_to_v<R, smd::free_value_or::deref_t<T>>,
                  "or_construct requires static_cast<R>(*m) to be well-formed");
    return bool(m) ? static_cast<R>(*std::forward<T>(m)) : R(std::forward<Args>(args)...);
}

template <class Ret = void,
          smd::free_value_or::nullable T,
          class E,
          class R = std::conditional_t<std::is_void_v<Ret>, std::remove_cvref_t<smd::free_value_or::deref_t<T>>, Ret>,
          class... Args>
constexpr R smd::free_value_or::or_construct(T&& m, std::initializer_list<E> il, Args&&... args) {
    static_assert(!std::is_reference_v<R>, "or_construct requires a non-reference result type");
    static_assert(std::is_constructible_v<R, std::initializer_list<E>&, Args...>,
                  "or_construct requires is_constructible_v<R, initializer_list<E>&, Args...>");
    static_assert(smd::free_value_or::detail::explicitly_convertible_to_v<R, smd::free_value_or::deref_t<T>>,
                  "or_construct requires static_cast<R>(*m) to be well-formed");
    return bool(m) ? static_cast<R>(*std::forward<T>(m)) : R(il, std::forward<Args>(args)...);
}

#undef BEMAN_FREE_VALUE_OR_DETAIL_HAS_STD_EXPECTED_HEADER
#undef BEMAN_FREE_VALUE_OR_DETAIL_HAS_BEMAN_OPTIONAL_HEADER
#undef BEMAN_FREE_VALUE_OR_DETAIL_HAS_BEMAN_EXPECTED_HEADER

#endif
