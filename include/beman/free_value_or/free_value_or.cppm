module;

// Macros only, no declarations: makes the library feature-test macros
// (__cpp_lib_reference_from_temporary, used by the polyfill in value_or.hpp)
// visible in the purview below. `import std;` exports declarations, not macros,
// so without this the polyfill would silently fall back to the compiler builtin
// in module builds while using the standard trait everywhere else.
#include <version>

#if defined(__has_include) && __has_include(<expected>)
    #define BEMAN_FREE_VALUE_OR_DETAIL_HAS_STD_EXPECTED_HEADER 1
#endif

// The reference implementation integrates the vendored reference-nullable
// types when they are available. Place their declarations in the global
// module fragment so value_or.hpp can export the corresponding opt-ins.
#if defined(__has_include) && __has_include(<beman/optional/optional.hpp>)
    #include <beman/optional/optional.hpp>
    #define BEMAN_FREE_VALUE_OR_DETAIL_HAS_BEMAN_OPTIONAL_HEADER 1
#endif
#if defined(__has_include) && __has_include(<beman/expected/expected.hpp>)
    #include <beman/expected/expected.hpp>
    #define BEMAN_FREE_VALUE_OR_DETAIL_HAS_BEMAN_EXPECTED_HEADER 1
#endif

export module beman.free_value_or;

import std;

#define BEMAN_FREE_VALUE_OR_INCLUDED_FROM_INTERFACE_UNIT
export {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#include <beman/free_value_or/free_value_or.hpp>
#pragma clang diagnostic pop
}
