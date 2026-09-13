// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Must NOT compile: converting the dereferenced value to the selected common
// reference would materialize a temporary.

#include <beman/free_value_or/value_or.hpp>

#include <type_traits>

struct target {};

struct source {
    operator target() const { return {}; }
};

struct nullable_source {
    source value;

    explicit operator bool() const { return true; }
    source& operator*() const { return const_cast<source&>(value); }
};

template <>
struct std::common_reference<source&, target&> {
    using type = const target&;
};

void check() {
    nullable_source nullable;
    target          fallback;
    (void)smd::free_value_or::reference_or(nullable, fallback);
}
