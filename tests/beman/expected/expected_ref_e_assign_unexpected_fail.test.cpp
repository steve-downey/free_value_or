// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<T, E&> cannot be assigned from unexpected<G>.
// No operator=(unexpected<G>...) exists: binding E& to temporary unexpected
// storage would create a dangling reference. Use copy-assign from another
// expected<T, E&> to rebind the error reference safely.
// EXPECT: "no matching function"
#include <beman/expected/expected.hpp>
using namespace beman::expected;
void test() {
    expected<int, int&> e(42);
    e = unexpected(7); // must not compile
}
