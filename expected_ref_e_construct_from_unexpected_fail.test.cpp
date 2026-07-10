// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<T, E&> cannot be constructed from unexpected<G>.
// A value-typed unexpected<G> stores G by value; binding E& to it would create a
// dangling reference when the unexpected object is destroyed. Construction from a
// reference-typed unexpected<G&> (which holds an external object) is allowed.
// EXPECT: "no matching function"
#include <beman/expected/expected.hpp>
using namespace beman::expected;
void test() {
    expected<int, int&> e = unexpected(7); // must not compile
}
