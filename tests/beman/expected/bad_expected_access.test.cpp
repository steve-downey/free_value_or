// tests/beman/expected/bad_expected_access.test.cpp                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "test_expected.hpp"

#include <catch2/catch_test_macros.hpp>

#include <beman/expected/testing/type_name.hpp>

#include <exception>
#include <string>
#include <type_traits>
#include <utility>

namespace expt = test_ns;

using beman::expected::testing::type_name;

// =============================================================================
// [expected.bad.void] and [expected.bad] — type-level properties
//
// These are checked at runtime rather than with static_assert so that a
// violated property is reported by the test run, with the responsible type
// named, instead of stopping the build at the first failure and reporting
// nothing. Type identity is checked by comparing the compiler's spelling of
// the two types, so a mismatch prints what was deduced next to what was
// wanted rather than the bare word `false`.
// =============================================================================

TEST_CASE("bad_expected_access: inheritance chain", "[BadExpectedAccessTest]") {
    CHECK(std::is_base_of_v<std::exception, expt::bad_expected_access<void>>);
    CHECK(std::is_base_of_v<expt::bad_expected_access<void>, expt::bad_expected_access<int>>);
    CHECK(std::is_base_of_v<std::exception, expt::bad_expected_access<int>>);
}

TEST_CASE("bad_expected_access: error() ref-qualification return types", "[BadExpectedAccessTest]") {
    using bad_access_t = expt::bad_expected_access<int>;
    CHECK(type_name<decltype(std::declval<bad_access_t&>().error())>() == type_name<int&>());
    CHECK(type_name<decltype(std::declval<const bad_access_t&>().error())>() == type_name<const int&>());
    CHECK(type_name<decltype(std::declval<bad_access_t&&>().error())>() == type_name<int&&>());
    CHECK(type_name<decltype(std::declval<const bad_access_t&&>().error())>() == type_name<const int&&>());
}

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
