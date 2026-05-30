// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_EXPECTED_EXPECTED_HPP
#define BEMAN_EXPECTED_EXPECTED_HPP

#include <beman/expected/config.hpp>

#if BEMAN_EXPECTED_USE_MODULES() && !defined(BEMAN_EXPECTED_INCLUDED_FROM_INTERFACE_UNIT)

import beman.expected;

#else

    #include <beman/expected/todo.hpp>

#endif // BEMAN_EXPECTED_USE_MODULES() &&
       // !defined(BEMAN_EXPECTED_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_EXPECTED_EXPECTED_HPP
