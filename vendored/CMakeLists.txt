# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Minimal wrapper: expose the vendored beman::optional header-only library.
# Tests that need optional<T&> link this target; it adds no link overhead.

add_library(beman.optional INTERFACE)
add_library(beman::optional ALIAS beman.optional)
target_include_directories(
    beman.optional
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/beman.optional26/include>
)
