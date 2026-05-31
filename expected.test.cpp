// beman/expected/expected.test.cpp                                 -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/expected.hpp>
#include <beman/expected/expected.hpp> // ensure idempotent header

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <functional>

namespace expt = beman::expected;

TEST_CASE("breathing", "[ExpectedTest]") { CHECK(true == true); }
