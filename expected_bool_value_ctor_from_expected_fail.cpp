// tests/beman/expected/expected_bool_value_ctor_from_expected_fail.cpp -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Negative compile test (WILL_FAIL): expected<bool, E> cannot be constructed
// from expected<U, G> via the value ctor when the converting ctor is not viable.
//
// Constraint 23.6: if T is cv bool, remove_cvref_t<U> must not be a
// specialization of expected. This blocks the value ctor from selecting
// expected<int,int> -> bool via operator bool when the error types do not match
// (making the converting ctor also unavailable).
//
// Converting ctor: requires is_constructible_v<bool, int> && is_constructible_v<std::string, int>
//   -> is_constructible_v<std::string, int> is false, so converting ctor is NOT viable.
// Value ctor: constraint 23.6 blocks U=expected<int,int> when T=bool.
// -> no viable constructor: must fail to compile.

#include <beman/expected/expected.hpp>
#include <string>

using namespace beman::expected;

int main() {
    expected<int, int>      src(42);
    expected<bool, std::string> dst(src); // constraint 23.6 blocks value ctor; converting ctor not viable
}
