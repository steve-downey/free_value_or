// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/free_value_or/free_value_or.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>
#include <type_traits>

namespace fvo = smd::free_value_or;

TEST_CASE("public umbrella header exposes the free_value_or API", "[public_header]") {
    std::optional<int> engaged{42};
    std::optional<int> empty;

    CHECK(fvo::value_or(engaged, 0) == 42);
    CHECK(fvo::value_or(empty, 0) == 0);

    bool invoked = false;
    CHECK(fvo::or_invoke(empty, [&] {
        invoked = true;
        return 7;
    }) == 7);
    CHECK(invoked);

    std::optional<std::string> text;
    CHECK(fvo::or_construct(text, 3, 'x') == "xxx");

    int  value    = 11;
    int  fallback = 22;
    int* pointer  = &value;
    int& result   = fvo::reference_or(pointer, fallback);

    static_assert(std::is_same_v<decltype(result), int&>);
    CHECK(&result == &value);
    CHECK(result == 11);
}
