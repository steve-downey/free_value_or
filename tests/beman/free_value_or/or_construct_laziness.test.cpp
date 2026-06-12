// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Include-guard idempotency check
#include <beman/free_value_or/value_or.hpp>
#include <beman/free_value_or/value_or.hpp>

#include "test_types.hpp"

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <memory>
#include <optional>
#include <vector>

// ==========================================================================
// Instrumented payload
//
// ctor_count counts only the value-constructing constructors (default, int,
// initializer_list).  Copy and move constructors are NOT counted, so the
// static_cast<R>(*m) copy on the engaged path does not pollute the fallback
// count.  A zero count on the engaged path means the fallback was never built.
// ==========================================================================

struct Tracked {
    static inline int ctor_count = 0;
    int v = 0;

    Tracked() { ++ctor_count; }
    explicit Tracked(int x) : v(x) { ++ctor_count; }
    Tracked(std::initializer_list<int> il) : v(static_cast<int>(il.size())) { ++ctor_count; }

    // Copy and move: NOT counted — we only want to observe fallback construction.
    Tracked(const Tracked&) = default;
    Tracked(Tracked&&)      = default;

    static void reset() { ctor_count = 0; }
};

// ==========================================================================
// Pack overload — laziness
// ==========================================================================

TEST_CASE("or_construct pack: fallback NOT constructed when engaged", "[or_construct][laziness]") {
    SECTION("optional<Tracked> engaged, zero args") {
        std::optional<Tracked> m{std::in_place, 99};
        Tracked::reset();
        auto r = fvo::or_construct(m);
        CHECK(Tracked::ctor_count == 0);  // fallback default-ctor never ran
        CHECK(r.v == 99);
    }

    SECTION("optional<Tracked> engaged, one arg") {
        std::optional<Tracked> m{std::in_place, 99};
        Tracked::reset();
        auto r = fvo::or_construct(m, 7);
        CHECK(Tracked::ctor_count == 0);  // fallback Tracked(int) never ran
        CHECK(r.v == 99);
    }
}

TEST_CASE("or_construct pack: fallback constructed exactly once when disengaged", "[or_construct][laziness]") {
    SECTION("optional<Tracked> disengaged, zero args — default ctor") {
        std::optional<Tracked> m{};
        Tracked::reset();
        auto r = fvo::or_construct(m);
        CHECK(Tracked::ctor_count == 1);
        CHECK(r.v == 0);
    }

    SECTION("optional<Tracked> disengaged, one arg — Tracked(int) ctor") {
        std::optional<Tracked> m{};
        Tracked::reset();
        auto r = fvo::or_construct(m, 7);
        CHECK(Tracked::ctor_count == 1);
        CHECK(r.v == 7);
    }
}

// ==========================================================================
// init-list overload — laziness
// ==========================================================================

TEST_CASE("or_construct init-list: fallback NOT constructed when engaged", "[or_construct][laziness]") {
    std::optional<Tracked> m{std::in_place, 99};
    Tracked::reset();
    auto r = fvo::or_construct(m, {1, 2, 3});
    CHECK(Tracked::ctor_count == 0);  // init-list ctor never ran
    CHECK(r.v == 99);
}

TEST_CASE("or_construct init-list: fallback constructed exactly once when disengaged", "[or_construct][laziness]") {
    std::optional<Tracked> m{};
    Tracked::reset();
    auto r = fvo::or_construct(m, {1, 2, 3});
    CHECK(Tracked::ctor_count == 1);
    CHECK(r.v == 3);  // il.size() == 3
}

// ==========================================================================
// Laziness across nullable types (pack overload)
// ==========================================================================

TEST_CASE("or_construct laziness: expected<Tracked,int>", "[or_construct][laziness]") {
    SECTION("engaged — fallback NOT constructed") {
        std::expected<Tracked, int> m{std::in_place, 42};
        Tracked::reset();
        auto r = fvo::or_construct(m, 7);
        CHECK(Tracked::ctor_count == 0);
        CHECK(r.v == 42);
    }

    SECTION("disengaged — fallback constructed once") {
        std::expected<Tracked, int> m{std::unexpected(0)};
        Tracked::reset();
        auto r = fvo::or_construct(m, 7);
        CHECK(Tracked::ctor_count == 1);
        CHECK(r.v == 7);
    }
}

TEST_CASE("or_construct laziness: raw pointer", "[or_construct][laziness]") {
    SECTION("non-null pointer — fallback NOT constructed") {
        Tracked held{};  // this construction does not count (reset follows)
        Tracked* p = &held;
        Tracked::reset();
        auto r = fvo::or_construct(p, 7);
        CHECK(Tracked::ctor_count == 0);
        CHECK(r.v == held.v);
    }

    SECTION("null pointer — fallback constructed once") {
        Tracked* p = nullptr;
        Tracked::reset();
        auto r = fvo::or_construct(p, 7);
        CHECK(Tracked::ctor_count == 1);
        CHECK(r.v == 7);
    }
}

TEST_CASE("or_construct laziness: unique_ptr (move-only)", "[or_construct][laziness]") {
    SECTION("non-null unique_ptr — fallback NOT constructed") {
        auto p = std::make_unique<Tracked>(42);
        Tracked::reset();
        // must move the unique_ptr since it is non-copyable
        auto r = fvo::or_construct(std::move(p), 7);
        CHECK(Tracked::ctor_count == 0);
        CHECK(r.v == 42);
    }

    SECTION("null unique_ptr — fallback constructed once") {
        std::unique_ptr<Tracked> p;
        Tracked::reset();
        auto r = fvo::or_construct(std::move(p), 7);
        CHECK(Tracked::ctor_count == 1);
        CHECK(r.v == 7);
    }
}
