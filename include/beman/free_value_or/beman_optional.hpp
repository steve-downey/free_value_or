// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef BEMAN_FREE_VALUE_OR_BEMAN_OPTIONAL_HPP
#define BEMAN_FREE_VALUE_OR_BEMAN_OPTIONAL_HPP

#include <beman/free_value_or/config.hpp>

// Follow the umbrella's configured module/header mode unless a caller has
// deliberately selected the implementation header already. In either case,
// establish the free_value_or declarations before adding the specialization.
#ifdef BEMAN_FREE_VALUE_OR_VALUE_OR_HPP
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

namespace beman::free_value_or {
template <class T>
inline constexpr bool enable_borrowed_nullable<beman::optional::optional<T&>> = true;
} // namespace beman::free_value_or

#endif // BEMAN_FREE_VALUE_OR_BEMAN_OPTIONAL_HPP
