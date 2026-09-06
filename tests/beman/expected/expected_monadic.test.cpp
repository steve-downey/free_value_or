// tests/beman/expected/expected_monadic.test.cpp                     -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "test_expected.hpp"

#include <catch2/catch_test_macros.hpp>

#include <beman/expected/testing/type_name.hpp>

#include "testing/types.hpp"

#include <functional>
#include <string>
#include <utility>

using namespace test_ns;

using beman::expected::testing::type_name;

// ---------------------------------------------------------------------------
// and_then
// ---------------------------------------------------------------------------

TEST_CASE("and_then: has value - calls F", "[expected_monadic]") {
    expected<int, std::string> e(42);
    auto                       result = e.and_then([](int v) -> expected<int, std::string> { return v * 2; });
    REQUIRE(result.has_value());
    CHECK(*result == 84);
}

TEST_CASE("and_then: has error - short-circuits", "[expected_monadic]") {
    expected<int, std::string> e(unexpect, "fail");
    bool                       called = false;
    auto                       result = e.and_then([&](int) -> expected<int, std::string> {
        called = true;
        return 0;
    });
    CHECK(!called);
    REQUIRE(!result.has_value());
    CHECK(result.error() == "fail");
}

TEST_CASE("and_then: chaining", "[expected_monadic]") {
    expected<int, std::string> e(1);
    auto                       r = e.and_then([](int v) -> expected<int, std::string> {
                  return v + 1;
                                    }).and_then([](int v) -> expected<int, std::string> { return v * 10; });
    REQUIRE(r.has_value());
    CHECK(*r == 20);
}

TEST_CASE("and_then: rvalue overload moves value", "[expected_monadic]") {
    expected<std::string, int> e("hello");
    std::string                moved_from;
    auto                       r = std::move(e).and_then([&](std::string s) -> expected<std::string, int> {
        moved_from = std::move(s);
        return "done";
    });
    CHECK(moved_from == "hello");
    REQUIRE(r.has_value());
    CHECK(*r == "done");
}

TEST_CASE("and_then: const lvalue overload", "[expected_monadic]") {
    const expected<int, int> e(5);
    auto                     r = e.and_then([](int v) -> expected<long, int> { return static_cast<long>(v); });
    REQUIRE(r.has_value());
    CHECK(*r == 5L);
}

TEST_CASE("and_then: const rvalue overload", "[expected_monadic]") {
    const expected<int, int> e(7);
    auto                     r = std::move(e).and_then([](int v) -> expected<int, int> { return v + 1; });
    REQUIRE(r.has_value());
    CHECK(*r == 8);
}

TEST_CASE("and_then: error propagated through chain", "[expected_monadic]") {
    expected<int, std::string> e(unexpect, "stop");
    auto                       r = e.and_then([](int v) -> expected<int, std::string> {
                  return v + 1;
                                    }).and_then([](int v) -> expected<int, std::string> { return v * 2; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "stop");
}

// ---------------------------------------------------------------------------
// or_else
// ---------------------------------------------------------------------------

TEST_CASE("or_else: has error - calls F", "[expected_monadic]") {
    expected<int, std::string> e(unexpect, "problem");
    auto result = e.or_else([](std::string s) -> expected<int, std::string> { return static_cast<int>(s.size()); });
    REQUIRE(result.has_value());
    CHECK(*result == 7);
}

TEST_CASE("or_else: has value - short-circuits", "[expected_monadic]") {
    expected<int, std::string> e(42);
    bool                       called = false;
    auto                       result = e.or_else([&](std::string) -> expected<int, std::string> {
        called = true;
        return 0;
    });
    CHECK(!called);
    REQUIRE(result.has_value());
    CHECK(*result == 42);
}

TEST_CASE("or_else: rvalue overload moves error", "[expected_monadic]") {
    expected<int, std::string> e(unexpect, "err");
    std::string                got;
    auto                       r = std::move(e).or_else([&](std::string s) -> expected<int, std::string> {
        got = std::move(s);
        return 0;
    });
    CHECK(got == "err");
    REQUIRE(r.has_value());
}

TEST_CASE("or_else: const lvalue overload", "[expected_monadic]") {
    const expected<int, int> e(unexpect, 3);
    auto                     r = e.or_else([](int v) -> expected<int, int> { return v * 10; });
    REQUIRE(r.has_value());
    CHECK(*r == 30);
}

TEST_CASE("or_else: const rvalue overload", "[expected_monadic]") {
    const expected<int, int> e(unexpect, 5);
    auto                     r = std::move(e).or_else([](int v) -> expected<int, int> { return v + 1; });
    REQUIRE(r.has_value());
    CHECK(*r == 6);
}

TEST_CASE("or_else: value passes through chain", "[expected_monadic]") {
    expected<int, std::string> e(99);
    auto                       r = e.or_else([](std::string) -> expected<int, std::string> {
                  return 0;
                                    }).or_else([](std::string) -> expected<int, std::string> { return 1; });
    REQUIRE(r.has_value());
    CHECK(*r == 99);
}

// ---------------------------------------------------------------------------
// transform
// ---------------------------------------------------------------------------

TEST_CASE("transform: has value - transforms", "[expected_monadic]") {
    expected<int, std::string> e(6);
    auto                       r = e.transform([](int v) { return v * 7; });
    CHECK(type_name<decltype(r)>() == type_name<expected<int, std::string>>());
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("transform: has error - propagates", "[expected_monadic]") {
    expected<int, std::string> e(unexpect, "oops");
    bool                       called = false;
    auto                       r      = e.transform([&](int) {
        called = true;
        return 0;
    });
    CHECK(!called);
    REQUIRE(!r.has_value());
    CHECK(r.error() == "oops");
}

TEST_CASE("transform: void return type", "[expected_monadic]") {
    expected<int, std::string> e(1);
    int                        count = 0;
    auto                       r     = e.transform([&](int) { ++count; });
    CHECK(type_name<decltype(r)>() == type_name<expected<void, std::string>>());
    REQUIRE(r.has_value());
    CHECK(count == 1);
}

TEST_CASE("transform: void return - error state does not call F", "[expected_monadic]") {
    expected<int, std::string> e(unexpect, "no");
    int                        count = 0;
    auto                       r     = e.transform([&](int) { ++count; });
    CHECK(type_name<decltype(r)>() == type_name<expected<void, std::string>>());
    REQUIRE(!r.has_value());
    CHECK(count == 0);
    CHECK(r.error() == "no");
}

TEST_CASE("transform: type change", "[expected_monadic]") {
    expected<int, int> e(42);
    auto               r = e.transform([](int v) -> std::string { return std::to_string(v); });
    CHECK(type_name<decltype(r)>() == type_name<expected<std::string, int>>());
    REQUIRE(r.has_value());
    CHECK(*r == "42");
}

TEST_CASE("transform: rvalue overload", "[expected_monadic]") {
    expected<std::string, int> e("hello");
    auto                       r = std::move(e).transform([](std::string s) { return s.size(); });
    REQUIRE(r.has_value());
    CHECK(*r == 5u);
}

TEST_CASE("transform: const lvalue overload", "[expected_monadic]") {
    const expected<int, int> e(10);
    auto                     r = e.transform([](int v) { return v + 1; });
    REQUIRE(r.has_value());
    CHECK(*r == 11);
}

TEST_CASE("transform: const rvalue overload", "[expected_monadic]") {
    const expected<int, int> e(3);
    auto                     r = std::move(e).transform([](int v) { return v * v; });
    REQUIRE(r.has_value());
    CHECK(*r == 9);
}

// ---------------------------------------------------------------------------
// transform_error
// ---------------------------------------------------------------------------

TEST_CASE("transform_error: has error - transforms", "[expected_monadic]") {
    expected<int, int> e(unexpect, 3);
    auto               r = e.transform_error([](int v) -> std::string { return std::to_string(v); });
    CHECK(type_name<decltype(r)>() == type_name<expected<int, std::string>>());
    REQUIRE(!r.has_value());
    CHECK(r.error() == "3");
}

TEST_CASE("transform_error: has value - preserves", "[expected_monadic]") {
    expected<int, int> e(42);
    bool               called = false;
    auto               r      = e.transform_error([&](int) -> std::string {
        called = true;
        return "";
    });
    CHECK(!called);
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("transform_error: rvalue overload moves error", "[expected_monadic]") {
    expected<int, std::string> e(unexpect, "err");
    std::string                got;
    auto                       r = std::move(e).transform_error([&](std::string s) -> int {
        got = std::move(s);
        return 99;
    });
    CHECK(got == "err");
    REQUIRE(!r.has_value());
    CHECK(r.error() == 99);
}

TEST_CASE("transform_error: const lvalue overload", "[expected_monadic]") {
    const expected<int, int> e(unexpect, 4);
    auto                     r = e.transform_error([](int v) { return v * 2; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 8);
}

TEST_CASE("transform_error: const rvalue overload", "[expected_monadic]") {
    const expected<int, int> e(unexpect, 5);
    auto                     r = std::move(e).transform_error([](int v) -> std::string { return std::to_string(v); });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "5");
}

// ---------------------------------------------------------------------------
// Mixed chaining
// ---------------------------------------------------------------------------

TEST_CASE("monadic chaining: parse, double, stringify", "[expected_monadic]") {
    auto parse = [](const std::string& s) -> expected<int, std::string> {
        if (s.empty())
            return unexpected<std::string>("empty");
        return std::stoi(s);
    };
    auto double_it = [](int v) -> expected<int, std::string> { return v * 2; };
    auto to_str    = [](int v) { return std::to_string(v); };

    auto r = parse("21").and_then(double_it).transform(to_str);
    REQUIRE(r.has_value());
    CHECK(*r == "42");

    auto r2 = parse("").and_then(double_it).transform(to_str);
    REQUIRE(!r2.has_value());
    CHECK(r2.error() == "empty");
}

TEST_CASE("monadic chaining: or_else recovery", "[expected_monadic]") {
    auto r = expected<int, std::string>(unexpect, "err")
                 .or_else([](std::string) -> expected<int, std::string> { return 0; })
                 .transform([](int v) { return v + 1; });
    REQUIRE(r.has_value());
    CHECK(*r == 1);
}

TEST_CASE("monadic chaining: transform_error then or_else", "[expected_monadic]") {
    expected<int, int> e(unexpect, 42);
    auto               r = e.transform_error([](int v) -> std::string {
                  return std::to_string(v);
                            }).or_else([](std::string s) -> expected<int, std::string> { return static_cast<int>(s.size()); });
    REQUIRE(r.has_value());
    CHECK(*r == 2);
}

// ---------------------------------------------------------------------------
// All 4 ref qualifications compile
// ---------------------------------------------------------------------------

TEST_CASE("and_then: all 4 ref qualifications compile", "[expected_monadic]") {
    using E = expected<int, std::string>;
    auto f  = [](int v) -> E { return v; };

    E e(1);
    (void)(e.and_then(f));
    (void)(std::as_const(e).and_then(f));
    (void)(std::move(e).and_then(f));
    (void)(std::move(std::as_const(e)).and_then(f));
}

TEST_CASE("or_else: all 4 ref qualifications compile", "[expected_monadic]") {
    using E = expected<int, std::string>;
    auto f  = [](std::string) -> E { return 0; };

    E e(unexpect, "x");
    (void)(e.or_else(f));
    (void)(std::as_const(e).or_else(f));
    (void)(std::move(e).or_else(f));
    (void)(std::move(std::as_const(e)).or_else(f));
}

TEST_CASE("transform: all 4 ref qualifications compile", "[expected_monadic]") {
    expected<int, int> e(1);
    auto               f = [](int v) { return v; };
    (void)(e.transform(f));
    (void)(std::as_const(e).transform(f));
    (void)(std::move(e).transform(f));
    (void)(std::move(std::as_const(e)).transform(f));
}

TEST_CASE("transform_error: all 4 ref qualifications compile", "[expected_monadic]") {
    expected<int, int> e(unexpect, 1);
    auto               f = [](int v) { return v; };
    (void)(e.transform_error(f));
    (void)(std::as_const(e).transform_error(f));
    (void)(std::move(e).transform_error(f));
    (void)(std::move(std::as_const(e)).transform_error(f));
}

// =============================================================================
// Additional coverage tests: rvalue/const-rvalue monadic overloads
// =============================================================================
using namespace beman::expected::testing;

// --- Monadic: and_then rvalue/const-rvalue on ERROR state (short-circuit) ---
TEST_CASE("and_then rvalue on error short-circuits", "[expected_monadic]") {
    expected<int, std::string> e(unexpect, "err");
    bool                       called = false;
    auto                       r      = std::move(e).and_then([&](int) -> expected<int, std::string> {
        called = true;
        return 0;
    });
    CHECK(!called);
    REQUIRE(!r.has_value());
    CHECK(r.error() == "err");
}

TEST_CASE("and_then const rvalue on error short-circuits", "[expected_monadic]") {
    const expected<int, std::string> e(unexpect, "err");
    bool                             called = false;
    auto                             r      = std::move(e).and_then([&](int) -> expected<int, std::string> {
        called = true;
        return 0;
    });
    CHECK(!called);
    REQUIRE(!r.has_value());
    CHECK(r.error() == "err");
}

// --- Monadic: or_else rvalue/const-rvalue on VALUE state (short-circuit) ---
TEST_CASE("or_else rvalue on value short-circuits", "[expected_monadic]") {
    expected<std::string, int> e("val");
    bool                       called = false;
    auto                       r      = std::move(e).or_else([&](int) -> expected<std::string, int> {
        called = true;
        return "x";
    });
    CHECK(!called);
    REQUIRE(r.has_value());
}

TEST_CASE("or_else const rvalue on value short-circuits", "[expected_monadic]") {
    const expected<int, int> e(42);
    bool                     called = false;
    auto                     r      = std::move(e).or_else([&](int) -> expected<int, int> {
        called = true;
        return 0;
    });
    CHECK(!called);
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

// --- Monadic: transform rvalue/const-rvalue on ERROR state (short-circuit) ---
TEST_CASE("transform rvalue on error short-circuits", "[expected_monadic]") {
    expected<int, std::string> e(unexpect, "err");
    auto                       r = std::move(e).transform([](int v) { return v * 2; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "err");
}

TEST_CASE("transform const rvalue on error short-circuits", "[expected_monadic]") {
    const expected<int, std::string> e(unexpect, "err");
    auto                             r = std::move(e).transform([](int v) { return v * 2; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "err");
}

// --- Monadic: transform_error rvalue/const-rvalue on VALUE state (short-circuit) ---
TEST_CASE("transform_error rvalue on value short-circuits", "[expected_monadic]") {
    expected<std::string, int> e("val");
    auto                       r = std::move(e).transform_error([](int v) { return v * 2; });
    REQUIRE(r.has_value());
}

TEST_CASE("transform_error const rvalue on value short-circuits", "[expected_monadic]") {
    const expected<int, int> e(42);
    auto                     r = std::move(e).transform_error([](int v) { return v * 2; });
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

// --- Monadic: transform const-rvalue on VALUE state (the actual call path) ---
TEST_CASE("transform const rvalue on value calls F", "[expected_monadic]") {
    const expected<int, int> e(5);
    auto                     r = std::move(e).transform([](int v) { return v * 3; });
    REQUIRE(r.has_value());
    CHECK(*r == 15);
}

// --- Monadic: transform_error const-rvalue on ERROR state (the actual call path) ---
TEST_CASE("transform_error const rvalue on error calls F", "[expected_monadic]") {
    const expected<int, int> e(unexpect, 7);
    auto                     r = std::move(e).transform_error([](int v) { return v + 1; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 8);
}

// ---------------------------------------------------------------------------
// traced<T,E> monadic: all 4 overloads × value + error paths
// ---------------------------------------------------------------------------

TEST_CASE("traced<T,E>: and_then lvalue value path", "[expected_monadic][traced]") {
    expected<traced, traced> e(std::in_place, 5);
    auto                     r = e.and_then(
        [](traced& v) -> expected<traced, traced> { return expected<traced, traced>(std::in_place, v.val * 2); });
    REQUIRE(r.has_value());
    CHECK(r->val == 10);
}

TEST_CASE("traced<T,E>: and_then lvalue error path", "[expected_monadic][traced]") {
    expected<traced, traced> e(unexpect, 5);
    auto                     r =
        e.and_then([](traced&) -> expected<traced, traced> { return expected<traced, traced>(std::in_place, 0); });
    REQUIRE(!r.has_value());
    CHECK(r.error().val == 5);
}

TEST_CASE("traced<T,E>: and_then const lvalue error path", "[expected_monadic][traced]") {
    const expected<traced, traced> e(unexpect, 5);
    auto                           r = e.and_then(
        [](const traced&) -> expected<traced, traced> { return expected<traced, traced>(std::in_place, 0); });
    REQUIRE(!r.has_value());
}

TEST_CASE("traced<T,E>: or_else lvalue value path", "[expected_monadic][traced]") {
    expected<traced, traced> e(std::in_place, 5);
    auto r = e.or_else([](traced&) -> expected<traced, traced> { return expected<traced, traced>(unexpect, 0); });
    REQUIRE(r.has_value());
    CHECK(r->val == 5);
}

TEST_CASE("traced<T,E>: or_else lvalue error path", "[expected_monadic][traced]") {
    expected<traced, traced> e(unexpect, 3);
    auto                     r = e.or_else(
        [](traced& v) -> expected<traced, traced> { return expected<traced, traced>(std::in_place, v.val * 10); });
    REQUIRE(r.has_value());
    CHECK(r->val == 30);
}

TEST_CASE("traced<T,E>: or_else const lvalue value path", "[expected_monadic][traced]") {
    const expected<traced, traced> e(std::in_place, 5);
    auto                           r =
        e.or_else([](const traced&) -> expected<traced, traced> { return expected<traced, traced>(unexpect, 0); });
    REQUIRE(r.has_value());
}

TEST_CASE("traced<T,E>: or_else rvalue error path", "[expected_monadic][traced]") {
    expected<traced, traced> e(unexpect, 3);
    auto                     r = std::move(e).or_else(
        [](traced&& v) -> expected<traced, traced> { return expected<traced, traced>(std::in_place, v.val); });
    REQUIRE(r.has_value());
}

TEST_CASE("traced<T,E>: or_else const rvalue value path", "[expected_monadic][traced]") {
    const expected<traced, traced> e(std::in_place, 5);
    auto                           r = std::move(e).or_else(
        [](const traced&) -> expected<traced, traced> { return expected<traced, traced>(unexpect, 0); });
    REQUIRE(r.has_value());
}

TEST_CASE("traced<T,E>: transform lvalue value path", "[expected_monadic][traced]") {
    expected<traced, traced> e(std::in_place, 3);
    auto                     r = e.transform([](traced& v) { return traced(v.val + 1); });
    REQUIRE(r.has_value());
    CHECK(r->val == 4);
}

TEST_CASE("traced<T,E>: transform lvalue error path", "[expected_monadic][traced]") {
    expected<traced, traced> e(unexpect, 3);
    auto                     r = e.transform([](traced&) { return traced(0); });
    REQUIRE(!r.has_value());
}

TEST_CASE("traced<T,E>: transform rvalue value path", "[expected_monadic][traced]") {
    expected<traced, traced> e(std::in_place, 3);
    auto                     r = std::move(e).transform([](traced&& v) { return traced(v.val + 1); });
    REQUIRE(r.has_value());
    CHECK(r->val == 4);
}

TEST_CASE("traced<T,E>: transform const lvalue error path", "[expected_monadic][traced]") {
    const expected<traced, traced> e(unexpect, 3);
    auto                           r = e.transform([](const traced&) { return traced(0); });
    REQUIRE(!r.has_value());
}

TEST_CASE("traced<T,E>: transform const rvalue value path", "[expected_monadic][traced]") {
    const expected<traced, traced> e(std::in_place, 3);
    auto                           r = std::move(e).transform([](const traced& v) { return traced(v.val + 1); });
    REQUIRE(r.has_value());
}

TEST_CASE("traced<T,E>: transform to void lvalue", "[expected_monadic][traced]") {
    expected<traced, traced> e(std::in_place, 1);
    auto                     r = e.transform([](traced&) {});
    REQUIRE(r.has_value());
}

TEST_CASE("traced<T,E>: transform_error lvalue value path", "[expected_monadic][traced]") {
    expected<traced, traced> e(std::in_place, 5);
    auto                     r = e.transform_error([](traced&) { return traced(0); });
    REQUIRE(r.has_value());
    CHECK(r->val == 5);
}

TEST_CASE("traced<T,E>: transform_error lvalue error path", "[expected_monadic][traced]") {
    expected<traced, traced> e(unexpect, 5);
    auto                     r = e.transform_error([](traced& v) { return traced(v.val + 1); });
    REQUIRE(!r.has_value());
    CHECK(r.error().val == 6);
}

TEST_CASE("traced<T,E>: transform_error rvalue value path", "[expected_monadic][traced]") {
    expected<traced, traced> e(std::in_place, 5);
    auto                     r = std::move(e).transform_error([](traced&&) { return traced(0); });
    REQUIRE(r.has_value());
}

TEST_CASE("traced<T,E>: transform_error rvalue error path", "[expected_monadic][traced]") {
    expected<traced, traced> e(unexpect, 5);
    auto                     r = std::move(e).transform_error([](traced&& v) { return traced(v.val + 1); });
    REQUIRE(!r.has_value());
    CHECK(r.error().val == 6);
}

TEST_CASE("traced<T,E>: transform_error const lvalue value path", "[expected_monadic][traced]") {
    const expected<traced, traced> e(std::in_place, 5);
    auto                           r = e.transform_error([](const traced&) { return traced(0); });
    REQUIRE(r.has_value());
}

TEST_CASE("traced<T,E>: transform_error const rvalue value path", "[expected_monadic][traced]") {
    const expected<traced, traced> e(std::in_place, 5);
    auto                           r = std::move(e).transform_error([](const traced&) { return traced(0); });
    REQUIRE(r.has_value());
}

// --- value() throw with non-trivial E ---
TEST_CASE("traced<T,E>: value() lvalue throws", "[expected_monadic][traced]") {
    expected<traced, traced> e(unexpect, 42);
    CHECK_THROWS_AS(e.value(), bad_expected_access<traced>);
}

TEST_CASE("traced<T,E>: value() const rvalue throws", "[expected_monadic][traced]") {
    const expected<traced, traced> e(unexpect, 42);
    CHECK_THROWS_AS(std::move(e).value(), bad_expected_access<traced>);
}
