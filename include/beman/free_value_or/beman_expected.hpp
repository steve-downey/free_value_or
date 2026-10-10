// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef BEMAN_FREE_VALUE_OR_BEMAN_EXPECTED_HPP
#define BEMAN_FREE_VALUE_OR_BEMAN_EXPECTED_HPP

#include <beman/free_value_or/config.hpp>

// Follow the umbrella's configured module/header mode unless a caller has
// deliberately selected the implementation header already. In either case,
// establish the free_value_or declarations before adding the specialization.
#ifdef BEMAN_FREE_VALUE_OR_VALUE_OR_HPP
    #include <beman/expected/expected.hpp>
#else
    #include <beman/free_value_or/free_value_or.hpp>
    #if BEMAN_FREE_VALUE_OR_USE_MODULES()
        #define BEMAN_EXPECTED_INCLUDED_FROM_INTERFACE_UNIT
        #include <beman/expected/expected.hpp>
        #undef BEMAN_EXPECTED_INCLUDED_FROM_INTERFACE_UNIT
    #else
        #include <beman/expected/expected.hpp>
    #endif
#endif

#if defined(BEMAN_EXPECTED_HAS_REFERENCES) && BEMAN_EXPECTED_HAS_REFERENCES
namespace beman::free_value_or {
template <class T, class E>
inline constexpr bool enable_borrowed_nullable<beman::expected::expected<T&, E>> = true;
} // namespace beman::free_value_or
#endif

#endif // BEMAN_FREE_VALUE_OR_BEMAN_EXPECTED_HPP
