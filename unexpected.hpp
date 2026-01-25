// beman/expected/unexpected.hpp                                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef BEMAN_EXPECTED_UNEXPECTED_HPP
#define BEMAN_EXPECTED_UNEXPECTED_HPP

/***
22.8.3 Class template unexpected[expected.unexpected]
22.8.3.1 General[expected.un.general]
1
#
Subclause [expected.unexpected] describes the class template unexpected that represents unexpected objects stored in
expected objects.

namespace std {
  template<class E>
  class unexpected {
  public:
    // [expected.un.cons], constructors
    constexpr unexpected(const unexpected&) = default;
    constexpr unexpected(unexpected&&) = default;
    template<class Err = E>
      constexpr explicit unexpected(Err&&);
    template<class... Args>
      constexpr explicit unexpected(in_place_t, Args&&...);
    template<class U, class... Args>
      constexpr explicit unexpected(in_place_t, initializer_list<U>, Args&&...);

    constexpr unexpected& operator=(const unexpected&) = default;
    constexpr unexpected& operator=(unexpected&&) = default;

    constexpr const E& error() const & noexcept;
    constexpr E& error() & noexcept;
    constexpr const E&& error() const && noexcept;
    constexpr E&& error() && noexcept;

    constexpr void swap(unexpected& other) noexcept(see below);

    template<class E2>
      friend constexpr bool operator==(const unexpected&, const unexpected<E2>&);

    friend constexpr void swap(unexpected& x, unexpected& y) noexcept(noexcept(x.swap(y)));

  private:
    E unex;             // exposition only
  };

  template<class E> unexpected(E) -> unexpected<E>;
}
*/

namespace beman {
namespace expected {}
} // namespace beman

#endif
