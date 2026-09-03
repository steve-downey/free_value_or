// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/free_value_or/free_value_or.hpp>

#include <iostream>
#include <optional>
#include <string>

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
