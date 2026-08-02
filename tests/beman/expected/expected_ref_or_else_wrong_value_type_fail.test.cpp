// tests/beman/expected/expected_ref_or_else_wrong_value_type_fail.cpp -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: or_else on expected<T&,E> must return expected with same value_type
// EXPECT: "F must return expected with the same value_type"
#include <beman/expected/expected.hpp>
namespace {
using E = beman::expected::expected<int&, int>;
using G = beman::expected::expected<double&, int>;
} // namespace
void test() {
    double d = 0;
    E      e = beman::expected::unexpected(0);
    // F returns G (value_type == double&), but e has value_type == int&
    e.or_else([&](int) -> G { return d; });
}
