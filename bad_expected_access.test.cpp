// tests/beman/expected/bad_expected_access.test.cpp                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/bad_expected_access.hpp>
#include <beman/expected/bad_expected_access.hpp> // test 2nd include OK

#include <gtest/gtest.h>

namespace expt = beman::expected;

TEST(BadExpectedAccessTest, breathing) { SUCCEED(); }
