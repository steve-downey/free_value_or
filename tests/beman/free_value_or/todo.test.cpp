// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/free_value_or/config.hpp>
#include <catch2/catch_all.hpp>
#include <beman/free_value_or/todo.hpp>

TEST_CASE("todo", "[free_value_or::todo]") {
    const bool todo = true;
    CHECK(todo);
}
