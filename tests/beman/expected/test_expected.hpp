// tests/beman/expected/test_expected.hpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Test-suite parameterization shim.
//
// Selects which `expected` implementation the behavioral test files exercise,
// so the same test sources can be compiled and run against more than one
// implementation. The selected implementation's namespace is aliased as
// `test_ns`; behavioral tests refer to `test_ns::expected`,
// `test_ns::unexpected`, `test_ns::unexpect`, `test_ns::bad_expected_access`,
// etc. instead of hard-coding `beman::expected`.
//
//   (default)                 -> beman::expected   (aliased as test_ns)
//   -DBEMAN_EXPECTED_TEST_STD -> ::std             (requires <expected>, C++23)
//
// Only the portable *behavioral* tests are parameterized. Tests that encode
// beman-specific behavior (triviality of SMFs, SFINAE-friendliness of
// constraints, hardened preconditions, reference specializations) are NOT
// parameterized; see the per-file "Beman-only" markers and the build wiring in
// tests/beman/expected/CMakeLists.txt.

#ifndef BEMAN_EXPECTED_TESTS_TEST_EXPECTED_HPP
#define BEMAN_EXPECTED_TESTS_TEST_EXPECTED_HPP

#if defined(BEMAN_EXPECTED_TEST_STD)

    #include <expected>

namespace test_ns = ::std;

#else

    #include <beman/expected/expected.hpp>
    #include <beman/expected/expected.hpp> // ensure idempotent header

namespace test_ns = ::beman::expected;

#endif

#endif // BEMAN_EXPECTED_TESTS_TEST_EXPECTED_HPP
