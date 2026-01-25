// beman/expected/expected.hpp                                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef BEMAN_EXPECTED_EXPECTED_HPP
#define BEMAN_EXPECTED_EXPECTED_HPP

#include <beman/expected/unexpected.hpp>
#include <beman/expected/bad_expected_access.hpp>

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

/***
22.8.6 Class template expected[expected.expected]
22.8.6.1 General[expected.object.general]
namespace std {
  template<class T, class E>
  class expected {
  public:
    using value_type = T;
    using error_type = E;
    using unexpected_type = unexpected<E>;

    template<class U>
    using rebind = expected<U, error_type>;

    // [expected.object.cons], constructors
    constexpr expected();
    constexpr expected(const expected&);
    constexpr expected(expected&&) noexcept(see below);
    template<class U, class G>
      constexpr explicit(see below) expected(const expected<U, G>&);
    template<class U, class G>
      constexpr explicit(see below) expected(expected<U, G>&&);

    template<class U = remove_cv_t<T>>
      constexpr explicit(see below) expected(U&& v);

    template<class G>
      constexpr explicit(see below) expected(const unexpected<G>&);
    template<class G>
      constexpr explicit(see below) expected(unexpected<G>&&);

    template<class... Args>
      constexpr explicit expected(in_place_t, Args&&...);
    template<class U, class... Args>
      constexpr explicit expected(in_place_t, initializer_list<U>, Args&&...);
    template<class... Args>
      constexpr explicit expected(unexpect_t, Args&&...);
    template<class U, class... Args>
      constexpr explicit expected(unexpect_t, initializer_list<U>, Args&&...);

    // [expected.object.dtor], destructor
    constexpr ~expected();

    // [expected.object.assign], assignment
    constexpr expected& operator=(const expected&);
    constexpr expected& operator=(expected&&) noexcept(see below);
    template<class U = remove_cv_t<T>> constexpr expected& operator=(U&&);
    template<class G>
      constexpr expected& operator=(const unexpected<G>&);
    template<class G>
      constexpr expected& operator=(unexpected<G>&&);

    template<class... Args>
      constexpr T& emplace(Args&&...) noexcept;
    template<class U, class... Args>
      constexpr T& emplace(initializer_list<U>, Args&&...) noexcept;

    // [expected.object.swap], swap
    constexpr void swap(expected&) noexcept(see below);
    friend constexpr void swap(expected& x, expected& y) noexcept(noexcept(x.swap(y)));

    // [expected.object.obs], observers
    constexpr const T* operator->() const noexcept;
    constexpr T* operator->() noexcept;
    constexpr const T& operator*() const & noexcept;
    constexpr T& operator*() & noexcept;
    constexpr const T&& operator*() const && noexcept;
    constexpr T&& operator*() && noexcept;
    constexpr explicit operator bool() const noexcept;
    constexpr bool has_value() const noexcept;
    constexpr const T& value() const &;                                     // freestanding-deleted
    constexpr T& value() &;                                                 // freestanding-deleted
    constexpr const T&& value() const &&;                                   // freestanding-deleted
    constexpr T&& value() &&;                                               // freestanding-deleted
    constexpr const E& error() const & noexcept;
    constexpr E& error() & noexcept;
    constexpr const E&& error() const && noexcept;
    constexpr E&& error() && noexcept;
    template<class U = remove_cv_t<T>> constexpr T value_or(U&&) const &;
    template<class U = remove_cv_t<T>> constexpr T value_or(U&&) &&;
    template<class G = E> constexpr E error_or(G&&) const &;
    template<class G = E> constexpr E error_or(G&&) &&;

    // [expected.object.monadic], monadic operations
    template<class F> constexpr auto and_then(F&& f) &;
    template<class F> constexpr auto and_then(F&& f) &&;
    template<class F> constexpr auto and_then(F&& f) const &;
    template<class F> constexpr auto and_then(F&& f) const &&;
    template<class F> constexpr auto or_else(F&& f) &;
    template<class F> constexpr auto or_else(F&& f) &&;
    template<class F> constexpr auto or_else(F&& f) const &;
    template<class F> constexpr auto or_else(F&& f) const &&;
    template<class F> constexpr auto transform(F&& f) &;
    template<class F> constexpr auto transform(F&& f) &&;
    template<class F> constexpr auto transform(F&& f) const &;
    template<class F> constexpr auto transform(F&& f) const &&;
    template<class F> constexpr auto transform_error(F&& f) &;
    template<class F> constexpr auto transform_error(F&& f) &&;
    template<class F> constexpr auto transform_error(F&& f) const &;
    template<class F> constexpr auto transform_error(F&& f) const &&;

    // [expected.object.eq], equality operators
    template<class T2, class E2> requires (!is_void_v<T2>)
      friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y);
    template<class T2>
      friend constexpr bool operator==(const expected&, const T2&);
    template<class E2>
      friend constexpr bool operator==(const expected&, const unexpected<E2>&);

  private:
    bool has_val;       // exposition only
    union {
      T val;            // exposition only
      E unex;           // exposition only
    };
  };
}
*/

/***
22.8.7 Partial specialization of expected for void types[expected.void]
22.8.7.1 General[expected.void.general]
template<class T, class E> requires is_void_v<T>
class expected<T, E> {
public:
  using value_type = T;
  using error_type = E;
  using unexpected_type = unexpected<E>;

  template<class U>
  using rebind = expected<U, error_type>;

  // [expected.void.cons], constructors
  constexpr expected() noexcept;
  constexpr expected(const expected&);
  constexpr expected(expected&&) noexcept(see below);
  template<class U, class G>
    constexpr explicit(see below) expected(const expected<U, G>&);
  template<class U, class G>
    constexpr explicit(see below) expected(expected<U, G>&&);

  template<class G>
    constexpr explicit(see below) expected(const unexpected<G>&);
  template<class G>
    constexpr explicit(see below) expected(unexpected<G>&&);

  constexpr explicit expected(in_place_t) noexcept;
  template<class... Args>
    constexpr explicit expected(unexpect_t, Args&&...);
  template<class U, class... Args>
    constexpr explicit expected(unexpect_t, initializer_list<U>, Args&&...);


  // [expected.void.dtor], destructor
  constexpr ~expected();

  // [expected.void.assign], assignment
  constexpr expected& operator=(const expected&);
  constexpr expected& operator=(expected&&) noexcept(see below);
  template<class G>
    constexpr expected& operator=(const unexpected<G>&);
  template<class G>
    constexpr expected& operator=(unexpected<G>&&);
  constexpr void emplace() noexcept;

  // [expected.void.swap], swap
  constexpr void swap(expected&) noexcept(see below);
  friend constexpr void swap(expected& x, expected& y) noexcept(noexcept(x.swap(y)));

  // [expected.void.obs], observers
  constexpr explicit operator bool() const noexcept;
  constexpr bool has_value() const noexcept;
  constexpr void operator*() const noexcept;
  constexpr void value() const &;                                           // freestanding-deleted
  constexpr void value() &&;                                                // freestanding-deleted
  constexpr const E& error() const & noexcept;
  constexpr E& error() & noexcept;
  constexpr const E&& error() const && noexcept;
  constexpr E&& error() && noexcept;
  template<class G = E> constexpr E error_or(G&&) const &;
  template<class G = E> constexpr E error_or(G&&) &&;

  // [expected.void.monadic], monadic operations
  template<class F> constexpr auto and_then(F&& f) &;
  template<class F> constexpr auto and_then(F&& f) &&;
  template<class F> constexpr auto and_then(F&& f) const &;
  template<class F> constexpr auto and_then(F&& f) const &&;
  template<class F> constexpr auto or_else(F&& f) &;
  template<class F> constexpr auto or_else(F&& f) &&;
  template<class F> constexpr auto or_else(F&& f) const &;
  template<class F> constexpr auto or_else(F&& f) const &&;
  template<class F> constexpr auto transform(F&& f) &;
  template<class F> constexpr auto transform(F&& f) &&;
  template<class F> constexpr auto transform(F&& f) const &;
  template<class F> constexpr auto transform(F&& f) const &&;
  template<class F> constexpr auto transform_error(F&& f) &;
  template<class F> constexpr auto transform_error(F&& f) &&;
  template<class F> constexpr auto transform_error(F&& f) const &;
  template<class F> constexpr auto transform_error(F&& f) const &&;

  // [expected.void.eq], equality operators
  template<class T2, class E2> requires is_void_v<T2>
    friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y);
  template<class E2>
    friend constexpr bool operator==(const expected&, const unexpected<E2>&);

private:
  bool has_val;         // exposition only
  union {
    E unex;             // exposition only
  };
};
*/

namespace beman {
namespace expected {}
} // namespace beman

#endif
