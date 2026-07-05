// tests/beman/expected/expected_unexpected_value_type_fail.cpp          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// NEGATIVE: T must not be a specialization of unexpected
// EXPECT: "T must not be a specialization of unexpected"

#include <beman/expected/expected.hpp>

using E = beman::expected::expected<beman::expected::unexpected<int>, int>;
E e;
