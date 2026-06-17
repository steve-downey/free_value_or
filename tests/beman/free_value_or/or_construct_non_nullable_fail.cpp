// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Must NOT compile: int does not satisfy the nullable concept, so or_construct (pack overload) must be rejected.

#include <beman/free_value_or/value_or.hpp>

void check() { (void)smd::free_value_or::or_construct(5, 3); }
