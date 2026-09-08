// beman/expected/expected.hpp                                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef BEMAN_EXPECTED_EXPECTED_HPP
#define BEMAN_EXPECTED_EXPECTED_HPP

#include <beman/expected/unexpected.hpp>
#include <beman/expected/bad_expected_access.hpp>

#ifndef BEMAN_EXPECTED_INCLUDED_FROM_INTERFACE_UNIT
    #include <cstdlib>
    #include <functional>
    #include <initializer_list>
    #include <memory>
    #include <type_traits>
    #include <utility>
#endif

#if defined(_MSC_VER)
    #define BEMAN_EXPECTED_TRAP() __debugbreak()
#elif defined(__has_builtin) && __has_builtin(__builtin_trap)
    #define BEMAN_EXPECTED_TRAP() __builtin_trap()
#else
    #define BEMAN_EXPECTED_TRAP() std::abort()
#endif

/***
22.8.2 Header <expected> synopsis[expected.syn]

// mostly freestanding
namespace std {
  // [expected.unexpected], class template unexpected
  template<class E> class unexpected;

  // [expected.bad], class template bad_expected_access
  template<class E> class bad_expected_access;

  // [expected.bad.void], specialization for void
  template<> class bad_expected_access<void>;

  // in-place construction of unexpected values
  struct unexpect_t {
    explicit unexpect_t() = default;
  };
  inline constexpr unexpect_t unexpect{};

  // [expected.expected], class template expected
  template<class T, class E> class expected;                                // partially freestanding

  // [expected.void], partial specialization of expected for void types
  template<class T, class E> requires is_void_v<T> class expected<T, E>;    // partially freestanding
}
 */

namespace beman {
namespace expected {

namespace detail {

//! \expos
template <class T>
struct is_expected_specialization : std::false_type {};

// forward-declared in primary template below; specializations added after class definition

// [expected.object.assign] reinit_expected helper
//! \expos
template <class NewVal, class CurVal, class... Args>
constexpr void reinit_expected(NewVal& newval, CurVal& oldval, Args&&... args) {
    if constexpr (std::is_nothrow_constructible_v<NewVal, Args...>) {
        std::destroy_at(std::addressof(oldval));
        std::construct_at(std::addressof(newval), std::forward<Args>(args)...);
    } else if constexpr (std::is_nothrow_move_constructible_v<NewVal>) {
        NewVal tmp(std::forward<Args>(args)...);
        std::destroy_at(std::addressof(oldval));
        std::construct_at(std::addressof(newval), std::move(tmp));
    } else {
        CurVal tmp(std::move(oldval));
        std::destroy_at(std::addressof(oldval));
        try {
            std::construct_at(std::addressof(newval), std::forward<Args>(args)...);
        } catch (...) {
            std::construct_at(std::addressof(oldval), std::move(tmp));
            throw;
        }
    }
}

// reference_constructs_from_temporary_v / reference_converts_from_temporary_v now live in
// unexpected.hpp's detail namespace (beman::expected::detail), since unexpected<E&> needs them
// too and unexpected.hpp must not depend on expected.hpp.

// unexpect_dangles_v<E, Args...>: true iff constructing expected's error in place from Args...
// would bind a reference E to a temporary. False whenever E is not a reference, or arity != 1
// (a reference can only ever bind from a single argument), so it never affects the value-E path.
//! \expos
template <class E, class... Args>
inline constexpr bool unexpect_dangles_v = false;

template <class E, class G>
inline constexpr bool unexpect_dangles_v<E, G> = std::is_reference_v<E> && reference_constructs_from_temporary_v<E, G>;

} // namespace detail

template <class T, class E>
class expected;

namespace detail {
template <class T, class E>
struct is_expected_specialization<expected<T, E>> : std::true_type {};

//! \expos
template <class T, class W>
constexpr bool converts_from_any_cvref = std::disjunction_v<std::is_constructible<T, W&>,
                                                            std::is_convertible<W&, T>,
                                                            std::is_constructible<T, W>,
                                                            std::is_convertible<W, T>,
                                                            std::is_constructible<T, const W&>,
                                                            std::is_convertible<const W&, T>,
                                                            std::is_constructible<T, const W>,
                                                            std::is_convertible<const W, T>>;
} // namespace detail

// \rSec2[expected.expected]{Class template expected}
// \rSec3[expected.object.general]{General}
//! \mandates A program that instantiates the definition of `expected<T, E>`
//! with a `T` that is not a valid value type for `expected` (that is,
//! `remove_cv_t<T>` is `void`, or a complete non-array object type other
//! than `in_place_t`, `unexpect_t`, or a specialization of `unexpected`) is
//! ill-formed. A program that instantiates the definition of `expected<T,
//! E>` with an `E` that is not a valid template argument for `unexpected`
//! is ill-formed.
//! \remarks Any object of type `expected<T, E>` either contains a value of
//! type `T` or a value of type `E` nested within it. Member `has_val`
//! indicates whether the `expected<T, E>` object contains an object of type
//! `T`. When `has_value()` is `false`, the error is `unex.error()`. The
//! error is held as an `unexpected<E>`, and not as an `E`, so that `E` may
//! be an lvalue reference type: an `E&` cannot be a union member, whereas
//! `unexpected<E&>` holds a pointer to an external object.
template <class T, class E>
class expected {
    static_assert(!std::is_reference_v<T>, "T must not be a reference (use expected<T&,E> specialization)");
    static_assert(!std::is_rvalue_reference_v<E>, "E must not be an rvalue reference");
    static_assert(!std::is_void_v<std::remove_reference_t<E>>, "E must not be void");
    static_assert(!std::is_same_v<std::remove_cv_t<T>, std::in_place_t>, "T must not be in_place_t");
    static_assert(!std::is_same_v<std::remove_cv_t<T>, unexpect_t>, "T must not be unexpect_t");
    static_assert(!std::is_array_v<T>, "T must not be an array type");
    static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<T>>::value,
                  "T must not be a specialization of unexpected");
    static_assert(!std::is_array_v<std::remove_reference_t<E>>, "E must not be an array type");

  private:
    //! \expos
    using error_value_type = std::remove_cv_t<std::remove_reference_t<E>>;

  public:
    using value_type      = T;
    using error_type      = E;
    using unexpected_type = unexpected<E>;

    template <class U>
    using rebind = expected<U, error_type>;

    // -------------------------------------------------------------------------
    // [expected.object.cons] Constructors
    // -------------------------------------------------------------------------

    // Default constructor: value-initializes T
    constexpr expected() noexcept(std::is_nothrow_default_constructible_v<T>)
        requires std::is_default_constructible_v<T>;

    // Copy constructor (trivial path). Unconstrained on purpose: this is the
    // sole declaration when T or E is not copy constructible at all (it is
    // then implicitly defined as deleted), and the more-constrained
    // non-trivial-path overload below is selected over it by constraint
    // subsumption whenever it is viable.
    //! \at expected.object.cons
    //! \effects Direct-non-list-initializes `val` or `unex` (matching
    //! `rhs`'s active member) by trivial copy construction.
    //! \ensures `rhs.has_value() == this->has_value()`.
    //! \remarks This constructor is trivial.
    constexpr expected(const expected&) = default;

    // Copy constructor (non-trivial path)
    constexpr expected(const expected& rhs) noexcept(std::is_nothrow_copy_constructible_v<T> &&
                                                     std::is_nothrow_copy_constructible_v<E>)
        requires(std::is_copy_constructible_v<T> && std::is_copy_constructible_v<E> &&
                 !(std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_constructible_v<E>));

    // Move constructor (trivial path). Unconstrained; see the copy
    // constructor above for why. No explicit noexcept: let the compiler
    // deduce it, so a non-movable-at-all E deletes rather than mismatches.
    //! \at expected.object.cons
    //! \effects Direct-non-list-initializes `val` or `unex` (matching
    //! `rhs`'s active member) by trivial move construction.
    //! \ensures `rhs.has_value() == this->has_value()`.
    //! \remarks This constructor is trivial.
    constexpr expected(expected&&) = default;

    // Move constructor (non-trivial path)
    constexpr expected(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                std::is_nothrow_move_constructible_v<E>)
        requires(std::is_move_constructible_v<T> && std::is_move_constructible_v<E> &&
                 !(std::is_trivially_move_constructible_v<T> && std::is_trivially_move_constructible_v<E>));

    // Converting copy constructor from expected<U, G> — value-E path
    template <class U, class G>
        requires(!std::is_reference_v<E> && std::is_constructible_v<T, const U&> &&
                 std::is_constructible_v<E, const G&> &&
                 (std::is_same_v<bool, std::remove_cv_t<T>> || !detail::converts_from_any_cvref<T, expected<U, G>>) &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
    constexpr explicit(!std::is_convertible_v<const U&, T> || !std::is_convertible_v<const G&, E>)
        expected(const expected<U, G>& rhs);

    // Converting move constructor from expected<U, G> — value-E path
    template <class U, class G>
        requires(!std::is_reference_v<E> && std::is_constructible_v<T, U> && std::is_constructible_v<E, G> &&
                 (std::is_same_v<bool, std::remove_cv_t<T>> || !detail::converts_from_any_cvref<T, expected<U, G>>) &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
    constexpr explicit(!std::is_convertible_v<U, T> || !std::is_convertible_v<G, E>) expected(expected<U, G>&& rhs);

    // Converting constructor from expected<U, G> — reference-E path: only accepts sources
    // whose error type G is itself a reference convertible to E.
    template <class U, class G>
        requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<T, const U&> &&
                 std::is_convertible_v<G, E>)
    constexpr explicit(!std::is_convertible_v<const U&, T> || !std::is_convertible_v<G, E>)
        expected(const expected<U, G>& rhs);

    template <class U, class G>
        requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<T, U &&> &&
                 std::is_convertible_v<G, E>)
    constexpr explicit(!std::is_convertible_v<U&&, T> || !std::is_convertible_v<G, E>) expected(expected<U, G>&& rhs);

    // Constructor from value U&&
    template <class U = std::remove_cv_t<T>>
        requires(!std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
                 !std::is_same_v<std::remove_cvref_t<U>, unexpect_t> &&
                 !std::is_same_v<std::remove_cvref_t<U>, expected> && std::is_constructible_v<T, U> &&
                 !detail::is_unexpected_specialization<std::remove_cvref_t<U>>::value &&
                 (!std::is_same_v<bool, std::remove_cv_t<T>> ||
                  !detail::is_expected_specialization<std::remove_cvref_t<U>>::value))
    constexpr explicit(!std::is_convertible_v<U, T>) expected(U&& v);

    // Constructor from unexpected<G> const& / && — value-E path
    template <class G>
        requires(!std::is_reference_v<E> && std::is_constructible_v<E, const G&>)
    constexpr explicit(!std::is_convertible_v<const G&, E>) expected(const unexpected<G>& e);

    template <class G>
        requires(!std::is_reference_v<E> && std::is_constructible_v<E, G>)
    constexpr explicit(!std::is_convertible_v<G, E>) expected(unexpected<G>&& e);

    // Constructor from unexpected<G> — reference-E path. Allowed only when G is itself a reference,
    // i.e. the source unexpected holds a reference to an external object, so binding E& to e.error()
    // cannot dangle regardless of the source's value category. No const_cast is needed.
    template <class G>
        requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
                 !detail::reference_constructs_from_temporary_v<E, G>)
    constexpr explicit(!std::is_convertible_v<G, E>) expected(const unexpected<G>& e) noexcept;

    template <class G>
        requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
                 !detail::reference_constructs_from_temporary_v<E, G>)
    constexpr explicit(!std::is_convertible_v<G, E>) expected(unexpected<G>&& e) noexcept;

    // Deleted for reference E with value G: the referent lives inside the unexpected<G> object, so
    // binding E& to it would dangle once a temporary source is destroyed. Use (unexpect, lvalue), or
    // an unexpected<E&> holding an external object, instead.
    //! \at expected.object.cons
    //! \group cvt-unexpected-ctor-deleted
    //! \remarks When `E` is a reference type, an overload taking
    //! `unexpected<G>` for a non-reference `G` is defined as deleted: the
    //! referent would live inside the (possibly temporary) source
    //! `unexpected<G>` object, and binding `E&` to it would dangle. Use an
    //! `unexpected<E&>` holding an external object instead.
    template <class G>
        requires(std::is_reference_v<E> && !std::is_reference_v<G>)
    constexpr expected(const unexpected<G>&) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<T,E&>: cannot construct from unexpected<value>; the value would dangle — use unexpected<E&>");

    //! \at expected.object.cons
    //! \also cvt-unexpected-ctor-deleted
    template <class G>
        requires(std::is_reference_v<E> && !std::is_reference_v<G>)
    constexpr expected(unexpected<G>&&) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<T,E&>: cannot construct from unexpected<value>; the value would dangle — use unexpected<E&>");

    // In-place constructor for value
    template <class... Args>
        requires std::is_constructible_v<T, Args...>
    constexpr explicit expected(std::in_place_t, Args&&... args);

    // In-place constructor for value with initializer_list
    template <class U, class... Args>
        requires std::is_constructible_v<T, std::initializer_list<U>&, Args...>
    constexpr explicit expected(std::in_place_t, std::initializer_list<U> il, Args&&... args);

    // In-place constructor for error
    template <class... Args>
        requires(std::is_constructible_v<E, Args...> && !detail::unexpect_dangles_v<E, Args...>)
    constexpr explicit expected(unexpect_t, Args&&... args);

    // Deleted: single argument would bind E& to a temporary — dangling prevention
    //! \at expected.object.cons
    //! \group unexpect-ctor-deleted
    //! \remarks When `E` is a reference type, an overload with the same
    //! parameter types is defined as deleted if the single argument would
    //! bind `E&` to a temporary, or if it is otherwise not usable to
    //! construct `E`.
    template <class... Args>
        requires(detail::unexpect_dangles_v<E, Args...>)
    constexpr expected(unexpect_t, Args&&...) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<T,E&>: unexpect argument would bind a temporary that dangles; pass an lvalue reference");

    // Deleted catch-all: reference E, argument neither constructible nor a dangling case
    // (e.g. binding a non-const E& from a const lvalue).
    //! \at expected.object.cons
    //! \also unexpect-ctor-deleted
    template <class... Args>
        requires(std::is_reference_v<E> && !std::is_constructible_v<E, Args...> &&
                 !detail::unexpect_dangles_v<E, Args...>)
    constexpr expected(unexpect_t, Args&&...) =
        BEMAN_EXPECTED_DELETE_MSG("expected<T,E&>: no viable conversion from the given argument(s) to E&");

    // In-place constructor for error with initializer_list
    template <class U, class... Args>
        requires(!std::is_reference_v<E> && std::is_constructible_v<E, std::initializer_list<U>&, Args...>)
    constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&... args);

    //! \at expected.object.cons
    //! \remarks An overload with the same parameter types is defined as
    //! deleted when `E` is an lvalue reference type. An initializer list
    //! cannot provide the required long-lived error referent.
    template <class U, class... Args>
        requires std::is_reference_v<E>
    constexpr expected(unexpect_t, std::initializer_list<U>, Args&&...) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<T,E&>: initializer-list error construction cannot bind a reference; pass an lvalue reference");

    // -------------------------------------------------------------------------
    // [expected.object.dtor] Destructor
    // -------------------------------------------------------------------------

    //! \at expected.object.dtor
    //! \effects None: `val` or `unex` (whichever is active) has a trivial
    //! destructor.
    //! \remarks This destructor is trivial.
    constexpr ~expected()
        requires(std::is_trivially_destructible_v<T> && std::is_trivially_destructible_v<E>)
    = default;

    constexpr ~expected()
        requires(!(std::is_trivially_destructible_v<T> && std::is_trivially_destructible_v<E>));

    // -------------------------------------------------------------------------
    // [expected.object.assign] Assignment
    // -------------------------------------------------------------------------

    // Copy assignment (trivial path)
    //! \at expected.object.assign
    //! \effects Trivially copies `rhs`'s active member into `*this`.
    //! \returns `*this`.
    //! \remarks This operator is trivial.
    constexpr expected& operator=(const expected&)
        requires(std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T> &&
                 std::is_trivially_destructible_v<T> && std::is_trivially_copy_constructible_v<E> &&
                 std::is_trivially_copy_assignable_v<E> && std::is_trivially_destructible_v<E>)
    = default;

    // Copy assignment (non-trivial path)
    constexpr expected& operator=(const expected& rhs) noexcept(std::is_nothrow_copy_constructible_v<T> &&
                                                                std::is_nothrow_copy_assignable_v<T> &&
                                                                std::is_nothrow_copy_constructible_v<E> &&
                                                                std::is_nothrow_copy_assignable_v<E>)
        requires(std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T> &&
                 (std::is_reference_v<E> || (std::is_copy_constructible_v<E> && std::is_copy_assignable_v<E>)) &&
                 (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>) &&
                 !(std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T> &&
                   std::is_trivially_destructible_v<T> && std::is_trivially_copy_constructible_v<E> &&
                   std::is_trivially_copy_assignable_v<E> && std::is_trivially_destructible_v<E>));

    // Move assignment (trivial path)
    //! \at expected.object.assign
    //! \effects Trivially moves `rhs`'s active member into `*this`.
    //! \returns `*this`.
    //! \remarks This operator is trivial.
    constexpr expected& operator=(expected&&) noexcept
        requires(std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T> &&
                 std::is_trivially_destructible_v<T> && std::is_trivially_move_constructible_v<E> &&
                 std::is_trivially_move_assignable_v<E> && std::is_trivially_destructible_v<E>)
    = default;

    // Move assignment (non-trivial path)
    constexpr expected& operator=(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                           std::is_nothrow_move_assignable_v<T> &&
                                                           std::is_nothrow_move_constructible_v<E> &&
                                                           std::is_nothrow_move_assignable_v<E>)
        requires(std::is_move_constructible_v<T> && std::is_move_assignable_v<T> &&
                 (std::is_reference_v<E> || (std::is_move_constructible_v<E> && std::is_move_assignable_v<E>)) &&
                 (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>) &&
                 !(std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T> &&
                   std::is_trivially_destructible_v<T> && std::is_trivially_move_constructible_v<E> &&
                   std::is_trivially_move_assignable_v<E> && std::is_trivially_destructible_v<E>));

    // Assignment from value U&&
    template <class U = std::remove_cv_t<T>>
        requires(!std::is_same_v<expected, std::remove_cvref_t<U>> &&
                 !detail::is_unexpected_specialization<std::remove_cvref_t<U>>::value &&
                 std::is_constructible_v<T, U> && std::is_assignable_v<T&, U> &&
                 (std::is_nothrow_constructible_v<T, U> || std::is_nothrow_move_constructible_v<T> ||
                  std::is_nothrow_move_constructible_v<E>))
    constexpr expected& operator=(U&& v);

    // Assignment from unexpected<G> — value-E path
    template <class G>
        requires(!std::is_reference_v<E> && std::is_constructible_v<E, const G&> &&
                 std::is_assignable_v<E&, const G&> &&
                 (std::is_nothrow_constructible_v<E, const G&> || std::is_nothrow_move_constructible_v<T> ||
                  std::is_nothrow_move_constructible_v<E>))
    constexpr expected& operator=(const unexpected<G>& e);

    template <class G>
        requires(!std::is_reference_v<E> && std::is_constructible_v<E, G> && std::is_assignable_v<E&, G> &&
                 (std::is_nothrow_constructible_v<E, G> || std::is_nothrow_move_constructible_v<T> ||
                  std::is_nothrow_move_constructible_v<E>))
    constexpr expected& operator=(unexpected<G>&& e);

    // Rebinding assignment for reference E from reference G — binds unex_ to the external
    // referent (never dangles; pointer store is noexcept). Mirrors the reference-E constructor.
    template <class G>
        requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
                 !detail::reference_constructs_from_temporary_v<E, G>)
    constexpr expected& operator=(const unexpected<G>& e);

    template <class G>
        requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
                 !detail::reference_constructs_from_temporary_v<E, G>)
    constexpr expected& operator=(unexpected<G>&& e);

    // Deleted for reference E with value G: would rebind E& to unexpected<G>'s temporary storage.
    //! \at expected.object.assign
    //! \group cvt-unexpected-assign-deleted
    //! \remarks When `E` is a reference type, an overload taking
    //! `unexpected<G>` for a non-reference `G` is defined as deleted: it
    //! would rebind `unex` to `unexpected<G>`'s temporary storage.
    template <class G>
        requires(std::is_reference_v<E> && !std::is_reference_v<G>)
    constexpr expected& operator=(const unexpected<G>&) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<T,E&>: cannot assign from unexpected<value>; the value would dangle — use unexpected<E&>");

    //! \at expected.object.assign
    //! \also cvt-unexpected-assign-deleted
    template <class G>
        requires(std::is_reference_v<E> && !std::is_reference_v<G>)
    constexpr expected& operator=(unexpected<G>&&) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<T,E&>: cannot assign from unexpected<value>; the value would dangle — use unexpected<E&>");

    // Emplace: destroy current value/error, construct value in-place
    template <class... Args>
        requires std::is_nothrow_constructible_v<T, Args...>
    constexpr T& emplace(Args&&... args) noexcept;

    template <class U, class... Args>
        requires std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>
    constexpr T& emplace(std::initializer_list<U> il, Args&&... args) noexcept;

    // -------------------------------------------------------------------------
    // [expected.object.swap] Swap
    // -------------------------------------------------------------------------

    constexpr void swap(expected& rhs) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                std::is_nothrow_swappable_v<T> &&
                                                std::is_nothrow_move_constructible_v<E> &&
                                                (std::is_reference_v<E> || std::is_nothrow_swappable_v<E>))
        requires(std::is_swappable_v<T> && (std::is_reference_v<E> || std::is_swappable_v<E>) &&
                 std::is_move_constructible_v<T> && std::is_move_constructible_v<E> &&
                 (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>));

    //! \at expected.object.swap
    //! \effects Equivalent to `x.swap(y)`.
    friend constexpr void swap(expected& x, expected& y) noexcept(noexcept(x.swap(y)))
        requires(std::is_swappable_v<T> && (std::is_reference_v<E> || std::is_swappable_v<E>) &&
                 std::is_move_constructible_v<T> && std::is_move_constructible_v<E> &&
                 (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>))
    {
        x.swap(y);
    }

    // -------------------------------------------------------------------------
    // [expected.object.obs] Observers
    // -------------------------------------------------------------------------

    constexpr const T* operator->() const noexcept;
    constexpr T*       operator->() noexcept;

    constexpr const T&  operator*() const& noexcept;
    constexpr T&        operator*() & noexcept;
    constexpr const T&& operator*() const&& noexcept;
    constexpr T&&       operator*() && noexcept;

    constexpr explicit operator bool() const noexcept;
    constexpr bool     has_value() const noexcept;

    constexpr const T&  value() const&;
    constexpr T&        value() &;
    constexpr const T&& value() const&&;
    constexpr T&&       value() &&;

    constexpr const E&  error() const& noexcept;
    constexpr E&        error() & noexcept;
    constexpr const E&& error() const&& noexcept;
    constexpr E&&       error() && noexcept;

    template <class U = std::remove_cv_t<T>>
    constexpr T value_or(U&& def) const&;

    template <class U = std::remove_cv_t<T>>
    constexpr T value_or(U&& def) &&;

    template <class G = error_value_type>
        requires(std::is_copy_constructible_v<error_value_type> && std::is_convertible_v<G, error_value_type>)
    constexpr error_value_type error_or(G&& def) const&;

    template <class G = error_value_type>
        requires(std::is_move_constructible_v<error_value_type> && std::is_convertible_v<G, error_value_type>)
    constexpr error_value_type error_or(G&& def) &&;

    // -------------------------------------------------------------------------
    // [expected.object.monadic] Monadic operations
    // -------------------------------------------------------------------------

    template <class F>
        requires std::is_constructible_v<E, E&>
    constexpr auto and_then(F&& f) &;
    template <class F>
        requires std::is_constructible_v<E, E&&>
    constexpr auto and_then(F&& f) &&;
    template <class F>
        requires std::is_constructible_v<E, const E&>
    constexpr auto and_then(F&& f) const&;
    template <class F>
        requires std::is_constructible_v<E, const E&&>
    constexpr auto and_then(F&& f) const&&;

    template <class F>
        requires std::is_constructible_v<T, T&>
    constexpr auto or_else(F&& f) &;
    template <class F>
        requires std::is_constructible_v<T, T&&>
    constexpr auto or_else(F&& f) &&;
    template <class F>
        requires std::is_constructible_v<T, const T&>
    constexpr auto or_else(F&& f) const&;
    template <class F>
        requires std::is_constructible_v<T, const T&&>
    constexpr auto or_else(F&& f) const&&;

    template <class F>
        requires std::is_constructible_v<E, E&>
    constexpr auto transform(F&& f) &;
    template <class F>
        requires std::is_constructible_v<E, E&&>
    constexpr auto transform(F&& f) &&;
    template <class F>
        requires std::is_constructible_v<E, const E&>
    constexpr auto transform(F&& f) const&;
    template <class F>
        requires std::is_constructible_v<E, const E&&>
    constexpr auto transform(F&& f) const&&;

    template <class F>
        requires std::is_constructible_v<T, T&>
    constexpr auto transform_error(F&& f) &;
    template <class F>
        requires std::is_constructible_v<T, T&&>
    constexpr auto transform_error(F&& f) &&;
    template <class F>
        requires std::is_constructible_v<T, const T&>
    constexpr auto transform_error(F&& f) const&;
    template <class F>
        requires std::is_constructible_v<T, const T&&>
    constexpr auto transform_error(F&& f) const&&;

    // -------------------------------------------------------------------------
    // [expected.object.eq] Equality operators (hidden friends)
    // -------------------------------------------------------------------------

    //! \at expected.object.eq
    //! \mandates `!is_void_v<T2>` is `true`. The expression `*x == *y` is
    //! well-formed and its result is convertible to `bool`. The expression
    //! `x.error() == y.error()` is well-formed and its result is
    //! convertible to `bool`.
    //! \returns If `x.has_value() != y.has_value()`, `false`; otherwise, if
    //! `x.has_value()` is `true`, `*x == *y`; otherwise `x.error() ==
    //! y.error()`.
    template <class T2, class E2>
        requires(!std::is_void_v<T2>)
    friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y) {
        if (x.has_value() != y.has_value())
            return false;
        if (x.has_value())
            return *x == *y;
        return x.error() == y.error();
    }

    //! \at expected.object.eq
    //! \mandates `T2` is not a specialization of `expected`. The expression
    //! `*x == val` is well-formed and its result is convertible to `bool`.
    //! \returns `x.has_value() && static_cast<bool>(*x == val)`.
    template <class T2>
        requires(!detail::is_expected_specialization<T2>::value)
    friend constexpr bool operator==(const expected& x, const T2& val) {
        return x.has_value() && static_cast<bool>(*x == val);
    }

    //! \at expected.object.eq
    //! \mandates The expression `x.error() == e.error()` is well-formed and
    //! its result is convertible to `bool`.
    //! \returns `!x.has_value() && static_cast<bool>(x.error() ==
    //! e.error())`.
    template <class E2>
    friend constexpr bool operator==(const expected& x, const unexpected<E2>& e) {
        return !x.has_value() && static_cast<bool>(x.error() == e.error());
    }

  private:
    //! \expos
    bool has_val_;
    union {
        //! \expos
        T val_;
        //! \expos
        unexpected<E> unex_;
    };
};

// \rSec3[expected.object.cons]{Constructors}

//! \effects Value-initializes `val`.
//! \ensures `has_value()` is `true`.
//! \throws Any exception thrown by the initialization of `val`.
template <class T, class E>
constexpr expected<T, E>::expected() noexcept(std::is_nothrow_default_constructible_v<T>)
    requires std::is_default_constructible_v<T>
    : has_val_(true) {
    std::construct_at(std::addressof(val_));
}

//! \effects If `rhs.has_value()` is `true`, direct-non-list-initializes
//! `val` with `*rhs`. Otherwise, direct-non-list-initializes `unex` with
//! `rhs.error()`.
//! \ensures `rhs.has_value() == this->has_value()`.
//! \throws Any exception thrown by the initialization of `val` or `unex`.
//! \remarks This constructor is defined as deleted unless
//! `is_copy_constructible_v<T>` is `true` and `is_copy_constructible_v<E>`
//! is `true` or `is_reference_v<E>` is `true`. This constructor is trivial
//! if `is_trivially_copy_constructible_v<T>` is `true` and
//! `is_trivially_copy_constructible_v<E>` is `true`.
template <class T, class E>
constexpr expected<T, E>::expected(const expected& rhs) noexcept(std::is_nothrow_copy_constructible_v<T> &&
                                                                 std::is_nothrow_copy_constructible_v<E>)
    requires(std::is_copy_constructible_v<T> && std::is_copy_constructible_v<E> &&
             !(std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_constructible_v<E>))
    : has_val_(rhs.has_val_) {
    if (has_val_)
        std::construct_at(std::addressof(val_), rhs.val_);
    else
        std::construct_at(std::addressof(unex_), rhs.unex_);
}

//! \effects If `rhs.has_value()` is `true`, direct-non-list-initializes
//! `val` with `std::move(*rhs)`. Otherwise, direct-non-list-initializes
//! `unex` with `std::move(rhs.error())`.
//! \ensures `rhs.has_value()` is unchanged; `rhs.has_value() ==
//! this->has_value()` is `true`.
//! \throws Any exception thrown by the initialization of `val` or `unex`.
//! \remarks This constructor is trivial if
//! `is_trivially_move_constructible_v<T>` is `true` and
//! `is_trivially_move_constructible_v<E>` is `true`.
template <class T, class E>
constexpr expected<T, E>::expected(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                            std::is_nothrow_move_constructible_v<E>)
    requires(std::is_move_constructible_v<T> && std::is_move_constructible_v<E> &&
             !(std::is_trivially_move_constructible_v<T> && std::is_trivially_move_constructible_v<E>))
    : has_val_(rhs.has_val_) {
    if (has_val_)
        std::construct_at(std::addressof(val_), std::move(rhs.val_));
    else
        std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
}

//! \group cvt-copy-ctor
//! \constraints `is_constructible_v<T, const U&>` is `true`; and
//! `is_constructible_v<E, const G&>` is `true`; and if `T` is not `bool`,
//! `converts-from-any-cvref<T, expected<U, G>>` is `false`; and
//! `is_constructible_v<unexpected<E>, expected<U, G>&>` is `false`; and
//! `is_constructible_v<unexpected<E>, expected<U, G>>` is `false`; and
//! `is_constructible_v<unexpected<E>, const expected<U, G>&>` is `false`;
//! and `is_constructible_v<unexpected<E>, const expected<U, G>>` is
//! `false`.
//! \effects If `rhs.has_value()`, direct-non-list-initializes `val` with
//! `*rhs`. Otherwise, direct-non-list-initializes `unex` with
//! `rhs.error()`.
//! \ensures `rhs.has_value()` is unchanged; `rhs.has_value() ==
//! this->has_value()` is `true`.
//! \throws Any exception thrown by the initialization of `val` or `unex`.
template <class T, class E>
template <class U, class G>
    requires(!std::is_reference_v<E> && std::is_constructible_v<T, const U&> && std::is_constructible_v<E, const G&> &&
             (std::is_same_v<bool, std::remove_cv_t<T>> || !detail::converts_from_any_cvref<T, expected<U, G>>) &&
             !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
constexpr expected<T, E>::expected(const expected<U, G>& rhs) : has_val_(rhs.has_value()) {
    if (has_val_)
        std::construct_at(std::addressof(val_), *rhs);
    else
        std::construct_at(std::addressof(unex_), rhs.error());
}

//! \also cvt-copy-ctor
template <class T, class E>
template <class U, class G>
    requires(!std::is_reference_v<E> && std::is_constructible_v<T, U> && std::is_constructible_v<E, G> &&
             (std::is_same_v<bool, std::remove_cv_t<T>> || !detail::converts_from_any_cvref<T, expected<U, G>>) &&
             !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
constexpr expected<T, E>::expected(expected<U, G>&& rhs) : has_val_(rhs.has_value()) {
    if (has_val_)
        std::construct_at(std::addressof(val_), *std::move(rhs));
    else
        std::construct_at(std::addressof(unex_), std::move(rhs).error());
}

//! \group cvt-copy-ctor-ref
//! \constraints `is_reference_v<G>` is `true` and `is_convertible_v<G, E>`
//! is `true`.
//! \effects If `rhs.has_value()`, direct-non-list-initializes `val` with
//! `*rhs`. Otherwise, direct-non-list-initializes `unex` with
//! `rhs.error()`.
//! \ensures `rhs.has_value()` is unchanged; `rhs.has_value() ==
//! this->has_value()` is `true`.
//! \throws Any exception thrown by the initialization of `val` or `unex`.
//! \remarks Unlike the value-`E` overload above, this overload
//! participates in overload resolution only when `E` and `G` are both
//! reference types, so the referenced error object is never copied.
template <class T, class E>
template <class U, class G>
    requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<T, const U&> &&
             std::is_convertible_v<G, E>)
constexpr expected<T, E>::expected(const expected<U, G>& rhs) : has_val_(rhs.has_value()) {
    if (has_val_)
        std::construct_at(std::addressof(val_), *rhs);
    else
        std::construct_at(std::addressof(unex_), rhs.error());
}

//! \also cvt-copy-ctor-ref
template <class T, class E>
template <class U, class G>
    requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<T, U &&> &&
             std::is_convertible_v<G, E>)
constexpr expected<T, E>::expected(expected<U, G>&& rhs) : has_val_(rhs.has_value()) {
    if (has_val_)
        std::construct_at(std::addressof(val_), *std::move(rhs));
    else
        std::construct_at(std::addressof(unex_), rhs.error());
}

//! \constraints `is_same_v<remove_cvref_t<U>, in_place_t>` is `false`; and
//! `is_same_v<remove_cvref_t<U>, unexpect_t>` is `false`; and
//! `is_same_v<remove_cvref_t<U>, expected>` is `false`; and
//! `is_constructible_v<T, U>` is `true`; and `remove_cvref_t<U>` is not a
//! specialization of `unexpected`; and if `T` is `bool`,
//! `remove_cvref_t<U>` is not a specialization of `expected`.
//! \effects Direct-non-list-initializes `val` with `std::forward<U>(v)`.
//! \ensures `has_value()` is `true`.
//! \throws Any exception thrown by the initialization of `val`.
template <class T, class E>
template <class U>
    requires(!std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
             !std::is_same_v<std::remove_cvref_t<U>, unexpect_t> &&
             !std::is_same_v<std::remove_cvref_t<U>, expected<T, E>> && std::is_constructible_v<T, U> &&
             !detail::is_unexpected_specialization<std::remove_cvref_t<U>>::value &&
             (!std::is_same_v<bool, std::remove_cv_t<T>> ||
              !detail::is_expected_specialization<std::remove_cvref_t<U>>::value))
constexpr expected<T, E>::expected(U&& v) : has_val_(true) {
    std::construct_at(std::addressof(val_), std::forward<U>(v));
}

//! \group cvt-unexpected-ctor
//! \constraints `is_constructible_v<E, const G&>` is `true`.
//! \effects Direct-non-list-initializes `unex` with
//! `std::forward<const G&>(e.error())`.
//! \ensures `has_value()` is `false`.
//! \throws Any exception thrown by the initialization of `unex`.
template <class T, class E>
template <class G>
    requires(!std::is_reference_v<E> && std::is_constructible_v<E, const G&>)
constexpr expected<T, E>::expected(const unexpected<G>& e) : has_val_(false) {
    std::construct_at(std::addressof(unex_), e.error());
}

//! \also cvt-unexpected-ctor
template <class T, class E>
template <class G>
    requires(!std::is_reference_v<E> && std::is_constructible_v<E, G>)
constexpr expected<T, E>::expected(unexpected<G>&& e) : has_val_(false) {
    std::construct_at(std::addressof(unex_), std::move(e).error());
}

//! \group cvt-unexpected-ctor-ref
//! \constraints `is_reference_v<G>` is `true`; and `is_constructible_v<E,
//! G>` is `true`; and `reference_constructs_from_temporary_v<E, G>` is
//! `false`.
//! \effects Initializes `unex` with `e.error()`.
//! \ensures `has_value()` is `false`.
//! \remarks This constructor never throws: the referent is bound, not
//! copied.
template <class T, class E>
template <class G>
    requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
             !detail::reference_constructs_from_temporary_v<E, G>)
constexpr expected<T, E>::expected(const unexpected<G>& e) noexcept : has_val_(false) {
    std::construct_at(std::addressof(unex_), e.error());
}

//! \also cvt-unexpected-ctor-ref
template <class T, class E>
template <class G>
    requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
             !detail::reference_constructs_from_temporary_v<E, G>)
constexpr expected<T, E>::expected(unexpected<G>&& e) noexcept : has_val_(false) {
    std::construct_at(std::addressof(unex_), e.error());
}

//! \constraints `is_constructible_v<T, Args...>` is `true`.
//! \effects Direct-non-list-initializes `val` with
//! `std::forward<Args>(args)...`.
//! \ensures `has_value()` is `true`.
//! \throws Any exception thrown by the initialization of `val`.
template <class T, class E>
template <class... Args>
    requires std::is_constructible_v<T, Args...>
constexpr expected<T, E>::expected(std::in_place_t, Args&&... args) : has_val_(true) {
    std::construct_at(std::addressof(val_), std::forward<Args>(args)...);
}

//! \constraints `is_constructible_v<T, initializer_list<U>&, Args...>` is
//! `true`.
//! \effects Direct-non-list-initializes `val` with `il,
//! std::forward<Args>(args)...`.
//! \ensures `has_value()` is `true`.
//! \throws Any exception thrown by the initialization of `val`.
template <class T, class E>
template <class U, class... Args>
    requires std::is_constructible_v<T, std::initializer_list<U>&, Args...>
constexpr expected<T, E>::expected(std::in_place_t, std::initializer_list<U> il, Args&&... args) : has_val_(true) {
    std::construct_at(std::addressof(val_), il, std::forward<Args>(args)...);
}

//! \constraints `is_constructible_v<E, Args...>` is `true`.
//! \effects Direct-non-list-initializes `unex` with
//! `std::forward<Args>(args)...`.
//! \ensures `has_value()` is `false`.
//! \throws Any exception thrown by the initialization of `unex`.
template <class T, class E>
template <class... Args>
    requires(std::is_constructible_v<E, Args...> && !detail::unexpect_dangles_v<E, Args...>)
constexpr expected<T, E>::expected(unexpect_t, Args&&... args) : has_val_(false) {
    std::construct_at(std::addressof(unex_), std::in_place, std::forward<Args>(args)...);
}

//! \constraints `is_reference_v<E>` is `false`, and `is_constructible_v<E,
//! initializer_list<U>&, Args...>` is `true`.
//! \effects Direct-non-list-initializes `unex` with `il,
//! std::forward<Args>(args)...`.
//! \ensures `has_value()` is `false`.
//! \throws Any exception thrown by the initialization of `unex`.
//! \remarks An overload with the same parameter types is defined as
//! deleted when `E` is an lvalue reference type. An initializer list
//! cannot provide the required long-lived error referent.
template <class T, class E>
template <class U, class... Args>
    requires(!std::is_reference_v<E> && std::is_constructible_v<E, std::initializer_list<U>&, Args...>)
constexpr expected<T, E>::expected(unexpect_t, std::initializer_list<U> il, Args&&... args) : has_val_(false) {
    std::construct_at(std::addressof(unex_), std::in_place, il, std::forward<Args>(args)...);
}

// \rSec3[expected.object.dtor]{Destructor}

//! \effects If `has_value()` is `true`, destroys `val`, otherwise destroys
//! `unex`.
template <class T, class E>
constexpr expected<T, E>::~expected()
    requires(!(std::is_trivially_destructible_v<T> && std::is_trivially_destructible_v<E>))
{
    if (has_val_)
        std::destroy_at(std::addressof(val_));
    else
        std::destroy_at(std::addressof(unex_));
}

// =============================================================================
// \rSec3[expected.object.assign]{Assignment}

//! \effects If `this->has_value() && rhs.has_value()`, equivalent to `val
//! = *rhs`. Otherwise, if `this->has_value()`, equivalent to
//! `reinit-expected(unex, val, rhs.error())`. Otherwise, if
//! `rhs.has_value()`, equivalent to `reinit-expected(val, unex, *rhs)`.
//! Otherwise, equivalent to `unex = rhs.unex`. Then, if no exception was
//! thrown, equivalent to: `has_val = rhs.has_value(); return *this;` When
//! `E` is an lvalue reference type, each of the cases above that
//! initializes or assigns `unex` rebinds it: `unex` comes to refer to the
//! same object as `rhs`'s error. No previously or subsequently referenced
//! object is assigned through.
//! \returns `*this`.
//! \remarks This operator is defined as deleted unless
//! `is_copy_assignable_v<T>` is `true` and `is_copy_constructible_v<T>` is
//! `true` and `is_copy_assignable_v<E>` is `true` or `is_reference_v<E>`
//! is `true` and `is_copy_constructible_v<E>` is `true` or
//! `is_reference_v<E>` is `true` and `is_nothrow_move_constructible_v<T>
//! || is_nothrow_move_constructible_v<E>` is `true`. This operator is
//! trivial if `is_trivially_copy_constructible_v<T>`,
//! `is_trivially_copy_assignable_v<T>`, `is_trivially_destructible_v<T>`,
//! `is_trivially_copy_constructible_v<E>`,
//! `is_trivially_copy_assignable_v<E>`, and
//! `is_trivially_destructible_v<E>` are all `true`.
template <class T, class E>
constexpr expected<T, E>& expected<T, E>::operator=(const expected& rhs) noexcept(
    std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T> &&
    std::is_nothrow_copy_constructible_v<E> && std::is_nothrow_copy_assignable_v<E>)
    requires(std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T> &&
             (std::is_reference_v<E> || (std::is_copy_constructible_v<E> && std::is_copy_assignable_v<E>)) &&
             (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>) &&
             !(std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T> &&
               std::is_trivially_destructible_v<T> && std::is_trivially_copy_constructible_v<E> &&
               std::is_trivially_copy_assignable_v<E> && std::is_trivially_destructible_v<E>))
{
    if (has_val_ && rhs.has_val_) {
        val_ = rhs.val_;
    } else if (!has_val_ && !rhs.has_val_) {
        unex_ = rhs.unex_;
    } else if (has_val_) {
        // was value, now error
        detail::reinit_expected(unex_, val_, rhs.unex_);
        has_val_ = false;
    } else {
        // was error, now value
        detail::reinit_expected(val_, unex_, rhs.val_);
        has_val_ = true;
    }
    return *this;
}

//! \effects If `this->has_value() && rhs.has_value()`, equivalent to `val
//! = std::move(*rhs)`. Otherwise, if `this->has_value()`, equivalent to
//! `reinit-expected(unex, val, std::move(rhs.error()))`. Otherwise, if
//! `rhs.has_value()`, equivalent to `reinit-expected(val, unex,
//! std::move(*rhs))`. Otherwise, equivalent to `unex =
//! std::move(rhs.unex)`. Then, if no exception was thrown, equivalent to:
//! `has_val = rhs.has_value(); return *this;` When `E` is an lvalue
//! reference type, each of the cases above that initializes or assigns
//! `unex` rebinds it: `unex` comes to refer to the same object as `rhs`'s
//! error. No previously or subsequently referenced object is assigned
//! through.
//! \returns `*this`.
//! \remarks The exception specification is equivalent to
//! `is_nothrow_move_assignable_v<T> && is_nothrow_move_constructible_v<T>
//! && is_nothrow_move_assignable_v<E> &&
//! is_nothrow_move_constructible_v<E>`. This operator is trivial if
//! `is_trivially_move_constructible_v<T>`,
//! `is_trivially_move_assignable_v<T>`, `is_trivially_destructible_v<T>`,
//! `is_trivially_move_constructible_v<E>`,
//! `is_trivially_move_assignable_v<E>`, and
//! `is_trivially_destructible_v<E>` are all `true`.
template <class T, class E>
constexpr expected<T, E>& expected<T, E>::operator=(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                                             std::is_nothrow_move_assignable_v<T> &&
                                                                             std::is_nothrow_move_constructible_v<E> &&
                                                                             std::is_nothrow_move_assignable_v<E>)
    requires(std::is_move_constructible_v<T> && std::is_move_assignable_v<T> &&
             (std::is_reference_v<E> || (std::is_move_constructible_v<E> && std::is_move_assignable_v<E>)) &&
             (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>) &&
             !(std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T> &&
               std::is_trivially_destructible_v<T> && std::is_trivially_move_constructible_v<E> &&
               std::is_trivially_move_assignable_v<E> && std::is_trivially_destructible_v<E>))
{
    if (has_val_ && rhs.has_val_) {
        val_ = std::move(rhs.val_);
    } else if (!has_val_ && !rhs.has_val_) {
        unex_ = std::move(rhs.unex_);
    } else if (has_val_) {
        detail::reinit_expected(unex_, val_, std::move(rhs.unex_));
        has_val_ = false;
    } else {
        detail::reinit_expected(val_, unex_, std::move(rhs.val_));
        has_val_ = true;
    }
    return *this;
}

//! \constraints `is_same_v<expected, remove_cvref_t<U>>` is `false`; and
//! `remove_cvref_t<U>` is not a specialization of `unexpected`; and
//! `is_constructible_v<T, U>` is `true`; and `is_assignable_v<T&, U>` is
//! `true`; and `is_nothrow_constructible_v<T, U> ||
//! is_nothrow_move_constructible_v<T> ||
//! is_nothrow_move_constructible_v<E>` is `true`.
//! \effects If `has_value()` is `true`, equivalent to: `val =
//! std::forward<U>(v);` Otherwise, equivalent to: `reinit-expected(val,
//! unex, std::forward<U>(v)); has_val = true;`
//! \returns `*this`.
template <class T, class E>
template <class U>
    requires(!std::is_same_v<expected<T, E>, std::remove_cvref_t<U>> &&
             !detail::is_unexpected_specialization<std::remove_cvref_t<U>>::value && std::is_constructible_v<T, U> &&
             std::is_assignable_v<T&, U> &&
             (std::is_nothrow_constructible_v<T, U> || std::is_nothrow_move_constructible_v<T> ||
              std::is_nothrow_move_constructible_v<E>))
constexpr expected<T, E>& expected<T, E>::operator=(U&& v) {
    if (has_val_) {
        val_ = std::forward<U>(v);
    } else {
        detail::reinit_expected(val_, unex_, std::forward<U>(v));
        has_val_ = true;
    }
    return *this;
}

//! \group cvt-unexpected-assign
//! \constraints `is_constructible_v<E, const G&>` is `true`; and
//! `is_assignable_v<E&, const G&>` is `true`; and
//! `is_nothrow_constructible_v<E, const G&> ||
//! is_nothrow_move_constructible_v<T> ||
//! is_nothrow_move_constructible_v<E>` is `true`.
//! \effects If `has_value()` is `true`, equivalent to:
//! `reinit-expected(unex, val, e.error()); has_val = false;` Otherwise,
//! equivalent to: `unex = unexpected<E>(e.error());`
//! \returns `*this`.
template <class T, class E>
template <class G>
    requires(!std::is_reference_v<E> && std::is_constructible_v<E, const G&> && std::is_assignable_v<E&, const G&> &&
             (std::is_nothrow_constructible_v<E, const G&> || std::is_nothrow_move_constructible_v<T> ||
              std::is_nothrow_move_constructible_v<E>))
constexpr expected<T, E>& expected<T, E>::operator=(const unexpected<G>& e) {
    if (!has_val_) {
        unex_.error() = e.error();
    } else {
        detail::reinit_expected(unex_, val_, e.error());
        has_val_ = false;
    }
    return *this;
}

//! \also cvt-unexpected-assign
template <class T, class E>
template <class G>
    requires(!std::is_reference_v<E> && std::is_constructible_v<E, G> && std::is_assignable_v<E&, G> &&
             (std::is_nothrow_constructible_v<E, G> || std::is_nothrow_move_constructible_v<T> ||
              std::is_nothrow_move_constructible_v<E>))
constexpr expected<T, E>& expected<T, E>::operator=(unexpected<G>&& e) {
    if (!has_val_) {
        unex_.error() = std::move(e).error();
    } else {
        detail::reinit_expected(unex_, val_, std::move(e).error());
        has_val_ = false;
    }
    return *this;
}

// Rebinding assignment for reference E from reference G. Repoints unex_ (unexpected<E&>) to the
// external referent via construct_at — NOT `unex_.error() = ...`, which would mutate the old
// pointee instead of rebinding. Binding is noexcept; e.error() is the shallow external E&.
//! \group cvt-unexpected-assign-ref
//! \constraints `is_reference_v<G>` is `true`; and `is_constructible_v<E,
//! G>` is `true`; and `reference_constructs_from_temporary_v<E, G>` is
//! `false`.
//! \effects Rebinds `unex` to refer to the same object as `e.error()`,
//! destroying `val` first if `has_value()` is `true`.
//! \ensures `has_value()` is `false`.
//! \returns `*this`.
//! \remarks This operator never throws: the referent is bound, not
//! copied.
template <class T, class E>
template <class G>
    requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
             !detail::reference_constructs_from_temporary_v<E, G>)
constexpr expected<T, E>& expected<T, E>::operator=(const unexpected<G>& e) {
    if (has_val_) {
        std::destroy_at(std::addressof(val_));
        std::construct_at(std::addressof(unex_), e.error());
        has_val_ = false;
    } else {
        std::construct_at(std::addressof(unex_), e.error());
    }
    return *this;
}

//! \also cvt-unexpected-assign-ref
template <class T, class E>
template <class G>
    requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
             !detail::reference_constructs_from_temporary_v<E, G>)
constexpr expected<T, E>& expected<T, E>::operator=(unexpected<G>&& e) {
    if (has_val_) {
        std::destroy_at(std::addressof(val_));
        std::construct_at(std::addressof(unex_), e.error());
        has_val_ = false;
    } else {
        std::construct_at(std::addressof(unex_), e.error());
    }
    return *this;
}

// =============================================================================
// [expected.object.assign] Out-of-line emplace definitions
// =============================================================================

//! \constraints `is_nothrow_constructible_v<T, Args...>` is `true`.
//! \effects Equivalent to: `if (has_value()) { destroy_at(addressof(val));
//! } else { destroy_at(addressof(unex)); has_val = true; } return
//! *construct_at(addressof(val), std::forward<Args>(args)...);`
template <class T, class E>
template <class... Args>
    requires std::is_nothrow_constructible_v<T, Args...>
constexpr T& expected<T, E>::emplace(Args&&... args) noexcept {
    if (has_val_)
        std::destroy_at(std::addressof(val_));
    else
        std::destroy_at(std::addressof(unex_));
    std::construct_at(std::addressof(val_), std::forward<Args>(args)...);
    has_val_ = true;
    return val_;
}

//! \constraints `is_nothrow_constructible_v<T, initializer_list<U>&,
//! Args...>` is `true`.
//! \effects Equivalent to: `if (has_value()) {
//! destroy_at(addressof(val)); } else { destroy_at(addressof(unex));
//! has_val = true; } return *construct_at(addressof(val), il,
//! std::forward<Args>(args)...);`
template <class T, class E>
template <class U, class... Args>
    requires std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>
constexpr T& expected<T, E>::emplace(std::initializer_list<U> il, Args&&... args) noexcept {
    if (has_val_)
        std::destroy_at(std::addressof(val_));
    else
        std::destroy_at(std::addressof(unex_));
    std::construct_at(std::addressof(val_), il, std::forward<Args>(args)...);
    has_val_ = true;
    return val_;
}

// \rSec3[expected.object.swap]{Swap}

//! \constraints `is_swappable_v<T>` is `true` and (`is_reference_v<E>` or
//! `is_swappable_v<E>`) is `true`, and `is_move_constructible_v<T> &&
//! is_move_constructible_v<E>` is `true`, and
//! `is_nothrow_move_constructible_v<T> ||
//! is_nothrow_move_constructible_v<E>` is `true`.
//! \effects If `this->has_value()` and `rhs.has_value()`, equivalent to
//! `using std::swap; swap(val, rhs.val);`. If neither `*this` nor `rhs`
//! contains a value, equivalent to `using std::swap; swap(unex,
//! rhs.unex);`. If `rhs.has_value()` is `false` and `this->has_value()` is
//! `true`, exchanges the value and error between `*this` and `rhs`
//! (moving through a temporary so a failed move leaves both objects
//! unchanged), leaving `has_value()` `false` and `rhs.has_value()` `true`.
//! If `rhs.has_value()` is `true` and `this->has_value()` is `false`,
//! equivalent to `rhs.swap(*this)`.
//! \throws Any exception thrown by the expressions in the Effects.
//! \remarks The exception specification is equivalent to
//! `is_nothrow_move_constructible_v<T> && is_nothrow_swappable_v<T> &&
//! is_nothrow_move_constructible_v<E> && (is_reference_v<E> ||
//! is_nothrow_swappable_v<E>)`.
template <class T, class E>
constexpr void expected<T, E>::swap(expected& rhs) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                            std::is_nothrow_swappable_v<T> &&
                                                            std::is_nothrow_move_constructible_v<E> &&
                                                            (std::is_reference_v<E> || std::is_nothrow_swappable_v<E>))
    requires(std::is_swappable_v<T> && (std::is_reference_v<E> || std::is_swappable_v<E>) &&
             std::is_move_constructible_v<T> && std::is_move_constructible_v<E> &&
             (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>))
{
    if (has_val_ && rhs.has_val_) {
        using std::swap;
        swap(val_, rhs.val_);
    } else if (!has_val_ && !rhs.has_val_) {
        using std::swap;
        swap(unex_, rhs.unex_);
    } else if (has_val_) {
        // this has value, rhs has error
        if constexpr (std::is_nothrow_move_constructible_v<E>) {
            unexpected<E> tmp(std::move(rhs.unex_));
            std::destroy_at(std::addressof(rhs.unex_));
            if constexpr (std::is_nothrow_move_constructible_v<T>) {
                std::construct_at(std::addressof(rhs.val_), std::move(val_));
                std::destroy_at(std::addressof(val_));
                std::construct_at(std::addressof(unex_), std::move(tmp));
            } else {
                try {
                    std::construct_at(std::addressof(rhs.val_), std::move(val_));
                    std::destroy_at(std::addressof(val_));
                    std::construct_at(std::addressof(unex_), std::move(tmp));
                } catch (...) {
                    std::construct_at(std::addressof(rhs.unex_), std::move(tmp));
                    throw;
                }
            }
        } else {
            T tmp(std::move(val_));
            std::destroy_at(std::addressof(val_));
            try {
                std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
                std::destroy_at(std::addressof(rhs.unex_));
                std::construct_at(std::addressof(rhs.val_), std::move(tmp));
            } catch (...) {
                std::construct_at(std::addressof(val_), std::move(tmp));
                throw;
            }
        }
        has_val_     = false;
        rhs.has_val_ = true;
    } else {
        // this has error, rhs has value
        rhs.swap(*this);
    }
}

// =============================================================================
// \rSec3[expected.object.obs]{Observers}

//! \group obs-arrow
//! \hardexpects `has_value()` is `true`.
//! \returns `addressof(val)`.
template <class T, class E>
constexpr const T* expected<T, E>::operator->() const noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (!has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return std::addressof(val_);
}

//! \also obs-arrow
template <class T, class E>
constexpr T* expected<T, E>::operator->() noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (!has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return std::addressof(val_);
}

//! \group obs-star-lval
//! \hardexpects `has_value()` is `true`.
//! \returns `val`.
template <class T, class E>
constexpr const T& expected<T, E>::operator*() const& noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (!has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return val_;
}

//! \also obs-star-lval
template <class T, class E>
constexpr T& expected<T, E>::operator*() & noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (!has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return val_;
}

//! \group obs-star-rval
//! \hardexpects `has_value()` is `true`.
//! \returns `std::move(val)`.
template <class T, class E>
constexpr const T&& expected<T, E>::operator*() const&& noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (!has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return std::move(val_);
}

//! \also obs-star-rval
template <class T, class E>
constexpr T&& expected<T, E>::operator*() && noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (!has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return std::move(val_);
}

//! \group obs-bool
//! \returns `has_val`.
template <class T, class E>
constexpr expected<T, E>::operator bool() const noexcept {
    return has_val_;
}

//! \also obs-bool
template <class T, class E>
constexpr bool expected<T, E>::has_value() const noexcept {
    return has_val_;
}

//! \group obs-value-lval
//! \mandates `is_copy_constructible_v<E>` is `true`.
//! \returns `val`, if `has_value()` is `true`.
//! \throws `bad_expected_access(as_const(error()))` if `has_value()` is
//! `false`.
template <class T, class E>
constexpr const T& expected<T, E>::value() const& {
    static_assert(std::is_copy_constructible_v<error_value_type>, "value() requires is_copy_constructible_v<E>");
    if (!has_val_)
        throw bad_expected_access<error_value_type>(unex_.error());
    return val_;
}

//! \also obs-value-lval
template <class T, class E>
constexpr T& expected<T, E>::value() & {
    static_assert(std::is_copy_constructible_v<error_value_type>, "value() requires is_copy_constructible_v<E>");
    if (!has_val_)
        throw bad_expected_access<error_value_type>(unex_.error());
    return val_;
}

//! \group obs-value-rval
//! \mandates `is_copy_constructible_v<E>` is `true` and
//! `is_constructible_v<E, decltype(std::move(error()))>` is `true`.
//! \returns `std::move(val)`, if `has_value()` is `true`.
//! \throws `bad_expected_access(std::move(error()))` if `has_value()` is
//! `false`.
template <class T, class E>
constexpr const T&& expected<T, E>::value() const&& {
    if constexpr (std::is_reference_v<E>) {
        static_assert(std::is_copy_constructible_v<error_value_type> && std::is_move_constructible_v<error_value_type>,
                      "value() const&& requires E to be copy and move constructible");
    } else {
        static_assert(std::is_copy_constructible_v<E> && std::is_constructible_v<E, decltype(std::move(error()))>,
                      "value() && requires E be copy-constructible and constructible from move(error())");
    }
    if (!has_val_)
        throw bad_expected_access<error_value_type>(std::move(unex_).error());
    return std::move(val_);
}

//! \also obs-value-rval
template <class T, class E>
constexpr T&& expected<T, E>::value() && {
    if constexpr (std::is_reference_v<E>) {
        static_assert(std::is_copy_constructible_v<error_value_type> && std::is_move_constructible_v<error_value_type>,
                      "value() && requires E to be copy and move constructible");
    } else {
        static_assert(std::is_copy_constructible_v<E> && std::is_constructible_v<E, decltype(std::move(error()))>,
                      "value() && requires E be copy-constructible and constructible from move(error())");
    }
    if (!has_val_)
        throw bad_expected_access<error_value_type>(std::move(unex_).error());
    return std::move(val_);
}

//! \group obs-error-lval
//! \hardexpects `has_value()` is `false`.
//! \returns `unex.error()`.
template <class T, class E>
constexpr const E& expected<T, E>::error() const& noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return unex_.error();
}

//! \also obs-error-lval
template <class T, class E>
constexpr E& expected<T, E>::error() & noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return unex_.error();
}

//! \group obs-error-rval
//! \hardexpects `has_value()` is `false`.
//! \returns `std::move(unex).error()`.
template <class T, class E>
constexpr const E&& expected<T, E>::error() const&& noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return std::move(unex_).error();
}

//! \also obs-error-rval
template <class T, class E>
constexpr E&& expected<T, E>::error() && noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return std::move(unex_).error();
}

//! \mandates `is_copy_constructible_v<T>` is `true` and
//! `is_convertible_v<U, T>` is `true`.
//! \returns `has_value() ? **this : static_cast<T>(std::forward<U>(def))`.
template <class T, class E>
template <class U>
constexpr T expected<T, E>::value_or(U&& def) const& {
    static_assert(std::is_copy_constructible_v<T>, "value_or requires is_copy_constructible_v<T>");
    static_assert(std::is_convertible_v<U, T>, "value_or requires is_convertible_v<U, T>");
    if (has_val_)
        return val_;
    return static_cast<T>(std::forward<U>(def));
}

//! \mandates `is_move_constructible_v<T>` is `true` and
//! `is_convertible_v<U, T>` is `true`.
//! \returns `has_value() ? std::move(**this) :
//! static_cast<T>(std::forward<U>(def))`.
template <class T, class E>
template <class U>
constexpr T expected<T, E>::value_or(U&& def) && {
    static_assert(std::is_move_constructible_v<T>, "value_or requires is_move_constructible_v<T>");
    static_assert(std::is_convertible_v<U, T>, "value_or requires is_convertible_v<U, T>");
    if (has_val_)
        return std::move(val_);
    return static_cast<T>(std::forward<U>(def));
}

//! \mandates `is_copy_constructible_v<error_value_type>` is `true` and
//! `is_convertible_v<G, error_value_type>` is `true`.
//! \returns `std::forward<G>(def)` if `has_value()` is `true`, `error()`
//! otherwise.
template <class T, class E>
template <class G>
    requires(std::is_copy_constructible_v<typename expected<T, E>::error_value_type> &&
             std::is_convertible_v<G, typename expected<T, E>::error_value_type>)
constexpr typename expected<T, E>::error_value_type expected<T, E>::error_or(G&& def) const& {
    if (!has_val_)
        return unex_.error();
    return static_cast<error_value_type>(std::forward<G>(def));
}

//! \mandates `is_move_constructible_v<error_value_type>` is `true` and
//! `is_convertible_v<G, error_value_type>` is `true`.
//! \returns `std::forward<G>(def)` if `has_value()` is `true`,
//! `std::move(error())` otherwise.
template <class T, class E>
template <class G>
    requires(std::is_move_constructible_v<typename expected<T, E>::error_value_type> &&
             std::is_convertible_v<G, typename expected<T, E>::error_value_type>)
constexpr typename expected<T, E>::error_value_type expected<T, E>::error_or(G&& def) && {
    if (!has_val_)
        return std::move(unex_).error();
    return static_cast<error_value_type>(std::forward<G>(def));
}

// =============================================================================
// \rSec3[expected.object.monadic]{Monadic operations}

//! \group monadic-and-then-lval
//! \constraints `is_constructible_v<E, decltype(error())>` is `true`.
//! \mandates `remove_cvref_t<invoke_result_t<F, decltype((val))>>` is a
//! specialization of `expected` and its `error_type` is the same type as
//! `E`.
//! \effects Equivalent to: `if (has_value()) return
//! invoke(std::forward<F>(f), val); else return U(unexpect, error());`
//! where `U` is `remove_cvref_t<invoke_result_t<F, decltype((val))>>`.
template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, E&>
constexpr auto expected<T, E>::and_then(F&& f) & {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f), val_);
    return U(unexpect, unex_.error());
}

//! \group monadic-and-then-rval
//! \constraints `is_constructible_v<E, decltype(std::move(error()))>` is
//! `true`.
//! \mandates `remove_cvref_t<invoke_result_t<F, decltype(std::move(val))>>`
//! is a specialization of `expected` and its `error_type` is the same
//! type as `E`.
//! \effects Equivalent to: `if (has_value()) return
//! invoke(std::forward<F>(f), std::move(val)); else return U(unexpect,
//! std::move(error()));` where `U` is `remove_cvref_t<invoke_result_t<F,
//! decltype(std::move(val))>>`.
template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, E&&>
constexpr auto expected<T, E>::and_then(F&& f) && {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T&&>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f), std::move(val_));
    return U(unexpect, std::move(unex_).error());
}

//! \also monadic-and-then-lval
template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, const E&>
constexpr auto expected<T, E>::and_then(F&& f) const& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const T&>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f), val_);
    return U(unexpect, unex_.error());
}

//! \also monadic-and-then-rval
template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, const E&&>
constexpr auto expected<T, E>::and_then(F&& f) const&& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const T&&>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f), std::move(val_));
    return U(unexpect, std::move(unex_).error());
}

//! \group monadic-or-else-lval
//! \constraints `is_constructible_v<T, decltype((val))>` is `true`.
//! \mandates `remove_cvref_t<invoke_result_t<F, decltype(error())>>` is a
//! specialization of `expected` and its `value_type` is the same type as
//! `T`.
//! \effects Equivalent to: `if (has_value()) return G(in_place, val); else
//! return invoke(std::forward<F>(f), error());` where `G` is
//! `remove_cvref_t<invoke_result_t<F, decltype(error())>>`.
template <class T, class E>
template <class F>
    requires std::is_constructible_v<T, T&>
constexpr auto expected<T, E>::or_else(F&& f) & {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, T>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G(std::in_place, val_);
    return std::invoke(std::forward<F>(f), unex_.error());
}

//! \group monadic-or-else-rval
//! \constraints `is_constructible_v<T, decltype(std::move(val))>` is
//! `true`.
//! \mandates `remove_cvref_t<invoke_result_t<F,
//! decltype(std::move(error()))>>` is a specialization of `expected` and
//! its `value_type` is the same type as `T`.
//! \effects Equivalent to: `if (has_value()) return G(in_place,
//! std::move(val)); else return invoke(std::forward<F>(f),
//! std::move(error()));` where `G` is `remove_cvref_t<invoke_result_t<F,
//! decltype(std::move(error()))>>`.
template <class T, class E>
template <class F>
    requires std::is_constructible_v<T, T&&>
constexpr auto expected<T, E>::or_else(F&& f) && {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, T>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G(std::in_place, std::move(val_));
    return std::invoke(std::forward<F>(f), std::move(unex_).error());
}

//! \also monadic-or-else-lval
template <class T, class E>
template <class F>
    requires std::is_constructible_v<T, const T&>
constexpr auto expected<T, E>::or_else(F&& f) const& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, T>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G(std::in_place, val_);
    return std::invoke(std::forward<F>(f), unex_.error());
}

//! \also monadic-or-else-rval
template <class T, class E>
template <class F>
    requires std::is_constructible_v<T, const T&&>
constexpr auto expected<T, E>::or_else(F&& f) const&& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, T>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G(std::in_place, std::move(val_));
    return std::invoke(std::forward<F>(f), std::move(unex_).error());
}

//! \group monadic-transform-lval
//! \constraints `is_constructible_v<E, decltype(error())>` is `true`.
//! \effects Equivalent to: `if (!has_value()) return U(unexpect,
//! error()); else return expected<U2, E>(in_place,
//! invoke(std::forward<F>(f), val));` where `U2` is
//! `remove_cv_t<invoke_result_t<F, decltype((val))>>` and `U` is
//! `expected<U2, E>`.
template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, E&>
constexpr auto expected<T, E>::transform(F&& f) & {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f), val_);
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, unex_.error());
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f), val_));
        return expected<U, E>(unexpect, unex_.error());
    }
}

//! \group monadic-transform-rval
//! \constraints `is_constructible_v<E, decltype(std::move(error()))>` is
//! `true`.
//! \effects Equivalent to: `if (!has_value()) return
//! U(unexpect, std::move(error())); else return expected<U2,
//! E>(in_place, invoke(std::forward<F>(f), std::move(val)));` where `U2`
//! is `remove_cv_t<invoke_result_t<F, decltype(std::move(val))>>` and `U`
//! is `expected<U2, E>`.
template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, E&&>
constexpr auto expected<T, E>::transform(F&& f) && {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&&>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f), std::move(val_));
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, std::move(unex_).error());
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f), std::move(val_)));
        return expected<U, E>(unexpect, std::move(unex_).error());
    }
}

//! \also monadic-transform-lval
template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, const E&>
constexpr auto expected<T, E>::transform(F&& f) const& {
    using U = std::remove_cv_t<std::invoke_result_t<F, const T&>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f), val_);
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, unex_.error());
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f), val_));
        return expected<U, E>(unexpect, unex_.error());
    }
}

//! \also monadic-transform-rval
template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, const E&&>
constexpr auto expected<T, E>::transform(F&& f) const&& {
    using U = std::remove_cv_t<std::invoke_result_t<F, const T&&>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f), std::move(val_));
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, std::move(unex_).error());
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f), std::move(val_)));
        return expected<U, E>(unexpect, std::move(unex_).error());
    }
}

//! \group monadic-transform-error-lval
//! \constraints `is_constructible_v<T, decltype((val))>` is `true`.
//! \effects Equivalent to: `if (has_value()) return G(in_place, val); else
//! return expected<T, G2>(unexpect, invoke(std::forward<F>(f),
//! error()));` where `G2` is `remove_cv_t<invoke_result_t<F,
//! decltype(error())>>` and `G` is `expected<T, G2>`.
template <class T, class E>
template <class F>
    requires std::is_constructible_v<T, T&>
constexpr auto expected<T, E>::transform_error(F&& f) & {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<T, G>(std::in_place, val_);
    return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), unex_.error()));
}

//! \group monadic-transform-error-rval
//! \constraints `is_constructible_v<T, decltype(std::move(val))>` is
//! `true`.
//! \effects Equivalent to: `if (has_value()) return G(in_place,
//! std::move(val)); else return expected<T, G2>(unexpect,
//! invoke(std::forward<F>(f), std::move(error())));` where `G2` is
//! `remove_cv_t<invoke_result_t<F, decltype(std::move(error()))>>` and
//! `G` is `expected<T, G2>`.
template <class T, class E>
template <class F>
    requires std::is_constructible_v<T, T&&>
constexpr auto expected<T, E>::transform_error(F&& f) && {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<T, G>(std::in_place, std::move(val_));
    return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), std::move(unex_).error()));
}

//! \also monadic-transform-error-lval
template <class T, class E>
template <class F>
    requires std::is_constructible_v<T, const T&>
constexpr auto expected<T, E>::transform_error(F&& f) const& {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<T, G>(std::in_place, val_);
    return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), unex_.error()));
}

//! \also monadic-transform-error-rval
template <class T, class E>
template <class F>
    requires std::is_constructible_v<T, const T&&>
constexpr auto expected<T, E>::transform_error(F&& f) const&& {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<T, G>(std::in_place, std::move(val_));
    return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), std::move(unex_).error()));
}

// \rSec3[expected.object.eq]{Equality operators}

// =============================================================================
// [expected.void] Partial specialization for void value type
// =============================================================================

// \rSec2[expected.void]{Partial specialization of expected for void types}
// \rSec3[expected.void.general]{General}
//! \at expected.void.general
//! \mandates A program that instantiates the definition of `expected<T, E>`
//! with an `E` that is not a valid template argument for `unexpected` is
//! ill-formed.
//! \remarks Any object of type `expected<T, E>` either represents a value
//! of type `T`, or contains a value of type `E` nested within it. Member
//! `has_val` indicates whether the `expected<T, E>` object represents a
//! value of type `T`. When `has_value()` is `false`, the error is
//! `unex.error()`.
template <class E>
class expected<void, E> {
    static_assert(!std::is_rvalue_reference_v<E>, "E must not be an rvalue reference");
    static_assert(!std::is_void_v<std::remove_reference_t<E>>, "E must not be void");
    static_assert(!std::is_array_v<std::remove_reference_t<E>>, "E must not be an array type");
    static_assert(std::is_reference_v<E> || std::is_same_v<std::remove_cv_t<E>, E>, "E must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<std::remove_reference_t<E>>>::value,
                  "E must not be an unexpected<X> specialization");

  private:
    //! \expos
    using error_value_type = std::remove_cv_t<std::remove_reference_t<E>>;

  public:
    using value_type      = void;
    using error_type      = E;
    using unexpected_type = unexpected<E>;

    template <class U>
    using rebind = expected<U, error_type>;

    // -------------------------------------------------------------------------
    // [expected.void.cons] Constructors
    // -------------------------------------------------------------------------

    constexpr expected() noexcept;

    // Unconstrained trivial-path candidate: see the primary template's copy
    // constructor for why (the sole declaration when E is not copy
    // constructible at all; subsumed by the non-trivial path otherwise).
    //! \at expected.void.cons
    //! \merge
    constexpr expected(const expected&) = default;

    constexpr expected(const expected& rhs) noexcept(std::is_nothrow_copy_constructible_v<E>)
        requires(std::is_copy_constructible_v<E> && !std::is_trivially_copy_constructible_v<E>);

    // Unconstrained; no explicit noexcept — see the primary template's move
    // constructor for why.
    //! \at expected.void.cons
    //! \merge
    constexpr expected(expected&&) = default;

    constexpr expected(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E>)
        requires(std::is_move_constructible_v<E> && !std::is_trivially_move_constructible_v<E>);

    // Converting constructor from expected<U, G> where is_void_v<U>. Excludes U,G exactly
    // matching this class's own void,E (the real copy/move constructors already handle that
    // case) — instantiating this template for the self-referential case would otherwise probe
    // unexpected<E>'s constructibility from this very class, which some standard library
    // implementations of reference_constructs_from_temporary_v resolve as a circular constraint.
    template <class U, class G>
        requires(std::is_void_v<U> && !std::is_reference_v<E> && !std::is_same_v<G, E> &&
                 std::is_constructible_v<E, const G&> && !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
    constexpr explicit(!std::is_convertible_v<const G&, E>) expected(const expected<U, G>& rhs);

    template <class U, class G>
        requires(std::is_void_v<U> && !std::is_reference_v<E> && !std::is_same_v<G, E> &&
                 std::is_constructible_v<E, G> && !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
    constexpr explicit(!std::is_convertible_v<G, E>) expected(expected<U, G>&& rhs);

    // Constructor from unexpected<G> const& / && — value-E path
    template <class G>
        requires(!std::is_reference_v<E> && std::is_constructible_v<E, const G&>)
    constexpr explicit(!std::is_convertible_v<const G&, E>) expected(const unexpected<G>& e);

    template <class G>
        requires(!std::is_reference_v<E> && std::is_constructible_v<E, G>)
    constexpr explicit(!std::is_convertible_v<G, E>) expected(unexpected<G>&& e);

    // Constructor from unexpected<G> — reference-E path. Allowed only when G is itself a reference,
    // so e.error() refers to an external object and binding E& cannot dangle. No const_cast needed.
    template <class G>
        requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
                 !detail::reference_constructs_from_temporary_v<E, G>)
    constexpr explicit(!std::is_convertible_v<G, E>) expected(const unexpected<G>& e) noexcept;

    template <class G>
        requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
                 !detail::reference_constructs_from_temporary_v<E, G>)
    constexpr explicit(!std::is_convertible_v<G, E>) expected(unexpected<G>&& e) noexcept;

    // Deleted for reference E with value G: the referent lives inside the temporary unexpected<G>, so
    // binding E& to it would dangle once the source is destroyed. Use (unexpect, lvalue) instead.
    //! \at expected.void.cons
    //! \group void-cvt-unexpected-ctor-deleted
    //! \remarks When `E` is a reference type, an overload taking
    //! `unexpected<G>` for a non-reference `G` is defined as deleted: the
    //! referent would live inside the (possibly temporary) source
    //! `unexpected<G>` object, and binding `E&` to it would dangle. Use an
    //! `unexpected<E&>` holding an external object instead.
    template <class G>
        requires(std::is_reference_v<E> && !std::is_reference_v<G>)
    constexpr expected(const unexpected<G>&) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<void,E&>: cannot construct from unexpected<value>; the value would dangle — use unexpected<E&>");

    //! \at expected.void.cons
    //! \also void-cvt-unexpected-ctor-deleted
    template <class G>
        requires(std::is_reference_v<E> && !std::is_reference_v<G>)
    constexpr expected(unexpected<G>&&) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<void,E&>: cannot construct from unexpected<value>; the value would dangle — use unexpected<E&>");

    // In-place constructor for value (no args, just marks has-value)
    constexpr explicit expected(std::in_place_t) noexcept;

    // In-place constructor for error
    template <class... Args>
        requires(std::is_constructible_v<E, Args...> && !detail::unexpect_dangles_v<E, Args...>)
    constexpr explicit expected(unexpect_t, Args&&... args);

    // Deleted: single argument would bind E& to a temporary — dangling prevention
    //! \at expected.void.cons
    //! \group void-unexpect-ctor-deleted
    //! \remarks When `E` is a reference type, an overload with the same
    //! parameter types is defined as deleted if the single argument would
    //! bind `E&` to a temporary, or if it is otherwise not usable to
    //! construct `E`.
    template <class... Args>
        requires(detail::unexpect_dangles_v<E, Args...>)
    constexpr expected(unexpect_t, Args&&...) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<void,E&>: unexpect argument would bind a temporary that dangles; pass an lvalue reference");

    // Deleted catch-all: reference E, argument neither constructible nor a dangling case
    // (e.g. binding a non-const E& from a const lvalue).
    //! \at expected.void.cons
    //! \also void-unexpect-ctor-deleted
    template <class... Args>
        requires(std::is_reference_v<E> && !std::is_constructible_v<E, Args...> &&
                 !detail::unexpect_dangles_v<E, Args...>)
    constexpr expected(unexpect_t, Args&&...) =
        BEMAN_EXPECTED_DELETE_MSG("expected<void,E&>: no viable conversion from the given argument(s) to E&");

    // In-place constructor for error with initializer_list
    template <class U, class... Args>
        requires(!std::is_reference_v<E> && std::is_constructible_v<E, std::initializer_list<U>&, Args...>)
    constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&... args);

    //! \at expected.void.cons
    //! \remarks An overload with the same parameter types is defined as
    //! deleted when `E` is an lvalue reference type. An initializer list
    //! cannot provide the required long-lived error referent.
    template <class U, class... Args>
        requires std::is_reference_v<E>
    constexpr expected(unexpect_t, std::initializer_list<U>, Args&&...) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<void,E&>: initializer-list error construction cannot bind a reference; pass an lvalue reference");

    // Converting constructor from expected<void, G&> — reference-E path only. G is itself a
    // reference to an external object, so binding E& to it cannot dangle regardless of the
    // source's value category, provided the reference conversion itself does not materialize a
    // temporary (e.g. a base-from-derived or qualification conversion is fine; a user-defined
    // conversion that returns by value is not). Mirrors the unexpected<G> reference-E path above.
    template <class G>
        requires(std::is_reference_v<E> && std::is_convertible_v<G&, E> &&
                 !detail::reference_constructs_from_temporary_v<E, G&>)
    constexpr explicit(!std::is_convertible_v<G&, E>) expected(const expected<void, G&>& rhs);

    template <class G>
        requires(std::is_reference_v<E> && std::is_convertible_v<G&, E> &&
                 !detail::reference_constructs_from_temporary_v<E, G&>)
    constexpr explicit(!std::is_convertible_v<G&, E>) expected(expected<void, G&>&& rhs);

    // -------------------------------------------------------------------------
    // [expected.void.dtor] Destructor
    // -------------------------------------------------------------------------

    //! \at expected.void.dtor
    //! \merge
    constexpr ~expected()
        requires std::is_trivially_destructible_v<E>
    = default;

    constexpr ~expected()
        requires(!std::is_trivially_destructible_v<E>);

    // -------------------------------------------------------------------------
    // [expected.void.assign] Assignment
    // -------------------------------------------------------------------------

    // Copy assignment (trivial path)
    //! \at expected.void.assign
    //! \merge
    constexpr expected& operator=(const expected&)
        requires(std::is_trivially_copy_constructible_v<E> && std::is_trivially_copy_assignable_v<E> &&
                 std::is_trivially_destructible_v<E>)
    = default;

    // Copy assignment (non-trivial path)
    constexpr expected& operator=(const expected& rhs) noexcept(std::is_nothrow_copy_constructible_v<E> &&
                                                                std::is_nothrow_copy_assignable_v<E>)
        requires((std::is_reference_v<E> || (std::is_copy_constructible_v<E> && std::is_copy_assignable_v<E>)) &&
                 !(std::is_trivially_copy_constructible_v<E> && std::is_trivially_copy_assignable_v<E> &&
                   std::is_trivially_destructible_v<E>));

    // Move assignment (trivial path)
    //! \at expected.void.assign
    //! \merge
    constexpr expected& operator=(expected&&) noexcept
        requires(std::is_trivially_move_constructible_v<E> && std::is_trivially_move_assignable_v<E> &&
                 std::is_trivially_destructible_v<E>)
    = default;

    // Move assignment (non-trivial path)
    constexpr expected& operator=(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                           std::is_nothrow_move_assignable_v<E>)
        requires((std::is_reference_v<E> || (std::is_move_constructible_v<E> && std::is_move_assignable_v<E>)) &&
                 !(std::is_trivially_move_constructible_v<E> && std::is_trivially_move_assignable_v<E> &&
                   std::is_trivially_destructible_v<E>));

    // Assignment from unexpected<G> — value-E path.
    template <class G>
        requires(!std::is_reference_v<E> && std::is_constructible_v<E, const G&> && std::is_assignable_v<E&, const G&>)
    constexpr expected& operator=(const unexpected<G>& e);

    template <class G>
        requires(!std::is_reference_v<E> && std::is_constructible_v<E, G> && std::is_assignable_v<E&, G>)
    constexpr expected& operator=(unexpected<G>&& e);

    // Rebinding assignment for reference E from reference G — binds unex_ to the external referent.
    template <class G>
        requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
                 !detail::reference_constructs_from_temporary_v<E, G>)
    constexpr expected& operator=(const unexpected<G>& e);

    template <class G>
        requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
                 !detail::reference_constructs_from_temporary_v<E, G>)
    constexpr expected& operator=(unexpected<G>&& e);

    // Deleted for reference E with value G: would bind E& to storage inside the temporary unexpected.
    //! \at expected.void.assign
    //! \group void-cvt-unexpected-assign-deleted
    //! \remarks When `E` is a reference type, an overload taking
    //! `unexpected<G>` for a non-reference `G` is defined as deleted: it
    //! would rebind `unex` to `unexpected<G>`'s temporary storage.
    template <class G>
        requires(std::is_reference_v<E> && !std::is_reference_v<G>)
    constexpr expected& operator=(const unexpected<G>&) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<void,E&>: cannot assign from unexpected<value>; the value would dangle — use unexpected<E&>");

    //! \at expected.void.assign
    //! \also void-cvt-unexpected-assign-deleted
    template <class G>
        requires(std::is_reference_v<E> && !std::is_reference_v<G>)
    constexpr expected& operator=(unexpected<G>&&) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<void,E&>: cannot assign from unexpected<value>; the value would dangle — use unexpected<E&>");

    constexpr void emplace() noexcept;

    // -------------------------------------------------------------------------
    // [expected.void.swap] Swap
    // -------------------------------------------------------------------------

    constexpr void swap(expected& rhs) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                (std::is_reference_v<E> || std::is_nothrow_swappable_v<E>))
        requires((std::is_reference_v<E> || std::is_swappable_v<E>) && std::is_move_constructible_v<E>);

    //! \at expected.void.swap
    //! \effects Equivalent to `x.swap(y)`.
    friend constexpr void swap(expected& x, expected& y) noexcept(noexcept(x.swap(y)))
        requires((std::is_reference_v<E> || std::is_swappable_v<E>) && std::is_move_constructible_v<E>)
    {
        x.swap(y);
    }

    // -------------------------------------------------------------------------
    // [expected.void.obs] Observers
    // -------------------------------------------------------------------------

    constexpr explicit operator bool() const noexcept;
    constexpr bool     has_value() const noexcept;

    constexpr void operator*() const noexcept;

    constexpr void value() const&;
    constexpr void value() &&;

    // error() — shallow const for reference E: always returns E& regardless of const on expected
    constexpr const E&  error() const& noexcept;
    constexpr E&        error() & noexcept;
    constexpr const E&& error() const&& noexcept;
    constexpr E&&       error() && noexcept;

    template <class G = error_value_type>
        requires(std::is_copy_constructible_v<std::remove_cv_t<std::remove_reference_t<E>>> &&
                 std::is_convertible_v<G, std::remove_cv_t<std::remove_reference_t<E>>>)
    constexpr error_value_type error_or(G&& def) const&;

    template <class G = error_value_type>
        requires(std::is_move_constructible_v<std::remove_cv_t<std::remove_reference_t<E>>> &&
                 std::is_convertible_v<G, std::remove_cv_t<std::remove_reference_t<E>>>)
    constexpr error_value_type error_or(G&& def) &&;

    // Deleted: value_or is not available for void expected. Gated to reference E only so that,
    // for value E, no value_or overload is declared at all (there is nothing to delete against).
    //! \at expected.void.obs
    //! \remarks `expected<void, E>` has no `value_or` member: there is no
    //! value to fall back from. This overload exists only to give a clear
    //! diagnostic when `E` is a reference type, and is defined as deleted.
    template <class U>
        requires std::is_reference_v<E>
    constexpr void value_or(U&&) const =
        BEMAN_EXPECTED_DELETE_MSG("expected<void,E>: value_or is not defined for void value_type; there is no "
                                  "value to fall back from — use has_value()/error()");

    // -------------------------------------------------------------------------
    // [expected.void.monadic] Monadic operations
    // -------------------------------------------------------------------------

    template <class F>
        requires std::is_constructible_v<E, E&>
    constexpr auto and_then(F&& f) &;
    template <class F>
        requires std::is_constructible_v<E, E&&>
    constexpr auto and_then(F&& f) &&;
    template <class F>
        requires std::is_constructible_v<E, const E&>
    constexpr auto and_then(F&& f) const&;
    template <class F>
        requires std::is_constructible_v<E, const E&&>
    constexpr auto and_then(F&& f) const&&;

    template <class F>
    constexpr auto or_else(F&& f) &;
    template <class F>
    constexpr auto or_else(F&& f) &&;
    template <class F>
    constexpr auto or_else(F&& f) const&;
    template <class F>
    constexpr auto or_else(F&& f) const&&;

    template <class F>
        requires std::is_constructible_v<E, E&>
    constexpr auto transform(F&& f) &;
    template <class F>
        requires std::is_constructible_v<E, E&&>
    constexpr auto transform(F&& f) &&;
    template <class F>
        requires std::is_constructible_v<E, const E&>
    constexpr auto transform(F&& f) const&;
    template <class F>
        requires std::is_constructible_v<E, const E&&>
    constexpr auto transform(F&& f) const&&;

    template <class F>
    constexpr auto transform_error(F&& f) &;
    template <class F>
    constexpr auto transform_error(F&& f) &&;
    template <class F>
    constexpr auto transform_error(F&& f) const&;
    template <class F>
    constexpr auto transform_error(F&& f) const&&;

    // -------------------------------------------------------------------------
    // [expected.void.eq] Equality operators (hidden friends)
    // -------------------------------------------------------------------------

    //! \at expected.void.eq
    //! \mandates `is_void_v<T2>` is `true`. The expression `x.error() ==
    //! y.error()` is well-formed and its result is convertible to `bool`.
    //! \returns If `x.has_value() != y.has_value()`, `false`; otherwise, if
    //! `x.has_value()` is `true`, `true`; otherwise `x.error() ==
    //! y.error()`.
    template <class T2, class E2>
        requires std::is_void_v<T2>
    friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y) {
        if (x.has_value() != y.has_value())
            return false;
        if (x.has_value())
            return true;
        return x.error() == y.error();
    }

    //! \at expected.void.eq
    //! \mandates The expression `x.error() == e.error()` is well-formed and
    //! its result is convertible to `bool`.
    //! \returns `!x.has_value() && static_cast<bool>(x.error() ==
    //! e.error())`.
    template <class E2>
    friend constexpr bool operator==(const expected& x, const unexpected<E2>& e) {
        return !x.has_value() && static_cast<bool>(x.error() == e.error());
    }

  private:
    //! \expos
    bool has_val_;
    union {
        //! \expos
        unexpected<E> unex_;
    };
};

// =============================================================================
// \rSec3[expected.void.cons]{Constructors}

//! \ensures `has_value()` is `true`.
template <class E>
constexpr expected<void, E>::expected() noexcept : has_val_(true) {}

//! \ensures `has_value()` is `true`.
template <class E>
constexpr expected<void, E>::expected(std::in_place_t) noexcept : has_val_(true) {}

//! \effects If `rhs.has_value()` is `false`, direct-non-list-initializes
//! `unex` with `rhs.error()`.
//! \ensures `rhs.has_value() == this->has_value()`.
//! \throws Any exception thrown by the initialization of `unex`.
//! \remarks This constructor is defined as deleted unless
//! `is_copy_constructible_v<E>` is `true` or `is_reference_v<E>` is
//! `true`. This constructor is trivial if
//! `is_trivially_copy_constructible_v<E>` is `true`.
template <class E>
constexpr expected<void, E>::expected(const expected& rhs) noexcept(std::is_nothrow_copy_constructible_v<E>)
    requires(std::is_copy_constructible_v<E> && !std::is_trivially_copy_constructible_v<E>)
    : has_val_(rhs.has_val_) {
    if (!has_val_)
        std::construct_at(std::addressof(unex_), rhs.unex_);
}

//! \effects If `rhs.has_value()` is `false`, direct-non-list-initializes
//! `unex` with `std::move(rhs.error())`.
//! \ensures `rhs.has_value()` is unchanged; `rhs.has_value() ==
//! this->has_value()` is `true`.
//! \throws Any exception thrown by the initialization of `unex`.
//! \remarks This constructor is trivial if
//! `is_trivially_move_constructible_v<E>` is `true`.
template <class E>
constexpr expected<void, E>::expected(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E>)
    requires(std::is_move_constructible_v<E> && !std::is_trivially_move_constructible_v<E>)
    : has_val_(rhs.has_val_) {
    if (!has_val_)
        std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
}

//! \group void-cvt-copy-ctor
//! \constraints `is_void_v<U>` is `true`; and `is_constructible_v<E, const
//! G&>` is `true`; and `is_constructible_v<unexpected<E>, expected<U,
//! G>&>` is `false`; and `is_constructible_v<unexpected<E>, expected<U,
//! G>>` is `false`; and `is_constructible_v<unexpected<E>, const
//! expected<U, G>&>` is `false`; and `is_constructible_v<unexpected<E>,
//! const expected<U, G>>` is `false`.
//! \effects If `rhs.has_value()` is `false`, direct-non-list-initializes
//! `unex` with `rhs.error()`.
//! \ensures `rhs.has_value()` is unchanged; `rhs.has_value() ==
//! this->has_value()` is `true`.
//! \throws Any exception thrown by the initialization of `unex`.
template <class E>
template <class U, class G>
    requires(std::is_void_v<U> && !std::is_reference_v<E> && !std::is_same_v<G, E> &&
             std::is_constructible_v<E, const G&> && !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
constexpr expected<void, E>::expected(const expected<U, G>& rhs) : has_val_(rhs.has_value()) {
    if (!has_val_)
        std::construct_at(std::addressof(unex_), rhs.error());
}

//! \also void-cvt-copy-ctor
template <class E>
template <class U, class G>
    requires(std::is_void_v<U> && !std::is_reference_v<E> && !std::is_same_v<G, E> && std::is_constructible_v<E, G> &&
             !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
constexpr expected<void, E>::expected(expected<U, G>&& rhs) : has_val_(rhs.has_value()) {
    if (!has_val_)
        std::construct_at(std::addressof(unex_), std::move(rhs).error());
}

//! \group void-cvt-unexpected-ctor
//! \constraints `is_constructible_v<E, const G&>` is `true`.
//! \effects Direct-non-list-initializes `unex` with `e.error()`.
//! \ensures `has_value()` is `false`.
//! \throws Any exception thrown by the initialization of `unex`.
template <class E>
template <class G>
    requires(!std::is_reference_v<E> && std::is_constructible_v<E, const G&>)
constexpr expected<void, E>::expected(const unexpected<G>& e) : has_val_(false) {
    std::construct_at(std::addressof(unex_), e.error());
}

//! \also void-cvt-unexpected-ctor
template <class E>
template <class G>
    requires(!std::is_reference_v<E> && std::is_constructible_v<E, G>)
constexpr expected<void, E>::expected(unexpected<G>&& e) : has_val_(false) {
    std::construct_at(std::addressof(unex_), std::move(e).error());
}

//! \group void-cvt-unexpected-ctor-ref
//! \constraints `is_reference_v<G>` is `true`; and `is_constructible_v<E,
//! G>` is `true`; and `reference_constructs_from_temporary_v<E, G>` is
//! `false`.
//! \effects Initializes `unex` with `e.error()`.
//! \ensures `has_value()` is `false`.
//! \remarks This constructor never throws: the referent is bound, not
//! copied.
template <class E>
template <class G>
    requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
             !detail::reference_constructs_from_temporary_v<E, G>)
constexpr expected<void, E>::expected(const unexpected<G>& e) noexcept : has_val_(false) {
    std::construct_at(std::addressof(unex_), e.error());
}

//! \also void-cvt-unexpected-ctor-ref
template <class E>
template <class G>
    requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
             !detail::reference_constructs_from_temporary_v<E, G>)
constexpr expected<void, E>::expected(unexpected<G>&& e) noexcept : has_val_(false) {
    std::construct_at(std::addressof(unex_), e.error());
}

//! \constraints `is_constructible_v<E, Args...>` is `true`.
//! \effects Direct-non-list-initializes `unex` with
//! `std::forward<Args>(args)...`.
//! \ensures `has_value()` is `false`.
//! \throws Any exception thrown by the initialization of `unex`.
template <class E>
template <class... Args>
    requires(std::is_constructible_v<E, Args...> && !detail::unexpect_dangles_v<E, Args...>)
constexpr expected<void, E>::expected(unexpect_t, Args&&... args) : has_val_(false) {
    std::construct_at(std::addressof(unex_), std::in_place, std::forward<Args>(args)...);
}

//! \constraints `is_reference_v<E>` is `false`, and `is_constructible_v<E,
//! initializer_list<U>&, Args...>` is `true`.
//! \effects Direct-non-list-initializes `unex` with `il,
//! std::forward<Args>(args)...`.
//! \ensures `has_value()` is `false`.
//! \throws Any exception thrown by the initialization of `unex`.
//! \remarks An overload with the same parameter types is defined as
//! deleted when `E` is an lvalue reference type. An initializer list
//! cannot provide the required long-lived error referent.
template <class E>
template <class U, class... Args>
    requires(!std::is_reference_v<E> && std::is_constructible_v<E, std::initializer_list<U>&, Args...>)
constexpr expected<void, E>::expected(unexpect_t, std::initializer_list<U> il, Args&&... args) : has_val_(false) {
    std::construct_at(std::addressof(unex_), std::in_place, il, std::forward<Args>(args)...);
}

//! \group void-cvt-copy-ctor-ref
//! \constraints `is_convertible_v<G&, E>` is `true`; and
//! `reference_constructs_from_temporary_v<E, G&>` is `false`.
//! \effects If `rhs.has_value()` is `false`, direct-non-list-initializes
//! `unex` with `rhs.error()`.
//! \ensures `rhs.has_value()` is unchanged; `rhs.has_value() ==
//! this->has_value()` is `true`.
//! \remarks This constructor never throws: the referent is bound, not
//! copied. It participates in overload resolution only when `E` is a
//! reference type, mirroring the `unexpected<G>` reference-`E` path
//! above.
template <class E>
template <class G>
    requires(std::is_reference_v<E> && std::is_convertible_v<G&, E> &&
             !detail::reference_constructs_from_temporary_v<E, G&>)
constexpr expected<void, E>::expected(const expected<void, G&>& rhs) : has_val_(rhs.has_value()) {
    if (!has_val_)
        std::construct_at(std::addressof(unex_), rhs.error());
}

//! \also void-cvt-copy-ctor-ref
template <class E>
template <class G>
    requires(std::is_reference_v<E> && std::is_convertible_v<G&, E> &&
             !detail::reference_constructs_from_temporary_v<E, G&>)
constexpr expected<void, E>::expected(expected<void, G&>&& rhs) : has_val_(rhs.has_value()) {
    if (!has_val_)
        std::construct_at(std::addressof(unex_), rhs.error());
}

// =============================================================================
// \rSec3[expected.void.dtor]{Destructor}

//! \effects If `has_value()` is `false`, destroys `unex`.
//! \remarks If `is_trivially_destructible_v<E>` is `true`, then this
//! destructor is a trivial destructor.
template <class E>
constexpr expected<void, E>::~expected()
    requires(!std::is_trivially_destructible_v<E>)
{
    if (!has_val_)
        std::destroy_at(std::addressof(unex_));
}

// =============================================================================
// \rSec3[expected.void.assign]{Assignment}

//! \effects If `this->has_value() && rhs.has_value()`, no effects.
//! Otherwise, if `this->has_value()`, equivalent to:
//! `construct_at(addressof(unex), rhs.unex); has_val = false;`
//! Otherwise, if `rhs.has_value()`, destroys `unex` and sets `has_val` to
//! `true`. Otherwise, equivalent to `unex = rhs.unex`.
//! \returns `*this`.
//! \remarks This operator is defined as deleted unless
//! `is_copy_assignable_v<E>` is `true` or `is_reference_v<E>` is `true`
//! and `is_copy_constructible_v<E>` is `true` or `is_reference_v<E>` is
//! `true`. This operator is trivial if
//! `is_trivially_copy_constructible_v<E>`,
//! `is_trivially_copy_assignable_v<E>`, and
//! `is_trivially_destructible_v<E>` are all `true`.
template <class E>
constexpr expected<void, E>&
expected<void, E>::operator=(const expected& rhs) noexcept(std::is_nothrow_copy_constructible_v<E> &&
                                                           std::is_nothrow_copy_assignable_v<E>)
    requires((std::is_reference_v<E> || (std::is_copy_constructible_v<E> && std::is_copy_assignable_v<E>)) &&
             !(std::is_trivially_copy_constructible_v<E> && std::is_trivially_copy_assignable_v<E> &&
               std::is_trivially_destructible_v<E>))
{
    if (has_val_ && rhs.has_val_) {
        // both value: no-op
    } else if (!has_val_ && !rhs.has_val_) {
        unex_ = rhs.unex_;
    } else if (has_val_) {
        std::construct_at(std::addressof(unex_), rhs.unex_);
        has_val_ = false;
    } else {
        std::destroy_at(std::addressof(unex_));
        has_val_ = true;
    }
    return *this;
}

//! \effects If `this->has_value() && rhs.has_value()`, no effects.
//! Otherwise, if `this->has_value()`, equivalent to:
//! `construct_at(addressof(unex), std::move(rhs.unex)); has_val = false;`
//! Otherwise, if `rhs.has_value()`, destroys `unex` and sets `has_val` to
//! `true`. Otherwise, equivalent to `unex = std::move(rhs.unex)`.
//! \returns `*this`.
//! \remarks The exception specification is equivalent to
//! `is_nothrow_move_constructible_v<E> &&
//! is_nothrow_move_assignable_v<E>`. This operator is trivial if
//! `is_trivially_move_constructible_v<E>`,
//! `is_trivially_move_assignable_v<E>`, and
//! `is_trivially_destructible_v<E>` are all `true`.
template <class E>
constexpr expected<void, E>&
expected<void, E>::operator=(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                      std::is_nothrow_move_assignable_v<E>)
    requires((std::is_reference_v<E> || (std::is_move_constructible_v<E> && std::is_move_assignable_v<E>)) &&
             !(std::is_trivially_move_constructible_v<E> && std::is_trivially_move_assignable_v<E> &&
               std::is_trivially_destructible_v<E>))
{
    if (has_val_ && rhs.has_val_) {
        // both value: no-op
    } else if (!has_val_ && !rhs.has_val_) {
        unex_ = std::move(rhs.unex_);
    } else if (has_val_) {
        std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
        has_val_ = false;
    } else {
        std::destroy_at(std::addressof(unex_));
        has_val_ = true;
    }
    return *this;
}

//! \group void-cvt-unexpected-assign
//! \constraints `is_constructible_v<E, const G&>` is `true` and
//! `is_assignable_v<E&, const G&>` is `true`.
//! \effects If `has_value()` is `true`, equivalent to:
//! `construct_at(addressof(unex), e.error()); has_val = false;`
//! Otherwise, equivalent to: `unex = unexpected<E>(e.error());`
//! \returns `*this`.
template <class E>
template <class G>
    requires(!std::is_reference_v<E> && std::is_constructible_v<E, const G&> && std::is_assignable_v<E&, const G&>)
constexpr expected<void, E>& expected<void, E>::operator=(const unexpected<G>& e) {
    if (!has_val_) {
        unex_.error() = e.error();
    } else {
        std::construct_at(std::addressof(unex_), e.error());
        has_val_ = false;
    }
    return *this;
}

//! \also void-cvt-unexpected-assign
template <class E>
template <class G>
    requires(!std::is_reference_v<E> && std::is_constructible_v<E, G> && std::is_assignable_v<E&, G>)
constexpr expected<void, E>& expected<void, E>::operator=(unexpected<G>&& e) {
    if (!has_val_) {
        unex_.error() = std::move(e).error();
    } else {
        std::construct_at(std::addressof(unex_), std::move(e).error());
        has_val_ = false;
    }
    return *this;
}

// Rebinding assignment for reference E from reference G. No value member to destroy; repoint unex_
// via construct_at (not `unex_.error() = ...`, which would mutate the old pointee).
//! \group void-cvt-unexpected-assign-ref
//! \constraints `is_reference_v<G>` is `true`; and `is_constructible_v<E,
//! G>` is `true`; and `reference_constructs_from_temporary_v<E, G>` is
//! `false`.
//! \effects Rebinds `unex` to refer to the same object as `e.error()`.
//! \ensures `has_value()` is `false`.
//! \returns `*this`.
//! \remarks This operator never throws: the referent is bound, not
//! copied.
template <class E>
template <class G>
    requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
             !detail::reference_constructs_from_temporary_v<E, G>)
constexpr expected<void, E>& expected<void, E>::operator=(const unexpected<G>& e) {
    std::construct_at(std::addressof(unex_), e.error());
    has_val_ = false;
    return *this;
}

//! \also void-cvt-unexpected-assign-ref
template <class E>
template <class G>
    requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
             !detail::reference_constructs_from_temporary_v<E, G>)
constexpr expected<void, E>& expected<void, E>::operator=(unexpected<G>&& e) {
    std::construct_at(std::addressof(unex_), e.error());
    has_val_ = false;
    return *this;
}

//! \effects If `has_value()` is `false`, destroys `unex` and sets
//! `has_val` to `true`.
template <class E>
constexpr void expected<void, E>::emplace() noexcept {
    if (!has_val_) {
        std::destroy_at(std::addressof(unex_));
        has_val_ = true;
    }
}

// =============================================================================
// \rSec3[expected.void.swap]{Swap}

//! \constraints `is_swappable_v<E>` is `true` and
//! `is_move_constructible_v<E>` is `true`.
//! \effects If `this->has_value()` and `rhs.has_value()`, no effects. If
//! neither `*this` nor `rhs` contains a value, equivalent to `using
//! std::swap; swap(unex, rhs.unex);`. If `rhs.has_value()` is `false`
//! and `this->has_value()` is `true`, initializes `rhs.unex` from
//! `std::move(unex)`, destroys `unex`, and leaves `has_value()` `false`
//! and `rhs.has_value()` `true`. If `rhs.has_value()` is `true` and
//! `this->has_value()` is `false`, equivalent to `rhs.swap(*this)`.
//! \throws Any exception thrown by the expressions in the Effects.
//! \remarks The exception specification is equivalent to
//! `is_nothrow_move_constructible_v<E> && (is_reference_v<E> ||
//! is_nothrow_swappable_v<E>)`.
template <class E>
constexpr void expected<void, E>::swap(expected& rhs) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                               (std::is_reference_v<E> ||
                                                                std::is_nothrow_swappable_v<E>))
    requires((std::is_reference_v<E> || std::is_swappable_v<E>) && std::is_move_constructible_v<E>)
{
    if (has_val_ && rhs.has_val_) {
        // both value: no-op
    } else if (!has_val_ && !rhs.has_val_) {
        using std::swap;
        swap(unex_, rhs.unex_);
    } else if (has_val_) {
        std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
        std::destroy_at(std::addressof(rhs.unex_));
        has_val_     = false;
        rhs.has_val_ = true;
    } else {
        rhs.swap(*this);
    }
}

// =============================================================================
// \rSec3[expected.void.obs]{Observers}

//! \group void-obs-bool
//! \returns `has_val`.
template <class E>
constexpr expected<void, E>::operator bool() const noexcept {
    return has_val_;
}

//! \also void-obs-bool
template <class E>
constexpr bool expected<void, E>::has_value() const noexcept {
    return has_val_;
}

//! \hardexpects `has_value()` is `true`.
template <class E>
constexpr void expected<void, E>::operator*() const noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (!has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
}

//! \mandates `is_copy_constructible_v<E>` is `true`.
//! \throws `bad_expected_access(error())` if `has_value()` is `false`.
template <class E>
constexpr void expected<void, E>::value() const& {
    static_assert(std::is_copy_constructible_v<error_value_type>, "value() requires E to be copy constructible");
    if (!has_val_)
        throw bad_expected_access<error_value_type>(unex_.error());
}

//! \mandates `is_copy_constructible_v<E>` is `true` and
//! `is_move_constructible_v<E>` is `true`.
//! \throws `bad_expected_access(std::move(error()))` if `has_value()` is
//! `false`.
template <class E>
constexpr void expected<void, E>::value() && {
    static_assert(std::is_copy_constructible_v<error_value_type> && std::is_move_constructible_v<error_value_type>,
                  "value() && requires E to be copy and move constructible");
    if (!has_val_)
        throw bad_expected_access<error_value_type>(std::move(unex_).error());
}

//! \group void-obs-error-lval
//! \hardexpects `has_value()` is `false`.
//! \returns `unex.error()`.
template <class E>
constexpr const E& expected<void, E>::error() const& noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return unex_.error();
}

//! \also void-obs-error-lval
template <class E>
constexpr E& expected<void, E>::error() & noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return unex_.error();
}

//! \group void-obs-error-rval
//! \hardexpects `has_value()` is `false`.
//! \returns `std::move(unex).error()`.
template <class E>
constexpr const E&& expected<void, E>::error() const&& noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return std::move(unex_).error();
}

//! \also void-obs-error-rval
template <class E>
constexpr E&& expected<void, E>::error() && noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return std::move(unex_).error();
}

//! \mandates `is_copy_constructible_v<error_value_type>` is `true` and
//! `is_convertible_v<G, error_value_type>` is `true`.
//! \returns `std::forward<G>(def)` if `has_value()` is `true`, `error()`
//! otherwise.
template <class E>
template <class G>
    requires(std::is_copy_constructible_v<std::remove_cv_t<std::remove_reference_t<E>>> &&
             std::is_convertible_v<G, std::remove_cv_t<std::remove_reference_t<E>>>)
constexpr typename expected<void, E>::error_value_type expected<void, E>::error_or(G&& def) const& {
    if (!has_val_)
        return unex_.error();
    return static_cast<error_value_type>(std::forward<G>(def));
}

//! \mandates `is_move_constructible_v<error_value_type>` is `true` and
//! `is_convertible_v<G, error_value_type>` is `true`.
//! \returns `std::forward<G>(def)` if `has_value()` is `true`,
//! `std::move(error())` otherwise.
template <class E>
template <class G>
    requires(std::is_move_constructible_v<std::remove_cv_t<std::remove_reference_t<E>>> &&
             std::is_convertible_v<G, std::remove_cv_t<std::remove_reference_t<E>>>)
constexpr typename expected<void, E>::error_value_type expected<void, E>::error_or(G&& def) && {
    if (!has_val_)
        return std::move(unex_).error();
    return static_cast<error_value_type>(std::forward<G>(def));
}

// \rSec3[expected.void.monadic]{Monadic operations}

//! \group void-monadic-and-then-lval
//! \constraints `is_constructible_v<E, decltype(error())>` is `true`.
//! \mandates `remove_cvref_t<invoke_result_t<F>>` is a specialization of
//! `expected` and its `error_type` is the same type as `E`.
//! \effects Equivalent to: `if (has_value()) return
//! invoke(std::forward<F>(f)); else return U(unexpect, error());` where
//! `U` is `remove_cvref_t<invoke_result_t<F>>`.
template <class E>
template <class F>
    requires std::is_constructible_v<E, E&>
constexpr auto expected<void, E>::and_then(F&& f) & {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f));
    return U(unexpect, unex_.error());
}

//! \group void-monadic-and-then-rval
//! \constraints `is_constructible_v<E, decltype(std::move(error()))>` is
//! `true`.
//! \mandates `remove_cvref_t<invoke_result_t<F>>` is a specialization of
//! `expected` and its `error_type` is the same type as `E`.
//! \effects Equivalent to: `if (has_value()) return
//! invoke(std::forward<F>(f)); else return U(unexpect,
//! std::move(error()));` where `U` is `remove_cvref_t<invoke_result_t<F>>`.
template <class E>
template <class F>
    requires std::is_constructible_v<E, E&&>
constexpr auto expected<void, E>::and_then(F&& f) && {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f));
    return U(unexpect, std::move(unex_).error());
}

//! \also void-monadic-and-then-lval
template <class E>
template <class F>
    requires std::is_constructible_v<E, const E&>
constexpr auto expected<void, E>::and_then(F&& f) const& {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f));
    return U(unexpect, unex_.error());
}

//! \also void-monadic-and-then-rval
template <class E>
template <class F>
    requires std::is_constructible_v<E, const E&&>
constexpr auto expected<void, E>::and_then(F&& f) const&& {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f));
    return U(unexpect, std::move(unex_).error());
}

//! \group void-monadic-or-else-lval
//! \mandates `remove_cvref_t<invoke_result_t<F, decltype(error())>>` is a
//! specialization of `expected` and its `value_type` is the same type as
//! `T`.
//! \effects Equivalent to: `if (has_value()) return G(); else return
//! invoke(std::forward<F>(f), error());` where `G` is
//! `remove_cvref_t<invoke_result_t<F, decltype(error())>>`.
template <class E>
template <class F>
constexpr auto expected<void, E>::or_else(F&& f) & {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, void>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G();
    return std::invoke(std::forward<F>(f), unex_.error());
}

//! \group void-monadic-or-else-rval
//! \mandates `remove_cvref_t<invoke_result_t<F,
//! decltype(std::move(error()))>>` is a specialization of `expected` and
//! its `value_type` is the same type as `T`.
//! \effects Equivalent to: `if (has_value()) return G(); else return
//! invoke(std::forward<F>(f), std::move(error()));` where `G` is
//! `remove_cvref_t<invoke_result_t<F, decltype(std::move(error()))>>`.
template <class E>
template <class F>
constexpr auto expected<void, E>::or_else(F&& f) && {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, void>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G();
    return std::invoke(std::forward<F>(f), std::move(unex_).error());
}

//! \also void-monadic-or-else-lval
template <class E>
template <class F>
constexpr auto expected<void, E>::or_else(F&& f) const& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, void>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G();
    return std::invoke(std::forward<F>(f), unex_.error());
}

//! \also void-monadic-or-else-rval
template <class E>
template <class F>
constexpr auto expected<void, E>::or_else(F&& f) const&& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, void>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G();
    return std::invoke(std::forward<F>(f), std::move(unex_).error());
}

//! \group void-monadic-transform-lval
//! \constraints `is_constructible_v<E, decltype(error())>` is `true`.
//! \mandates `U` is a valid value type for `expected`, where `U` is
//! `remove_cv_t<invoke_result_t<F>>`.
//! \effects If `has_value()` is `false`, returns `expected<U, E>(unexpect,
//! error())`. Otherwise, if `is_void_v<U>` is `false`, returns an
//! `expected<U, E>` object whose `has_val` member is `true` and `val`
//! member is direct-non-list-initialized with `invoke(std::forward<F>(f))`.
//! Otherwise, evaluates `invoke(std::forward<F>(f))` and then returns
//! `expected<U, E>()`.
template <class E>
template <class F>
    requires std::is_constructible_v<E, E&>
constexpr auto expected<void, E>::transform(F&& f) & {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f));
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, unex_.error());
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f)));
        return expected<U, E>(unexpect, unex_.error());
    }
}

//! \group void-monadic-transform-rval
//! \constraints `is_constructible_v<E, decltype(std::move(error()))>` is
//! `true`.
//! \mandates `U` is a valid value type for `expected`, where `U` is
//! `remove_cv_t<invoke_result_t<F>>`.
//! \effects If `has_value()` is `false`, returns `expected<U, E>(unexpect,
//! std::move(error()))`. Otherwise, if `is_void_v<U>` is `false`, returns
//! an `expected<U, E>` object whose `has_val` member is `true` and `val`
//! member is direct-non-list-initialized with `invoke(std::forward<F>(f))`.
//! Otherwise, evaluates `invoke(std::forward<F>(f))` and then returns
//! `expected<U, E>()`.
template <class E>
template <class F>
    requires std::is_constructible_v<E, E&&>
constexpr auto expected<void, E>::transform(F&& f) && {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f));
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, std::move(unex_).error());
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f)));
        return expected<U, E>(unexpect, std::move(unex_).error());
    }
}

//! \also void-monadic-transform-lval
template <class E>
template <class F>
    requires std::is_constructible_v<E, const E&>
constexpr auto expected<void, E>::transform(F&& f) const& {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f));
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, unex_.error());
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f)));
        return expected<U, E>(unexpect, unex_.error());
    }
}

//! \also void-monadic-transform-rval
template <class E>
template <class F>
    requires std::is_constructible_v<E, const E&&>
constexpr auto expected<void, E>::transform(F&& f) const&& {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f));
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, std::move(unex_).error());
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f)));
        return expected<U, E>(unexpect, std::move(unex_).error());
    }
}

//! \group void-monadic-transform-error-lval
//! \mandates `G` is a valid template argument for `unexpected` and the
//! declaration `G g(invoke(std::forward<F>(f), error()));` is
//! well-formed, where `G` is `remove_cv_t<invoke_result_t<F,
//! decltype(error())>>`.
//! \returns If `has_value()` is `true`, `expected<T, G>()`; otherwise, an
//! `expected<T, G>` object whose `has_val` member is `false` and `unex`
//! member is direct-non-list-initialized with `invoke(std::forward<F>(f),
//! error())`.
template <class E>
template <class F>
constexpr auto expected<void, E>::transform_error(F&& f) & {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<void, G>();
    return expected<void, G>(unexpect, std::invoke(std::forward<F>(f), unex_.error()));
}

//! \group void-monadic-transform-error-rval
//! \mandates `G` is a valid template argument for `unexpected` and the
//! declaration `G g(invoke(std::forward<F>(f), std::move(error())));` is
//! well-formed, where `G` is `remove_cv_t<invoke_result_t<F,
//! decltype(std::move(error()))>>`.
//! \returns If `has_value()` is `true`, `expected<T, G>()`; otherwise, an
//! `expected<T, G>` object whose `has_val` member is `false` and `unex`
//! member is direct-non-list-initialized with `invoke(std::forward<F>(f),
//! std::move(error()))`.
template <class E>
template <class F>
constexpr auto expected<void, E>::transform_error(F&& f) && {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<void, G>();
    return expected<void, G>(unexpect, std::invoke(std::forward<F>(f), std::move(unex_).error()));
}

//! \also void-monadic-transform-error-lval
template <class E>
template <class F>
constexpr auto expected<void, E>::transform_error(F&& f) const& {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<void, G>();
    return expected<void, G>(unexpect, std::invoke(std::forward<F>(f), unex_.error()));
}

//! \also void-monadic-transform-error-rval
template <class E>
template <class F>
constexpr auto expected<void, E>::transform_error(F&& f) const&& {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<void, G>();
    return expected<void, G>(unexpect, std::invoke(std::forward<F>(f), std::move(unex_).error()));
}

// \rSec3[expected.void.eq]{Equality operators}

// =============================================================================
// Partial specialization: expected<T&, E> — reference value type
// (E may be an object type or an lvalue reference to one)
// =============================================================================

// \rSec2[expected.ref]{Partial specialization of expected for reference types}
// \rSec3[expected.ref.general]{General}
//! \at expected.ref.general
//! \mandates A program that instantiates the definition of `expected<T&,
//! E>` with an `E` that is not a valid template argument for `unexpected`
//! is ill-formed. `T` shall be an object type that is not an array type.
//! \remarks An object of type `expected<T&, E>` either represents a
//! reference to an object of type `T`, or holds an error. Member `has_val`
//! indicates whether the object represents a reference. When it represents
//! a reference, member `val` points to the referenced object, which is not
//! owned by the `expected` object. Otherwise, the error is `unex.error()`.
template <class T, class E>
class expected<T&, E> {
    static_assert(!std::is_array_v<T>, "T must not be an array type");
    static_assert(std::is_object_v<T>, "T must be an object type");
    static_assert(!std::is_same_v<std::remove_cv_t<T>, std::in_place_t>, "T must not be in_place_t");
    static_assert(!std::is_same_v<std::remove_cv_t<T>, unexpect_t>, "T must not be unexpect_t");
    static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<T>>::value,
                  "T must not be a specialization of unexpected");
    static_assert(!std::is_rvalue_reference_v<E>, "E must not be an rvalue reference");
    static_assert(!std::is_void_v<std::remove_reference_t<E>>, "E must not be void");
    static_assert(!std::is_array_v<std::remove_reference_t<E>>, "E must not be an array type");
    static_assert(std::is_object_v<std::remove_reference_t<E>>, "E must be an object type");
    static_assert(std::is_reference_v<E> || std::is_same_v<std::remove_cv_t<E>, E>, "E must not be cv-qualified");

  private:
    //! \expos
    using error_value_type = std::remove_cv_t<std::remove_reference_t<E>>;

  public:
    using value_type      = T&;
    using error_type      = E;
    using unexpected_type = unexpected<E>;

    template <class U>
    using rebind = expected<U, error_type>;

    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------

    //! \at expected.ref.cons
    //! \remarks `expected<T&, E>` has no default constructor: a reference
    //! cannot be null, so there is no empty state to default-construct
    //! into.
    expected() = BEMAN_EXPECTED_DELETE_MSG("expected<T&,E>: no default constructor; T& cannot be null");

    // Copy constructor (trivial path). Unconstrained; see the primary
    // template's copy constructor for why.
    //! \at expected.ref.cons
    //! \effects If `rhs.has_value()` is `true`, initializes `val` with
    //! `rhs.val`, so that `*this` and `rhs` refer to the same object;
    //! otherwise, initializes `unex` with `rhs.unex`.
    //! \ensures `rhs.has_value() == this->has_value()`.
    //! \remarks This constructor is trivial.
    constexpr expected(const expected&) = default;

    // Copy constructor (non-trivial path)
    //! \at expected.ref.cons
    //! \effects If `rhs.has_value()` is `true`, initializes `val` with
    //! `rhs.val`, so that `*this` and `rhs` refer to the same object;
    //! otherwise, initializes `unex` with `rhs.unex`.
    //! \ensures `rhs.has_value() == this->has_value()`.
    //! \remarks This constructor is defined as deleted unless
    //! `is_copy_constructible_v<E>` is `true`.
    constexpr expected(const expected& rhs) noexcept(std::is_nothrow_copy_constructible_v<E>)
        requires(std::is_copy_constructible_v<E> && !std::is_trivially_copy_constructible_v<E>);

    // Move constructor (trivial path). Unconstrained; no explicit noexcept.
    //! \at expected.ref.cons
    //! \effects If `rhs.has_value()` is `true`, initializes `val` with
    //! `rhs.val`, so that `*this` and `rhs` refer to the same object;
    //! otherwise, initializes `unex` with `std::move(rhs.unex)`.
    //! \ensures `rhs.has_value() == this->has_value()`.
    //! \remarks This constructor is trivial.
    constexpr expected(expected&&) = default;

    // Move constructor (non-trivial path)
    //! \at expected.ref.cons
    //! \effects If `rhs.has_value()` is `true`, initializes `val` with
    //! `rhs.val`, so that `*this` and `rhs` refer to the same object;
    //! otherwise, initializes `unex` with `std::move(rhs.unex)`.
    //! \ensures `rhs.has_value() == this->has_value()`.
    //! \remarks The exception specification is equivalent to
    //! `is_nothrow_move_constructible_v<E>`. This constructor is defined as
    //! deleted unless `is_move_constructible_v<E>` is `true`.
    constexpr expected(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E>)
        requires(std::is_move_constructible_v<E> && !std::is_trivially_move_constructible_v<E>);

    // Deleted: no in-place value constructor — T& cannot be constructed in-place
    //! \at expected.ref.cons
    //! \remarks `expected<T&, E>` has no in-place value constructor: `T&`
    //! cannot be constructed in-place. Pass a `U` convertible to `T&`
    //! instead.
    template <class... Args>
    constexpr expected(std::in_place_t, Args&&...) =
        BEMAN_EXPECTED_DELETE_MSG("expected<T&,E>: no in-place value constructor; T& cannot be constructed "
                                  "in-place — pass a U convertible to T&");

    // Value constructor — takes U that can bind to T&
    //! \at expected.ref.cons
    //! \constraints `remove_cvref_t<U>` is not `in_place_t`, `expected`, or
    //! a specialization of `unexpected`; `is_constructible_v<T&, U>` is
    //! `true`; and `reference_constructs_from_temporary_v<T&, U>` is
    //! `false`.
    //! \effects Let `r` be the lvalue result of `T& r =
    //! std::forward<U>(u);`. Initializes `val` with `addressof(r)`.
    //! \ensures `has_value()` is `true`.
    template <class U = T>
        requires(!std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
                 !std::is_same_v<std::remove_cvref_t<U>, expected<T&, E>> &&
                 !detail::is_unexpected_specialization<std::remove_cvref_t<U>>::value &&
                 std::is_constructible_v<T&, U> && !detail::reference_constructs_from_temporary_v<T&, U>)
    constexpr explicit(!std::is_convertible_v<U, T&>) expected(U&& u) noexcept(std::is_nothrow_constructible_v<T&, U>)
        : has_val_(true) {
        T& r = std::forward<U>(u);
        val_ = std::addressof(r);
    }

    // Deleted: binding a temporary to T& creates a dangling reference
    //! \at expected.ref.cons
    //! \remarks A constructor for which
    //! `reference_constructs_from_temporary_v<T&, U>` is `true` — one that
    //! would bind `T&` to a temporary — is defined as deleted.
    template <class U>
        requires(detail::reference_constructs_from_temporary_v<T&, U>)
    constexpr expected(U&&) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<T&,E>: argument would bind a temporary that dangles; pass an lvalue reference");

    // Converting constructor from expected<U&, G> (copy) — value-E path
    template <class U, class G>
        requires(!std::is_reference_v<E> && std::is_constructible_v<T&, U&> && std::is_constructible_v<E, const G&> &&
                 !detail::reference_constructs_from_temporary_v<T&, U&>)
    constexpr explicit(!std::is_convertible_v<U&, T&> || !std::is_convertible_v<const G&, E>)
        expected(const expected<U&, G>& rhs);

    // Converting constructor from expected<U&, G> (move) — value-E path
    template <class U, class G>
        requires(!std::is_reference_v<E> && std::is_constructible_v<T&, U&> && std::is_constructible_v<E, G> &&
                 !detail::reference_constructs_from_temporary_v<T&, U&>)
    constexpr explicit(!std::is_convertible_v<U&, T&> || !std::is_convertible_v<G, E>) expected(expected<U&, G>&& rhs);

    // Converting constructor from expected<U&, G&> (copy/move) — reference-E path: only accepts
    // sources whose error type G is itself a reference convertible to E.
    template <class U, class G>
        requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<T&, U&> &&
                 std::is_convertible_v<G, E> && !detail::reference_constructs_from_temporary_v<T&, U&>)
    constexpr explicit(!std::is_convertible_v<U&, T&> || !std::is_convertible_v<G, E>)
        expected(const expected<U&, G>& rhs);

    template <class U, class G>
        requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<T&, U&> &&
                 std::is_convertible_v<G, E> && !detail::reference_constructs_from_temporary_v<T&, U&>)
    constexpr explicit(!std::is_convertible_v<U&, T&> || !std::is_convertible_v<G, E>) expected(expected<U&, G>&& rhs);

    // Constructor from unexpected<G> const& / && — value-E path
    template <class G>
        requires(!std::is_reference_v<E> && std::is_constructible_v<E, const G&>)
    constexpr explicit(!std::is_convertible_v<const G&, E>) expected(const unexpected<G>& e);

    template <class G>
        requires(!std::is_reference_v<E> && std::is_constructible_v<E, G>)
    constexpr explicit(!std::is_convertible_v<G, E>) expected(unexpected<G>&& e);

    // Constructor from unexpected<G> — reference-E path. Allowed only when G is itself a reference,
    // i.e. the source unexpected holds a reference to an external object, so binding E& to e.error()
    // cannot dangle regardless of the source's value category. No const_cast is needed.
    template <class G>
        requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
                 !detail::reference_constructs_from_temporary_v<E, G>)
    constexpr explicit(!std::is_convertible_v<G, E>) expected(const unexpected<G>& e) noexcept;

    template <class G>
        requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
                 !detail::reference_constructs_from_temporary_v<E, G>)
    constexpr explicit(!std::is_convertible_v<G, E>) expected(unexpected<G>&& e) noexcept;

    // Deleted for reference E with value G: the referent lives inside the unexpected<G> object, so
    // binding E& to it would dangle once a temporary source is destroyed. Use (unexpect, lvalue), or
    // an unexpected<E&> holding an external object, instead.
    //! \at expected.ref.cons
    //! \group ref-cvt-unexpected-ctor-deleted
    //! \remarks When `E` is a reference type, an overload taking
    //! `unexpected<G>` for a non-reference `G` is defined as deleted: the
    //! referent would live inside the (possibly temporary) source
    //! `unexpected<G>` object, and binding `E&` to it would dangle. Use an
    //! `unexpected<E&>` holding an external object instead.
    template <class G>
        requires(std::is_reference_v<E> && !std::is_reference_v<G>)
    constexpr expected(const unexpected<G>&) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<T&,E&>: cannot construct from unexpected<value>; the value would dangle — use unexpected<E&>");

    //! \at expected.ref.cons
    //! \also ref-cvt-unexpected-ctor-deleted
    template <class G>
        requires(std::is_reference_v<E> && !std::is_reference_v<G>)
    constexpr expected(unexpected<G>&&) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<T&,E&>: cannot construct from unexpected<value>; the value would dangle — use unexpected<E&>");

    // In-place constructor for error
    template <class... Args>
        requires(std::is_constructible_v<E, Args...> && !detail::unexpect_dangles_v<E, Args...>)
    constexpr explicit expected(unexpect_t, Args&&... args);

    // Deleted: single argument would bind E& to a temporary — dangling prevention
    //! \at expected.ref.cons
    //! \group ref-unexpect-ctor-deleted
    //! \remarks When `E` is a reference type, an overload with the same
    //! parameter types is defined as deleted if the single argument would
    //! bind `E&` to a temporary, or if it is otherwise not usable to
    //! construct `E`.
    template <class... Args>
        requires(detail::unexpect_dangles_v<E, Args...>)
    constexpr expected(unexpect_t, Args&&...) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<T&,E&>: unexpect argument would bind a temporary that dangles; pass an lvalue reference");

    // Deleted catch-all: reference E, argument neither constructible nor a dangling case
    //! \at expected.ref.cons
    //! \also ref-unexpect-ctor-deleted
    template <class... Args>
        requires(std::is_reference_v<E> && !std::is_constructible_v<E, Args...> &&
                 !detail::unexpect_dangles_v<E, Args...>)
    constexpr expected(unexpect_t, Args&&...) =
        BEMAN_EXPECTED_DELETE_MSG("expected<T&,E&>: no viable conversion from the given argument(s) to E&");

    // In-place constructor for error with initializer_list
    template <class U, class... Args>
        requires(!std::is_reference_v<E> && std::is_constructible_v<E, std::initializer_list<U>&, Args...>)
    constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&... args);

    //! \at expected.ref.cons
    //! \remarks An overload with the same parameter types is defined as
    //! deleted when `E` is an lvalue reference type. An initializer list
    //! cannot provide the required long-lived error referent.
    template <class U, class... Args>
        requires std::is_reference_v<E>
    constexpr expected(unexpect_t, std::initializer_list<U>, Args&&...) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<T&,E&>: initializer-list error construction cannot bind a reference; pass an lvalue reference");

    // -------------------------------------------------------------------------
    // Destructor
    // -------------------------------------------------------------------------

    //! \at expected.ref.dtor
    //! \effects None: `*this` never owns the object it refers to; `T` is
    //! never destroyed.
    //! \remarks This destructor is trivial.
    constexpr ~expected()
        requires std::is_trivially_destructible_v<E>
    = default;

    //! \at expected.ref.dtor
    //! \effects If `has_value()` is `false`, destroys `unex`. `T` is not
    //! destroyed; `*this` never owns the object it refers to.
    //! \remarks This destructor is trivial if `E` is trivially destructible.
    constexpr ~expected()
        requires(!std::is_trivially_destructible_v<E>);

    // -------------------------------------------------------------------------
    // Assignment (rebind semantics)
    // -------------------------------------------------------------------------

    //! \at expected.ref.assign
    //! \remarks Assignment rebinds: assigning to an `expected<T&, E>` that
    //! holds a value changes which object it refers to. It never assigns
    //! through to the referent.
    //! \effects If `rhs.has_value()` is `true`: if `has_value()` is `true`,
    //! assigns `rhs.val` to `val`; otherwise destroys `unex` and
    //! initializes `val` with `rhs.val`. If `rhs.has_value()` is `false`,
    //! the error of `rhs` is assigned to or used to initialize `unex`, as
    //! for the primary template. In every case `*this` comes to refer to
    //! the object `rhs` refers to, or to hold the error of `rhs`.
    //! \returns `*this`.
    //! \remarks This operator is trivial.
    // Copy assignment (trivial path)
    constexpr expected& operator=(const expected&)
        requires(std::is_trivially_copy_constructible_v<E> && std::is_trivially_copy_assignable_v<E> &&
                 std::is_trivially_destructible_v<E>)
    = default;

    // Copy assignment (non-trivial path)
    //! \at expected.ref.assign
    //! \effects If `rhs.has_value()` is `true`: if `has_value()` is `true`,
    //! assigns `rhs.val` to `val`; otherwise destroys `unex` and
    //! initializes `val` with `rhs.val`. If `rhs.has_value()` is `false`,
    //! the error of `rhs` is assigned to or used to initialize `unex`, as
    //! for the primary template. In every case `*this` comes to refer to
    //! the object `rhs` refers to, or to hold the error of `rhs`.
    //! \returns `*this`.
    //! \remarks This operator is defined as deleted unless
    //! `is_copy_assignable_v<E>` is `true` and `is_copy_constructible_v<E>`
    //! is `true`.
    constexpr expected& operator=(const expected& rhs) noexcept(std::is_nothrow_copy_constructible_v<E> &&
                                                                std::is_nothrow_copy_assignable_v<E>)
        requires((std::is_reference_v<E> || (std::is_copy_constructible_v<E> && std::is_copy_assignable_v<E>)) &&
                 !(std::is_trivially_copy_constructible_v<E> && std::is_trivially_copy_assignable_v<E> &&
                   std::is_trivially_destructible_v<E>));

    // Move assignment (trivial path)
    //! \at expected.ref.assign
    //! \effects If `rhs.has_value()` is `true`: if `has_value()` is `true`,
    //! assigns `rhs.val` to `val`; otherwise destroys `unex` and
    //! initializes `val` with `rhs.val`. If `rhs.has_value()` is `false`,
    //! the error of `rhs` is assigned to or used to initialize `unex`, as
    //! for the primary template.
    //! \returns `*this`.
    //! \remarks This operator is trivial.
    constexpr expected& operator=(expected&&) noexcept
        requires(std::is_trivially_move_constructible_v<E> && std::is_trivially_move_assignable_v<E> &&
                 std::is_trivially_destructible_v<E>)
    = default;

    // Move assignment (non-trivial path)
    //! \at expected.ref.assign
    //! \effects If `rhs.has_value()` is `true`: if `has_value()` is `true`,
    //! assigns `rhs.val` to `val`; otherwise destroys `unex` and
    //! initializes `val` with `rhs.val`. If `rhs.has_value()` is `false`,
    //! the error of `rhs` is assigned to or used to initialize `unex`, as
    //! for the primary template.
    //! \returns `*this`.
    //! \remarks The exception specification is equivalent to
    //! `is_nothrow_move_constructible_v<E> &&
    //! is_nothrow_move_assignable_v<E>`. This operator is defined as
    //! deleted unless `is_move_assignable_v<E>` is `true` and
    //! `is_move_constructible_v<E>` is `true`.
    constexpr expected& operator=(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                           std::is_nothrow_move_assignable_v<E>)
        requires((std::is_reference_v<E> || (std::is_move_constructible_v<E> && std::is_move_assignable_v<E>)) &&
                 !(std::is_trivially_move_constructible_v<E> && std::is_trivially_move_assignable_v<E> &&
                   std::is_trivially_destructible_v<E>));

    // Rebind reference from lvalue
    //! \at expected.ref.assign
    //! \constraints `remove_cvref_t<U>` is neither `expected` nor a
    //! specialization of `unexpected`, `is_constructible_v<T&, U>` is
    //! `true`, and `reference_constructs_from_temporary_v<T&, U>` is
    //! `false`.
    //! \effects Let `r` be the lvalue result of `T& r =
    //! std::forward<U>(u);`. If `has_value()` is `true`, assigns
    //! `addressof(r)` to `val`. Otherwise, destroys `unex`, initializes
    //! `val` with `addressof(r)`, and sets `has_val` to `true`; if binding
    //! `r` throws, `*this` is left unchanged.
    //! \returns `*this`.
    template <class U = T>
        requires(!std::is_same_v<std::remove_cvref_t<U>, expected<T&, E>> &&
                 !detail::is_unexpected_specialization<std::remove_cvref_t<U>>::value &&
                 std::is_constructible_v<T&, U> && !detail::reference_constructs_from_temporary_v<T&, U>)
    constexpr expected& operator=(U&& u) {
        if (has_val_) {
            T& r = std::forward<U>(u);
            val_ = std::addressof(r);
        } else {
            T& r = std::forward<U>(u); // bind first: if it throws, the error state is left intact
            std::destroy_at(std::addressof(unex_));
            val_     = std::addressof(r);
            has_val_ = true;
        }
        return *this;
    }

    // Assignment from unexpected<G> — value-E path
    template <class G>
        requires(!std::is_reference_v<E> && std::is_constructible_v<E, const G&> && std::is_assignable_v<E&, const G&>)
    constexpr expected& operator=(const unexpected<G>& e);

    template <class G>
        requires(!std::is_reference_v<E> && std::is_constructible_v<E, G> && std::is_assignable_v<E&, G>)
    constexpr expected& operator=(unexpected<G>&& e);

    // Rebinding assignment for reference E from reference G — binds unex_ to the external referent.
    template <class G>
        requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
                 !detail::reference_constructs_from_temporary_v<E, G>)
    constexpr expected& operator=(const unexpected<G>& e);

    template <class G>
        requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
                 !detail::reference_constructs_from_temporary_v<E, G>)
    constexpr expected& operator=(unexpected<G>&& e);

    // Deleted for reference E with value G: would rebind E& to unexpected<G>'s temporary storage.
    //! \at expected.ref.assign
    //! \group ref-cvt-unexpected-assign-deleted
    //! \remarks When `E` is a reference type, an overload taking
    //! `unexpected<G>` for a non-reference `G` is defined as deleted: it
    //! would rebind `unex` to `unexpected<G>`'s temporary storage.
    template <class G>
        requires(std::is_reference_v<E> && !std::is_reference_v<G>)
    constexpr expected& operator=(const unexpected<G>&) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<T&,E&>: cannot assign from unexpected<value>; the value would dangle — use unexpected<E&>");

    //! \at expected.ref.assign
    //! \also ref-cvt-unexpected-assign-deleted
    template <class G>
        requires(std::is_reference_v<E> && !std::is_reference_v<G>)
    constexpr expected& operator=(unexpected<G>&&) = BEMAN_EXPECTED_DELETE_MSG(
        "expected<T&,E&>: cannot assign from unexpected<value>; the value would dangle — use unexpected<E&>");

    // emplace — rebind the reference
    //! \at expected.ref.assign
    //! \constraints `is_constructible_v<T&, U>` is `true` and
    //! `reference_constructs_from_temporary_v<T&, U>` is `false`.
    //! \effects Rebinds `*this` to refer to the object bound by `T& r =
    //! std::forward<U>(u);`: if `has_value()` is `false`, destroys `unex`
    //! first. Sets `val` to `addressof(r)` and `has_val` to `true`.
    //! \returns `*val`.
    template <class U = T>
        requires(std::is_constructible_v<T&, U> && !detail::reference_constructs_from_temporary_v<T&, U>)
    constexpr T& emplace(U&& u) noexcept(std::is_nothrow_constructible_v<T&, U>);

    // -------------------------------------------------------------------------
    // Swap
    // -------------------------------------------------------------------------

    //! \at expected.ref.swap
    //! \effects Exchanges the states of `*this` and `rhs`. When both hold
    //! values, exchanges `val` and `rhs.val` — the referenced objects are
    //! not swapped. Otherwise behaves as the primary template's `swap`
    //! does for the error.
    //! \remarks The exception specification is equivalent to
    //! `is_nothrow_move_constructible_v<E> && (is_reference_v<E> ||
    //! is_nothrow_swappable_v<E>)`.
    constexpr void swap(expected& rhs) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                (std::is_reference_v<E> || std::is_nothrow_swappable_v<E>))
        requires((std::is_reference_v<E> || std::is_swappable_v<E>) && std::is_move_constructible_v<E>);

    //! \at expected.ref.swap
    //! \effects Equivalent to `x.swap(y)`.
    friend constexpr void swap(expected& x, expected& y) noexcept(noexcept(x.swap(y)))
        requires((std::is_reference_v<E> || std::is_swappable_v<E>) && std::is_move_constructible_v<E>)
    {
        x.swap(y);
    }

    // -------------------------------------------------------------------------
    // Observers
    // -------------------------------------------------------------------------

    constexpr T* operator->() const noexcept;
    constexpr T& operator*() const noexcept;

    constexpr explicit operator bool() const noexcept;
    constexpr bool     has_value() const noexcept;

    constexpr T& value() const&;
    constexpr T& value() &&;

    // error() — shallow const: always returns E& regardless of const on expected
    constexpr const E&  error() const& noexcept;
    constexpr E&        error() & noexcept;
    constexpr const E&& error() const&& noexcept;
    constexpr E&&       error() && noexcept;

    template <class U = std::remove_cv_t<T>>
        requires(std::is_object_v<T> && !std::is_array_v<T>)
    constexpr std::remove_cv_t<T> value_or(U&& def) const;

    // Constraints spell error_value_type as its underlying trait expression rather than the
    // member typedef: clang (through 22) fails to match an out-of-line constrained member of a
    // partial specialization when the requires-clause names a member typedef of the class.
    template <class G = error_value_type>
        requires(std::is_copy_constructible_v<std::remove_cv_t<std::remove_reference_t<E>>> &&
                 std::is_convertible_v<G, std::remove_cv_t<std::remove_reference_t<E>>>)
    constexpr error_value_type error_or(G&& def) const&;

    template <class G = error_value_type>
        requires(std::is_move_constructible_v<std::remove_cv_t<std::remove_reference_t<E>>> &&
                 std::is_convertible_v<G, std::remove_cv_t<std::remove_reference_t<E>>>)
    constexpr error_value_type error_or(G&& def) &&;

    // -------------------------------------------------------------------------
    // Monadic operations
    // -------------------------------------------------------------------------

    template <class F>
        requires std::is_constructible_v<E, E&>
    constexpr auto and_then(F&& f) &;
    template <class F>
        requires std::is_constructible_v<E, E&&>
    constexpr auto and_then(F&& f) &&;
    template <class F>
        requires std::is_constructible_v<E, const E&>
    constexpr auto and_then(F&& f) const&;
    template <class F>
        requires std::is_constructible_v<E, const E&&>
    constexpr auto and_then(F&& f) const&&;

    template <class F>
    constexpr auto or_else(F&& f) &;
    template <class F>
    constexpr auto or_else(F&& f) &&;
    template <class F>
    constexpr auto or_else(F&& f) const&;
    template <class F>
    constexpr auto or_else(F&& f) const&&;

    // transform: f receives T& (value); error propagates as E; result is expected<U, E>
    template <class F>
        requires std::is_constructible_v<E, E&>
    constexpr auto transform(F&& f) &;
    template <class F>
        requires std::is_constructible_v<E, E&&>
    constexpr auto transform(F&& f) &&;
    template <class F>
        requires std::is_constructible_v<E, const E&>
    constexpr auto transform(F&& f) const&;
    template <class F>
        requires std::is_constructible_v<E, const E&&>
    constexpr auto transform(F&& f) const&&;

    // transform_error: f receives E; value propagates as T&; result is expected<T&, G>
    template <class F>
    constexpr auto transform_error(F&& f) &;
    template <class F>
    constexpr auto transform_error(F&& f) &&;
    template <class F>
    constexpr auto transform_error(F&& f) const&;
    template <class F>
    constexpr auto transform_error(F&& f) const&&;

    // -------------------------------------------------------------------------
    // Equality operators (hidden friends)
    // -------------------------------------------------------------------------

    //! \at expected.ref.eq
    //! \remarks The equality operators behave as specified for the primary
    //! template, comparing referents through `operator*` and errors
    //! through `error()`.
    //! \mandates `!is_void_v<T2>` is `true`. The expression `*x == *y` is
    //! well-formed and its result is convertible to `bool`. The expression
    //! `x.error() == y.error()` is well-formed and its result is
    //! convertible to `bool`.
    //! \returns If `x.has_value() != y.has_value()`, `false`; otherwise, if
    //! `x.has_value()` is `true`, `*x == *y`; otherwise `x.error() ==
    //! y.error()`.
    template <class T2, class E2>
        requires(!std::is_void_v<T2>)
    friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y) {
        if (x.has_value() != y.has_value())
            return false;
        if (x.has_value())
            return *x == *y;
        return x.error() == y.error();
    }

    //! \at expected.ref.eq
    //! \mandates `T2` is not a specialization of `expected`. The expression
    //! `*x == val` is well-formed and its result is convertible to `bool`.
    //! \returns `x.has_value() && static_cast<bool>(*x == val)`.
    template <class T2>
        requires(!detail::is_expected_specialization<T2>::value)
    friend constexpr bool operator==(const expected& x, const T2& val) {
        return x.has_value() && static_cast<bool>(*x == val);
    }

    //! \at expected.ref.eq
    //! \mandates The expression `x.error() == e.error()` is well-formed and
    //! its result is convertible to `bool`.
    //! \returns `!x.has_value() && static_cast<bool>(x.error() ==
    //! e.error())`.
    template <class E2>
    friend constexpr bool operator==(const expected& x, const unexpected<E2>& e) {
        return !x.has_value() && static_cast<bool>(x.error() == e.error());
    }

  private:
    //! \expos
    bool has_val_;
    union {
        //! \expos
        T* val_;
        //! \expos
        unexpected<E> unex_;
    };
};

// \rSec3[expected.ref.cons]{Constructors}

//! \group ref-copy-move-ctor
//! \effects If `rhs.has_value()` is `true`, initializes `val` with
//! `rhs.val`, so that `*this` and `rhs` refer to the same object;
//! otherwise, initializes `unex` with `rhs.unex`.
//! \ensures `rhs.has_value() == this->has_value()`.
//! \remarks This constructor is trivial if the corresponding constructor
//! of `E` is trivial, and is defined as deleted unless
//! `is_copy_constructible_v<E>` is `true`.
template <class T, class E>
constexpr expected<T&, E>::expected(const expected& rhs) noexcept(std::is_nothrow_copy_constructible_v<E>)
    requires(std::is_copy_constructible_v<E> && !std::is_trivially_copy_constructible_v<E>)
    : has_val_(rhs.has_val_) {
    if (has_val_)
        val_ = rhs.val_;
    else
        std::construct_at(std::addressof(unex_), rhs.unex_);
}

//! \also ref-copy-move-ctor
template <class T, class E>
constexpr expected<T&, E>::expected(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E>)
    requires(std::is_move_constructible_v<E> && !std::is_trivially_move_constructible_v<E>)
    : has_val_(rhs.has_val_) {
    if (has_val_)
        val_ = rhs.val_;
    else
        std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
}

//! \group ref-cvt-copy-ctor
//! \constraints `is_constructible_v<T&, U&>` is `true`;
//! `reference_constructs_from_temporary_v<T&, U&>` is `false`; and
//! `is_constructible_v<E, const G&>` is `true`.
//! \effects If `rhs.has_value()` is `true`, initializes `val` with
//! `addressof(*rhs)`, so that `*this` refers to the object referred to by
//! `rhs`; otherwise, initializes `unex` with the error of `rhs`. No object
//! referred to by `rhs` is moved from.
template <class T, class E>
template <class U, class G>
    requires(!std::is_reference_v<E> && std::is_constructible_v<T&, U&> && std::is_constructible_v<E, const G&> &&
             !detail::reference_constructs_from_temporary_v<T&, U&>)
constexpr expected<T&, E>::expected(const expected<U&, G>& rhs) : has_val_(rhs.has_value()) {
    if (has_val_) {
        T& r = *rhs;
        val_ = std::addressof(r);
    } else {
        std::construct_at(std::addressof(unex_), rhs.error());
    }
}

//! \also ref-cvt-copy-ctor
template <class T, class E>
template <class U, class G>
    requires(!std::is_reference_v<E> && std::is_constructible_v<T&, U&> && std::is_constructible_v<E, G> &&
             !detail::reference_constructs_from_temporary_v<T&, U&>)
constexpr expected<T&, E>::expected(expected<U&, G>&& rhs) : has_val_(rhs.has_value()) {
    if (has_val_) {
        T& r = *rhs;
        val_ = std::addressof(r);
    } else {
        std::construct_at(std::addressof(unex_), std::move(rhs).error());
    }
}

//! \group ref-cvt-copy-ctor-ref
//! \constraints `is_constructible_v<T&, U&>` is `true`;
//! `reference_constructs_from_temporary_v<T&, U&>` is `false`; `G` is a
//! reference type; and `is_convertible_v<G, E>` is `true`.
//! \effects If `rhs.has_value()` is `true`, initializes `val` with
//! `addressof(*rhs)`, so that `*this` refers to the object referred to by
//! `rhs`; otherwise, initializes `unex` with the error of `rhs`. No object
//! referred to by `rhs` is moved from.
template <class T, class E>
template <class U, class G>
    requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<T&, U&> &&
             std::is_convertible_v<G, E> && !detail::reference_constructs_from_temporary_v<T&, U&>)
constexpr expected<T&, E>::expected(const expected<U&, G>& rhs) : has_val_(rhs.has_value()) {
    if (has_val_) {
        T& r = *rhs;
        val_ = std::addressof(r);
    } else {
        std::construct_at(std::addressof(unex_), rhs.error());
    }
}

//! \also ref-cvt-copy-ctor-ref
template <class T, class E>
template <class U, class G>
    requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<T&, U&> &&
             std::is_convertible_v<G, E> && !detail::reference_constructs_from_temporary_v<T&, U&>)
constexpr expected<T&, E>::expected(expected<U&, G>&& rhs) : has_val_(rhs.has_value()) {
    if (has_val_) {
        T& r = *rhs;
        val_ = std::addressof(r);
    } else {
        std::construct_at(std::addressof(unex_), rhs.error());
    }
}

//! \group ref-cvt-unexpected-ctor
//! \constraints `is_constructible_v<E, const G&>` is `true`.
//! \effects Initializes `unex` with the error of `e`.
//! \ensures `has_value()` is `false`.
template <class T, class E>
template <class G>
    requires(!std::is_reference_v<E> && std::is_constructible_v<E, const G&>)
constexpr expected<T&, E>::expected(const unexpected<G>& e) : has_val_(false) {
    std::construct_at(std::addressof(unex_), e.error());
}

//! \also ref-cvt-unexpected-ctor
template <class T, class E>
template <class G>
    requires(!std::is_reference_v<E> && std::is_constructible_v<E, G>)
constexpr expected<T&, E>::expected(unexpected<G>&& e) : has_val_(false) {
    std::construct_at(std::addressof(unex_), std::move(e).error());
}

//! \group ref-cvt-unexpected-ctor-ref
//! \constraints `is_reference_v<G>` is `true`; `is_convertible_v<G, E>` is
//! `true`; and `reference_constructs_from_temporary_v<E, G>` is `false`.
//! \effects Initializes `unex` with the error of `e`.
//! \ensures `has_value()` is `false`.
//! \remarks This constructor never throws: the referent is bound, not
//! copied.
template <class T, class E>
template <class G>
    requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
             !detail::reference_constructs_from_temporary_v<E, G>)
constexpr expected<T&, E>::expected(const unexpected<G>& e) noexcept : has_val_(false) {
    std::construct_at(std::addressof(unex_), e.error());
}

//! \also ref-cvt-unexpected-ctor-ref
template <class T, class E>
template <class G>
    requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
             !detail::reference_constructs_from_temporary_v<E, G>)
constexpr expected<T&, E>::expected(unexpected<G>&& e) noexcept : has_val_(false) {
    std::construct_at(std::addressof(unex_), e.error());
}

//! \constraints `is_constructible_v<E, Args...>` is `true`.
//! \effects Direct-non-list-initializes `unex` with `in_place` and
//! `std::forward<Args>(args)...`.
//! \ensures `has_value()` is `false`.
template <class T, class E>
template <class... Args>
    requires(std::is_constructible_v<E, Args...> && !detail::unexpect_dangles_v<E, Args...>)
constexpr expected<T&, E>::expected(unexpect_t, Args&&... args) : has_val_(false) {
    std::construct_at(std::addressof(unex_), std::in_place, std::forward<Args>(args)...);
}

//! \constraints `is_reference_v<E>` is `false`, and `is_constructible_v<E,
//! initializer_list<U>&, Args...>` is `true`.
//! \effects Direct-non-list-initializes `unex` with `in_place`, `il`, and
//! `std::forward<Args>(args)...`.
//! \ensures `has_value()` is `false`.
//! \remarks An overload with the same parameter types is defined as
//! deleted when `E` is an lvalue reference type. An initializer list
//! cannot provide the required long-lived error referent.
template <class T, class E>
template <class U, class... Args>
    requires(!std::is_reference_v<E> && std::is_constructible_v<E, std::initializer_list<U>&, Args...>)
constexpr expected<T&, E>::expected(unexpect_t, std::initializer_list<U> il, Args&&... args) : has_val_(false) {
    std::construct_at(std::addressof(unex_), std::in_place, il, std::forward<Args>(args)...);
}

// \rSec3[expected.ref.dtor]{Destructor}

//! \effects If `has_value()` is `false`, destroys `unex`. `T` is not
//! destroyed; `*this` never owns the object it refers to.
//! \remarks This destructor is trivial if `E` is trivially destructible.
template <class T, class E>
constexpr expected<T&, E>::~expected()
    requires(!std::is_trivially_destructible_v<E>)
{
    if (!has_val_)
        std::destroy_at(std::addressof(unex_));
}

// \rSec3[expected.ref.assign]{Assignment}

//! \group ref-copy-move-assign
//! \effects If `rhs.has_value()` is `true`: if `has_value()` is `true`,
//! assigns `rhs.val` to `val`; otherwise destroys `unex` and initializes
//! `val` with `rhs.val`. If `rhs.has_value()` is `false`, the error of
//! `rhs` is assigned to or used to initialize `unex`, as for the primary
//! template. In every case `*this` comes to refer to the object `rhs`
//! refers to, or to hold the error of `rhs`.
//! \returns `*this`.
//! \remarks This operator is defined as deleted unless
//! `is_copy_assignable_v<E>` is `true` and `is_copy_constructible_v<E>`
//! is `true`.
template <class T, class E>
constexpr expected<T&, E>&
expected<T&, E>::operator=(const expected& rhs) noexcept(std::is_nothrow_copy_constructible_v<E> &&
                                                         std::is_nothrow_copy_assignable_v<E>)
    requires((std::is_reference_v<E> || (std::is_copy_constructible_v<E> && std::is_copy_assignable_v<E>)) &&
             !(std::is_trivially_copy_constructible_v<E> && std::is_trivially_copy_assignable_v<E> &&
               std::is_trivially_destructible_v<E>))
{
    if (has_val_ && rhs.has_val_) {
        val_ = rhs.val_;
    } else if (!has_val_ && !rhs.has_val_) {
        unex_ = rhs.unex_;
    } else if (has_val_) {
        std::construct_at(std::addressof(unex_), rhs.unex_);
        has_val_ = false;
    } else {
        std::destroy_at(std::addressof(unex_));
        val_     = rhs.val_;
        has_val_ = true;
    }
    return *this;
}

//! \also ref-copy-move-assign
template <class T, class E>
constexpr expected<T&, E>&
expected<T&, E>::operator=(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                    std::is_nothrow_move_assignable_v<E>)
    requires((std::is_reference_v<E> || (std::is_move_constructible_v<E> && std::is_move_assignable_v<E>)) &&
             !(std::is_trivially_move_constructible_v<E> && std::is_trivially_move_assignable_v<E> &&
               std::is_trivially_destructible_v<E>))
{
    if (has_val_ && rhs.has_val_) {
        val_ = rhs.val_;
    } else if (!has_val_ && !rhs.has_val_) {
        unex_ = std::move(rhs.unex_);
    } else if (has_val_) {
        std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
        has_val_ = false;
    } else {
        std::destroy_at(std::addressof(unex_));
        val_     = rhs.val_;
        has_val_ = true;
    }
    return *this;
}

//! \group ref-cvt-unexpected-assign
//! \constraints `is_constructible_v<E, const G&>` is `true` and
//! `is_assignable_v<E&, const G&>` is `true`.
//! \effects Makes `*this` hold the error of `e`, reinitializing `unex`
//! from `e` rather than assigning through it.
//! \returns `*this`.
template <class T, class E>
template <class G>
    requires(!std::is_reference_v<E> && std::is_constructible_v<E, const G&> && std::is_assignable_v<E&, const G&>)
constexpr expected<T&, E>& expected<T&, E>::operator=(const unexpected<G>& e) {
    if (!has_val_) {
        unex_.error() = e.error();
    } else {
        std::construct_at(std::addressof(unex_), e.error());
        has_val_ = false;
    }
    return *this;
}

//! \also ref-cvt-unexpected-assign
template <class T, class E>
template <class G>
    requires(!std::is_reference_v<E> && std::is_constructible_v<E, G> && std::is_assignable_v<E&, G>)
constexpr expected<T&, E>& expected<T&, E>::operator=(unexpected<G>&& e) {
    if (!has_val_) {
        unex_.error() = std::move(e).error();
    } else {
        std::construct_at(std::addressof(unex_), std::move(e).error());
        has_val_ = false;
    }
    return *this;
}

// Rebinding assignment for reference E from reference G. val_ is a T* (trivially destructible),
// so no destroy is needed; repoint unex_ via construct_at (not `unex_.error() = ...`).
//! \group ref-cvt-unexpected-assign-ref
//! \constraints `is_reference_v<G>` is `true`; `is_convertible_v<G, E>` is
//! `true`; and `reference_constructs_from_temporary_v<E, G>` is `false`.
//! \effects Makes `*this` hold the error of `e`, reinitializing `unex`
//! from `e` rather than assigning through it. `unex.error()` thereafter
//! refers to the same object as `e.error()`; the previously referenced
//! object, if any, is not modified.
//! \returns `*this`.
//! \remarks This operator never throws: the referent is bound, not
//! copied.
template <class T, class E>
template <class G>
    requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
             !detail::reference_constructs_from_temporary_v<E, G>)
constexpr expected<T&, E>& expected<T&, E>::operator=(const unexpected<G>& e) {
    std::construct_at(std::addressof(unex_), e.error());
    has_val_ = false;
    return *this;
}

//! \also ref-cvt-unexpected-assign-ref
template <class T, class E>
template <class G>
    requires(std::is_reference_v<E> && std::is_reference_v<G> && std::is_constructible_v<E, G> &&
             !detail::reference_constructs_from_temporary_v<E, G>)
constexpr expected<T&, E>& expected<T&, E>::operator=(unexpected<G>&& e) {
    std::construct_at(std::addressof(unex_), e.error());
    has_val_ = false;
    return *this;
}

//! \constraints `is_constructible_v<T&, U>` is `true` and
//! `reference_constructs_from_temporary_v<T&, U>` is `false`.
//! \effects Rebinds `*this` to refer to the object bound by `T& r =
//! std::forward<U>(u);`: if `has_value()` is `false`, destroys `unex`
//! first. Sets `val` to `addressof(r)` and `has_val` to `true`.
//! \returns `*val`.
template <class T, class E>
template <class U>
    requires(std::is_constructible_v<T&, U> && !detail::reference_constructs_from_temporary_v<T&, U>)
constexpr T& expected<T&, E>::emplace(U&& u) noexcept(std::is_nothrow_constructible_v<T&, U>) {
    T& r = std::forward<U>(u); // bind first: if it throws, the current state is left intact
    if (!has_val_) {
        std::destroy_at(std::addressof(unex_));
        has_val_ = true;
    }
    val_ = std::addressof(r);
    return *val_;
}

// \rSec3[expected.ref.swap]{Swap}

//! \effects Exchanges the states of `*this` and `rhs`. When both hold
//! values, exchanges `val` and `rhs.val` — the referenced objects are not
//! swapped. Otherwise behaves as the primary template's `swap` does for
//! the error.
//! \remarks The exception specification is equivalent to
//! `is_nothrow_move_constructible_v<E> && (is_reference_v<E> ||
//! is_nothrow_swappable_v<E>)`.
template <class T, class E>
constexpr void expected<T&, E>::swap(expected& rhs) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                             (std::is_reference_v<E> ||
                                                              std::is_nothrow_swappable_v<E>))
    requires((std::is_reference_v<E> || std::is_swappable_v<E>) && std::is_move_constructible_v<E>)
{
    if (has_val_ && rhs.has_val_) {
        std::swap(val_, rhs.val_);
    } else if (!has_val_ && !rhs.has_val_) {
        using std::swap;
        swap(unex_, rhs.unex_);
    } else if (has_val_) {
        // this has value (pointer), rhs has error
        T* tmp = val_;
        std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
        std::destroy_at(std::addressof(rhs.unex_));
        rhs.val_     = tmp;
        has_val_     = false;
        rhs.has_val_ = true;
    } else {
        rhs.swap(*this);
    }
}

// \rSec3[expected.ref.obs]{Observers}

//! \expects `has_value()` is `true`.
//! \returns `val`.
//! \remarks This is a `const` member function that returns a non-`const`
//! `T*`; the constness of `*this` does not propagate to the referenced
//! object. For deep `const`, use `expected<const T&, E>`.
template <class T, class E>
constexpr T* expected<T&, E>::operator->() const noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (!has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return val_;
}

//! \expects `has_value()` is `true`.
//! \returns `*val`.
//! \remarks This is a `const` member function that returns a non-`const`
//! `T&`; the constness of `*this` does not propagate to the referenced
//! object. For deep `const`, use `expected<const T&, E>`.
template <class T, class E>
constexpr T& expected<T&, E>::operator*() const noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (!has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return *val_;
}

//! \group ref-obs-bool
//! \returns `has_val`.
template <class T, class E>
constexpr expected<T&, E>::operator bool() const noexcept {
    return has_val_;
}

//! \also ref-obs-bool
template <class T, class E>
constexpr bool expected<T&, E>::has_value() const noexcept {
    return has_val_;
}

//! \returns `*val` if `has_value()` is `true`.
//! \throws `bad_expected_access(as_const(error()))` if `has_value()` is
//! `false`.
template <class T, class E>
constexpr T& expected<T&, E>::value() const& {
    static_assert(std::is_copy_constructible_v<error_value_type>, "value() requires is_copy_constructible_v<E>");
    if (!has_val_)
        throw bad_expected_access<error_value_type>(unex_.error());
    return *val_;
}

//! \returns `*val` if `has_value()` is `true`.
//! \throws `bad_expected_access(std::move(error()))` if `has_value()` is
//! `false`.
template <class T, class E>
constexpr T& expected<T&, E>::value() && {
    if constexpr (std::is_reference_v<E>) {
        static_assert(std::is_copy_constructible_v<error_value_type>, "value() requires E to be copy constructible");
    } else {
        static_assert(std::is_copy_constructible_v<error_value_type> && std::is_move_constructible_v<error_value_type>,
                      "value() && requires E be copy and move constructible");
    }
    if (!has_val_)
        throw bad_expected_access<error_value_type>(std::move(unex_).error());
    return *val_;
}

//! \group ref-obs-error-lval
//! \expects `has_value()` is `false`.
//! \returns `unex.error()`.
template <class T, class E>
constexpr const E& expected<T&, E>::error() const& noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return unex_.error();
}

//! \also ref-obs-error-lval
template <class T, class E>
constexpr E& expected<T&, E>::error() & noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return unex_.error();
}

//! \group ref-obs-error-rval
//! \expects `has_value()` is `false`.
//! \returns `std::move(unex).error()`.
template <class T, class E>
constexpr const E&& expected<T&, E>::error() const&& noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return std::move(unex_).error();
}

//! \also ref-obs-error-rval
template <class T, class E>
constexpr E&& expected<T&, E>::error() && noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return std::move(unex_).error();
}

//! \mandates `is_convertible_v<T&, remove_cv_t<T>>` and
//! `is_convertible_v<U, remove_cv_t<T>>` are `true`.
//! \returns `has_value() ? static_cast<remove_cv_t<T>>(*val) :
//! static_cast<remove_cv_t<T>>(std::forward<U>(def))`. The result is an
//! object, never a reference.
template <class T, class E>
template <class U>
    requires(std::is_object_v<T> && !std::is_array_v<T>)
constexpr std::remove_cv_t<T> expected<T&, E>::value_or(U&& def) const {
    using X = std::remove_cv_t<T>;
    static_assert(std::is_convertible_v<T&, X>, "value_or requires T& convertible to remove_cv_t<T>");
    static_assert(std::is_convertible_v<U, X>, "value_or requires is_convertible_v<U, remove_cv_t<T>>");
    if (has_val_)
        return *val_;
    return static_cast<X>(std::forward<U>(def));
}

//! \returns `std::forward<G>(def)` if `has_value()` is `true`, `error()`
//! otherwise. The result is an object, never a reference.
template <class T, class E>
template <class G>
    requires(std::is_copy_constructible_v<std::remove_cv_t<std::remove_reference_t<E>>> &&
             std::is_convertible_v<G, std::remove_cv_t<std::remove_reference_t<E>>>)
constexpr typename expected<T&, E>::error_value_type expected<T&, E>::error_or(G&& def) const& {
    if (!has_val_)
        return unex_.error();
    return static_cast<error_value_type>(std::forward<G>(def));
}

//! \returns `std::forward<G>(def)` if `has_value()` is `true`,
//! `std::move(error())` otherwise. The result is an object, never a
//! reference.
template <class T, class E>
template <class G>
    requires(std::is_move_constructible_v<std::remove_cv_t<std::remove_reference_t<E>>> &&
             std::is_convertible_v<G, std::remove_cv_t<std::remove_reference_t<E>>>)
constexpr typename expected<T&, E>::error_value_type expected<T&, E>::error_or(G&& def) && {
    if (!has_val_)
        return std::move(unex_).error();
    return static_cast<error_value_type>(std::forward<G>(def));
}

// \rSec3[expected.ref.monadic]{Monadic operations}

//! \remarks The member templates `and_then`, `or_else`, `transform`, and
//! `transform_error` behave as specified for the primary template, with
//! one difference: the value is passed to the callable as `T&` for every
//! ref-qualification of `*this`. An rvalue `expected<T&, E>` does not pass
//! its referent as an rvalue; the object referred to is never moved from
//! by these operations.
//! \group ref-monadic-and-then-lval
//! \constraints `is_constructible_v<E, decltype(error())>` is `true`.
//! \mandates `remove_cvref_t<invoke_result_t<F, T&>>` is a specialization
//! of `expected` and its `error_type` is the same type as `E`.
//! \effects Equivalent to: `if (has_value()) return
//! invoke(std::forward<F>(f), *val); else return U(unexpect, error());`
//! where `U` is `remove_cvref_t<invoke_result_t<F, T&>>`.
template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, E&>
constexpr auto expected<T&, E>::and_then(F&& f) & {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f), *val_);
    return U(unexpect, unex_.error());
}

//! \group ref-monadic-and-then-rval
//! \constraints `is_constructible_v<E, decltype(std::move(error()))>` is
//! `true`.
//! \mandates `remove_cvref_t<invoke_result_t<F, T&>>` is a specialization
//! of `expected` and its `error_type` is the same type as `E`.
//! \effects Equivalent to: `if (has_value()) return
//! invoke(std::forward<F>(f), *val); else return U(unexpect,
//! std::move(error()));` where `U` is `remove_cvref_t<invoke_result_t<F,
//! T&>>`.
template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, E&&>
constexpr auto expected<T&, E>::and_then(F&& f) && {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f), *val_);
    return U(unexpect, std::move(unex_).error());
}

//! \also ref-monadic-and-then-lval
template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, const E&>
constexpr auto expected<T&, E>::and_then(F&& f) const& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f), *val_);
    return U(unexpect, unex_.error());
}

//! \also ref-monadic-and-then-rval
template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, const E&&>
constexpr auto expected<T&, E>::and_then(F&& f) const&& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f), *val_);
    return U(unexpect, std::move(unex_).error());
}

//! \group ref-monadic-or-else-lval
//! \mandates `remove_cvref_t<invoke_result_t<F, decltype(error())>>` is a
//! specialization of `expected` and its `value_type` is the same type as
//! `T&`.
//! \effects Equivalent to: `if (has_value()) return G(*val); else return
//! invoke(std::forward<F>(f), error());` where `G` is
//! `remove_cvref_t<invoke_result_t<F, decltype(error())>>`.
template <class T, class E>
template <class F>
constexpr auto expected<T&, E>::or_else(F&& f) & {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, value_type>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G(*val_);
    return std::invoke(std::forward<F>(f), unex_.error());
}

//! \group ref-monadic-or-else-rval
//! \mandates `remove_cvref_t<invoke_result_t<F,
//! decltype(std::move(error()))>>` is a specialization of `expected` and
//! its `value_type` is the same type as `T&`.
//! \effects Equivalent to: `if (has_value()) return G(*val); else return
//! invoke(std::forward<F>(f), std::move(error()));` where `G` is
//! `remove_cvref_t<invoke_result_t<F, decltype(std::move(error()))>>`.
template <class T, class E>
template <class F>
constexpr auto expected<T&, E>::or_else(F&& f) && {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, value_type>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G(*val_);
    return std::invoke(std::forward<F>(f), std::move(unex_).error());
}

//! \also ref-monadic-or-else-lval
template <class T, class E>
template <class F>
constexpr auto expected<T&, E>::or_else(F&& f) const& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, value_type>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G(*val_);
    return std::invoke(std::forward<F>(f), unex_.error());
}

//! \also ref-monadic-or-else-rval
template <class T, class E>
template <class F>
constexpr auto expected<T&, E>::or_else(F&& f) const&& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, value_type>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G(*val_);
    return std::invoke(std::forward<F>(f), std::move(unex_).error());
}

//! \group ref-monadic-transform-lval
//! \constraints `is_constructible_v<E, decltype(error())>` is `true`.
//! \effects Equivalent to: `if (!has_value()) return U(unexpect,
//! error()); else return expected<U2, E>(in_place,
//! invoke(std::forward<F>(f), *val));` where `U2` is
//! `remove_cv_t<invoke_result_t<F, T&>>` and `U` is `expected<U2, E>`.
template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, E&>
constexpr auto expected<T&, E>::transform(F&& f) & {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f), *val_);
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, unex_.error());
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f), *val_));
        return expected<U, E>(unexpect, unex_.error());
    }
}

//! \group ref-monadic-transform-rval
//! \constraints `is_constructible_v<E, decltype(std::move(error()))>` is
//! `true`.
//! \effects Equivalent to: `if (!has_value()) return U(unexpect,
//! std::move(error())); else return expected<U2, E>(in_place,
//! invoke(std::forward<F>(f), *val));` where `U2` is
//! `remove_cv_t<invoke_result_t<F, T&>>` and `U` is `expected<U2, E>`.
template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, E&&>
constexpr auto expected<T&, E>::transform(F&& f) && {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f), *val_);
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, std::move(unex_).error());
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f), *val_));
        return expected<U, E>(unexpect, std::move(unex_).error());
    }
}

//! \also ref-monadic-transform-lval
template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, const E&>
constexpr auto expected<T&, E>::transform(F&& f) const& {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f), *val_);
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, unex_.error());
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f), *val_));
        return expected<U, E>(unexpect, unex_.error());
    }
}

//! \also ref-monadic-transform-rval
template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, const E&&>
constexpr auto expected<T&, E>::transform(F&& f) const&& {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f), *val_);
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, std::move(unex_).error());
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f), *val_));
        return expected<U, E>(unexpect, std::move(unex_).error());
    }
}

//! \group ref-monadic-transform-error-lval
//! \effects Equivalent to: `if (has_value()) return G(*val); else return
//! expected<T&, G2>(unexpect, invoke(std::forward<F>(f), error()));`
//! where `G2` is `remove_cv_t<invoke_result_t<F, decltype(error())>>` and
//! `G` is `expected<T&, G2>`.
template <class T, class E>
template <class F>
constexpr auto expected<T&, E>::transform_error(F&& f) & {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<T&, G>(*val_);
    return expected<T&, G>(unexpect, std::invoke(std::forward<F>(f), unex_.error()));
}

//! \group ref-monadic-transform-error-rval
//! \effects Equivalent to: `if (has_value()) return G(*val); else return
//! expected<T&, G2>(unexpect, invoke(std::forward<F>(f),
//! std::move(error())));` where `G2` is
//! `remove_cv_t<invoke_result_t<F, decltype(std::move(error()))>>` and
//! `G` is `expected<T&, G2>`.
template <class T, class E>
template <class F>
constexpr auto expected<T&, E>::transform_error(F&& f) && {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<T&, G>(*val_);
    return expected<T&, G>(unexpect, std::invoke(std::forward<F>(f), std::move(unex_).error()));
}

//! \also ref-monadic-transform-error-lval
template <class T, class E>
template <class F>
constexpr auto expected<T&, E>::transform_error(F&& f) const& {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<T&, G>(*val_);
    return expected<T&, G>(unexpect, std::invoke(std::forward<F>(f), unex_.error()));
}

//! \also ref-monadic-transform-error-rval
template <class T, class E>
template <class F>
constexpr auto expected<T&, E>::transform_error(F&& f) const&& {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<T&, G>(*val_);
    return expected<T&, G>(unexpect, std::invoke(std::forward<F>(f), std::move(unex_).error()));
}

// \rSec3[expected.ref.eq]{Equality operators}

} // namespace expected
} // namespace beman

#undef BEMAN_EXPECTED_TRAP

#endif
