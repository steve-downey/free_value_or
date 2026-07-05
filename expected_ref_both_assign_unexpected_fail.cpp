// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<T&, E&> cannot be assigned from unexpected<G>.
// No operator=(unexpected<G>...) exists: binding E& to temporary unexpected
// storage would create a dangling reference. Copy-assign from another
// expected<T&, E&>(unexpect, new_err) to rebind the error reference safely.
// EXPECT: "no matching function"
#include <beman/expected/expected.hpp>
using namespace beman::expected;
void test() {
    int                  x = 1;
    expected<int&, int&> e(x);
    e = unexpected(7); // must not compile
}
