// tests/beman/expected/bad_expected_access.test.cpp                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/bad_expected_access.hpp>
#include <beman/expected/bad_expected_access.hpp> // test 2nd include OK

#include <catch2/catch_test_macros.hpp>

#include <exception>
#include <string>
#include <utility>

namespace expt = beman::expected;

TEST_CASE("bad_expected_access: breathing", "[BadExpectedAccessTest]") {}

TEST_CASE("bad_expected_access: construct from int", "[BadExpectedAccessTest]") {
    expt::bad_expected_access<int> e(42);
    CHECK(e.error() == 42);
}

TEST_CASE("bad_expected_access: what() returns message", "[BadExpectedAccessTest]") {
    expt::bad_expected_access<int> e(1);
    CHECK(e.what() != nullptr);
    CHECK(std::string_view(e.what()) == "bad expected access");
}

TEST_CASE("bad_expected_access: inherits from std::exception", "[BadExpectedAccessTest]") {
    expt::bad_expected_access<int> e(1);
    std::exception&                ex = e;
    CHECK(std::string_view(ex.what()) == "bad expected access");
}

TEST_CASE("bad_expected_access: error() lvalue ref mutable", "[BadExpectedAccessTest]") {
    expt::bad_expected_access<int> e(42);
    e.error() = 99;
    CHECK(e.error() == 99);
}

TEST_CASE("bad_expected_access: error() const lvalue ref", "[BadExpectedAccessTest]") {
    const expt::bad_expected_access<int> e(42);
    CHECK(e.error() == 42);
}

TEST_CASE("bad_expected_access: error() rvalue ref", "[BadExpectedAccessTest]") {
    expt::bad_expected_access<int> e(42);
    int                            v = std::move(e).error();
    CHECK(v == 42);
}

TEST_CASE("bad_expected_access: error() const rvalue ref", "[BadExpectedAccessTest]") {
    const expt::bad_expected_access<int> e(42);
    int                                  v = std::move(e).error();
    CHECK(v == 42);
}

TEST_CASE("bad_expected_access: string move semantics", "[BadExpectedAccessTest]") {
    expt::bad_expected_access<std::string> e(std::string("hello"));
    std::string                            s = std::move(e).error();
    CHECK(s == "hello");
}

TEST_CASE("bad_expected_access: catchable as std::exception", "[BadExpectedAccessTest]") {
    try {
        throw expt::bad_expected_access<int>(7);
    } catch (const std::exception& ex) {
        CHECK(std::string_view(ex.what()) == "bad expected access");
    }
}

TEST_CASE("bad_expected_access: catchable as bad_expected_access<void>", "[BadExpectedAccessTest]") {
    try {
        throw expt::bad_expected_access<int>(7);
    } catch (const expt::bad_expected_access<void>& ex) {
        CHECK(std::string_view(ex.what()) == "bad expected access");
    }
}
