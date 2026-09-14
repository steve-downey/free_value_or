// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_BEMAN_FREE_VALUE_OR_BEMAN_OPTIONAL
#define INCLUDED_BEMAN_FREE_VALUE_OR_BEMAN_OPTIONAL

// Keep the integration textual so this header can add a specialization even
// when the umbrella header is configured to import the module.
#include <beman/free_value_or/value_or.hpp>
#include <beman/optional/optional.hpp>

namespace smd::free_value_or {
template <class T>
inline constexpr bool enable_borrowed_nullable<beman::optional::optional<T&>> = true;
} // namespace smd::free_value_or

#endif
