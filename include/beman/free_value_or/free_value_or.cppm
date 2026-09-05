module;

// Macros only, no declarations: makes the library feature-test macros
// (__cpp_lib_reference_from_temporary, used by the polyfill in value_or.hpp)
// visible in the purview below. `import std;` exports declarations, not macros,
// so without this the polyfill would silently fall back to the compiler builtin
// in module builds while using the standard trait everywhere else.
#include <version>

export module beman.free_value_or;

import std;

#define BEMAN_FREE_VALUE_OR_INCLUDED_FROM_INTERFACE_UNIT
export {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#include <beman/free_value_or/free_value_or.hpp>
#pragma clang diagnostic pop
}
