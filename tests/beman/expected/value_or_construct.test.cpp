// tests/beman/expected/value_or_construct.test.cpp                   -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Beman-only: value_or_construct / value_or_else (P3413R0) are not in
// std::expected, so this is not parameterized against ::std.

#include <beman/expected/expected.hpp>
#include <beman/expected/expected.hpp> // ensure idempotent header

#include <catch2/catch_test_macros.hpp>

#include <initializer_list>
#include <string>
#include <type_traits>
#include <vector>

using namespace beman::expected;

namespace {

// Counts only argument-taking constructors (not copy/move), to prove the
// alternate is not constructed when *this holds a value.
struct ctor_counter {
    static int constructions;
    int        value;

    explicit ctor_counter(int v) : value(v) { ++constructions; }
    ctor_counter(std::initializer_list<int> il, int v) : value(static_cast<int>(il.size()) + v) {
        ++constructions;
    }
    ctor_counter(const ctor_counter&)            = default;
    ctor_counter(ctor_counter&&)                 = default;
    ctor_counter& operator=(const ctor_counter&) = default;
    ctor_counter& operator=(ctor_counter&&)      = default;
};
int ctor_counter::constructions = 0;

} // namespace

// ---------------------------------------------------------------------------
// expected<T, E>
// ---------------------------------------------------------------------------

TEST_CASE("expected value_or_construct: value state returns the value", "[expected][value_or_construct]") {
    expected<int, std::string> e = 7;
    CHECK(e.value_or_construct(42) == 7);
    CHECK(e.value_or_else([] { return 42; }) == 7);
}

TEST_CASE("expected value_or_construct: error state produces the alternate", "[expected][value_or_construct]") {
    expected<int, std::string> e = unexpected(std::string("boom"));
    CHECK(e.value_or_construct(42) == 42);
    CHECK(e.value_or_else([] { return 42; }) == 42);
}

TEST_CASE("expected value_or_construct is lazy", "[expected][value_or_construct]") {
    ctor_counter::constructions = 0;
    expected<ctor_counter, std::string> good{std::in_place, 1};
    const int                           base = ctor_counter::constructions;

    auto r = good.value_or_construct(99);
    CHECK(r.value == 1);
    CHECK(ctor_counter::constructions == base); // alternate NOT constructed

    expected<ctor_counter, std::string> bad = unexpected(std::string("e"));
    auto                                r2  = bad.value_or_construct(99);
    CHECK(r2.value == 99);
    CHECK(ctor_counter::constructions == base + 1);
}

TEST_CASE("expected value_or_construct initializer_list overload", "[expected][value_or_construct]") {
    expected<std::vector<int>, std::string> bad = unexpected(std::string("e"));
    auto                                    v   = bad.value_or_construct({1, 2, 3});
    CHECK(v.size() == 3u);

    expected<std::vector<int>, std::string> good{std::in_place, {9, 9}};
    auto                                    v2 = good.value_or_construct({1, 2, 3});
    CHECK(v2.size() == 2u);

    ctor_counter::constructions             = 0;
    expected<ctor_counter, std::string> bad2 = unexpected(std::string("e"));
    auto                                r    = bad2.value_or_construct({1, 2}, 10);
    CHECK(r.value == 12);
    CHECK(ctor_counter::constructions == 1);
}

TEST_CASE("expected value_or_else is lazy", "[expected][value_or_else]") {
    expected<int, std::string> good = 5;
    bool                       called = false;
    CHECK(good.value_or_else([&] {
        called = true;
        return 42;
    }) == 5);
    CHECK_FALSE(called);

    expected<int, std::string> bad = unexpected(std::string("e"));
    CHECK(bad.value_or_else([&] {
        called = true;
        return 42;
    }) == 42);
    CHECK(called);
}

TEST_CASE("expected rvalue value_or_construct/else moves out or produces alternate", "[expected][value_or_construct]") {
    expected<std::string, int> good{std::in_place, "keep"};
    CHECK(std::move(good).value_or_construct(3, 'x') == "keep");

    expected<std::string, int> bad = unexpected(1);
    CHECK(std::move(bad).value_or_construct(3, 'x') == "xxx");

    expected<std::string, int> good2{std::in_place, "keep"};
    CHECK(std::move(good2).value_or_else([] { return std::string("alt"); }) == "keep");

    expected<std::string, int> bad2 = unexpected(1);
    CHECK(std::move(bad2).value_or_else([] { return std::string("alt"); }) == "alt");
}

static_assert(std::is_same_v<decltype(std::declval<expected<int, std::string>&>().value_or_construct(0)), int>);
static_assert(
    std::is_same_v<decltype(std::declval<expected<int, std::string>&>().value_or_else(std::declval<int (*)()>())), int>);

constexpr int constexpr_probe() {
    expected<int, int> bad = unexpected(1);
    int                a   = bad.value_or_construct(11);
    int                b   = expected<int, int>(5).value_or_else([] { return 0; });
    return a + b;
}
static_assert(constexpr_probe() == 16);

// ---------------------------------------------------------------------------
// expected<T&, E>
// ---------------------------------------------------------------------------

TEST_CASE("expected<T&,E> value_or_construct / value_or_else", "[expected][ref][value_or_construct]") {
    int                         x   = 3;
    expected<int&, std::string> good = x;
    expected<int&, std::string> bad  = unexpected(std::string("e"));

    CHECK(good.value_or_construct(42) == 3);
    CHECK(bad.value_or_construct(42) == 42);

    bool called = false;
    CHECK(good.value_or_else([&] {
        called = true;
        return 42;
    }) == 3);
    CHECK_FALSE(called);
    CHECK(bad.value_or_else([] { return 42; }) == 42);

    static_assert(std::is_same_v<decltype(good.value_or_construct(0)), int>);
}

TEST_CASE("expected<T&,E> value_or_construct initializer_list overload",
          "[expected][ref][value_or_construct]") {
    using vector = std::vector<int>;

    vector                         existing{9, 9};
    expected<vector&, std::string> good = existing;
    expected<vector&, std::string> bad  = unexpected(std::string("e"));

    auto from_value = good.value_or_construct({1, 2, 3});
    auto from_error = bad.value_or_construct({1, 2, 3});

    CHECK(from_value == vector{9, 9});
    CHECK(from_error == vector{1, 2, 3});

    static_assert(std::is_same_v<decltype(good.value_or_construct(std::initializer_list<int>{})), vector>);
}
