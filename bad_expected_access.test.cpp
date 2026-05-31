// tests/beman/expected/bad_expected_access.test.cpp                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/bad_expected_access.hpp>
#include <beman/expected/bad_expected_access.hpp> // test 2nd include OK

#include <gtest/gtest.h>

#include <exception>
#include <string>
#include <utility>

namespace expt = beman::expected;

TEST(BadExpectedAccessTest, breathing) { SUCCEED(); }

TEST(BadExpectedAccessTest, construct_int) {
    expt::bad_expected_access<int> e(42);
    EXPECT_EQ(e.error(), 42);
}

TEST(BadExpectedAccessTest, what_returns_string) {
    expt::bad_expected_access<int> e(1);
    EXPECT_NE(e.what(), nullptr);
    EXPECT_STREQ(e.what(), "bad expected access");
}

TEST(BadExpectedAccessTest, inherits_from_std_exception) {
    expt::bad_expected_access<int> e(1);
    std::exception&                ex = e;
    EXPECT_STREQ(ex.what(), "bad expected access");
}

TEST(BadExpectedAccessTest, error_lvalue_ref_mutable) {
    expt::bad_expected_access<int> e(42);
    e.error() = 99;
    EXPECT_EQ(e.error(), 99);
}

TEST(BadExpectedAccessTest, error_const_lvalue_ref) {
    const expt::bad_expected_access<int> e(42);
    EXPECT_EQ(e.error(), 42);
}

TEST(BadExpectedAccessTest, error_rvalue_ref) {
    expt::bad_expected_access<int> e(42);
    int                            v = std::move(e).error();
    EXPECT_EQ(v, 42);
}

TEST(BadExpectedAccessTest, error_const_rvalue_ref) {
    const expt::bad_expected_access<int> e(42);
    int                                  v = std::move(e).error();
    EXPECT_EQ(v, 42);
}

TEST(BadExpectedAccessTest, string_move_semantics) {
    expt::bad_expected_access<std::string> e(std::string("hello"));
    std::string                            s = std::move(e).error();
    EXPECT_EQ(s, "hello");
}

TEST(BadExpectedAccessTest, catchable_as_std_exception) {
    try {
        throw expt::bad_expected_access<int>(7);
    } catch (const std::exception& ex) {
        EXPECT_STREQ(ex.what(), "bad expected access");
    }
}

TEST(BadExpectedAccessTest, catchable_as_bad_expected_access_void) {
    try {
        throw expt::bad_expected_access<int>(7);
    } catch (const expt::bad_expected_access<void>& ex) {
        EXPECT_STREQ(ex.what(), "bad expected access");
    }
}
