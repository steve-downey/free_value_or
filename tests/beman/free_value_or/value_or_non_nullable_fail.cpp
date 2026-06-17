// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Must NOT compile: int does not satisfy the nullable concept, so value_or must be rejected.

#include <beman/free_value_or/value_or.hpp>

void check() { (void)smd::free_value_or::value_or(5, 3); }
