// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/config.hpp>
#include <catch2/catch_test_macros.hpp>
#include <beman/expected/todo.hpp>

TEST_CASE("todo", "[TodoTest]") {
    const bool todo = true;
    CHECK(todo);
}
