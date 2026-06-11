# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(Catch2 3 REQUIRED)

# ---------------------------------------------------------------------------
# Helper: add a positive (passing) test executable at C++23
#
# Usage:
#   fvo_add_test(<name> <source>)
#
# Adds an executable, links beman::free_value_or + Catch2::Catch2WithMain,
# sets cxx_std_23, includes tests/beman/free_value_or/ for test_types.hpp,
# and registers with catch_discover_tests.
# ---------------------------------------------------------------------------
function(fvo_add_test name source)
    add_executable(${name})
    target_sources(${name} PRIVATE ${source})
    target_link_libraries(${name} PRIVATE beman::free_value_or Catch2::Catch2WithMain)
    target_compile_features(${name} PRIVATE cxx_std_23)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    catch_discover_tests(${name})
endfunction()

# ---------------------------------------------------------------------------
# Helper: add a negative-compile (WILL_FAIL) test
#
# Usage:
#   fvo_add_compile_fail_test(<test-name> <source> <regex>)
#
# Creates an OBJECT library excluded from the default build, then registers a
# ctest test that invokes cmake --build on that target.  The test passes when
# the build fails AND the compiler output matches <regex>.
# ---------------------------------------------------------------------------
function(fvo_add_compile_fail_test test_name source regex)
    set(tgt_name "beman.free_value_or.tests.${test_name}.fail")

    add_library(${tgt_name} OBJECT)
    target_sources(${tgt_name} PRIVATE ${source})
    target_link_libraries(${tgt_name} PRIVATE beman::free_value_or)
    target_compile_features(${tgt_name} PRIVATE cxx_std_23)
    target_include_directories(${tgt_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(
        ${tgt_name}
        PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true
    )

    add_test(
        NAME ${test_name}
        COMMAND ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target ${tgt_name}
                --config $<CONFIGURATION>
    )
    set_tests_properties(
        ${test_name}
        PROPERTIES PASS_REGULAR_EXPRESSION "${regex}"
    )
endfunction()

# ---------------------------------------------------------------------------
# C++26 variant option (auto-detect or manual override)
# optional<T&> requires __cpp_lib_optional >= 202406 AND C++26 dialect.
# ---------------------------------------------------------------------------
include(CheckCXXSourceCompiles)
include(CMakeDependentOption)
set(CMAKE_REQUIRED_FLAGS "-std=c++26")
check_cxx_source_compiles(
    "#include <optional>
     int main() {
         int x = 0;
         std::optional<int&> r{x};
         return r ? 0 : 1;
     }"
    BEMAN_FREE_VALUE_OR_TESTS_CXX26_OPTIONAL_REF
)
unset(CMAKE_REQUIRED_FLAGS)

cmake_dependent_option(
    BEMAN_FREE_VALUE_OR_TESTS_CXX26
    "Build C++26 test variant (requires optional<T&> support)"
    ON
    "BEMAN_FREE_VALUE_OR_TESTS_CXX26_OPTIONAL_REF"
    OFF
)

# ---------------------------------------------------------------------------
# Keep the existing placeholder test building (Step 02 will replace it)
# ---------------------------------------------------------------------------
add_executable(beman.free_value_or.tests.value_or)
target_sources(beman.free_value_or.tests.value_or PRIVATE value_or.test.cpp)
target_link_libraries(
    beman.free_value_or.tests.value_or
    PRIVATE beman::free_value_or Catch2::Catch2WithMain
)
target_compile_features(beman.free_value_or.tests.value_or PRIVATE cxx_std_23)
if(BEMAN_EXEMPLAR_USE_MODULES)
    set_target_properties(
        beman.free_value_or.tests.value_or
        PROPERTIES CXX_MODULE_STD ON
    )
endif()

include(Catch)
catch_discover_tests(beman.free_value_or.tests.value_or)

# ---------------------------------------------------------------------------
# Smoke test: proves the toolchain compiles the library at C++23
# ---------------------------------------------------------------------------
fvo_add_test(beman.free_value_or.tests.smoke smoke.test.cpp)

# ---------------------------------------------------------------------------
# Negative-compile self-check: int is not nullable
# The test passes when the build FAILS with the expected diagnostic.
# ---------------------------------------------------------------------------
fvo_add_compile_fail_test(
    fvo.fail.not_nullable
    fail_not_nullable.cpp
    "no matching function for call to"
)
