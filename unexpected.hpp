// beman/expected/unexpected.hpp                                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef BEMAN_EXPECTED_UNEXPECTED_HPP
#define BEMAN_EXPECTED_UNEXPECTED_HPP

#include <beman/expected/config.hpp>

#ifndef BEMAN_EXPECTED_INCLUDED_FROM_INTERFACE_UNIT
    #include <initializer_list>
    #include <memory>
    #include <type_traits>
    #include <utility>
#endif

// Deleted-function diagnostic messages (P2573, C++26 `= delete("reason")`). Falls back to a plain
// `= delete` pre-C++26 so this header keeps compiling at the project's configured floor; no
// behavioral difference either way, just a worse diagnostic on older compilers.
#if defined(__cpp_deleted_function) && __cpp_deleted_function >= 202403L
    #define BEMAN_EXPECTED_DELETE_MSG(msg) delete (msg)
#else
    #define BEMAN_EXPECTED_DELETE_MSG(msg) delete
#endif

namespace beman {
namespace expected {

// [expected.unexpect]
//! \omit
struct unexpect_t {
    explicit unexpect_t() = default;
};
//! \omit
inline constexpr unexpect_t unexpect{};

// Forward declaration for is_unexpected_specialization trait
template <class E>
class unexpected;

namespace detail {
//! \expos
template <class T>
struct is_unexpected_specialization : std::false_type {};
template <class E>
struct is_unexpected_specialization<unexpected<E>> : std::true_type {};

// reference_constructs_from_temporary — the only dangling-detection trait this library uses.
// (The sibling reference_converts_from_temporary is intentionally not defined: it is unused, and
// its builtin __reference_converts_from_temporary is absent on Clang 18, which has only the
// __reference_constructs_from_temporary builtin.)
#ifdef __cpp_lib_reference_from_temporary
//! \expos
using std::reference_constructs_from_temporary_v;
#elif __has_builtin(__reference_constructs_from_temporary)
//! \expos
template <class T, class U>
inline constexpr bool reference_constructs_from_temporary_v = __reference_constructs_from_temporary(T, U);
#endif

} // namespace detail

// \rSec2[expected.unexpected]{Class template unexpected}
// \rSec3[expected.un.general]{General}
//! \mandates A program that instantiates the definition of `unexpected` for
//! a non-object type other than an lvalue reference type, an array type, a
//! specialization of `unexpected`, or a cv-qualified type is ill-formed.
//! \remarks Subclause \iref{expected.unexpected} describes the class
//! template `unexpected` that represents unexpected objects stored in
//! `expected` objects.
template <class E>
class unexpected {
    // [expected.un.general] para 2: ill-formed instantiations
    static_assert(std::is_object_v<E>, "unexpected<E>: E must be an object type (not void, reference, or function)");
    static_assert(!std::is_array_v<E>, "unexpected<E>: E must not be an array type");
    static_assert(std::is_same_v<E, std::remove_cv_t<E>>, "unexpected<E>: E must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<E>::value,
                  "unexpected<E>: E must not be a specialization of unexpected");

  public:
    constexpr unexpected(const unexpected&) = default;
    constexpr unexpected(unexpected&&)      = default;

    //! \at expected.un.cons
    //! \constraints
    //! \item `is_same_v<remove_cvref_t<Err>, unexpected>` is `false`; and
    //! \item `is_same_v<remove_cvref_t<Err>, in_place_t>` is `false`; and
    //! \item `is_constructible_v<E, Err>` is `true`.
    //! \effects Direct-non-list-initializes `unex` with
    //! `std::forward<Err>(e)`.
    //! \throws Any exception thrown by the initialization of `unex`.
    template <class Err = E>
        requires(!std::is_same_v<std::remove_cvref_t<Err>, unexpected> &&
                 !std::is_same_v<std::remove_cvref_t<Err>, std::in_place_t> && std::is_constructible_v<E, Err>)
    constexpr explicit unexpected(Err&& e) noexcept(std::is_nothrow_constructible_v<E, Err>)
        : unex_(std::forward<Err>(e)) {}

    //! \at expected.un.cons
    //! \constraints `is_constructible_v<E, Args...>` is `true`.
    //! \effects Direct-non-list-initializes `unex` with
    //! `std::forward<Args>(args)...`.
    //! \throws Any exception thrown by the initialization of `unex`.
    template <class... Args>
        requires std::is_constructible_v<E, Args...>
    constexpr explicit unexpected(std::in_place_t,
                                  Args&&... args) noexcept(std::is_nothrow_constructible_v<E, Args...>)
        : unex_(std::forward<Args>(args)...) {}

    //! \at expected.un.cons
    //! \constraints `is_constructible_v<E, initializer_list<U>&, Args...>` is
    //! `true`.
    //! \effects Direct-non-list-initializes `unex` with `il,
    //! std::forward<Args>(args)...`.
    //! \throws Any exception thrown by the initialization of `unex`.
    template <class U, class... Args>
        requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
    constexpr explicit unexpected(std::in_place_t, std::initializer_list<U> il, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<E, std::initializer_list<U>&, Args...>)
        : unex_(il, std::forward<Args>(args)...) {}

    constexpr unexpected& operator=(const unexpected&) = default;
    constexpr unexpected& operator=(unexpected&&)      = default;

    //! \at expected.un.obs
    //! \group un-error
    //! \returns `unex`.
    constexpr const E& error() const& noexcept { return unex_; }
    //! \at expected.un.obs
    //! \also un-error
    constexpr E& error() & noexcept { return unex_; }
    //! \at expected.un.obs
    //! \group un-error-rvalue
    //! \returns `std::move(unex)`.
    constexpr const E&& error() const&& noexcept { return std::move(unex_); }
    //! \at expected.un.obs
    //! \also un-error-rvalue
    constexpr E&& error() && noexcept { return std::move(unex_); }

    //! \at expected.un.swap
    //! \mandates `is_swappable_v<E>` is `true`.
    //! \effects Equivalent to: `using std::swap; swap(unex, other.unex);`
    constexpr void swap(unexpected& other) noexcept(std::is_nothrow_swappable_v<E>) {
        using std::swap;
        swap(unex_, other.unex_);
    }

    //! \at expected.un.eq
    //! \mandates The expression `x.error() == y.error()` is well-formed and
    //! its result is convertible to `bool`.
    //! \returns `x.error() == y.error()`.
    template <class E2>
    friend constexpr bool operator==(const unexpected& x, const unexpected<E2>& y) {
        return x.unex_ == y.error();
    }

    //! \at expected.un.swap
    //! \effects Equivalent to `x.swap(y)`.
    friend constexpr void swap(unexpected& x, unexpected& y) noexcept(noexcept(x.swap(y)))
        requires std::is_swappable_v<E>
    {
        x.swap(y);
    }

  private:
    //! \expos
    E unex_;
};

template <class E>
unexpected(E) -> unexpected<E>;

// \rSec3[expected.un.cons]{Constructors}
// \rSec3[expected.un.obs]{Observers}
// \rSec3[expected.un.swap]{Swap}
// \rSec3[expected.un.eq]{Equality operator}

// \rSec3[expected.un.ref]{Partial specialization unexpected<E&>}
// Stores a pointer to the referenced object; keeps expected<> from needing a
// separate set of specializations just to hold a reference error type.
//! \at expected.un.ref
//! \mandates A program that instantiates the definition of `unexpected<E&>`
//! for an array type or a specialization of `unexpected` is ill-formed.
//! \remarks An object of type `unexpected<E&>` holds a pointer to an object
//! of type `E`. The referenced object is not owned by the `unexpected<E&>`
//! object. Unlike the primary template, `E` may be a cv-qualified type.
//! \verbatim-synopsis
//! template<class E>
//! class unexpected<E&> {
//! public:
//!   constexpr unexpected(const unexpected&) = default;
//!   constexpr unexpected(unexpected&&) = default;
//!   template<class G = E>
//!     constexpr explicit unexpected(G&&) noexcept;
//!   template<class G = E>
//!     constexpr explicit unexpected(in_place_t, G&&) noexcept;
//!
//!   constexpr unexpected& operator=(const unexpected&) = default;
//!   constexpr unexpected& operator=(unexpected&&) = default;
//!
//!   constexpr E& error() const noexcept;
//!
//!   constexpr void swap(unexpected& other) noexcept;
//!
//!   template<class E2>
//!     friend constexpr bool operator==(const unexpected&, const unexpected<E2>&);
//!
//!   friend constexpr void swap(unexpected& x, unexpected& y) noexcept;
//!
//! private:
//!   E* unex;              // exposition only
//! };
template <class E>
class unexpected<E&> {
    static_assert(std::is_object_v<E>,
                  "unexpected<E&>: referenced type must be an object type (not void, reference, or function)");
    static_assert(!std::is_array_v<E>, "unexpected<E&>: referenced type must not be an array type");
    static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<E>>::value,
                  "unexpected<E&>: referenced type must not be a specialization of unexpected");
    // Deliberately no cv-qualification static_assert: unlike the primary template, the referenced
    // type may be cv-qualified (e.g. unexpected<const int&>).

  public:
    constexpr unexpected(const unexpected&) = default;
    constexpr unexpected(unexpected&&)      = default;

    // Binds E& directly to the referenced object; deleted below when G would bind to a temporary.
    //! \at expected.un.ref
    //! \constraints
    //! \item `is_same_v<remove_cvref_t<G>, unexpected>` is `false`,
    //! \item `is_same_v<remove_cvref_t<G>, in_place_t>` is `false`,
    //! \item `is_constructible_v<E&, G>` is `true`, and
    //! \item `reference_constructs_from_temporary_v<E&, G>` is `false`.
    //! \effects Initializes `unex` with
    //! `addressof(static_cast<E&>(std::forward<G>(e)))`.
    //! \remarks A constructor for which
    //! `reference_constructs_from_temporary_v<E&, G>` is `true` — one that
    //! would bind `E&` to a temporary — is defined as deleted.
    template <class G = E>
        requires(!std::is_same_v<std::remove_cvref_t<G>, unexpected> &&
                 !std::is_same_v<std::remove_cvref_t<G>, std::in_place_t> && std::is_constructible_v<E&, G &&> &&
                 !detail::reference_constructs_from_temporary_v<E&, G>)
    constexpr explicit unexpected(G&& e) noexcept : ptr_(std::addressof(static_cast<E&>(std::forward<G>(e)))) {}

    // Deleted: binding would dangle (G materializes a temporary). Represented in wording by the
    // enabled overload above, whose \remarks says when it's deleted instead.
    //! \merge
    template <class G>
        requires(detail::reference_constructs_from_temporary_v<E&, G>)
    constexpr unexpected(G&&) = BEMAN_EXPECTED_DELETE_MSG(
        "unexpected<E&>: argument would bind a temporary that dangles; pass an lvalue reference");

    // Deleted catch-all: neither constructible nor a dangling case. Not part of the specified
    // overload set at all (its constraint is simply the enabled overload's negation).
    //! \merge
    template <class G>
        requires(!std::is_same_v<std::remove_cvref_t<G>, unexpected> &&
                 !std::is_same_v<std::remove_cvref_t<G>, std::in_place_t> && !std::is_constructible_v<E&, G &&> &&
                 !detail::reference_constructs_from_temporary_v<E&, G>)
    constexpr unexpected(G&&) =
        BEMAN_EXPECTED_DELETE_MSG("unexpected<E&>: no viable conversion from the given argument to E&");

    // Single-argument in_place_t overload — lets expected's uniform
    // construct_at(addressof(unex_), std::in_place, args...) pattern work whether E is a
    // reference or not. Naturally restricted to arity 1: there is no variadic overload here,
    // and expected only ever calls this when is_constructible_v<E&, Args...> already holds.
    //! \at expected.un.ref
    //! \constraints `is_constructible_v<E&, G>` is `true` and
    //! `reference_constructs_from_temporary_v<E&, G>` is `false`.
    //! \effects Initializes `unex` with
    //! `addressof(static_cast<E&>(std::forward<G>(e)))`.
    template <class G = E>
        requires(std::is_constructible_v<E&, G &&> && !detail::reference_constructs_from_temporary_v<E&, G>)
    constexpr explicit unexpected(std::in_place_t, G&& e) noexcept
        : ptr_(std::addressof(static_cast<E&>(std::forward<G>(e)))) {}

    // Deleted: in_place argument would dangle. Represented in wording by the enabled overload above.
    //! \merge
    template <class G>
        requires(detail::reference_constructs_from_temporary_v<E&, G>)
    constexpr unexpected(std::in_place_t, G&&) = BEMAN_EXPECTED_DELETE_MSG(
        "unexpected<E&>: in_place argument would bind a temporary that dangles; pass an lvalue reference");

    constexpr unexpected& operator=(const unexpected&) = default;
    constexpr unexpected& operator=(unexpected&&)      = default;

    // Single overload — shallow-const, matching expected<T&,E&>'s existing error() style:
    // there is nothing to move out of a pointer to an external object.
    //! \at expected.un.ref
    //! \returns `*unex`.
    //! \remarks The reference returned is not `const`-qualified even when
    //! `*this` is `const`; the constness of the `unexpected<E&>` object does
    //! not propagate to the referenced object.
    constexpr E& error() const noexcept { return *ptr_; }

    //! \at expected.un.ref
    //! \effects Exchanges `unex` and `other.unex`.
    constexpr void swap(unexpected& other) noexcept { std::swap(ptr_, other.ptr_); }

    //! \at expected.un.ref
    //! \mandates The expression `x.error() == y.error()` is well-formed and
    //! its result is convertible to `bool`.
    //! \returns `x.error() == y.error()`.
    template <class E2>
    friend constexpr bool operator==(const unexpected& x, const unexpected<E2>& y) {
        return *x.ptr_ == y.error();
    }

    //! \at expected.un.ref
    //! \effects Equivalent to `x.swap(y)`.
    friend constexpr void swap(unexpected& x, unexpected& y) noexcept { x.swap(y); }

  private:
    //! \expos(unex)
    E* ptr_;
};

} // namespace expected
} // namespace beman

#endif
