// beman/expected/expected.hpp                                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef BEMAN_EXPECTED_EXPECTED_HPP
#define BEMAN_EXPECTED_EXPECTED_HPP

#include <beman/expected/unexpected.hpp>
#include <beman/expected/bad_expected_access.hpp>

#include <functional>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

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

template <class T>
struct is_expected_specialization : std::false_type {};

// forward-declared in primary template below; specializations added after class definition

// [expected.object.assign] reinit_expected helper
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

} // namespace detail

template <class T, class E>
class expected;

namespace detail {
template <class T, class E>
struct is_expected_specialization<expected<T, E>> : std::true_type {};
} // namespace detail

// [expected.expected], class template expected
template <class T, class E>
class expected {
    static_assert(!std::is_reference_v<T>, "T must not be a reference (use expected<T&,E> specialization)");
    static_assert(!std::is_reference_v<E>, "E must not be a reference (use expected<T,E&> specialization)");
    static_assert(!std::is_void_v<E>, "E must not be void");
    static_assert(!std::is_same_v<std::remove_cv_t<T>, std::in_place_t>, "T must not be in_place_t");
    static_assert(!std::is_same_v<std::remove_cv_t<T>, unexpect_t>, "T must not be unexpect_t");
    static_assert(!std::is_array_v<T>, "T must not be an array type");
    static_assert(!std::is_array_v<E>, "E must not be an array type");

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

    // Copy constructor
    constexpr expected(const expected& rhs)
        requires(std::is_copy_constructible_v<T> && std::is_copy_constructible_v<E>);

    // Move constructor
    constexpr expected(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                std::is_nothrow_move_constructible_v<E>)
        requires(std::is_move_constructible_v<T> && std::is_move_constructible_v<E>);

    // Converting copy constructor from expected<U, G>
    template <class U, class G>
        requires(std::is_constructible_v<T, const U&> && std::is_constructible_v<E, const G&> &&
                 !std::is_constructible_v<T, expected<U, G>&> && !std::is_constructible_v<T, expected<U, G> &&> &&
                 !std::is_constructible_v<T, const expected<U, G>&> &&
                 !std::is_constructible_v<T, const expected<U, G> &&> && !std::is_convertible_v<expected<U, G>&, T> &&
                 !std::is_convertible_v<expected<U, G> &&, T> && !std::is_convertible_v<const expected<U, G>&, T> &&
                 !std::is_convertible_v<const expected<U, G> &&, T> &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
    constexpr explicit(!std::is_convertible_v<const U&, T> || !std::is_convertible_v<const G&, E>)
        expected(const expected<U, G>& rhs);

    // Converting move constructor from expected<U, G>
    template <class U, class G>
        requires(std::is_constructible_v<T, U> && std::is_constructible_v<E, G> &&
                 !std::is_constructible_v<T, expected<U, G>&> && !std::is_constructible_v<T, expected<U, G> &&> &&
                 !std::is_constructible_v<T, const expected<U, G>&> &&
                 !std::is_constructible_v<T, const expected<U, G> &&> && !std::is_convertible_v<expected<U, G>&, T> &&
                 !std::is_convertible_v<expected<U, G> &&, T> && !std::is_convertible_v<const expected<U, G>&, T> &&
                 !std::is_convertible_v<const expected<U, G> &&, T> &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
    constexpr explicit(!std::is_convertible_v<U, T> || !std::is_convertible_v<G, E>) expected(expected<U, G>&& rhs);

    // Constructor from value U&&
    template <class U = std::remove_cv_t<T>>
        requires(!std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
                 !std::is_same_v<std::remove_cvref_t<U>, unexpect_t> &&
                 !std::is_same_v<std::remove_cvref_t<U>, expected> && std::is_constructible_v<T, U>)
    constexpr explicit(!std::is_convertible_v<U, T>) expected(U&& v);

    // Constructor from unexpected<G> const&
    template <class G>
        requires std::is_constructible_v<E, const G&>
    constexpr explicit(!std::is_convertible_v<const G&, E>) expected(const unexpected<G>& e);

    // Constructor from unexpected<G>&&
    template <class G>
        requires std::is_constructible_v<E, G>
    constexpr explicit(!std::is_convertible_v<G, E>) expected(unexpected<G>&& e);

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
        requires std::is_constructible_v<E, Args...>
    constexpr explicit expected(unexpect_t, Args&&... args);

    // In-place constructor for error with initializer_list
    template <class U, class... Args>
        requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
    constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&... args);

    // -------------------------------------------------------------------------
    // [expected.object.dtor] Destructor
    // -------------------------------------------------------------------------

    constexpr ~expected()
        requires(std::is_trivially_destructible_v<T> && std::is_trivially_destructible_v<E>)
    = default;

    constexpr ~expected()
        requires(!(std::is_trivially_destructible_v<T> && std::is_trivially_destructible_v<E>));

    // -------------------------------------------------------------------------
    // [expected.object.assign] Assignment
    // -------------------------------------------------------------------------

    // Copy assignment
    constexpr expected& operator=(const expected& rhs)
        requires(std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T> && std::is_copy_constructible_v<E> &&
                 std::is_copy_assignable_v<E> &&
                 (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>));

    // Move assignment
    constexpr expected& operator=(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                           std::is_nothrow_move_assignable_v<T> &&
                                                           std::is_nothrow_move_constructible_v<E> &&
                                                           std::is_nothrow_move_assignable_v<E>)
        requires(std::is_move_constructible_v<T> && std::is_move_assignable_v<T> && std::is_move_constructible_v<E> &&
                 std::is_move_assignable_v<E>);

    // Assignment from value U&&
    template <class U = T>
        requires(!std::is_same_v<expected, std::remove_cvref_t<U>> &&
                 !std::is_same_v<std::remove_cvref_t<U>, unexpect_t> && std::is_constructible_v<T, U> &&
                 std::is_assignable_v<T&, U> &&
                 (std::is_nothrow_constructible_v<T, U> || std::is_nothrow_move_constructible_v<T> ||
                  std::is_nothrow_move_constructible_v<E>))
    constexpr expected& operator=(U&& v);

    // Assignment from unexpected<G> const&
    template <class G>
        requires(std::is_constructible_v<E, const G&> && std::is_assignable_v<E&, const G&> &&
                 (std::is_nothrow_constructible_v<E, const G&> || std::is_nothrow_move_constructible_v<T> ||
                  std::is_nothrow_move_constructible_v<E>))
    constexpr expected& operator=(const unexpected<G>& e);

    // Assignment from unexpected<G>&&
    template <class G>
        requires(std::is_constructible_v<E, G> && std::is_assignable_v<E&, G> &&
                 (std::is_nothrow_constructible_v<E, G> || std::is_nothrow_move_constructible_v<T> ||
                  std::is_nothrow_move_constructible_v<E>))
    constexpr expected& operator=(unexpected<G>&& e);

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

    constexpr void
    swap(expected& rhs) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_swappable_v<T> &&
                                 std::is_nothrow_move_constructible_v<E> && std::is_nothrow_swappable_v<E>)
        requires(std::is_swappable_v<T> && std::is_swappable_v<E> && std::is_move_constructible_v<T> &&
                 std::is_move_constructible_v<E> &&
                 (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>));

    friend constexpr void swap(expected& x, expected& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }

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

    template <class G = E>
    constexpr E error_or(G&& def) const&;

    template <class G = E>
    constexpr E error_or(G&& def) &&;

    // -------------------------------------------------------------------------
    // [expected.object.monadic] Monadic operations
    // -------------------------------------------------------------------------

    template <class F>
    constexpr auto and_then(F&& f) &;
    template <class F>
    constexpr auto and_then(F&& f) &&;
    template <class F>
    constexpr auto and_then(F&& f) const&;
    template <class F>
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
    constexpr auto transform(F&& f) &;
    template <class F>
    constexpr auto transform(F&& f) &&;
    template <class F>
    constexpr auto transform(F&& f) const&;
    template <class F>
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
    // [expected.object.eq] Equality operators (hidden friends)
    // -------------------------------------------------------------------------

    template <class T2, class E2>
        requires(!std::is_void_v<T2>)
    friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y) {
        if (x.has_value() != y.has_value())
            return false;
        if (x.has_value())
            return *x == *y;
        return x.error() == y.error();
    }

    template <class T2>
    friend constexpr bool operator==(const expected& x, const T2& val) {
        return x.has_value() && static_cast<bool>(*x == val);
    }

    template <class E2>
    friend constexpr bool operator==(const expected& x, const unexpected<E2>& e) {
        return !x.has_value() && static_cast<bool>(x.error() == e.error());
    }

  private:
    bool has_val_;
    union {
        T val_;
        E unex_;
    };
};

// =============================================================================
// [expected.object.cons] Out-of-line constructor definitions
// =============================================================================

template <class T, class E>
constexpr expected<T, E>::expected() noexcept(std::is_nothrow_default_constructible_v<T>)
    requires std::is_default_constructible_v<T>
    : has_val_(true) {
    std::construct_at(std::addressof(val_));
}

template <class T, class E>
constexpr expected<T, E>::expected(const expected& rhs)
    requires(std::is_copy_constructible_v<T> && std::is_copy_constructible_v<E>)
    : has_val_(rhs.has_val_) {
    if (has_val_)
        std::construct_at(std::addressof(val_), rhs.val_);
    else
        std::construct_at(std::addressof(unex_), rhs.unex_);
}

template <class T, class E>
constexpr expected<T, E>::expected(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                            std::is_nothrow_move_constructible_v<E>)
    requires(std::is_move_constructible_v<T> && std::is_move_constructible_v<E>)
    : has_val_(rhs.has_val_) {
    if (has_val_)
        std::construct_at(std::addressof(val_), std::move(rhs.val_));
    else
        std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
}

template <class T, class E>
template <class U, class G>
    requires(std::is_constructible_v<T, const U&> && std::is_constructible_v<E, const G&> &&
             !std::is_constructible_v<T, expected<U, G>&> && !std::is_constructible_v<T, expected<U, G> &&> &&
             !std::is_constructible_v<T, const expected<U, G>&> &&
             !std::is_constructible_v<T, const expected<U, G> &&> && !std::is_convertible_v<expected<U, G>&, T> &&
             !std::is_convertible_v<expected<U, G> &&, T> && !std::is_convertible_v<const expected<U, G>&, T> &&
             !std::is_convertible_v<const expected<U, G> &&, T> &&
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

template <class T, class E>
template <class U, class G>
    requires(std::is_constructible_v<T, U> && std::is_constructible_v<E, G> &&
             !std::is_constructible_v<T, expected<U, G>&> && !std::is_constructible_v<T, expected<U, G> &&> &&
             !std::is_constructible_v<T, const expected<U, G>&> &&
             !std::is_constructible_v<T, const expected<U, G> &&> && !std::is_convertible_v<expected<U, G>&, T> &&
             !std::is_convertible_v<expected<U, G> &&, T> && !std::is_convertible_v<const expected<U, G>&, T> &&
             !std::is_convertible_v<const expected<U, G> &&, T> &&
             !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
constexpr expected<T, E>::expected(expected<U, G>&& rhs) : has_val_(rhs.has_value()) {
    if (has_val_)
        std::construct_at(std::addressof(val_), std::move(*rhs));
    else
        std::construct_at(std::addressof(unex_), std::move(rhs.error()));
}

template <class T, class E>
template <class U>
    requires(!std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
             !std::is_same_v<std::remove_cvref_t<U>, unexpect_t> &&
             !std::is_same_v<std::remove_cvref_t<U>, expected<T, E>> && std::is_constructible_v<T, U>)
constexpr expected<T, E>::expected(U&& v) : has_val_(true) {
    std::construct_at(std::addressof(val_), std::forward<U>(v));
}

template <class T, class E>
template <class G>
    requires std::is_constructible_v<E, const G&>
constexpr expected<T, E>::expected(const unexpected<G>& e) : has_val_(false) {
    std::construct_at(std::addressof(unex_), e.error());
}

template <class T, class E>
template <class G>
    requires std::is_constructible_v<E, G>
constexpr expected<T, E>::expected(unexpected<G>&& e) : has_val_(false) {
    std::construct_at(std::addressof(unex_), std::move(e.error()));
}

template <class T, class E>
template <class... Args>
    requires std::is_constructible_v<T, Args...>
constexpr expected<T, E>::expected(std::in_place_t, Args&&... args) : has_val_(true) {
    std::construct_at(std::addressof(val_), std::forward<Args>(args)...);
}

template <class T, class E>
template <class U, class... Args>
    requires std::is_constructible_v<T, std::initializer_list<U>&, Args...>
constexpr expected<T, E>::expected(std::in_place_t, std::initializer_list<U> il, Args&&... args) : has_val_(true) {
    std::construct_at(std::addressof(val_), il, std::forward<Args>(args)...);
}

template <class T, class E>
template <class... Args>
    requires std::is_constructible_v<E, Args...>
constexpr expected<T, E>::expected(unexpect_t, Args&&... args) : has_val_(false) {
    std::construct_at(std::addressof(unex_), std::forward<Args>(args)...);
}

template <class T, class E>
template <class U, class... Args>
    requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
constexpr expected<T, E>::expected(unexpect_t, std::initializer_list<U> il, Args&&... args) : has_val_(false) {
    std::construct_at(std::addressof(unex_), il, std::forward<Args>(args)...);
}

// =============================================================================
// [expected.object.dtor] Out-of-line destructor
// =============================================================================

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
// [expected.object.assign] Out-of-line assignment definitions
// =============================================================================

template <class T, class E>
constexpr expected<T, E>& expected<T, E>::operator=(const expected& rhs)
    requires(std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T> && std::is_copy_constructible_v<E> &&
             std::is_copy_assignable_v<E> &&
             (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>))
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

template <class T, class E>
constexpr expected<T, E>& expected<T, E>::operator=(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                                             std::is_nothrow_move_assignable_v<T> &&
                                                                             std::is_nothrow_move_constructible_v<E> &&
                                                                             std::is_nothrow_move_assignable_v<E>)
    requires(std::is_move_constructible_v<T> && std::is_move_assignable_v<T> && std::is_move_constructible_v<E> &&
             std::is_move_assignable_v<E>)
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

template <class T, class E>
template <class U>
    requires(!std::is_same_v<expected<T, E>, std::remove_cvref_t<U>> &&
             !std::is_same_v<std::remove_cvref_t<U>, unexpect_t> && std::is_constructible_v<T, U> &&
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

template <class T, class E>
template <class G>
    requires(std::is_constructible_v<E, const G&> && std::is_assignable_v<E&, const G&> &&
             (std::is_nothrow_constructible_v<E, const G&> || std::is_nothrow_move_constructible_v<T> ||
              std::is_nothrow_move_constructible_v<E>))
constexpr expected<T, E>& expected<T, E>::operator=(const unexpected<G>& e) {
    if (!has_val_) {
        unex_ = e.error();
    } else {
        detail::reinit_expected(unex_, val_, e.error());
        has_val_ = false;
    }
    return *this;
}

template <class T, class E>
template <class G>
    requires(std::is_constructible_v<E, G> && std::is_assignable_v<E&, G> &&
             (std::is_nothrow_constructible_v<E, G> || std::is_nothrow_move_constructible_v<T> ||
              std::is_nothrow_move_constructible_v<E>))
constexpr expected<T, E>& expected<T, E>::operator=(unexpected<G>&& e) {
    if (!has_val_) {
        unex_ = std::move(e.error());
    } else {
        detail::reinit_expected(unex_, val_, std::move(e.error()));
        has_val_ = false;
    }
    return *this;
}

// =============================================================================
// [expected.object.assign] Out-of-line emplace definitions
// =============================================================================

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

// =============================================================================
// [expected.object.swap] Out-of-line swap definition
// =============================================================================

template <class T, class E>
constexpr void expected<T, E>::swap(expected& rhs) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                            std::is_nothrow_swappable_v<T> &&
                                                            std::is_nothrow_move_constructible_v<E> &&
                                                            std::is_nothrow_swappable_v<E>)
    requires(std::is_swappable_v<T> && std::is_swappable_v<E> && std::is_move_constructible_v<T> &&
             std::is_move_constructible_v<E> &&
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
            E tmp(std::move(rhs.unex_));
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
// [expected.object.obs] Out-of-line observer definitions
// =============================================================================

template <class T, class E>
constexpr const T* expected<T, E>::operator->() const noexcept {
    return std::addressof(val_);
}

template <class T, class E>
constexpr T* expected<T, E>::operator->() noexcept {
    return std::addressof(val_);
}

template <class T, class E>
constexpr const T& expected<T, E>::operator*() const& noexcept {
    return val_;
}

template <class T, class E>
constexpr T& expected<T, E>::operator*() & noexcept {
    return val_;
}

template <class T, class E>
constexpr const T&& expected<T, E>::operator*() const&& noexcept {
    return std::move(val_);
}

template <class T, class E>
constexpr T&& expected<T, E>::operator*() && noexcept {
    return std::move(val_);
}

template <class T, class E>
constexpr expected<T, E>::operator bool() const noexcept {
    return has_val_;
}

template <class T, class E>
constexpr bool expected<T, E>::has_value() const noexcept {
    return has_val_;
}

template <class T, class E>
constexpr const T& expected<T, E>::value() const& {
    if (!has_val_)
        throw bad_expected_access<E>(unex_);
    return val_;
}

template <class T, class E>
constexpr T& expected<T, E>::value() & {
    if (!has_val_)
        throw bad_expected_access<E>(unex_);
    return val_;
}

template <class T, class E>
constexpr const T&& expected<T, E>::value() const&& {
    if (!has_val_)
        throw bad_expected_access<E>(std::move(unex_));
    return std::move(val_);
}

template <class T, class E>
constexpr T&& expected<T, E>::value() && {
    if (!has_val_)
        throw bad_expected_access<E>(std::move(unex_));
    return std::move(val_);
}

template <class T, class E>
constexpr const E& expected<T, E>::error() const& noexcept {
    return unex_;
}

template <class T, class E>
constexpr E& expected<T, E>::error() & noexcept {
    return unex_;
}

template <class T, class E>
constexpr const E&& expected<T, E>::error() const&& noexcept {
    return std::move(unex_);
}

template <class T, class E>
constexpr E&& expected<T, E>::error() && noexcept {
    return std::move(unex_);
}

template <class T, class E>
template <class U>
constexpr T expected<T, E>::value_or(U&& def) const& {
    if (has_val_)
        return val_;
    return static_cast<T>(std::forward<U>(def));
}

template <class T, class E>
template <class U>
constexpr T expected<T, E>::value_or(U&& def) && {
    if (has_val_)
        return std::move(val_);
    return static_cast<T>(std::forward<U>(def));
}

template <class T, class E>
template <class G>
constexpr E expected<T, E>::error_or(G&& def) const& {
    if (!has_val_)
        return unex_;
    return static_cast<E>(std::forward<G>(def));
}

template <class T, class E>
template <class G>
constexpr E expected<T, E>::error_or(G&& def) && {
    if (!has_val_)
        return std::move(unex_);
    return static_cast<E>(std::forward<G>(def));
}

// =============================================================================
// [expected.object.monadic] Out-of-line monadic operation definitions
// =============================================================================

template <class T, class E>
template <class F>
constexpr auto expected<T, E>::and_then(F&& f) & {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f), val_);
    return U(unexpect, unex_);
}

template <class T, class E>
template <class F>
constexpr auto expected<T, E>::and_then(F&& f) && {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T&&>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f), std::move(val_));
    return U(unexpect, std::move(unex_));
}

template <class T, class E>
template <class F>
constexpr auto expected<T, E>::and_then(F&& f) const& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const T&>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f), val_);
    return U(unexpect, unex_);
}

template <class T, class E>
template <class F>
constexpr auto expected<T, E>::and_then(F&& f) const&& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const T&&>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f), std::move(val_));
    return U(unexpect, std::move(unex_));
}

template <class T, class E>
template <class F>
constexpr auto expected<T, E>::or_else(F&& f) & {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, T>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G(std::in_place, val_);
    return std::invoke(std::forward<F>(f), unex_);
}

template <class T, class E>
template <class F>
constexpr auto expected<T, E>::or_else(F&& f) && {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, T>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G(std::in_place, std::move(val_));
    return std::invoke(std::forward<F>(f), std::move(unex_));
}

template <class T, class E>
template <class F>
constexpr auto expected<T, E>::or_else(F&& f) const& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, T>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G(std::in_place, val_);
    return std::invoke(std::forward<F>(f), unex_);
}

template <class T, class E>
template <class F>
constexpr auto expected<T, E>::or_else(F&& f) const&& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, T>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G(std::in_place, std::move(val_));
    return std::invoke(std::forward<F>(f), std::move(unex_));
}

template <class T, class E>
template <class F>
constexpr auto expected<T, E>::transform(F&& f) & {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f), val_);
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, unex_);
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f), val_));
        return expected<U, E>(unexpect, unex_);
    }
}

template <class T, class E>
template <class F>
constexpr auto expected<T, E>::transform(F&& f) && {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&&>>;
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f), std::move(val_));
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, std::move(unex_));
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f), std::move(val_)));
        return expected<U, E>(unexpect, std::move(unex_));
    }
}

template <class T, class E>
template <class F>
constexpr auto expected<T, E>::transform(F&& f) const& {
    using U = std::remove_cv_t<std::invoke_result_t<F, const T&>>;
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f), val_);
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, unex_);
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f), val_));
        return expected<U, E>(unexpect, unex_);
    }
}

template <class T, class E>
template <class F>
constexpr auto expected<T, E>::transform(F&& f) const&& {
    using U = std::remove_cv_t<std::invoke_result_t<F, const T&&>>;
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f), std::move(val_));
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, std::move(unex_));
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f), std::move(val_)));
        return expected<U, E>(unexpect, std::move(unex_));
    }
}

template <class T, class E>
template <class F>
constexpr auto expected<T, E>::transform_error(F&& f) & {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
    if (has_val_)
        return expected<T, G>(std::in_place, val_);
    return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), unex_));
}

template <class T, class E>
template <class F>
constexpr auto expected<T, E>::transform_error(F&& f) && {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&&>>;
    if (has_val_)
        return expected<T, G>(std::in_place, std::move(val_));
    return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), std::move(unex_)));
}

template <class T, class E>
template <class F>
constexpr auto expected<T, E>::transform_error(F&& f) const& {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&>>;
    if (has_val_)
        return expected<T, G>(std::in_place, val_);
    return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), unex_));
}

template <class T, class E>
template <class F>
constexpr auto expected<T, E>::transform_error(F&& f) const&& {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&&>>;
    if (has_val_)
        return expected<T, G>(std::in_place, std::move(val_));
    return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), std::move(unex_)));
}

// =============================================================================
// [expected.void] Partial specialization for void value type
// =============================================================================

template <class T, class E>
    requires std::is_void_v<T>
class expected<T, E> {
    static_assert(!std::is_reference_v<E>, "E must not be a reference");
    static_assert(!std::is_void_v<E>, "E must not be void");
    static_assert(!std::is_array_v<E>, "E must not be an array type");
    static_assert(std::is_same_v<std::remove_cv_t<E>, E>, "E must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<E>::value, "E must not be an unexpected<X> specialization");

  public:
    using value_type      = T;
    using error_type      = E;
    using unexpected_type = unexpected<E>;

    template <class U>
    using rebind = expected<U, error_type>;

    // -------------------------------------------------------------------------
    // [expected.void.cons] Constructors
    // -------------------------------------------------------------------------

    constexpr expected() noexcept : has_val_(true) {}

    constexpr expected(const expected&)
        requires std::is_trivially_copy_constructible_v<E>
    = default;

    constexpr expected(const expected& rhs)
        requires(std::is_copy_constructible_v<E> && !std::is_trivially_copy_constructible_v<E>);

    constexpr expected(expected&&) noexcept
        requires std::is_trivially_move_constructible_v<E>
    = default;

    constexpr expected(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E>)
        requires(std::is_move_constructible_v<E> && !std::is_trivially_move_constructible_v<E>);

    // Converting constructor from expected<U, G> where is_void_v<U>
    template <class U, class G>
        requires(std::is_void_v<U> && std::is_constructible_v<E, const G&> &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
    constexpr explicit(!std::is_convertible_v<const G&, E>) expected(const expected<U, G>& rhs);

    template <class U, class G>
        requires(std::is_void_v<U> && std::is_constructible_v<E, G> &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
    constexpr explicit(!std::is_convertible_v<G, E>) expected(expected<U, G>&& rhs);

    // Constructor from unexpected<G> const&
    template <class G>
        requires std::is_constructible_v<E, const G&>
    constexpr explicit(!std::is_convertible_v<const G&, E>) expected(const unexpected<G>& e);

    // Constructor from unexpected<G>&&
    template <class G>
        requires std::is_constructible_v<E, G>
    constexpr explicit(!std::is_convertible_v<G, E>) expected(unexpected<G>&& e);

    // In-place constructor for value (no args, just marks has-value)
    constexpr explicit expected(std::in_place_t) noexcept : has_val_(true) {}

    // In-place constructor for error
    template <class... Args>
        requires std::is_constructible_v<E, Args...>
    constexpr explicit expected(unexpect_t, Args&&... args);

    // In-place constructor for error with initializer_list
    template <class U, class... Args>
        requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
    constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&... args);

    // -------------------------------------------------------------------------
    // [expected.void.dtor] Destructor
    // -------------------------------------------------------------------------

    constexpr ~expected()
        requires std::is_trivially_destructible_v<E>
    = default;

    constexpr ~expected()
        requires(!std::is_trivially_destructible_v<E>);

    // -------------------------------------------------------------------------
    // [expected.void.assign] Assignment
    // -------------------------------------------------------------------------

    constexpr expected& operator=(const expected& rhs)
        requires(std::is_copy_constructible_v<E> && std::is_copy_assignable_v<E>);

    constexpr expected& operator=(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                           std::is_nothrow_move_assignable_v<E>)
        requires(std::is_move_constructible_v<E> && std::is_move_assignable_v<E>);

    template <class G>
        requires(std::is_constructible_v<E, const G&> && std::is_assignable_v<E&, const G&>)
    constexpr expected& operator=(const unexpected<G>& e);

    template <class G>
        requires(std::is_constructible_v<E, G> && std::is_assignable_v<E&, G>)
    constexpr expected& operator=(unexpected<G>&& e);

    constexpr void emplace() noexcept;

    // -------------------------------------------------------------------------
    // [expected.void.swap] Swap
    // -------------------------------------------------------------------------

    constexpr void swap(expected& rhs) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                std::is_nothrow_swappable_v<E>)
        requires(std::is_swappable_v<E> && std::is_move_constructible_v<E>);

    friend constexpr void swap(expected& x, expected& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }

    // -------------------------------------------------------------------------
    // [expected.void.obs] Observers
    // -------------------------------------------------------------------------

    constexpr explicit operator bool() const noexcept { return has_val_; }
    constexpr bool     has_value() const noexcept { return has_val_; }

    constexpr void operator*() const noexcept {}

    constexpr void value() const&;
    constexpr void value() &&;

    constexpr const E&  error() const& noexcept { return unex_; }
    constexpr E&        error() & noexcept { return unex_; }
    constexpr const E&& error() const&& noexcept { return std::move(unex_); }
    constexpr E&&       error() && noexcept { return std::move(unex_); }

    template <class G = E>
    constexpr E error_or(G&& def) const&;

    template <class G = E>
    constexpr E error_or(G&& def) &&;

    // -------------------------------------------------------------------------
    // [expected.void.eq] Equality operators (hidden friends)
    // -------------------------------------------------------------------------

    template <class T2, class E2>
        requires std::is_void_v<T2>
    friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y) {
        if (x.has_value() != y.has_value())
            return false;
        if (x.has_value())
            return true;
        return x.error() == y.error();
    }

    template <class E2>
    friend constexpr bool operator==(const expected& x, const unexpected<E2>& e) {
        return !x.has_value() && static_cast<bool>(x.error() == e.error());
    }

  private:
    bool has_val_;
    union {
        E unex_;
    };
};

// =============================================================================
// [expected.void.cons] Out-of-line constructor definitions
// =============================================================================

template <class T, class E>
    requires std::is_void_v<T>
constexpr expected<T, E>::expected(const expected& rhs)
    requires(std::is_copy_constructible_v<E> && !std::is_trivially_copy_constructible_v<E>)
    : has_val_(rhs.has_val_) {
    if (!has_val_)
        std::construct_at(std::addressof(unex_), rhs.unex_);
}

template <class T, class E>
    requires std::is_void_v<T>
constexpr expected<T, E>::expected(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E>)
    requires(std::is_move_constructible_v<E> && !std::is_trivially_move_constructible_v<E>)
    : has_val_(rhs.has_val_) {
    if (!has_val_)
        std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
}

template <class T, class E>
    requires std::is_void_v<T>
template <class U, class G>
    requires(std::is_void_v<U> && std::is_constructible_v<E, const G&> &&
             !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
constexpr expected<T, E>::expected(const expected<U, G>& rhs) : has_val_(rhs.has_value()) {
    if (!has_val_)
        std::construct_at(std::addressof(unex_), rhs.error());
}

template <class T, class E>
    requires std::is_void_v<T>
template <class U, class G>
    requires(std::is_void_v<U> && std::is_constructible_v<E, G> &&
             !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
constexpr expected<T, E>::expected(expected<U, G>&& rhs) : has_val_(rhs.has_value()) {
    if (!has_val_)
        std::construct_at(std::addressof(unex_), std::move(rhs.error()));
}

template <class T, class E>
    requires std::is_void_v<T>
template <class G>
    requires std::is_constructible_v<E, const G&>
constexpr expected<T, E>::expected(const unexpected<G>& e) : has_val_(false) {
    std::construct_at(std::addressof(unex_), e.error());
}

template <class T, class E>
    requires std::is_void_v<T>
template <class G>
    requires std::is_constructible_v<E, G>
constexpr expected<T, E>::expected(unexpected<G>&& e) : has_val_(false) {
    std::construct_at(std::addressof(unex_), std::move(e.error()));
}

template <class T, class E>
    requires std::is_void_v<T>
template <class... Args>
    requires std::is_constructible_v<E, Args...>
constexpr expected<T, E>::expected(unexpect_t, Args&&... args) : has_val_(false) {
    std::construct_at(std::addressof(unex_), std::forward<Args>(args)...);
}

template <class T, class E>
    requires std::is_void_v<T>
template <class U, class... Args>
    requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
constexpr expected<T, E>::expected(unexpect_t, std::initializer_list<U> il, Args&&... args) : has_val_(false) {
    std::construct_at(std::addressof(unex_), il, std::forward<Args>(args)...);
}

// =============================================================================
// [expected.void.dtor] Out-of-line destructor
// =============================================================================

template <class T, class E>
    requires std::is_void_v<T>
constexpr expected<T, E>::~expected()
    requires(!std::is_trivially_destructible_v<E>)
{
    if (!has_val_)
        std::destroy_at(std::addressof(unex_));
}

// =============================================================================
// [expected.void.assign] Out-of-line assignment definitions
// =============================================================================

template <class T, class E>
    requires std::is_void_v<T>
constexpr expected<T, E>& expected<T, E>::operator=(const expected& rhs)
    requires(std::is_copy_constructible_v<E> && std::is_copy_assignable_v<E>)
{
    if (has_val_ && rhs.has_val_) {
        // both value: no-op
    } else if (!has_val_ && !rhs.has_val_) {
        unex_ = rhs.unex_;
    } else if (has_val_) {
        // was value, now error
        std::construct_at(std::addressof(unex_), rhs.unex_);
        has_val_ = false;
    } else {
        // was error, now value
        std::destroy_at(std::addressof(unex_));
        has_val_ = true;
    }
    return *this;
}

template <class T, class E>
    requires std::is_void_v<T>
constexpr expected<T, E>& expected<T, E>::operator=(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                                             std::is_nothrow_move_assignable_v<E>)
    requires(std::is_move_constructible_v<E> && std::is_move_assignable_v<E>)
{
    if (has_val_ && rhs.has_val_) {
        // both value: no-op
    } else if (!has_val_ && !rhs.has_val_) {
        unex_ = std::move(rhs.unex_);
    } else if (has_val_) {
        // was value, now error
        std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
        has_val_ = false;
    } else {
        // was error, now value
        std::destroy_at(std::addressof(unex_));
        has_val_ = true;
    }
    return *this;
}

template <class T, class E>
    requires std::is_void_v<T>
template <class G>
    requires(std::is_constructible_v<E, const G&> && std::is_assignable_v<E&, const G&>)
constexpr expected<T, E>& expected<T, E>::operator=(const unexpected<G>& e) {
    if (!has_val_) {
        unex_ = e.error();
    } else {
        std::construct_at(std::addressof(unex_), e.error());
        has_val_ = false;
    }
    return *this;
}

template <class T, class E>
    requires std::is_void_v<T>
template <class G>
    requires(std::is_constructible_v<E, G> && std::is_assignable_v<E&, G>)
constexpr expected<T, E>& expected<T, E>::operator=(unexpected<G>&& e) {
    if (!has_val_) {
        unex_ = std::move(e.error());
    } else {
        std::construct_at(std::addressof(unex_), std::move(e.error()));
        has_val_ = false;
    }
    return *this;
}

template <class T, class E>
    requires std::is_void_v<T>
constexpr void expected<T, E>::emplace() noexcept {
    if (!has_val_) {
        std::destroy_at(std::addressof(unex_));
        has_val_ = true;
    }
}

// =============================================================================
// [expected.void.swap] Out-of-line swap definition
// =============================================================================

template <class T, class E>
    requires std::is_void_v<T>
constexpr void expected<T, E>::swap(expected& rhs) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                            std::is_nothrow_swappable_v<E>)
    requires(std::is_swappable_v<E> && std::is_move_constructible_v<E>)
{
    if (has_val_ && rhs.has_val_) {
        // both value: no-op
    } else if (!has_val_ && !rhs.has_val_) {
        using std::swap;
        swap(unex_, rhs.unex_);
    } else if (has_val_) {
        // this has value, rhs has error
        std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
        std::destroy_at(std::addressof(rhs.unex_));
        has_val_     = false;
        rhs.has_val_ = true;
    } else {
        // this has error, rhs has value
        rhs.swap(*this);
    }
}

// =============================================================================
// [expected.void.obs] Out-of-line observer definitions
// =============================================================================

template <class T, class E>
    requires std::is_void_v<T>
constexpr void expected<T, E>::value() const& {
    if (!has_val_)
        throw bad_expected_access<E>(unex_);
}

template <class T, class E>
    requires std::is_void_v<T>
constexpr void expected<T, E>::value() && {
    if (!has_val_)
        throw bad_expected_access<E>(std::move(unex_));
}

template <class T, class E>
    requires std::is_void_v<T>
template <class G>
constexpr E expected<T, E>::error_or(G&& def) const& {
    if (!has_val_)
        return unex_;
    return static_cast<E>(std::forward<G>(def));
}

template <class T, class E>
    requires std::is_void_v<T>
template <class G>
constexpr E expected<T, E>::error_or(G&& def) && {
    if (!has_val_)
        return std::move(unex_);
    return static_cast<E>(std::forward<G>(def));
}

} // namespace expected
} // namespace beman

#endif
