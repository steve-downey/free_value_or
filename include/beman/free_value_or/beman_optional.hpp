// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_BEMAN_FREE_VALUE_OR_BEMAN_OPTIONAL
#define INCLUDED_BEMAN_FREE_VALUE_OR_BEMAN_OPTIONAL

// Follow the umbrella's configured module/header mode. It must precede the
// vendored header so module imports occur before textual includes.
#include <beman/free_value_or/free_value_or.hpp>
#include <beman/optional/optional.hpp>

namespace smd::free_value_or {
template <class T>
inline constexpr bool enable_borrowed_nullable<beman::optional::optional<T&>> = true;
} // namespace smd::free_value_or

#endif
