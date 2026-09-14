// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_BEMAN_FREE_VALUE_OR_BEMAN_EXPECTED
#define INCLUDED_BEMAN_FREE_VALUE_OR_BEMAN_EXPECTED

#include <beman/expected/expected.hpp>
// Keep the integration textual so this header can add a specialization even
// when the umbrella header is configured to import the module.
#include <beman/free_value_or/value_or.hpp>

#if defined(BEMAN_EXPECTED_HAS_REFERENCES) && BEMAN_EXPECTED_HAS_REFERENCES
namespace smd::free_value_or {
template <class T, class E>
inline constexpr bool enable_borrowed_nullable<beman::expected::expected<T&, E>> = true;
} // namespace smd::free_value_or
#endif

#endif
