// beman/expected/bad_expected_access.hpp                             -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef BEMAN_EXPECTED_BAD_EXPECTED_ACCESS_HPP
#define BEMAN_EXPECTED_BAD_EXPECTED_ACCESS_HPP

#include <exception>
#include <utility>
#include <version>

#ifdef __cpp_lib_constexpr_exceptions
    #define BEMAN_EXPECTED_CONSTEXPR_EXCEPTION constexpr
#else
    #define BEMAN_EXPECTED_CONSTEXPR_EXCEPTION
#endif

/***
22.8.4 Class template bad_expected_access[expected.bad]

namespace std {
  template<class E>
  class bad_expected_access : public bad_expected_access<void> {
  public:
    constexpr explicit bad_expected_access(E);
    constexpr const char* what() const noexcept override;
    constexpr E& error() & noexcept;
    constexpr const E& error() const & noexcept;
    constexpr E&& error() && noexcept;
    constexpr const E&& error() const && noexcept;

  private:
    E unex;             // exposition only
  };
}
 */

/***
22.8.5 Class template specialization bad_expected_access<void>[expected.bad.void]
namespace std {
  template<>
  class bad_expected_access<void> : public exception {
  protected:
    constexpr bad_expected_access() noexcept;
    constexpr bad_expected_access(const bad_expected_access&) noexcept;
    constexpr bad_expected_access(bad_expected_access&&) noexcept;
    constexpr bad_expected_access& operator=(const bad_expected_access&) noexcept;
    constexpr bad_expected_access& operator=(bad_expected_access&&) noexcept;
    constexpr ~bad_expected_access();

  public:
    constexpr const char* what() const noexcept override;
  };
}
 */

namespace beman {
namespace expected {

template <class E>
class bad_expected_access;

template <>
class bad_expected_access<void> : public std::exception {
  protected:
    BEMAN_EXPECTED_CONSTEXPR_EXCEPTION bad_expected_access() noexcept                                      = default;
    BEMAN_EXPECTED_CONSTEXPR_EXCEPTION bad_expected_access(const bad_expected_access&) noexcept            = default;
    BEMAN_EXPECTED_CONSTEXPR_EXCEPTION bad_expected_access(bad_expected_access&&) noexcept                 = default;
    BEMAN_EXPECTED_CONSTEXPR_EXCEPTION bad_expected_access& operator=(const bad_expected_access&) noexcept = default;
    BEMAN_EXPECTED_CONSTEXPR_EXCEPTION bad_expected_access& operator=(bad_expected_access&&) noexcept      = default;
    BEMAN_EXPECTED_CONSTEXPR_EXCEPTION ~bad_expected_access()                                              = default;

  public:
    BEMAN_EXPECTED_CONSTEXPR_EXCEPTION const char* what() const noexcept override;
};

template <class E>
class bad_expected_access : public bad_expected_access<void> {
  public:
    BEMAN_EXPECTED_CONSTEXPR_EXCEPTION explicit bad_expected_access(E e);
    BEMAN_EXPECTED_CONSTEXPR_EXCEPTION const char* what() const noexcept override;
    constexpr E&                                   error() & noexcept;
    constexpr const E&                             error() const& noexcept;
    constexpr E&&                                  error() && noexcept;
    constexpr const E&&                            error() const&& noexcept;

  private:
    E unex;
};

// bad_expected_access<void> out-of-line definitions

inline BEMAN_EXPECTED_CONSTEXPR_EXCEPTION const char* bad_expected_access<void>::what() const noexcept {
    return "bad expected access";
}

// bad_expected_access<E> out-of-line definitions

template <class E>
BEMAN_EXPECTED_CONSTEXPR_EXCEPTION bad_expected_access<E>::bad_expected_access(E e) : unex(std::move(e)) {}

template <class E>
BEMAN_EXPECTED_CONSTEXPR_EXCEPTION const char* bad_expected_access<E>::what() const noexcept {
    return "bad expected access";
}

template <class E>
constexpr E& bad_expected_access<E>::error() & noexcept {
    return unex;
}

template <class E>
constexpr const E& bad_expected_access<E>::error() const& noexcept {
    return unex;
}

template <class E>
constexpr E&& bad_expected_access<E>::error() && noexcept {
    return std::move(unex);
}

template <class E>
constexpr const E&& bad_expected_access<E>::error() const&& noexcept {
    return std::move(unex);
}

} // namespace expected
} // namespace beman

#undef BEMAN_EXPECTED_CONSTEXPR_EXCEPTION

#endif
