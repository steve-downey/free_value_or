// beman/expected/bad_expected_access.hpp                             -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef BEMAN_EXPECTED_BAD_EXPECTED_ACCESS
#define BEMAN_EXPECTED_BAD_EXPECTED_ACCESS

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
pcc*/
namespace beman {
namespace expected {}
} // namespace beman

#endif
