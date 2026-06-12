// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Must NOT compile: reference_or with an rvalue string fallback would bind
// R=const string& to a temporary, tripping static_assert(!reference_constructs_from_temporary_v<R, U>).

#include <beman/free_value_or/value_or.hpp>
#include <optional>
#include <string>

void check() {
    std::optional<std::string> o;
    (void)smd::free_value_or::reference_or(o, std::string{"x"});
}
