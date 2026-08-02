# cmake/vendored-libs.cmake                                         -*-cmake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Wire the co-located, subtree-vendored proposal dependencies into this build:
#   beman::optional  (bemanproject/optional,  include/beman/optional)
#   beman::expected  (bemanproject/expected,  include/beman/expected)
#
# These are part of the proposal *surface*, not test-only infrastructure: their
# headers carry the P3413R0 member functions (value_or_construct / value_or_else)
# that pair with our free functions, and are edited in place then pushed back
# upstream (see scripts/vendor-*.sh).
#
# Each library's upstream leaf CMakeLists (include/beman/<lib>/CMakeLists.txt and
# tests/beman/<lib>/CMakeLists.txt) is vendored UNMODIFIED so edits round-trip
# cleanly. This file, plus cmake/vendored-tests.cmake, supplies the surrounding
# context those leaf scripts assume — i.e. it stands in for each upstream root
# CMakeLists.txt.

# --- beman::optional (header-only) -----------------------------------------
# Mirrors bemanproject/optional root CMakeLists: declare the header set with its
# BASE_DIRS here, then let the leaf populate FILES.
add_library(beman.optional INTERFACE)
add_library(beman::optional ALIAS beman.optional)
target_sources(
    beman.optional
    PUBLIC FILE_SET beman_optional_headers TYPE HEADERS BASE_DIRS include
)
add_subdirectory(include/beman/optional)

# --- beman::expected (header-only) -----------------------------------------
# Mirrors bemanproject/expected root CMakeLists, including the generated config
# header that expected/config.hpp includes. We consume expected as headers only
# (never as a module) here, independent of how free_value_or itself is built.
option(BEMAN_EXPECTED_USE_MODULES "Provide beman.expected as a C++ module" OFF)
configure_file(
    "${PROJECT_SOURCE_DIR}/include/beman/expected/config_generated.hpp.in"
    "${PROJECT_BINARY_DIR}/include/beman/expected/config_generated.hpp"
    @ONLY
)
add_library(beman.expected INTERFACE)
add_library(beman::expected ALIAS beman.expected)
target_sources(
    beman.expected
    PUBLIC
        FILE_SET HEADERS
            BASE_DIRS
                "${CMAKE_CURRENT_SOURCE_DIR}/include"
                "${CMAKE_CURRENT_BINARY_DIR}/include"
)
add_subdirectory(include/beman/expected)
