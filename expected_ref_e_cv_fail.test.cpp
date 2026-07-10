// tests/beman/expected/expected_ref_e_cv_fail.cpp                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<T&, E> where E is cv-qualified is ill-formed
// EXPECT: "E must not be cv-qualified"
#include <beman/expected/expected.hpp>
void test() {
    int                                        x = 0;
    beman::expected::expected<int&, const int> e(x); // must not compile
}
