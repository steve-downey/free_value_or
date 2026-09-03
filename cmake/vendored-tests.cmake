# cmake/vendored-tests.cmake                                        -*-cmake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Build the vendored beman::optional and beman::expected test suites as part of
# *this* project's ctest run. Unlike a typical vendored dependency (whose tests
# are excluded), we run them here: this repo edits those libraries in place
# (P3413R0 members) and their own suites are how we prove the edits are correct
# before pushing back upstream.
#
# The tests/beman/<lib>/CMakeLists.txt leaf scripts are vendored unmodified; this
# file supplies the context each upstream root CMakeLists.txt would provide.
# Must be included after enable_testing().

# --- beman::optional suite (GoogleTest) ------------------------------------
# Mirrors bemanproject/optional root CMakeLists: create the test executable and
# its header set (BASE_DIRS tests, so <beman/optional/test_types.hpp> resolves),
# then let the leaf populate sources. GTest is redirected to FetchContent by the
# lockfile-driven dependency provider (infra/cmake/use-fetch-content.cmake).
find_package(GTest QUIET)
if(GTest_FOUND)
    add_executable(beman.optional.test)
    target_sources(
        beman.optional.test
        PRIVATE
            FILE_SET beman_optional_test_headers TYPE HEADERS BASE_DIRS tests
    )
    add_subdirectory(tests/beman/optional)
    include(GoogleTest)
    gtest_discover_tests(beman.optional.test)
else()
    message(
        WARNING
        "beman::optional vendored tests skipped: no GTest provider. "
        "Add GTest to lockfile.json or install it to enable them."
    )
endif()

# --- beman::expected suite (Catch2) ----------------------------------------
# The expected leaf is self-contained: it does find_package(Catch2 3 REQUIRED)
# (redirected to FetchContent) and defines its own executables and negative
# compile tests. It references ${PROJECT_SOURCE_DIR}/tests as an include dir; the
# harness headers it needs (test_expected.hpp, testing/types.hpp) are co-located
# under tests/beman/expected and resolve relative to each test source.
add_subdirectory(tests/beman/expected)
