export module beman.expected;

import std;

#define BEMAN_EXPECTED_INCLUDED_FROM_INTERFACE_UNIT
export {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#include <beman/expected/expected.hpp>
#pragma clang diagnostic pop
}
