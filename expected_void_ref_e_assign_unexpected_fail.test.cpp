// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<void, E&> cannot be assigned from unexpected<G>.
// No operator=(unexpected<G>...) exists: binding E& to temporary unexpected
// storage would create a dangling reference.
// EXPECT: "no matching function"
#include <beman/expected/expected.hpp>
using namespace beman::expected;
void test() {
    expected<void, int&> e;
    e = unexpected(7); // must not compile
}
