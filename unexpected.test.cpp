// beman/expected/unexpected.test.cpp                                 -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/unexpected.hpp>
#include <beman/expected/unexpected.hpp> // ensure idempotent header

#include <gtest/gtest.h>

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace expt = beman::expected;

// Construction from value
TEST(UnexpectedTest, ConstructFromInt) {
    expt::unexpected<int> u(42);
    EXPECT_EQ(u.error(), 42);
}

TEST(UnexpectedTest, ConstructFromString) {
    expt::unexpected<std::string> u(std::string("error"));
    EXPECT_EQ(u.error(), "error");
}

TEST(UnexpectedTest, ConstructFromStringLiteral) {
    expt::unexpected<std::string> u("literal");
    EXPECT_EQ(u.error(), "literal");
}

// In-place construction
TEST(UnexpectedTest, InPlaceConstructString) {
    expt::unexpected<std::string> u(std::in_place, "hello");
    EXPECT_EQ(u.error(), "hello");
}

TEST(UnexpectedTest, InPlaceConstructVector) {
    expt::unexpected<std::vector<int>> u(std::in_place, std::initializer_list<int>{1, 2, 3});
    EXPECT_EQ(u.error().size(), 3u);
    EXPECT_EQ(u.error()[0], 1);
    EXPECT_EQ(u.error()[2], 3);
}

TEST(UnexpectedTest, InPlaceConstructStringMultiArg) {
    expt::unexpected<std::string> u(std::in_place, 3u, 'x');
    EXPECT_EQ(u.error(), "xxx");
}

// Copy and move construction
TEST(UnexpectedTest, CopyConstruct) {
    expt::unexpected<int> a(10);
    expt::unexpected<int> b(a);
    EXPECT_EQ(b.error(), 10);
}

TEST(UnexpectedTest, MoveConstruct) {
    expt::unexpected<std::string> a("moved");
    expt::unexpected<std::string> b(std::move(a));
    EXPECT_EQ(b.error(), "moved");
}

// error() observers in all 4 ref-qualified overloads
TEST(UnexpectedTest, ErrorConstLRef) {
    const expt::unexpected<int> u(7);
    const int&                  r = u.error();
    EXPECT_EQ(r, 7);
}

TEST(UnexpectedTest, ErrorLRef) {
    expt::unexpected<int> u(7);
    int&                  r = u.error();
    r                       = 99;
    EXPECT_EQ(u.error(), 99);
}

TEST(UnexpectedTest, ErrorRRef) {
    expt::unexpected<std::string> u("rval");
    std::string                   s = std::move(u).error();
    EXPECT_EQ(s, "rval");
}

TEST(UnexpectedTest, ErrorConstRRef) {
    const expt::unexpected<std::string> u("crval");
    std::string                         s = std::move(u).error();
    EXPECT_EQ(s, "crval");
}

// swap member
TEST(UnexpectedTest, SwapMember) {
    expt::unexpected<int> a(1);
    expt::unexpected<int> b(2);
    a.swap(b);
    EXPECT_EQ(a.error(), 2);
    EXPECT_EQ(b.error(), 1);
}

// swap friend (ADL)
TEST(UnexpectedTest, SwapFriend) {
    expt::unexpected<std::string> a("hello");
    expt::unexpected<std::string> b("world");
    swap(a, b);
    EXPECT_EQ(a.error(), "world");
    EXPECT_EQ(b.error(), "hello");
}

// operator== with same E type
TEST(UnexpectedTest, EqualitySameType) {
    expt::unexpected<int> a(5);
    expt::unexpected<int> b(5);
    expt::unexpected<int> c(6);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
}

// operator== with different E types
TEST(UnexpectedTest, EqualityDifferentTypes) {
    expt::unexpected<int>  a(42);
    expt::unexpected<long> b(42L);
    EXPECT_TRUE(a == b);
}

// CTAD deduction
TEST(UnexpectedTest, CTADInt) {
    expt::unexpected u(42);
    static_assert(std::is_same_v<decltype(u), expt::unexpected<int>>);
    EXPECT_EQ(u.error(), 42);
}

TEST(UnexpectedTest, CTADString) {
    std::string      s("deduced");
    expt::unexpected u(s);
    static_assert(std::is_same_v<decltype(u), expt::unexpected<std::string>>);
    EXPECT_EQ(u.error(), "deduced");
}

// The converting template constructor excludes unexpected; copy/move ctors handle it
TEST(UnexpectedTest, CopyMoveConstructible) {
    static_assert(std::is_copy_constructible_v<expt::unexpected<int>>);
    static_assert(std::is_move_constructible_v<expt::unexpected<std::string>>);
}

// unexpect_t tag
TEST(UnexpectedTest, UnexpectTag) { static_assert(std::is_same_v<decltype(expt::unexpect), const expt::unexpect_t>); }

// constexpr basic usage
TEST(UnexpectedTest, Constexpr) {
    constexpr expt::unexpected<int> u(123);
    static_assert(u.error() == 123);
}
