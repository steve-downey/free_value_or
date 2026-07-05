// tests/beman/expected/header_idempotence.test.cpp                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Beman-only: verify each public header is idempotent (safe to include more
// than once in a translation unit). These checks previously lived as duplicate
// includes at the top of the behavioral test files; they were consolidated
// here when those files were parameterized over the implementation under test
// (see test_expected.hpp). Nothing to run at runtime — success is compilation.

#include <beman/expected/unexpected.hpp>
#include <beman/expected/unexpected.hpp>

#include <beman/expected/bad_expected_access.hpp>
#include <beman/expected/bad_expected_access.hpp>

#include <beman/expected/expected.hpp>
#include <beman/expected/expected.hpp>
