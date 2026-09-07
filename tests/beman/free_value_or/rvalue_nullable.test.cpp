// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Include-guard idempotency check
#include <beman/free_value_or/value_or.hpp>
#include <beman/free_value_or/value_or.hpp>

#include "test_types.hpp"

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

// ==========================================================================
// Rvalue nullables: the functions observe, they do not consume.
//
// iter_reference_t<T> is decltype(*declval<T&>()) -- an LVALUE dereference,
// whatever the value category of the argument.  So an rvalue nullable does
// not move its payload out: the engaged path runs static_cast<R>(*m) on an
// lvalue and copies, where the member std::move(o).value_or(u) moves.
//
// This is a design decision the paper puts to LEWG, not an accident.  The
// alternative -- computing the dereference type as decltype(*forward<T>(m))
// -- recovers the move and makes the return type depend on the value
// category of the nullable.  These tests pin the behavior either way: if the
// decision reverses, they are the ones that must be updated, deliberately.
//
// The discriminator is the COPY count on the engaged path.  Under the
// current rule the engaged path copies; under the alternative it moves.  A
// test that only checks the resulting value passes under both, which is why
// the pre-existing rvalue coverage did not pin anything.
//
// The smart-pointer cases below are the same shape as the optional and
// expected ones even though they do NOT change under the alternative, and
// the difference is ownership rather than an accident of specification.
// optional and expected contain their value, so an expiring one has an
// expiring payload and operator* is ref-qualified to say so.  Pointers
// contain nothing: an rvalue shared_ptr, unique_ptr or T* is an expiring
// handle to a referent that ordinarily outlives it, so operator* returns T&.
// optional<T&> settles which property is doing the work -- it is an
// optional, and it dereferences to T&, because it does not own its referent.
// Under the alternative every model would still be right; the copy counts
// here make that division visible instead of leaving it implicit.
// ==========================================================================

namespace {

struct Counted {
    inline static int copies = 0;
    inline static int moves  = 0;

    int v = 0;

    Counted() = default;
    explicit Counted(int x) : v(x) {}

    Counted(const Counted& o) : v(o.v) { ++copies; }
    Counted(Counted&& o) noexcept : v(o.v) { ++moves; }

    Counted& operator=(const Counted& o) {
        v = o.v;
        ++copies;
        return *this;
    }
    Counted& operator=(Counted&& o) noexcept {
        v = o.v;
        ++moves;
        return *this;
    }

    static void reset() {
        copies = 0;
        moves  = 0;
    }
};

} // namespace

// ==========================================================================
// Return types do not depend on the value category of the nullable.
//
// common_type decays, so common_type_t<int&, int&&> and
// common_type_t<int&&, int&&> are both int -- the value-producing functions
// have identical signatures for lvalue and rvalue nullables.  reference_or
// is the one that would change: common_reference_t<int&, int&> is int&,
// while common_reference_t<int&&, int&> is const int&.
// ==========================================================================

static_assert(std::is_same_v<decltype(fvo::value_or(std::declval<std::optional<int>&>(), std::declval<int>())),
                             decltype(fvo::value_or(std::declval<std::optional<int>>(), std::declval<int>()))>);

static_assert(std::is_same_v<decltype(fvo::or_invoke(std::declval<std::optional<int>&>(), std::declval<int (*)()>())),
                             decltype(fvo::or_invoke(std::declval<std::optional<int>>(), std::declval<int (*)()>()))>);

static_assert(std::is_same_v<decltype(fvo::or_construct(std::declval<std::optional<int>&>())),
                             decltype(fvo::or_construct(std::declval<std::optional<int>>()))>);

// reference_or over an rvalue nullable stays a non-const lvalue reference.
// Under the moving alternative this becomes const int&.
static_assert(
    std::is_same_v<decltype(fvo::reference_or(std::declval<std::optional<int>>(), std::declval<int&>())), int&>);

static_assert(
    std::is_same_v<decltype(fvo::reference_or(std::declval<std::optional<int>&>(), std::declval<int&>())), int&>);

// ==========================================================================
// Bootstrap
// ==========================================================================

TEST_CASE("rvalue_nullable bootstrap", "[rvalue]") { CHECK(true); }

// ==========================================================================
// value_or
// ==========================================================================

TEST_CASE("value_or on an engaged rvalue optional copies, does not move", "[rvalue][value_or]") {
    std::optional<Counted> o{Counted{7}};
    Counted::reset();

    auto r = fvo::value_or(std::move(o), Counted{0});

    CHECK(r.v == 7);
    CHECK(Counted::copies == 1);
    CHECK(Counted::moves == 0);
    // the payload is still there: nothing was consumed
    CHECK(o.has_value());
    CHECK(o->v == 7);
}

TEST_CASE("member value_or on an rvalue optional moves -- the contrast", "[rvalue][value_or]") {
    std::optional<Counted> o{Counted{7}};
    Counted::reset();

    auto r = std::move(o).value_or(Counted{0});

    CHECK(r.v == 7);
    CHECK(Counted::copies == 0);
    CHECK(Counted::moves == 1);
}

TEST_CASE("value_or on an engaged rvalue expected copies", "[rvalue][value_or]") {
#if FVO_HAS_STD_EXPECTED
    std::expected<Counted, int> e{Counted{7}};
    Counted::reset();

    auto r = fvo::value_or(std::move(e), Counted{0});

    CHECK(r.v == 7);
    CHECK(Counted::copies == 1);
    CHECK(Counted::moves == 0);
#else
    SUCCEED("std::expected unavailable on this toolchain");
#endif
}

TEST_CASE("value_or on engaged rvalue smart pointers copies the payload", "[rvalue][value_or]") {
    SECTION("shared_ptr") {
        auto p = std::make_shared<Counted>(7);
        Counted::reset();

        auto r = fvo::value_or(std::move(p), Counted{0});

        CHECK(r.v == 7);
        CHECK(Counted::copies == 1);
        CHECK(Counted::moves == 0);
    }

    SECTION("unique_ptr") {
        auto p = std::make_unique<Counted>(7);
        Counted::reset();

        auto r = fvo::value_or(std::move(p), Counted{0});

        CHECK(r.v == 7);
        CHECK(Counted::copies == 1);
        CHECK(Counted::moves == 0);
    }
}

TEST_CASE("value_or on a disengaged rvalue nullable takes the fallback", "[rvalue][value_or]") {
    std::optional<Counted> o;
    Counted::reset();

    auto r = fvo::value_or(std::move(o), Counted{3});

    CHECK(r.v == 3);
    // the fallback is an rvalue and is moved, on both sides of the question
    CHECK(Counted::copies == 0);
    CHECK(Counted::moves == 1);
}

// ==========================================================================
// or_invoke
// ==========================================================================

TEST_CASE("or_invoke on an engaged rvalue nullable copies, does not move", "[rvalue][or_invoke]") {
    std::optional<Counted> o{Counted{7}};
    Counted::reset();

    auto r = fvo::or_invoke(std::move(o), [] { return Counted{0}; });

    CHECK(r.v == 7);
    CHECK(Counted::copies == 1);
    CHECK(Counted::moves == 0);
    CHECK(o.has_value());
}

// ==========================================================================
// or_construct
// ==========================================================================

TEST_CASE("or_construct on an engaged rvalue nullable copies, does not move", "[rvalue][or_construct]") {
    std::optional<Counted> o{Counted{7}};
    Counted::reset();

    auto r = fvo::or_construct(std::move(o), 0);

    CHECK(r.v == 7);
    CHECK(Counted::copies == 1);
    CHECK(Counted::moves == 0);
    CHECK(o.has_value());
}

TEST_CASE("or_construct initializer_list overload, engaged rvalue nullable", "[rvalue][or_construct]") {
    struct FromList {
        int n = 0;
        FromList(std::initializer_list<int> il, int extra) : n(static_cast<int>(il.size()) + extra) {}
    };

    std::optional<FromList> o{FromList{{1, 2, 3}, 0}};

    auto r = fvo::or_construct(std::move(o), {1, 2}, 10);

    // engaged: the payload comes through, the list is never used
    CHECK(r.n == 3);
    CHECK(o.has_value());
}

TEST_CASE("or_invoke on a disengaged rvalue nullable calls the invocable", "[rvalue][or_invoke]") {
    std::optional<Counted> o;
    int                    calls = 0;
    Counted::reset();

    auto r = fvo::or_invoke(std::move(o), [&] {
        ++calls;
        return Counted{3};
    });

    CHECK(r.v == 3);
    CHECK(calls == 1);
}

TEST_CASE("or_construct on a disengaged rvalue nullable builds the fallback", "[rvalue][or_construct]") {
    std::optional<Counted> o;
    Counted::reset();

    auto r = fvo::or_construct(std::move(o), 3);

    CHECK(r.v == 3);
    // built in place from the arguments: neither copied nor moved
    CHECK(Counted::copies == 0);
    CHECK(Counted::moves == 0);
}

TEST_CASE("or_construct initializer_list overload, disengaged rvalue nullable", "[rvalue][or_construct]") {
    struct FromList {
        int n = 0;
        FromList(std::initializer_list<int> il, int extra) : n(static_cast<int>(il.size()) + extra) {}
    };

    std::optional<FromList> o;

    auto r = fvo::or_construct(std::move(o), {1, 2}, 10);

    CHECK(r.n == 12);
}

// ==========================================================================
// reference_or
// ==========================================================================

TEST_CASE("reference_or on a disengaged rvalue nullable refers to the fallback", "[rvalue][reference_or]") {
    std::optional<Counted> o;
    Counted                fallback{3};
    Counted::reset();

    decltype(auto) r = fvo::reference_or(std::move(o), fallback);

    STATIC_REQUIRE(std::is_same_v<decltype(r), Counted&>);
    CHECK(Counted::copies == 0);
    CHECK(Counted::moves == 0);
    CHECK(&r == &fallback);
}

TEST_CASE("reference_or on an rvalue nullable still refers, and does not copy", "[rvalue][reference_or]") {
    std::optional<Counted> o{Counted{7}};
    Counted                fallback{3};
    Counted::reset();

    decltype(auto) r = fvo::reference_or(std::move(o), fallback);

    STATIC_REQUIRE(std::is_same_v<decltype(r), Counted&>);
    CHECK(Counted::copies == 0);
    CHECK(Counted::moves == 0);

    // it refers to the optional's payload, not to a copy of it
    CHECK(&r == &*o);

    r.v = 11;
    CHECK(o->v == 11);
}
