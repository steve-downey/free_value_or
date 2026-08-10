// tests/beman/expected/testing/type_name.hpp                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Type identity as a value, reported by name.
//
// A type-identity check written as `static_assert(std::is_same_v<A, B>)` is a
// translation failure that names neither type usefully; written as
// `CHECK(std::is_same_v<A, B>)` it is reported, but the expansion is the bare
// word `false`. `type_name` gives both halves:
//
//   FAILED: CHECK( type_name<decltype(*e)>() == type_name<int&>() )
//   with expansion: const int& == int&
//
// `type_name<T>()` returns the *identity* of T, not its spelling. Comparison
// is `std::is_same_v`, so the verdict is exact and a false pass is not
// possible. The spelling is consulted only when a comparison has already
// failed and the test framework needs to explain it. Two distinct types that
// the compiler happened to print identically would therefore be reported as
// `foo == foo` — a confusing explanation of a correct verdict, rather than
// the silent false pass that comparing spellings would have given.
//
// That collision is close to unreachable in practice — gcc distinguishes even
// unnamed-namespace and function-local classes by their enclosing scope — so
// this is exactness by construction, not a fix for an observed defect. The
// point is that the guarantee no longer depends on how good any particular
// compiler's names happen to be.
//
// `display_name<T>()` is the spelling on its own, for when a test wants the
// string rather than the comparison.
//
// The spelling is implementation-defined under either implementation below —
// `int&` on one compiler, `int &` on another — so never compare a
// `display_name` against a string literal.
//
// There are two implementations of the spelling, selected automatically. Both
// satisfy the same contract — a non-empty spelling that distinguishes
// distinct types — and the self-checks at the bottom of this header hold each
// of them to it. Which one is in use never changes what a test looks like.
//
// Reflection (P2996), when the compiler offers it:
//
//     std::meta::display_string_of(^^T)
//
// This is what reflection is for. It asks the compiler for the name directly
// instead of recovering it from a diagnostic string, so there is no parsing
// and nothing to calibrate. Note that `display_string_of` is itself specified
// as implementation-defined, so this buys robustness, not a canonical
// spelling. No `dealias` is needed: substituting a template argument already
// resolves an alias, so `display_name<some_alias>()` names the aliased type.
//
// Without reflection, the spelling is recovered from
// `std::source_location::function_name()` by calibration: the signatures of
// `signature<bool>` and `signature<char>` differ only where the template
// argument is named, so the common prefix and common suffix of the two give
// the offsets at which any type's name sits. Nothing about any compiler's
// format is hard-coded.
//
// Define BEMAN_EXPECTED_TESTING_TYPE_NAME_USE_REFLECTION to 0 or 1 to force
// one implementation, which is how the unselected one gets exercised.

#ifndef BEMAN_EXPECTED_TESTING_TYPE_NAME_HPP
#define BEMAN_EXPECTED_TESTING_TYPE_NAME_HPP

// `__cpp_impl_reflection` is predefined by the compiler, but
// `__cpp_lib_reflection` is a library macro: without <version> first it is
// never defined here and the reflection path silently never activates.
#include <version>

#ifndef BEMAN_EXPECTED_TESTING_TYPE_NAME_USE_REFLECTION
    #if defined(__cpp_lib_reflection) && defined(__cpp_impl_reflection)
        #define BEMAN_EXPECTED_TESTING_TYPE_NAME_USE_REFLECTION 1
    #else
        #define BEMAN_EXPECTED_TESTING_TYPE_NAME_USE_REFLECTION 0
    #endif
#endif

#include <ostream>
#include <string_view>
#include <type_traits>

#if BEMAN_EXPECTED_TESTING_TYPE_NAME_USE_REFLECTION

    #include <meta>

#else

    #include <cstddef>
    #include <source_location>

namespace beman::expected::testing::detail {

// The compiler's spelling of this function, template argument included.
template <class T>
consteval std::string_view signature();

// Length of the longest common prefix of `a` and `b`.
consteval std::size_t common_prefix(std::string_view a, std::string_view b);

// Length of the longest common suffix of `a` and `b`.
consteval std::size_t common_suffix(std::string_view a, std::string_view b);

} // namespace beman::expected::testing::detail

#endif

namespace beman::expected::testing {

// The compiler's spelling of `T`.
template <class T>
consteval std::string_view display_name();

// The identity of `T`, carrying its spelling for diagnostics. Tests do not
// name this type; they compare the results of `type_name`.
template <class T>
struct type_name_t {
    // HIDDEN FRIENDS
    //
    // Reached only once a comparison has failed and the test framework is
    // explaining it. Nothing about the verdict depends on this text.
    friend std::ostream& operator<<(std::ostream& os, type_name_t) {
        constexpr std::string_view name = display_name<T>();
        return os << name;
    }
};

// Exact type identity. The spelling is never consulted.
template <class T, class U>
constexpr bool operator==(type_name_t<T>, type_name_t<U>);

// The identity of `T`, for comparison in a test.
template <class T>
consteval type_name_t<T> type_name();

} // namespace beman::expected::testing

template <class T, class U>
constexpr bool beman::expected::testing::operator==(type_name_t<T>, type_name_t<U>) {
    return std::is_same_v<T, U>;
}

template <class T>
consteval beman::expected::testing::type_name_t<T> beman::expected::testing::type_name() {
    return {};
}

#if BEMAN_EXPECTED_TESTING_TYPE_NAME_USE_REFLECTION

template <class T>
consteval std::string_view beman::expected::testing::display_name() {
    return std::meta::display_string_of(^^T);
}

#else

template <class T>
consteval std::string_view beman::expected::testing::detail::signature() {
    return std::source_location::current().function_name();
}

consteval std::size_t beman::expected::testing::detail::common_prefix(std::string_view a, std::string_view b) {
    std::size_t n = 0;
    while (n < a.size() && n < b.size() && a[n] == b[n])
        ++n;
    return n;
}

consteval std::size_t beman::expected::testing::detail::common_suffix(std::string_view a, std::string_view b) {
    std::size_t n = 0;
    while (n < a.size() && n < b.size() && a[a.size() - 1 - n] == b[b.size() - 1 - n])
        ++n;
    return n;
}

template <class T>
consteval std::string_view beman::expected::testing::display_name() {
    // `bool` and `char` share no leading and no trailing character, so
    // neither contributes to the common prefix or the common suffix. Pairs
    // that look equally good can fail: gcc spells `long` as `long int`, whose
    // trailing `int` is also the whole of `int`, so `int`/`long` would fold
    // part of the name itself into the common suffix.
    constexpr std::string_view as_bool = detail::signature<bool>();
    constexpr std::string_view as_char = detail::signature<char>();
    constexpr std::size_t      prefix  = detail::common_prefix(as_bool, as_char);
    constexpr std::size_t      suffix  = detail::common_suffix(as_bool, as_char);
    static_assert(prefix + suffix < as_bool.size(),
                  "display_name: this compiler's function_name() does not name the template argument");

    const std::string_view signature = detail::signature<T>();
    return signature.substr(prefix, signature.size() - prefix - suffix);
}

#endif

namespace beman::expected::testing {

// Self-check on whichever spelling implementation was selected: a recovered
// name must be non-empty, and must not have had a cv-qualifier, a reference,
// or a pointer trimmed off either end.
static_assert(!display_name<int>().empty());
static_assert(display_name<int>() != display_name<long>());
static_assert(display_name<int>() != display_name<const int>());
static_assert(display_name<int>() != display_name<int*>());
static_assert(display_name<int>() != display_name<int&>());
static_assert(display_name<int&>() != display_name<int&&>());

// Self-check on comparison. These cannot distinguish identity from spelling,
// because no pair of types can be written here where the two disagree: gcc
// qualifies even unnamed-namespace and function-local classes distinctly, so
// a spelling collision is not constructible within one translation unit.
// Exactness is a property of how the comparison is defined, not something
// these assertions can demonstrate.
static_assert(type_name<int>() == type_name<int>());
static_assert(type_name<int>() == type_name<signed int>());
static_assert(type_name<int>() != type_name<long>());
static_assert(type_name<int>() != type_name<const int>());
static_assert(type_name<int>() != type_name<int&>());
static_assert(type_name<int&>() != type_name<int&&>());

} // namespace beman::expected::testing

#endif // BEMAN_EXPECTED_TESTING_TYPE_NAME_HPP
