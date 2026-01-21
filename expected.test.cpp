// beman/expected/expected.test.cpp                                 -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/expected.hpp>
#include <beman/expected/expected.hpp> // ensure idempotent header

#include <gtest/gtest.h>

#include <algorithm>
#include <functional>

namespace expt = beman::expected;

TEST(ExpectedTest, breathing) { EXPECT_EQ(true, true); }
