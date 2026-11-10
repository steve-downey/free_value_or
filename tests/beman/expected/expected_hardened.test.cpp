// tests/beman/expected/expected_hardened.test.cpp                     -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Beman-only: compiled with -DBEMAN_EXPECTED_HARDENED to verify
// precondition-check code compiles and happy paths work correctly.

#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>

#include <beman/expected/testing/type_name.hpp>

#include <string>
#include <type_traits>
#include <utility>

using namespace beman::expected;

using beman::expected::testing::type_name;

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
// Reference specializations
//
// Before this section the hardened build only ever instantiated expected<T, E>
// and expected<void, E> with a value E, so the precondition checks in the
// expected<T&, E> specialization were never compiled at all, and neither the
// primary nor the void specialization was ever instantiated with a reference E.
// The checks themselves are the same `if (!has_val_) TRAP` shape throughout;
// what these add is proof that the guarded bodies still compile and return the
// right thing once T or E is a reference -- in particular that the shallow-const
// observers (T*, T&, E& from a const expected) are unaffected by hardening.
//
// Happy paths only, as above: a violated precondition traps, which cannot be
// observed from inside the process.
// ---------------------------------------------------------------------------

// --- expected<T&, E>: operator-> and operator* (one const overload each) ---

TEST_CASE("hardened: observers on value-state expected<T&,E>", "[hardened]") {
    std::string                 s = "hello";
    expected<std::string&, int> e(s);
    CHECK(e->size() == 5);
    CHECK(*e == "hello");
    CHECK(&*e == &s);

    // Shallow const: a const expected<T&, E> still hands out a mutable T& / T*.
    const expected<std::string&, int> ce(s);
    CHECK(type_name<decltype(ce.operator->())>() == type_name<std::string*>());
    CHECK(type_name<decltype(*ce)>() == type_name<std::string&>());
    CHECK(ce->size() == 5);
    CHECK(&*ce == &s);
}

// --- expected<T&, E>: error() ---

TEST_CASE("hardened: error() on error-state expected<T&,E>", "[hardened]") {
    expected<int&, int> e(unexpect, 42);
    CHECK(e.error() == 42);

    const expected<int&, int> ce(unexpect, 99);
    CHECK(ce.error() == 99);

    expected<int&, int> e2(unexpect, 7);
    CHECK(std::move(e2).error() == 7);

    const expected<int&, int> ce2(unexpect, 13);
    CHECK(std::move(ce2).error() == 13);
}

// --- expected<T, E&>: primary template instantiated with a reference E ---

TEST_CASE("hardened: observers on expected<T,E&>", "[hardened]") {
    expected<std::string, int&> e(std::in_place, "world");
    CHECK(e->size() == 5);
    CHECK(*e == "world");

    const expected<std::string, int&> ce(std::in_place, "there");
    CHECK(ce->size() == 5);
    CHECK(*ce == "there");

    expected<std::string, int&> e2(std::in_place, "moved");
    CHECK(*std::move(e2) == "moved");
}

TEST_CASE("hardened: error() on error-state expected<T,E&>", "[hardened]") {
    int                 err = 42;
    expected<int, int&> e(unexpect, err);
    // Shallow const: error() yields int&, not const int&, even from a const
    // expected -- `const E&` with E = int& collapses back to int&.
    CHECK(type_name<decltype(e.error())>() == type_name<int&>());
    CHECK(&e.error() == &err);

    const expected<int, int&> ce(unexpect, err);
    CHECK(type_name<decltype(ce.error())>() == type_name<int&>());
    CHECK(&ce.error() == &err);

    CHECK(&std::move(e).error() == &err);
    CHECK(&std::move(ce).error() == &err);
}

// --- expected<T&, E&>: T& specialization instantiated with a reference E ---

TEST_CASE("hardened: observers on expected<T&,E&>", "[hardened]") {
    int                  x = 5;
    expected<int&, int&> e(x);
    CHECK(*e == 5);
    CHECK(&*e == &x);

    const expected<int&, int&> ce(x);
    CHECK(&*ce == &x);
}

TEST_CASE("hardened: error() on error-state expected<T&,E&>", "[hardened]") {
    int                  err = 7;
    expected<int&, int&> e(unexpect, err);
    CHECK(&e.error() == &err);

    const expected<int&, int&> ce(unexpect, err);
    CHECK(&ce.error() == &err);

    CHECK(&std::move(e).error() == &err);
    CHECK(&std::move(ce).error() == &err);
}

// --- expected<void, E&>: void specialization with a reference E ---

TEST_CASE("hardened: operator* on value-state expected<void,E&>", "[hardened]") {
    expected<void, int&> e;
    *e;

    const expected<void, int&> ce;
    *ce;
}

TEST_CASE("hardened: error() on error-state expected<void,E&>", "[hardened]") {
    int                  err = 42;
    expected<void, int&> e(unexpect, err);
    CHECK(&e.error() == &err);

    const expected<void, int&> ce(unexpect, err);
    CHECK(&ce.error() == &err);

    CHECK(&std::move(e).error() == &err);
    CHECK(&std::move(ce).error() == &err);
}

// ---------------------------------------------------------------------------
// unexpected friend swap: constraint check (beman-only)
//
// The constraint is checked at runtime rather than with static_assert so that a
// violation is reported by the test run, with the responsible type named,
// instead of stopping the build and reporting nothing. Querying the trait at
// all is still the point: an unconstrained hidden-friend swap would make
// is_swappable_v a hard error rather than a well-formed `false`.
// ---------------------------------------------------------------------------

struct NonSwappable {
    NonSwappable()                               = default;
    NonSwappable(const NonSwappable&)            = delete;
    NonSwappable(NonSwappable&&)                 = delete;
    NonSwappable& operator=(const NonSwappable&) = delete;
    NonSwappable& operator=(NonSwappable&&)      = delete;
};

TEST_CASE("hardened: unexpected<E> is not swappable when E is not", "[hardened]") {
    CHECK_FALSE(std::is_swappable_v<NonSwappable>);
    CHECK_FALSE(std::is_swappable_v<unexpected<NonSwappable>>);
}
