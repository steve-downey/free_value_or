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
// Rvalue nullables: the value category of the nullable reaches its payload.
//
// deref_t<T> is decltype(*declval<T>()), so an expiring nullable dereferences
// to an expiring payload and the engaged path moves rather than copies.
// value_or(std::move(o), u) matches std::move(o).value_or(u).
//
// The models divide by OWNERSHIP, and each side is right:
//
//   optional<T>, expected<T,E>   own the value.  An rvalue of one is expiring
//                                and so is its payload -- operator* is
//                                ref-qualified, deref_t is T&&, and the
//                                engaged path MOVES.
//
//   T*, shared_ptr<T>,           are handles to a referent that ordinarily
//   unique_ptr<T>, optional<T&>  outlives them.  An rvalue handle says nothing
//                                about the referent -- operator* gives T&,
//                                deref_t is T&, and the engaged path COPIES.
//
// optional<T&> is the case that shows the split is ownership and not
// optional-ness: it is an optional, and it copies, because it does not own.
//
// So the smart-pointer cases below assert COPIES while the optional and
// expected cases assert MOVES.  That asymmetry is the specification, not an
// oversight, and these tests are what would catch it being flattened in
// either direction.
//
// The discriminator is the copy/move count on the engaged path.  A test that
// only checks the resulting value passes under both rules, which is why the
// coverage that existed before pinned nothing.
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

// reference_or over an OWNING rvalue nullable: deref_t is int&&, and the
// common reference of int&& and int& is const int&.
static_assert(
    std::is_same_v<decltype(fvo::reference_or(std::declval<std::optional<int>>(), std::declval<int&>())), const int&>);

// over an lvalue nullable, and over an rvalue HANDLE, it stays int&
static_assert(
    std::is_same_v<decltype(fvo::reference_or(std::declval<std::optional<int>&>(), std::declval<int&>())), int&>);

static_assert(
    std::is_same_v<decltype(fvo::reference_or(std::declval<std::shared_ptr<int>>(), std::declval<int&>())), int&>);

// ==========================================================================
// Bootstrap
// ==========================================================================

TEST_CASE("rvalue_nullable bootstrap", "[rvalue]") { CHECK(true); }

// ==========================================================================
// value_or
// ==========================================================================

TEST_CASE("value_or on an engaged rvalue optional moves the payload", "[rvalue][value_or]") {
    std::optional<Counted> o{Counted{7}};
    Counted::reset();

    auto r = fvo::value_or(std::move(o), Counted{0});

    CHECK(r.v == 7);
    CHECK(Counted::copies == 0);
    CHECK(Counted::moves == 1);
    // the optional is still engaged; its payload is moved-from, not destroyed
    CHECK(o.has_value());
}

TEST_CASE("free and member value_or agree on an rvalue optional", "[rvalue][value_or]") {
    std::optional<Counted> o{Counted{7}};
    Counted::reset();

    auto r = std::move(o).value_or(Counted{0});

    CHECK(r.v == 7);
    CHECK(Counted::copies == 0);
    CHECK(Counted::moves == 1);
}

TEST_CASE("value_or on an engaged rvalue expected moves the payload", "[rvalue][value_or]") {
#if FVO_HAS_STD_EXPECTED
    std::expected<Counted, int> e{Counted{7}};
    Counted::reset();

    auto r = fvo::value_or(std::move(e), Counted{0});

    CHECK(r.v == 7);
    CHECK(Counted::copies == 0);
    CHECK(Counted::moves == 1);
#else
    SUCCEED("std::expected unavailable on this toolchain");
#endif
}

// Handles do not own, so an rvalue handle leaves its referent alone.  These
// stay COPIES while the optional and expected cases above are MOVES.
TEST_CASE("value_or on engaged rvalue smart pointers copies the referent", "[rvalue][value_or]") {
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

TEST_CASE("or_invoke on an engaged rvalue nullable moves the payload", "[rvalue][or_invoke]") {
    std::optional<Counted> o{Counted{7}};
    Counted::reset();

    auto r = fvo::or_invoke(std::move(o), [] { return Counted{0}; });

    CHECK(r.v == 7);
    CHECK(Counted::copies == 0);
    CHECK(Counted::moves == 1);
    CHECK(o.has_value());
}

// ==========================================================================
// or_construct
// ==========================================================================

TEST_CASE("or_construct on an engaged rvalue nullable moves the payload", "[rvalue][or_construct]") {
    std::optional<Counted> o{Counted{7}};
    Counted::reset();

    auto r = fvo::or_construct(std::move(o), 0);

    CHECK(r.v == 7);
    CHECK(Counted::copies == 0);
    CHECK(Counted::moves == 1);
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

    // R is computed from the types, not the engaged state: an owning rvalue
    // nullable gives const Counted& on both paths
    STATIC_REQUIRE(std::is_same_v<decltype(r), const Counted&>);
    CHECK(Counted::copies == 0);
    CHECK(Counted::moves == 0);
    CHECK(&r == &fallback);
}

TEST_CASE("reference_or on an owning rvalue nullable yields a const reference", "[rvalue][reference_or]") {
    std::optional<Counted> o{Counted{7}};
    Counted                fallback{3};
    Counted::reset();

    decltype(auto) r = fvo::reference_or(std::move(o), fallback);

    // common_reference_t<Counted&&, Counted&> is const Counted&
    STATIC_REQUIRE(std::is_same_v<decltype(r), const Counted&>);
    CHECK(Counted::copies == 0);
    CHECK(Counted::moves == 0);

    // it still refers to the optional's payload, not to a copy of it
    CHECK(&r == &*o);
    CHECK(r.v == 7);
}

TEST_CASE("reference_or on an rvalue handle stays a mutable reference", "[rvalue][reference_or]") {
    Counted  owned{7};
    Counted  fallback{3};
    Counted* p = &owned;
    Counted::reset();

    decltype(auto) r = fvo::reference_or(std::move(p), fallback);

    // a handle does not own, so the referent is not expiring
    STATIC_REQUIRE(std::is_same_v<decltype(r), Counted&>);
    CHECK(Counted::copies == 0);
    CHECK(Counted::moves == 0);
    CHECK(&r == &owned);

    r.v = 11;
    CHECK(owned.v == 11);
}
