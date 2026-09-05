// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Standard headers come before the umbrella header on purpose. In a modules
// build the umbrella resolves to `import beman.free_value_or;`, and GCC cannot
// merge textually-included standard library declarations that follow an import
// of a module that already imported them. Including them first avoids that.
#include <iostream>
#include <optional>
#include <string>

#include <beman/free_value_or/free_value_or.hpp>

int main() {
    namespace fvo = smd::free_value_or;

    std::optional<std::string> configured_name;
    const auto                 name = fvo::or_construct<std::string>(configured_name, "default");

    std::optional<int> configured_count = 3;
    const auto         count            = fvo::value_or(configured_count, 0);

    int  live_value = 42;
    int  fallback   = 0;
    int* maybe_live = &live_value;
    int& selected   = fvo::reference_or(maybe_live, fallback);

    std::cout << name << ": " << count << ", selected=" << selected << '\n';
}
