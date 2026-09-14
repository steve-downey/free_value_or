// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_BEMAN_FREE_VALUE_OR_BEMAN_OPTIONAL
#define INCLUDED_BEMAN_FREE_VALUE_OR_BEMAN_OPTIONAL

#include <beman/free_value_or/config.hpp>

// Follow the umbrella's configured module/header mode unless a caller has
// deliberately selected the implementation header already. In either case,
// establish the free_value_or declarations before adding the specialization.
#ifdef INCLUDED_BEMAN_FREE_VALUE_OR_VALUE_OR
    #include <beman/optional/optional.hpp>
#else
    #include <beman/free_value_or/free_value_or.hpp>
    #if BEMAN_FREE_VALUE_OR_USE_MODULES()
        #define BEMAN_OPTIONAL_INCLUDED_FROM_INTERFACE_UNIT
        #include <beman/optional/optional.hpp>
        #undef BEMAN_OPTIONAL_INCLUDED_FROM_INTERFACE_UNIT
    #else
        #include <beman/optional/optional.hpp>
    #endif
#endif

namespace smd::free_value_or {
template <class T>
inline constexpr bool enable_borrowed_nullable<beman::optional::optional<T&>> = true;
} // namespace smd::free_value_or

#endif
