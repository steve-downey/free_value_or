// tests/beman/optional/value_or_construct.test.cpp                   -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/optional/optional.hpp>

#include <gtest/gtest.h>

#include <initializer_list>
#include <string>
#include <type_traits>
#include <vector>

namespace {

// Counts only the argument-taking constructors (not copy/move), so we can prove
// that value_or_construct does not construct the alternate when *this is engaged.
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
// optional<T>::value_or_construct
// ---------------------------------------------------------------------------

TEST(OptionalValueOrConstruct, EngagedReturnsValue) {
    beman::optional::optional<int> o = 7;
    EXPECT_EQ(o.value_or_construct(42), 7);
}

TEST(OptionalValueOrConstruct, EmptyConstructsFromArgs) {
    beman::optional::optional<int> o;
    EXPECT_EQ(o.value_or_construct(42), 42);
}

TEST(OptionalValueOrConstruct, LazyWhenEngaged) {
    ctor_counter::constructions = 0;
    beman::optional::optional<ctor_counter> engaged{beman::optional::in_place, 1};
    const int                               base = ctor_counter::constructions;

    auto r = engaged.value_or_construct(99);
    EXPECT_EQ(r.value, 1);
    EXPECT_EQ(ctor_counter::constructions, base); // alternate NOT constructed
}

TEST(OptionalValueOrConstruct, ConstructsWhenEmpty) {
    ctor_counter::constructions = 0;
    beman::optional::optional<ctor_counter> empty;

    auto r = empty.value_or_construct(99);
    EXPECT_EQ(r.value, 99);
    EXPECT_EQ(ctor_counter::constructions, 1);
}

TEST(OptionalValueOrConstruct, InitializerListOverload) {
    beman::optional::optional<std::vector<int>> empty;
    auto                                        v = empty.value_or_construct({1, 2, 3});
    EXPECT_EQ(v.size(), 3u);

    beman::optional::optional<std::vector<int>> engaged{beman::optional::in_place, {9, 9}};
    auto                                        v2 = engaged.value_or_construct({1, 2, 3});
    EXPECT_EQ(v2.size(), 2u);

    // initializer_list + trailing args, only constructed in the empty case
    ctor_counter::constructions = 0;
    beman::optional::optional<ctor_counter> e2;
    auto                                    r = e2.value_or_construct({1, 2}, 10);
    EXPECT_EQ(r.value, 12);
    EXPECT_EQ(ctor_counter::constructions, 1);
}

TEST(OptionalValueOrConstruct, RvalueMovesOutOrConstructs) {
    beman::optional::optional<std::string> engaged{beman::optional::in_place, "hello"};
    std::string                            s = std::move(engaged).value_or_construct(5, 'x');
    EXPECT_EQ(s, "hello");

    beman::optional::optional<std::string> empty;
    std::string                            s2 = std::move(empty).value_or_construct(3, 'x');
    EXPECT_EQ(s2, "xxx");
}

// ---------------------------------------------------------------------------
// optional<T>::value_or_else
// ---------------------------------------------------------------------------

TEST(OptionalValueOrElse, LazyWhenEngaged) {
    beman::optional::optional<int> engaged = 5;
    bool                           called  = false;
    int                            r        = engaged.value_or_else([&] {
        called = true;
        return 42;
    });
    EXPECT_EQ(r, 5);
    EXPECT_FALSE(called);
}

TEST(OptionalValueOrElse, InvokesWhenEmpty) {
    beman::optional::optional<int> empty;
    bool                           called = false;
    int                            r      = empty.value_or_else([&] {
        called = true;
        return 42;
    });
    EXPECT_EQ(r, 42);
    EXPECT_TRUE(called);
}

TEST(OptionalValueOrElse, RvalueMovesOut) {
    beman::optional::optional<std::string> engaged{beman::optional::in_place, "keep"};
    std::string                            s = std::move(engaged).value_or_else([] { return std::string("alt"); });
    EXPECT_EQ(s, "keep");

    beman::optional::optional<std::string> empty;
    std::string                            s2 = std::move(empty).value_or_else([] { return std::string("alt"); });
    EXPECT_EQ(s2, "alt");
}

// ---------------------------------------------------------------------------
// Return types (value, cv-stripped) and constexpr
// ---------------------------------------------------------------------------

static_assert(std::is_same_v<decltype(std::declval<beman::optional::optional<int>&>().value_or_construct(0)), int>);
static_assert(
    std::is_same_v<decltype(std::declval<beman::optional::optional<const int>&>().value_or_construct(0)), int>);
static_assert(std::is_same_v<decltype(std::declval<beman::optional::optional<int>&>().value_or_else(
                                 std::declval<int (*)()>())),
                             int>);

constexpr int constexpr_probe() {
    beman::optional::optional<int> empty;
    int                            a = empty.value_or_construct(11);
    int                            b = beman::optional::optional<int>(5).value_or_else([] { return 0; });
    return a + b;
}
static_assert(constexpr_probe() == 16);

// ---------------------------------------------------------------------------
// optional<T&> specialization
// ---------------------------------------------------------------------------

TEST(OptionalRefValueOrConstruct, EngagedAndEmpty) {
    int                             x = 3;
    beman::optional::optional<int&> engaged{x};
    beman::optional::optional<int&> empty;

    EXPECT_EQ(engaged.value_or_construct(42), 3);
    EXPECT_EQ(empty.value_or_construct(42), 42);

    static_assert(std::is_same_v<decltype(engaged.value_or_construct(0)), int>);
}

TEST(OptionalRefValueOrConstruct, InitializerListOverload) {
    using vector = std::vector<int>;

    vector                           existing{9, 9};
    beman::optional::optional<vector&> engaged{existing};
    beman::optional::optional<vector&> empty;

    auto from_engaged = engaged.value_or_construct({1, 2, 3});
    auto from_empty   = empty.value_or_construct({1, 2, 3});

    EXPECT_EQ(from_engaged, (vector{9, 9}));
    EXPECT_EQ(from_empty, (vector{1, 2, 3}));

    static_assert(
        std::is_same_v<decltype(engaged.value_or_construct(std::initializer_list<int>{})), vector>);
}

TEST(OptionalRefValueOrElse, EngagedAndEmpty) {
    int                             x      = 3;
    beman::optional::optional<int&> engaged{x};
    beman::optional::optional<int&> empty;
    bool                            called = false;

    EXPECT_EQ(engaged.value_or_else([&] {
        called = true;
        return 42;
    }),
              3);
    EXPECT_FALSE(called);
    EXPECT_EQ(empty.value_or_else([] { return 42; }), 42);
}
