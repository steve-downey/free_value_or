// tests/beman/expected/expected_hardened.test.cpp                     -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Beman-only: compiled with -DBEMAN_EXPECTED_HARDENED to verify
// precondition-check code compiles and happy paths work correctly.

#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

using namespace beman::expected;

// ---------------------------------------------------------------------------
// Primary template: operator-> happy path
// ---------------------------------------------------------------------------

TEST_CASE("hardened: operator-> on value-state expected", "[hardened]") {
    expected<std::string, int> e("hello");
    CHECK(e->size() == 5);

    const expected<std::string, int> ce("world");
    CHECK(ce->size() == 5);
}

// ---------------------------------------------------------------------------
// Primary template: operator* happy path
// ---------------------------------------------------------------------------

TEST_CASE("hardened: operator* on value-state expected", "[hardened]") {
    expected<int, int> e(42);
    CHECK(*e == 42);

    const expected<int, int> ce(99);
    CHECK(*ce == 99);

    CHECK(*std::move(e) == 42);

    const expected<int, int> ce2(7);
    CHECK(*std::move(ce2) == 7);
}

// ---------------------------------------------------------------------------
// Primary template: error() happy path
// ---------------------------------------------------------------------------

TEST_CASE("hardened: error() on error-state expected", "[hardened]") {
    expected<int, int> e(unexpect, 42);
    CHECK(e.error() == 42);

    const expected<int, int> ce(unexpect, 99);
    CHECK(ce.error() == 99);

    expected<int, int> e2(unexpect, 7);
    CHECK(std::move(e2).error() == 7);

    const expected<int, int> ce2(unexpect, 13);
    CHECK(std::move(ce2).error() == 13);
}

// ---------------------------------------------------------------------------
// Void specialization: operator* happy path
// ---------------------------------------------------------------------------

TEST_CASE("hardened: operator* on value-state expected<void,int>", "[hardened]") {
    expected<void, int> e;
    *e;

    const expected<void, int> ce;
    *ce;
}

// ---------------------------------------------------------------------------
// Void specialization: error() happy path
// ---------------------------------------------------------------------------

TEST_CASE("hardened: error() on error-state expected<void,int>", "[hardened]") {
    expected<void, int> e(unexpect, 42);
    CHECK(e.error() == 42);

    const expected<void, int> ce(unexpect, 99);
    CHECK(ce.error() == 99);

    expected<void, int> e2(unexpect, 7);
    CHECK(std::move(e2).error() == 7);

    const expected<void, int> ce2(unexpect, 13);
    CHECK(std::move(ce2).error() == 13);
}

// ---------------------------------------------------------------------------
// unexpected friend swap: constraint check (beman-only)
// ---------------------------------------------------------------------------

struct NonSwappable {
    NonSwappable()                       = default;
    NonSwappable(const NonSwappable&)    = delete;
    NonSwappable(NonSwappable&&)         = delete;
    NonSwappable& operator=(const NonSwappable&) = delete;
    NonSwappable& operator=(NonSwappable&&)      = delete;
};
static_assert(!std::is_swappable_v<NonSwappable>);
static_assert(!std::is_swappable_v<unexpected<NonSwappable>>);
