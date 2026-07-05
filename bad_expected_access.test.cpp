// tests/beman/expected/bad_expected_access.test.cpp                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "test_expected.hpp"

#include <catch2/catch_test_macros.hpp>

#include <exception>
#include <string>
#include <utility>

namespace expt = test_ns;

// =============================================================================
// [expected.bad.void] and [expected.bad] — type-level assertions
// =============================================================================

// Inheritance chain
static_assert(std::is_base_of_v<std::exception, expt::bad_expected_access<void>>);
static_assert(std::is_base_of_v<expt::bad_expected_access<void>, expt::bad_expected_access<int>>);
static_assert(std::is_base_of_v<std::exception, expt::bad_expected_access<int>>);

// error() ref-qualification return types
static_assert(std::is_same_v<decltype(std::declval<expt::bad_expected_access<int>&>().error()), int&>);
static_assert(std::is_same_v<decltype(std::declval<const expt::bad_expected_access<int>&>().error()), const int&>);
static_assert(std::is_same_v<decltype(std::declval<expt::bad_expected_access<int>&&>().error()), int&&>);
static_assert(std::is_same_v<decltype(std::declval<const expt::bad_expected_access<int>&&>().error()), const int&&>);

TEST_CASE("bad_expected_access: breathing", "[BadExpectedAccessTest]") {}

TEST_CASE("bad_expected_access: construct from int", "[BadExpectedAccessTest]") {
    expt::bad_expected_access<int> e(42);
    CHECK(e.error() == 42);
}

TEST_CASE("bad_expected_access: what() returns message", "[BadExpectedAccessTest]") {
    expt::bad_expected_access<int> e(1);
    CHECK(e.what() != nullptr);
    // what() returns an implementation-defined NTBS [expected.bad.void]; the
    // exact text is beman-specific (libstdc++/libc++ differ).
#ifndef BEMAN_EXPECTED_TEST_STD
    CHECK(std::string_view(e.what()) == "bad expected access");
#endif
}

TEST_CASE("bad_expected_access: inherits from std::exception", "[BadExpectedAccessTest]") {
    expt::bad_expected_access<int> e(1);
    std::exception&                ex = e;
    CHECK(ex.what() != nullptr);
#ifndef BEMAN_EXPECTED_TEST_STD
    CHECK(std::string_view(ex.what()) == "bad expected access");
#endif
}

TEST_CASE("bad_expected_access: error() lvalue ref mutable", "[BadExpectedAccessTest]") {
    expt::bad_expected_access<int> e(42);
    e.error() = 99;
    CHECK(e.error() == 99);
}

TEST_CASE("bad_expected_access: error() const lvalue ref", "[BadExpectedAccessTest]") {
    const expt::bad_expected_access<int> e(42);
    CHECK(e.error() == 42);
}

TEST_CASE("bad_expected_access: error() rvalue ref", "[BadExpectedAccessTest]") {
    expt::bad_expected_access<int> e(42);
    int                            v = std::move(e).error();
    CHECK(v == 42);
}

TEST_CASE("bad_expected_access: error() const rvalue ref", "[BadExpectedAccessTest]") {
    const expt::bad_expected_access<int> e(42);
    int                                  v = std::move(e).error();
    CHECK(v == 42);
}

TEST_CASE("bad_expected_access: string move semantics", "[BadExpectedAccessTest]") {
    expt::bad_expected_access<std::string> e(std::string("hello"));
    std::string                            s = std::move(e).error();
    CHECK(s == "hello");
}

TEST_CASE("bad_expected_access: catchable as std::exception", "[BadExpectedAccessTest]") {
    try {
        throw expt::bad_expected_access<int>(7);
    } catch (const std::exception& ex) {
        CHECK(ex.what() != nullptr);
#ifndef BEMAN_EXPECTED_TEST_STD
        CHECK(std::string_view(ex.what()) == "bad expected access");
#endif
    }
}

TEST_CASE("bad_expected_access: catchable as bad_expected_access<void>", "[BadExpectedAccessTest]") {
    try {
        throw expt::bad_expected_access<int>(7);
    } catch (const expt::bad_expected_access<void>& ex) {
        CHECK(ex.what() != nullptr);
#ifndef BEMAN_EXPECTED_TEST_STD
        CHECK(std::string_view(ex.what()) == "bad expected access");
#endif
    }
}

TEST_CASE("bad_expected_access: move-only error type", "[BadExpectedAccessTest]") {
    // The constructor takes E by value and uses std::move(e); works with move-only E
    struct MoveOnly {
        int v;
        explicit MoveOnly(int x) : v(x) {}
        MoveOnly(const MoveOnly&) = delete;
        MoveOnly(MoveOnly&&)      = default;
    };
    expt::bad_expected_access<MoveOnly> ex(MoveOnly{42});
    CHECK(ex.error().v == 42);
}

TEST_CASE("bad_expected_access<void>: accessible via base reference", "[BadExpectedAccessTest]") {
    expt::bad_expected_access<int>         ex(0);
    const expt::bad_expected_access<void>& base = ex;
    CHECK(base.what() != nullptr);
}

TEST_CASE("bad_expected_access: move constructor", "[BadExpectedAccessTest]") {
    expt::bad_expected_access<std::string> orig("test error");
    expt::bad_expected_access<std::string> moved(std::move(orig));
    CHECK(moved.error() == "test error");
}

TEST_CASE("bad_expected_access: rvalue error accessor (string move)", "[BadExpectedAccessTest]") {
    expt::bad_expected_access<std::string> e("val");
    std::string                            s = std::move(e).error();
    CHECK(s == "val");
}

TEST_CASE("bad_expected_access: const rvalue error accessor (string move)", "[BadExpectedAccessTest]") {
    const expt::bad_expected_access<std::string> e("val");
    std::string                                  s = std::move(e).error();
    CHECK(s == "val");
}
