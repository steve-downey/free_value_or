// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Must NOT compile: reference_or with a prvalue fallback would bind R=const int& to a
// temporary, tripping static_assert(!reference_constructs_from_temporary_v<R, U>).

#include <beman/free_value_or/value_or.hpp>
#include <optional>

void check() {
    std::optional<int> o;
    (void)smd::free_value_or::reference_or(o, int{42});
}
