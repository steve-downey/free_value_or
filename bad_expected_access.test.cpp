// tests/beman/expected/bad_expected_access.test.cpp                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/bad_expected_access.hpp>
#include <beman/expected/bad_expected_access.hpp> // test 2nd include OK

#include <catch2/catch_test_macros.hpp>

namespace expt = beman::expected;

TEST_CASE("breathing", "[BadExpectedAccessTest]") {}
