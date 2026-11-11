// tests/beman/expected/type_name.test.cpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Beman-only: a compiler with reflection gets the reflection implementation of
// testing/type_name.hpp.
//
// The selection needs both the compiler's __cpp_impl_reflection and the
// library's __cpp_lib_reflection, and falling back is silent: every other test
// still passes using the fallback. So whenever the compiler has reflection,
// check that it was used. A build that forces an implementation with
// BEMAN_EXPECTED_TESTING_TYPE_NAME_USE_REFLECTION has made its own choice, and
// is not second-guessed.

#if defined(BEMAN_EXPECTED_TESTING_TYPE_NAME_USE_REFLECTION)
    #define BEMAN_EXPECTED_TESTING_TYPE_NAME_FORCED
#endif

#include <catch2/catch_test_macros.hpp>

#include <beman/expected/testing/type_name.hpp>

#if defined(__cpp_impl_reflection) && !defined(BEMAN_EXPECTED_TESTING_TYPE_NAME_FORCED)
TEST_CASE("type_name: a compiler with reflection gets the reflection implementation", "[type_name]") {
    CHECK(BEMAN_EXPECTED_TESTING_TYPE_NAME_USE_REFLECTION == 1);
}
#endif
