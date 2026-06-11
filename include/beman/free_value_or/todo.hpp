// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_FREE_VALUE_OR_TODO_HPP
#define BEMAN_FREE_VALUE_OR_TODO_HPP

#include <beman/free_value_or/config.hpp>

#if BEMAN_FREE_VALUE_OR_USE_MODULES() && !defined(BEMAN_FREE_VALUE_OR_INCLUDED_FROM_INTERFACE_UNIT)

import beman.free_value_or;

#else

namespace beman::free_value_or {

// TODO

} // namespace beman::free_value_or

#endif // BEMAN_FREE_VALUE_OR_USE_MODULES() &&
       // !defined(BEMAN_FREE_VALUE_OR_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_FREE_VALUE_OR_TODO_HPP
