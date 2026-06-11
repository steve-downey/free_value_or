// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Negative-compile self-check: int is not nullable; this must fail to compile.
// Used by the fvo_add_compile_fail_test harness self-check in CMakeLists.txt.

#include <beman/free_value_or/value_or.hpp>
namespace fvo = smd::free_value_or;  // rename point: smd:: -> beman::

void check() {
    int x = 42;
    // Must fail: int does not satisfy nullable (no operator bool, no operator*).
    (void)fvo::value_or(x, 0);
}
