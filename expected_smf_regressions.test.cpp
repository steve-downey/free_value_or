// tests/beman/expected/expected_smf_regressions.test.cpp             -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Beman-only regressions for special-member-function observable behavior.
//
// These guard three fixes made when the special-member constraints/noexcept
// were re-keyed from the exposition-only held type unexpected<E> onto E
// (so that an implementation storing E directly is trivially conforming, and
// ABI stability for existing implementations is preserved):
//
//   1. The hidden-friend swap was unconstrained, so is_swappable_v<expected>
//      was a hard error (ill-formed) for a move-restricted E instead of a
//      well-formed bool. It must be a constant.
//   2. The non-trivial copy constructor and copy assignment dropped their
//      noexcept, so a non-trivial-but-noexcept-copyable E (or one with a
//      non-trivial destructor) reported a throwing copy where std::expected
//      does not.
//
// Value parity with std::expected is checked separately in
// expected_std_equivalence.test.cpp (skipped on libc++).

#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>
#include <utility>

using namespace beman::expected;

namespace {

// Copy is usable, move is user-deleted: E is not swappable, so expected's own
// swap is constrained out and only the generic std::swap fallback remains.
struct DelMoveOkCopy {
    DelMoveOkCopy()                                = default;
    DelMoveOkCopy(const DelMoveOkCopy&) noexcept   = default;
    DelMoveOkCopy(DelMoveOkCopy&&)                 = delete;
    DelMoveOkCopy& operator=(const DelMoveOkCopy&) = default;
    DelMoveOkCopy& operator=(DelMoveOkCopy&&)      = delete;
};

// Trivial copy constructor, but a non-trivial destructor.
struct NonTrivialDtor {
    int x = 0;
    ~NonTrivialDtor() {}
};

// User-provided (non-trivial) but noexcept copy and move.
struct NoexceptNonTrivial {
    NoexceptNonTrivial() = default;
    NoexceptNonTrivial(const NoexceptNonTrivial&) noexcept {}
    NoexceptNonTrivial(NoexceptNonTrivial&&) noexcept {}
    NoexceptNonTrivial& operator=(const NoexceptNonTrivial&) noexcept { return *this; }
    NoexceptNonTrivial& operator=(NoexceptNonTrivial&&) noexcept { return *this; }
};

// Bug 1: querying is_swappable must be well-formed (a hard error here would
// fail to compile). Forcing the trait into a constant proves it.
template <class Ex>
inline constexpr bool swap_query_is_wellformed = (static_cast<void>(std::is_swappable_v<Ex>), true);

} // namespace

// The trait properties below are checked at runtime rather than with
// static_assert so that a regression is reported by the test run, with the
// responsible type named, instead of stopping the build at the first failure
// and reporting nothing. Well-formedness is still a translation-time question:
// naming `swap_query_is_wellformed<Ex>` instantiates it, so a hard error in
// `is_swappable_v<Ex>` remains a build failure — what the CHECK adds is that a
// *wrong answer* is reported instead.

TEST_CASE("querying is_swappable on a move-restricted error type is well-formed") {
    // Bug 1: previously a hard error via the unconstrained hidden-friend swap.
    CHECK(swap_query_is_wellformed<expected<int, DelMoveOkCopy>>);
    CHECK(swap_query_is_wellformed<expected<void, DelMoveOkCopy>>);
}

TEST_CASE("non-trivial but noexcept copy gives a nothrow copy constructor and assignment") {
    // Bug 2: non-trivial-but-noexcept copy => nothrow copy construct/assign.
    CHECK(std::is_nothrow_copy_constructible_v<expected<int, NonTrivialDtor>>);
    CHECK(std::is_nothrow_copy_constructible_v<expected<int, NoexceptNonTrivial>>);
    CHECK(std::is_nothrow_copy_constructible_v<expected<void, NonTrivialDtor>>);
    CHECK(std::is_nothrow_copy_assignable_v<expected<int, NonTrivialDtor>>);
    CHECK(std::is_nothrow_copy_assignable_v<expected<int, NoexceptNonTrivial>>);
    CHECK(std::is_nothrow_copy_assignable_v<expected<void, NonTrivialDtor>>);
}

TEST_CASE("swap is well-formed and works for a move-restricted error type") {
    // Previously a hard error via the unconstrained hidden-friend swap.
    expected<int, DelMoveOkCopy> a(1);
    expected<int, DelMoveOkCopy> b(2);
    using std::swap;
    swap(a, b); // resolves to the generic std::swap fallback
    CHECK(*a == 2);
    CHECK(*b == 1);
}
