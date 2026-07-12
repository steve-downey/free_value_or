// tests/beman/expected/expected_std_equivalence.test.cpp             -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Observable-equivalence gate: beman::expected holds the exposition-only member
// unexpected<E>, but every observable special-member property is keyed on E.
// This asserts, across a battery of adversarial error types, that the resulting
// type traits match std::expected exactly -- i.e. that the unexpected<E> member
// is behaviourally inert against a T/E rendering. See D4280: ABI stability for
// existing implementations is a design goal, and any divergence found here is a
// defect whose resolution is fixed in advance (behave like the T/E rendering).
//
// Exceptions to strict equality: beman::expected is deliberately trivially
// copyable/assignable when T and E are (a drive-by fix; std::expected's
// non-trivial assignment is a specification accident). Those traits are checked
// as "beman is at least as trivial as std".
//
// Compiled against std::expected requires C++23. Skipped on libc++, whose
// std::expected has its own quirks for pathological types -- see the std-parity
// gate in CMakeLists.txt and docs/std-parity.md.

#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <string>
#include <type_traits>

namespace bx = beman::expected;

namespace {

struct Plain {
    int x;
};
struct DelMoveOkCopy {
    DelMoveOkCopy()                                = default;
    DelMoveOkCopy(const DelMoveOkCopy&) noexcept   = default;
    DelMoveOkCopy(DelMoveOkCopy&&)                 = delete;
    DelMoveOkCopy& operator=(const DelMoveOkCopy&) = default;
    DelMoveOkCopy& operator=(DelMoveOkCopy&&)      = delete;
};
struct MoveOnly {
    MoveOnly()                           = default;
    MoveOnly(const MoveOnly&)            = delete;
    MoveOnly(MoveOnly&&)                 = default;
    MoveOnly& operator=(const MoveOnly&) = delete;
    MoveOnly& operator=(MoveOnly&&)      = default;
};
struct ThrowMoveNoexceptCopy {
    int x = 0;
    ThrowMoveNoexceptCopy()                                        = default;
    ThrowMoveNoexceptCopy(const ThrowMoveNoexceptCopy&) noexcept   = default;
    ThrowMoveNoexceptCopy(ThrowMoveNoexceptCopy&&) {}
    ThrowMoveNoexceptCopy& operator=(const ThrowMoveNoexceptCopy&) noexcept = default;
    ThrowMoveNoexceptCopy& operator=(ThrowMoveNoexceptCopy&&) { return *this; }
};
struct NonTrivialDtor {
    int x = 0;
    ~NonTrivialDtor() {}
};
struct NoexceptMoveOnly {
    NoexceptMoveOnly()                                    = default;
    NoexceptMoveOnly(const NoexceptMoveOnly&)             = delete;
    NoexceptMoveOnly(NoexceptMoveOnly&&) noexcept         = default;
    NoexceptMoveOnly& operator=(const NoexceptMoveOnly&)  = delete;
    NoexceptMoveOnly& operator=(NoexceptMoveOnly&&) noexcept = default;
};
struct Immovable {
    Immovable()                            = default;
    Immovable(const Immovable&)            = delete;
    Immovable(Immovable&&)                 = delete;
    Immovable& operator=(const Immovable&) = delete;
    Immovable& operator=(Immovable&&)      = delete;
};
struct NoexceptNonTrivial {
    NoexceptNonTrivial()                                     = default;
    NoexceptNonTrivial(const NoexceptNonTrivial&) noexcept {}
    NoexceptNonTrivial(NoexceptNonTrivial&&) noexcept {}
    NoexceptNonTrivial& operator=(const NoexceptNonTrivial&) noexcept { return *this; }
    NoexceptNonTrivial& operator=(NoexceptNonTrivial&&) noexcept { return *this; }
};

template <class E, class T>
constexpr bool parity() {
    using Bm = bx::expected<T, E>;
    using St = std::expected<T, E>;
#define BEMAN_PARITY(TRAIT) static_assert(std::TRAIT##_v<Bm> == std::TRAIT##_v<St>, #TRAIT " parity")
    BEMAN_PARITY(is_copy_constructible);
    BEMAN_PARITY(is_move_constructible);
    BEMAN_PARITY(is_nothrow_copy_constructible);
    BEMAN_PARITY(is_nothrow_move_constructible);
    BEMAN_PARITY(is_trivially_copy_constructible);
    BEMAN_PARITY(is_trivially_move_constructible);
    BEMAN_PARITY(is_copy_assignable);
    BEMAN_PARITY(is_move_assignable);
    BEMAN_PARITY(is_nothrow_copy_assignable);
    BEMAN_PARITY(is_nothrow_move_assignable);
    BEMAN_PARITY(is_destructible);
    BEMAN_PARITY(is_nothrow_destructible);
    BEMAN_PARITY(is_trivially_destructible);
    BEMAN_PARITY(is_default_constructible);
    BEMAN_PARITY(is_swappable);
    BEMAN_PARITY(is_nothrow_swappable);
#undef BEMAN_PARITY
    // Drive-by fix: beman is at least as trivially copyable/assignable as std.
    static_assert(!std::is_trivially_copy_assignable_v<St> || std::is_trivially_copy_assignable_v<Bm>);
    static_assert(!std::is_trivially_move_assignable_v<St> || std::is_trivially_move_assignable_v<Bm>);
    static_assert(!std::is_trivially_copyable_v<St> || std::is_trivially_copyable_v<Bm>);
    return true;
}

#define BEMAN_RUN(E) static_assert(parity<E, int>() && parity<E, void>())
BEMAN_RUN(Plain);
BEMAN_RUN(DelMoveOkCopy);
BEMAN_RUN(MoveOnly);
BEMAN_RUN(ThrowMoveNoexceptCopy);
BEMAN_RUN(NonTrivialDtor);
BEMAN_RUN(NoexceptMoveOnly);
BEMAN_RUN(Immovable);
BEMAN_RUN(NoexceptNonTrivial);
BEMAN_RUN(int);
BEMAN_RUN(std::string);
#undef BEMAN_RUN

// Lock in the drive-by fix itself: beman is trivially copyable for trivial T,E
// (std::expected is not, by specification accident).
static_assert(std::is_trivially_copyable_v<bx::expected<int, int>>);

} // namespace

TEST_CASE("beman::expected type traits match std::expected") {
    SUCCEED("all parity checks are static_asserts evaluated at compile time");
}
