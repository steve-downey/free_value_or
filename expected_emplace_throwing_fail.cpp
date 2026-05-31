// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE COMPILE TEST: emplace requires nothrow_constructible_v<T, Args...>
// Calling emplace with a type whose constructor might throw is ill-formed.
#include <beman/expected/expected.hpp>

struct ThrowingCtor {
    explicit ThrowingCtor(int) noexcept(false) {}
};

void test() {
    beman::expected::expected<ThrowingCtor, int> e(std::in_place, 0);
    e.emplace(1); // must not compile: not nothrow-constructible
}
