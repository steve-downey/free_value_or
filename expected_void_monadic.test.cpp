// tests/beman/expected/expected_void_monadic.test.cpp                -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/expected.hpp>
#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

using namespace beman::expected;

// ---------------------------------------------------------------------------
// and_then — F called with no args when void
// ---------------------------------------------------------------------------

TEST_CASE("and_then void: has value — calls F with no args", "[expected_void_monadic]") {
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

TEST_CASE("and_then void: has error — short-circuits", "[expected_void_monadic]") {
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
// or_else — F called with error when void expected has error
// ---------------------------------------------------------------------------

TEST_CASE("or_else void: has error — calls F", "[expected_void_monadic]") {
    expected<void, int> e(unexpect, 7);
    auto                r = e.or_else([](int v) -> expected<void, int> {
        (void)v;
        return {};
    });
    CHECK(r.has_value());
}

TEST_CASE("or_else void: has value — short-circuits, returns G()", "[expected_void_monadic]") {
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
// transform — F called with no args when void
// ---------------------------------------------------------------------------

TEST_CASE("transform void: has value — calls F, returns expected<U, E>", "[expected_void_monadic]") {
    expected<void, int> e;
    auto                r = e.transform([]() { return 42; });
    static_assert(std::is_same_v<decltype(r), expected<int, int>>);
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("transform void: has error — propagates", "[expected_void_monadic]") {
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

TEST_CASE("transform void: F returns void — expected<void, E>()", "[expected_void_monadic]") {
    expected<void, int> e;
    int                 count = 0;
    auto                r     = e.transform([&]() { ++count; });
    static_assert(std::is_same_v<decltype(r), expected<void, int>>);
    CHECK(r.has_value());
    CHECK(count == 1);
}

TEST_CASE("transform void: rvalue overload", "[expected_void_monadic]") {
    expected<void, std::string> e;
    auto                        r = std::move(e).transform([]() -> std::string { return "done"; });
    REQUIRE(r.has_value());
    CHECK(*r == "done");
}

// ---------------------------------------------------------------------------
// transform_error — F called with error, same as primary
// ---------------------------------------------------------------------------

TEST_CASE("transform_error void: has error — transforms error", "[expected_void_monadic]") {
    expected<void, int> e(unexpect, 3);
    auto                r = e.transform_error([](int v) -> std::string { return std::to_string(v); });
    static_assert(std::is_same_v<decltype(r), expected<void, std::string>>);
    REQUIRE(!r.has_value());
    CHECK(r.error() == "3");
}

TEST_CASE("transform_error void: has value — returns expected<void, G>()", "[expected_void_monadic]") {
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

TEST_CASE("void monadic chaining: and_then → transform_error", "[expected_void_monadic]") {
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
