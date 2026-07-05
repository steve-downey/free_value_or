// tests/beman/expected/expected_void_monadic.test.cpp                -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/expected.hpp>
#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>

#include "testing/types.hpp"

#include <string>

using namespace beman::expected;

// ---------------------------------------------------------------------------
// and_then - F called with no args when void
// ---------------------------------------------------------------------------

TEST_CASE("and_then void: has value - calls F with no args", "[expected_void_monadic]") {
    expected<void, std::string> e;
    int                         calls = 0;
    auto                        r     = e.and_then([&]() -> expected<int, std::string> {
        ++calls;
        return 42;
    });
    CHECK(calls == 1);
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("and_then void: has error - short-circuits", "[expected_void_monadic]") {
    expected<void, std::string> e(unexpect, "bad");
    bool                        called = false;
    auto                        r      = e.and_then([&]() -> expected<int, std::string> {
        called = true;
        return 0;
    });
    CHECK(!called);
    REQUIRE(!r.has_value());
    CHECK(r.error() == "bad");
}

TEST_CASE("and_then void: rvalue overload propagates error by move", "[expected_void_monadic]") {
    expected<void, std::string> e(unexpect, "err");
    auto                        r = std::move(e).and_then([]() -> expected<int, std::string> { return 1; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "err");
}

TEST_CASE("and_then void: return void expected", "[expected_void_monadic]") {
    expected<void, int> e;
    auto                r = e.and_then([]() -> expected<void, int> { return {}; });
    static_assert(std::is_same_v<decltype(r), expected<void, int>>);
    CHECK(r.has_value());
}

TEST_CASE("and_then void: chaining void-to-value", "[expected_void_monadic]") {
    expected<void, int> e;
    auto r = e.and_then([]() -> expected<int, int> { return 1; }).and_then([](int v) -> expected<int, int> {
        return v + 1;
    });
    REQUIRE(r.has_value());
    CHECK(*r == 2);
}

// ---------------------------------------------------------------------------
// or_else - F called with error when void expected has error
// ---------------------------------------------------------------------------

TEST_CASE("or_else void: has error - calls F", "[expected_void_monadic]") {
    expected<void, int> e(unexpect, 7);
    auto                r = e.or_else([](int v) -> expected<void, int> {
        (void)v;
        return {};
    });
    CHECK(r.has_value());
}

TEST_CASE("or_else void: has value - short-circuits, returns G()", "[expected_void_monadic]") {
    expected<void, int> e;
    bool                called = false;
    auto                r      = e.or_else([&](int) -> expected<void, int> {
        called = true;
        return {};
    });
    CHECK(!called);
    CHECK(r.has_value());
}

TEST_CASE("or_else void: error propagated through lambda", "[expected_void_monadic]") {
    expected<void, std::string> e(unexpect, "original");
    auto r = e.or_else([](std::string s) -> expected<void, std::string> { return unexpected(s + "_fixed"); });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "original_fixed");
}

// ---------------------------------------------------------------------------
// transform - F called with no args when void
// ---------------------------------------------------------------------------

TEST_CASE("transform void: has value - calls F, returns expected<U, E>", "[expected_void_monadic]") {
    expected<void, int> e;
    auto                r = e.transform([]() { return 42; });
    static_assert(std::is_same_v<decltype(r), expected<int, int>>);
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("transform void: has error - propagates", "[expected_void_monadic]") {
    expected<void, int> e(unexpect, 5);
    bool                called = false;
    auto                r      = e.transform([&]() {
        called = true;
        return 0;
    });
    CHECK(!called);
    REQUIRE(!r.has_value());
    CHECK(r.error() == 5);
}

TEST_CASE("transform void: F returns void - expected<void, E>()", "[expected_void_monadic]") {
    expected<void, int> e;
    int                 count = 0;
    auto                r     = e.transform([&]() { ++count; });
    static_assert(std::is_same_v<decltype(r), expected<void, int>>);
    CHECK(r.has_value());
    CHECK(count == 1);
}

TEST_CASE("transform void: has error, F returns void - propagates", "[expected_void_monadic]") {
    expected<void, int> e(unexpect, 5);
    int                 count = 0;
    auto                r     = e.transform([&]() { ++count; });
    static_assert(std::is_same_v<decltype(r), expected<void, int>>);
    CHECK(count == 0);
    REQUIRE(!r.has_value());
    CHECK(r.error() == 5);
}

TEST_CASE("transform void: rvalue overload", "[expected_void_monadic]") {
    expected<void, std::string> e;
    auto                        r = std::move(e).transform([]() -> std::string { return "done"; });
    REQUIRE(r.has_value());
    CHECK(*r == "done");
}

// ---------------------------------------------------------------------------
// transform_error - F called with error, same as primary
// ---------------------------------------------------------------------------

TEST_CASE("transform_error void: has error - transforms error", "[expected_void_monadic]") {
    expected<void, int> e(unexpect, 3);
    auto                r = e.transform_error([](int v) -> std::string { return std::to_string(v); });
    static_assert(std::is_same_v<decltype(r), expected<void, std::string>>);
    REQUIRE(!r.has_value());
    CHECK(r.error() == "3");
}

TEST_CASE("transform_error void: has value - returns expected<void, G>()", "[expected_void_monadic]") {
    expected<void, int> e;
    bool                called = false;
    auto                r      = e.transform_error([&](int) -> std::string {
        called = true;
        return "";
    });
    CHECK(!called);
    static_assert(std::is_same_v<decltype(r), expected<void, std::string>>);
    CHECK(r.has_value());
}

// ---------------------------------------------------------------------------
// Chaining combinations
// ---------------------------------------------------------------------------

TEST_CASE("void monadic chaining: and_then -> transform_error", "[expected_void_monadic]") {
    expected<void, int> e;
    auto r = e.and_then([]() -> expected<void, int> { return {}; }).transform_error([](int v) -> std::string {
        return std::to_string(v);
    });
    static_assert(std::is_same_v<decltype(r), expected<void, std::string>>);
    CHECK(r.has_value());
}

TEST_CASE("void monadic chaining: error path end-to-end", "[expected_void_monadic]") {
    expected<void, int> e(unexpect, 42);
    auto r = e.and_then([]() -> expected<void, int> { return {}; }).or_else([](int v) -> expected<void, int> {
        if (v == 42)
            return {};
        return unexpected(v);
    });
    CHECK(r.has_value());
}

// ---------------------------------------------------------------------------
// All 4 ref-qualifications compile
// ---------------------------------------------------------------------------

TEST_CASE("void and_then: all ref qualifications compile", "[expected_void_monadic]") {
    using E = expected<void, int>;
    auto f  = []() -> E { return {}; };

    E e;
    (void)(e.and_then(f));
    (void)(std::as_const(e).and_then(f));
    (void)(std::move(e).and_then(f));
    (void)(std::move(std::as_const(e)).and_then(f));
}

// =============================================================================
// Additional coverage tests: rvalue/const-rvalue monadic overloads for void
// =============================================================================
using namespace beman::expected::testing;

// --- Monadic: and_then rvalue/const-rvalue on ERROR state ---
TEST_CASE("and_then rvalue on error short-circuits", "[expected_void_monadic]") {
    expected<void, std::string> e(unexpect, "err");
    bool                        called = false;
    auto                        r      = std::move(e).and_then([&]() -> expected<void, std::string> {
        called = true;
        return {};
    });
    CHECK(!called);
    REQUIRE(!r.has_value());
}

TEST_CASE("and_then const rvalue on error short-circuits", "[expected_void_monadic]") {
    const expected<void, std::string> e(unexpect, "err");
    bool                              called = false;
    auto                              r      = std::move(e).and_then([&]() -> expected<void, std::string> {
        called = true;
        return {};
    });
    CHECK(!called);
    REQUIRE(!r.has_value());
}

// --- Monadic: and_then rvalue/const-rvalue on VALUE state (actual call) ---
TEST_CASE("and_then rvalue on value calls F", "[expected_void_monadic]") {
    expected<void, int> e;
    bool                called = false;
    auto                r      = std::move(e).and_then([&]() -> expected<int, int> {
        called = true;
        return 42;
    });
    CHECK(called);
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("and_then const rvalue on value calls F", "[expected_void_monadic]") {
    const expected<void, int> e;
    auto                      r = std::move(e).and_then([]() -> expected<int, int> { return 99; });
    REQUIRE(r.has_value());
    CHECK(*r == 99);
}

// --- Monadic: or_else rvalue/const-rvalue on VALUE state ---
TEST_CASE("or_else rvalue on value short-circuits", "[expected_void_monadic]") {
    expected<void, int> e;
    bool                called = false;
    auto                r      = std::move(e).or_else([&](int) -> expected<void, int> {
        called = true;
        return {};
    });
    CHECK(!called);
    REQUIRE(r.has_value());
}

TEST_CASE("or_else const rvalue on value short-circuits", "[expected_void_monadic]") {
    const expected<void, int> e;
    auto                      r = std::move(e).or_else([](int) -> expected<void, int> { return {}; });
    REQUIRE(r.has_value());
}

TEST_CASE("or_else rvalue on error calls F", "[expected_void_monadic]") {
    expected<void, int> e(unexpect, 5);
    auto                r = std::move(e).or_else([](int v) -> expected<void, int> { return unexpected(v + 1); });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 6);
}

TEST_CASE("or_else const rvalue on error calls F", "[expected_void_monadic]") {
    const expected<void, int> e(unexpect, 5);
    auto                       r = std::move(e).or_else([](int v) -> expected<void, int> { return unexpected(v + 1); });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 6);
}

// --- Monadic: transform rvalue/const-rvalue on ERROR state ---
TEST_CASE("transform rvalue on error short-circuits", "[expected_void_monadic]") {
    expected<void, std::string> e(unexpect, "err");
    auto                        r = std::move(e).transform([]() { return 1; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "err");
}

TEST_CASE("transform const rvalue on error short-circuits", "[expected_void_monadic]") {
    const expected<void, std::string> e(unexpect, "err");
    auto                              r = std::move(e).transform([]() { return 1; });
    REQUIRE(!r.has_value());
}

// --- Monadic: transform rvalue/const-rvalue on VALUE state ---
TEST_CASE("transform rvalue on value calls F", "[expected_void_monadic]") {
    expected<void, int> e;
    auto                r = std::move(e).transform([]() { return 42; });
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("transform const rvalue on value calls F", "[expected_void_monadic]") {
    const expected<void, int> e;
    auto                      r = std::move(e).transform([]() { return 7; });
    REQUIRE(r.has_value());
    CHECK(*r == 7);
}

// --- Monadic: transform_error rvalue/const-rvalue ---
TEST_CASE("transform_error rvalue on value short-circuits", "[expected_void_monadic]") {
    expected<void, int> e;
    auto                r = std::move(e).transform_error([](int v) { return v * 2; });
    REQUIRE(r.has_value());
}

TEST_CASE("transform_error rvalue on error calls F", "[expected_void_monadic]") {
    expected<void, int> e(unexpect, 5);
    auto                r = std::move(e).transform_error([](int v) { return v * 2; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 10);
}

TEST_CASE("transform_error const rvalue on error calls F", "[expected_void_monadic]") {
    const expected<void, int> e(unexpect, 3);
    auto                      r = std::move(e).transform_error([](int v) { return v + 1; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 4);
}

// ---------------------------------------------------------------------------
// void<traced> monadic tests
// ---------------------------------------------------------------------------

TEST_CASE("void<traced>: and_then lvalue error path", "[expected_void_monadic][traced]") {
    expected<void, traced> e(unexpect, 5);
    auto                   r = e.and_then([]() -> expected<void, traced> { return {}; });
    REQUIRE(!r.has_value());
}

TEST_CASE("void<traced>: and_then const lvalue error path", "[expected_void_monadic][traced]") {
    const expected<void, traced> e(unexpect, 5);
    auto                         r = e.and_then([]() -> expected<void, traced> { return {}; });
    REQUIRE(!r.has_value());
}

TEST_CASE("void<traced>: or_else lvalue value path", "[expected_void_monadic][traced]") {
    expected<void, traced> e;
    auto                   r = e.or_else([](traced&) -> expected<void, traced> { return {}; });
    REQUIRE(r.has_value());
}

TEST_CASE("void<traced>: or_else rvalue value path", "[expected_void_monadic][traced]") {
    expected<void, traced> e;
    auto                   r = std::move(e).or_else([](traced&&) -> expected<void, traced> { return {}; });
    REQUIRE(r.has_value());
}

TEST_CASE("void<traced>: or_else const rvalue value path", "[expected_void_monadic][traced]") {
    const expected<void, traced> e;
    auto                         r = std::move(e).or_else([](const traced&) -> expected<void, traced> { return {}; });
    REQUIRE(r.has_value());
}

TEST_CASE("void<traced>: transform lvalue value path", "[expected_void_monadic][traced]") {
    expected<void, traced> e;
    auto                   r = e.transform([]() { return traced(1); });
    REQUIRE(r.has_value());
    CHECK(r->val == 1);
}

TEST_CASE("void<traced>: transform lvalue error path", "[expected_void_monadic][traced]") {
    expected<void, traced> e(unexpect, 5);
    auto                   r = e.transform([]() { return traced(0); });
    REQUIRE(!r.has_value());
}

TEST_CASE("void<traced>: transform rvalue value path", "[expected_void_monadic][traced]") {
    expected<void, traced> e;
    auto                   r = std::move(e).transform([]() { return traced(2); });
    REQUIRE(r.has_value());
}

TEST_CASE("void<traced>: transform rvalue error path", "[expected_void_monadic][traced]") {
    expected<void, traced> e(unexpect, 5);
    auto                   r = std::move(e).transform([]() { return traced(0); });
    REQUIRE(!r.has_value());
}

TEST_CASE("void<traced>: transform const rvalue value path", "[expected_void_monadic][traced]") {
    const expected<void, traced> e;
    auto                         r = std::move(e).transform([]() { return traced(3); });
    REQUIRE(r.has_value());
}

TEST_CASE("void<traced>: transform const rvalue error path", "[expected_void_monadic][traced]") {
    const expected<void, traced> e(unexpect, 5);
    auto                         r = std::move(e).transform([]() { return traced(0); });
    REQUIRE(!r.has_value());
}

TEST_CASE("void<traced>: transform to void lvalue", "[expected_void_monadic][traced]") {
    expected<void, traced> e;
    auto                   r = e.transform([]() {});
    REQUIRE(r.has_value());
}

TEST_CASE("void<traced>: transform_error lvalue value path", "[expected_void_monadic][traced]") {
    expected<void, traced> e;
    auto                   r = e.transform_error([](traced&) { return traced(0); });
    REQUIRE(r.has_value());
}

TEST_CASE("void<traced>: transform_error lvalue error path", "[expected_void_monadic][traced]") {
    expected<void, traced> e(unexpect, 5);
    auto                   r = e.transform_error([](traced& v) { return traced(v.val + 1); });
    REQUIRE(!r.has_value());
    CHECK(r.error().val == 6);
}

TEST_CASE("void<traced>: transform_error rvalue value path", "[expected_void_monadic][traced]") {
    expected<void, traced> e;
    auto                   r = std::move(e).transform_error([](traced&&) { return traced(0); });
    REQUIRE(r.has_value());
}

TEST_CASE("void<traced>: transform_error const rvalue value path", "[expected_void_monadic][traced]") {
    const expected<void, traced> e;
    auto                         r = std::move(e).transform_error([](const traced&) { return traced(0); });
    REQUIRE(r.has_value());
}

// --- void unexpect_t with init-list ---
TEST_CASE("void<init_list>: unexpect_t with initializer_list", "[expected_void_monadic][init_list]") {
    expected<void, init_list_type> e(unexpect, {1, 2, 3});
    REQUIRE(!e.has_value());
    CHECK(e.error().sum == 6);
}
