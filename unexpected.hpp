// beman/expected/unexpected.hpp                                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef BEMAN_EXPECTED_UNEXPECTED_HPP
#define BEMAN_EXPECTED_UNEXPECTED_HPP

#ifndef BEMAN_EXPECTED_INCLUDED_FROM_INTERFACE_UNIT
    #include <initializer_list>
    #include <type_traits>
    #include <utility>
#endif

namespace beman {
namespace expected {

// [expected.unexpect]
struct unexpect_t {
    explicit unexpect_t() = default;
};
inline constexpr unexpect_t unexpect{};

// Forward declaration for is_unexpected_specialization trait
template <class E>
class unexpected;

namespace detail {
template <class T>
struct is_unexpected_specialization : std::false_type {};
template <class E>
struct is_unexpected_specialization<unexpected<E>> : std::true_type {};
} // namespace detail

// [expected.unexpected]
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

    template <class Err = E>
        requires(!std::is_same_v<std::remove_cvref_t<Err>, unexpected> &&
                 !std::is_same_v<std::remove_cvref_t<Err>, std::in_place_t> && std::is_constructible_v<E, Err>)
    constexpr explicit unexpected(Err&& e) noexcept(std::is_nothrow_constructible_v<E, Err>)
        : unex_(std::forward<Err>(e)) {}

    template <class... Args>
        requires std::is_constructible_v<E, Args...>
    constexpr explicit unexpected(std::in_place_t,
                                  Args&&... args) noexcept(std::is_nothrow_constructible_v<E, Args...>)
        : unex_(std::forward<Args>(args)...) {}

    template <class U, class... Args>
        requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
    constexpr explicit unexpected(std::in_place_t, std::initializer_list<U> il, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<E, std::initializer_list<U>&, Args...>)
        : unex_(il, std::forward<Args>(args)...) {}

    constexpr unexpected& operator=(const unexpected&) = default;
    constexpr unexpected& operator=(unexpected&&)      = default;

    constexpr const E&  error() const& noexcept { return unex_; }
    constexpr E&        error() & noexcept { return unex_; }
    constexpr const E&& error() const&& noexcept { return std::move(unex_); }
    constexpr E&&       error() && noexcept { return std::move(unex_); }

    constexpr void swap(unexpected& other) noexcept(std::is_nothrow_swappable_v<E>) {
        using std::swap;
        swap(unex_, other.unex_);
    }

    template <class E2>
    friend constexpr bool operator==(const unexpected& x, const unexpected<E2>& y) {
        return x.unex_ == y.error();
    }

    friend constexpr void swap(unexpected& x, unexpected& y) noexcept(noexcept(x.swap(y)))
        requires std::is_swappable_v<E>
    {
        x.swap(y);
    }

  private:
    E unex_;
};

template <class E>
unexpected(E) -> unexpected<E>;

} // namespace expected
} // namespace beman

#endif
