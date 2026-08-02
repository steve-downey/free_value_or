// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: or_else on expected<T&, E&> must return expected with same value_type (T&)
// EXPECT: "F must return expected with the same value_type"
#include <beman/expected/expected.hpp>
namespace {
using E = beman::expected::expected<int&, int&>;
using G = beman::expected::expected<double&, int&>; // value_type double& != int&
} // namespace
void test() {
    double d   = 0;
    int    err = 0;
    E      e   = E(beman::expected::unexpect, err);
    // F returns G (value_type == double&), but e has value_type == int&
    e.or_else([&](int&) -> G { return d; });
}
