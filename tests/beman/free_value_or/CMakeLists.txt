# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(Catch2 3 REQUIRED)

# ---------------------------------------------------------------------------
# Helper: add a positive (passing) test executable at C++23
#
# Links beman::free_value_or + beman::optional (vendored, header-only) +
# Catch2::Catch2WithMain, sets cxx_std_23, injects FVO_HAS_OPTIONAL_REF=1,
# includes tests/beman/free_value_or/ for test_types.hpp, and registers with
# catch_discover_tests.
#
# Usage:
#   fvo_add_test(<name> <source>)
# ---------------------------------------------------------------------------
function(fvo_add_test name source)
    add_executable(${name})
    target_sources(${name} PRIVATE ${source})
    target_link_libraries(
        ${name} PRIVATE
        beman::free_value_or
        beman::optional
        Catch2::Catch2WithMain
    )
    target_compile_features(${name} PRIVATE cxx_std_23)
    target_compile_definitions(${name} PRIVATE FVO_HAS_OPTIONAL_REF=1)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    catch_discover_tests(${name})
endfunction()

# ---------------------------------------------------------------------------
# Helper: add a negative-compile (WILL_FAIL) test
#
# Creates an OBJECT library excluded from the default build, then registers a
# ctest test that invokes cmake --build on that target.  The test passes when
# the build fails AND the compiler output matches <regex>.
#
# Usage:
#   fvo_add_compile_fail_test(<test-name> <source> <regex>)
# ---------------------------------------------------------------------------
function(fvo_add_compile_fail_test test_name source regex)
    set(tgt_name "beman.free_value_or.tests.${test_name}.fail")

    add_library(${tgt_name} OBJECT)
    target_sources(${tgt_name} PRIVATE ${source})
    target_link_libraries(${tgt_name} PRIVATE beman::free_value_or beman::optional)
    target_compile_features(${tgt_name} PRIVATE cxx_std_23)
    target_compile_definitions(${tgt_name} PRIVATE FVO_HAS_OPTIONAL_REF=1)
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
# Step 02: value_or runtime + return-type + value categories
# ---------------------------------------------------------------------------
include(Catch)
fvo_add_test(beman.free_value_or.tests.value_or value_or.test.cpp)

# ---------------------------------------------------------------------------
# Smoke test: proves the toolchain compiles the library and vendored optional
# at C++23, including optional<T&> via beman::optional.
# ---------------------------------------------------------------------------
fvo_add_test(beman.free_value_or.tests.smoke smoke.test.cpp)

# ---------------------------------------------------------------------------
# Step 01: nullable concept static_assert coverage
# ---------------------------------------------------------------------------
fvo_add_test(beman.free_value_or.tests.concept concept.test.cpp)

# ---------------------------------------------------------------------------
# Step 03: reference_or reference semantics
# ---------------------------------------------------------------------------
fvo_add_test(beman.free_value_or.tests.reference_or reference_or.test.cpp)

# ---------------------------------------------------------------------------
# Negative-compile self-check: int is not nullable
# The test passes when the build FAILS with the expected diagnostic.
# ---------------------------------------------------------------------------
fvo_add_compile_fail_test(
    fvo.fail.not_nullable
    fail_not_nullable.cpp
    "no matching function for call to"
)

# ---------------------------------------------------------------------------
# Step 04A: dangling-reference rejection in reference_or
# Each TU must fail with "static assertion failed" from
# reference_constructs_from_temporary_v checks in reference_or.
# ---------------------------------------------------------------------------
fvo_add_compile_fail_test(
    fvo.fail.ref_or_temp_from_prvalue
    ref_or_temp_from_prvalue_fail.cpp
    "static assertion failed"
)

fvo_add_compile_fail_test(
    fvo.fail.ref_or_rvalue_string
    ref_or_rvalue_string_fail.cpp
    "static assertion failed"
)

# ---------------------------------------------------------------------------
# Step 04B: non-nullable first argument rejected by the nullable concept
# Each TU must fail with "no matching function for call to" — one per function.
# ---------------------------------------------------------------------------
fvo_add_compile_fail_test(
    fvo.fail.value_or_non_nullable
    value_or_non_nullable_fail.cpp
    "no matching function for call to"
)

fvo_add_compile_fail_test(
    fvo.fail.reference_or_non_nullable
    reference_or_non_nullable_fail.cpp
    "no matching function for call to"
)

fvo_add_compile_fail_test(
    fvo.fail.or_invoke_non_nullable
    or_invoke_non_nullable_fail.cpp
    "no matching function for call to"
)

# ---------------------------------------------------------------------------
# Step 05: or_invoke results + laziness
# ---------------------------------------------------------------------------
fvo_add_test(beman.free_value_or.tests.or_invoke or_invoke.test.cpp)

# ---------------------------------------------------------------------------
# Step 06: constexpr constant-evaluation tests
# The real proof is the static_asserts in the TU; the TEST_CASE gives ctest
# a runnable entry.
# ---------------------------------------------------------------------------
fvo_add_test(beman.free_value_or.tests.constexpr constexpr.test.cpp)

# ---------------------------------------------------------------------------
# or_construct Step 00: smoke test — both overloads compile and run
# ---------------------------------------------------------------------------
fvo_add_test(beman.free_value_or.tests.or_construct or_construct.test.cpp)

# ---------------------------------------------------------------------------
# or_construct Step 01: behavior — return type, engaged/disengaged, value
# categories, inward conversion, explicit Ret
# ---------------------------------------------------------------------------
fvo_add_test(beman.free_value_or.tests.or_construct_behavior or_construct_behavior.test.cpp)

# ---------------------------------------------------------------------------
# or_construct Step 02: zero-arg/default + multi-arg emplace + initializer_list
# ---------------------------------------------------------------------------
fvo_add_test(beman.free_value_or.tests.or_construct_construct or_construct_construct.test.cpp)

# ---------------------------------------------------------------------------
# or_construct Step 03: laziness — fallback constructed only when disengaged
# ---------------------------------------------------------------------------
fvo_add_test(beman.free_value_or.tests.or_construct_laziness or_construct_laziness.test.cpp)

# ---------------------------------------------------------------------------
# or_construct Step 04: negative-compile — non-nullable arg rejected (both overloads)
# and payload-not-convertible-to-Ret (body hard error).
# ---------------------------------------------------------------------------
fvo_add_compile_fail_test(
    fvo.fail.or_construct_non_nullable
    or_construct_non_nullable_fail.cpp
    "no matching function for call to"
)

fvo_add_compile_fail_test(
    fvo.fail.or_construct_initlist_non_nullable
    or_construct_initlist_non_nullable_fail.cpp
    "no matching function for call to"
)

fvo_add_compile_fail_test(
    fvo.fail.or_construct_payload_not_convertible
    or_construct_payload_not_convertible_fail.cpp
    "invalid .*static_cast"
)

# ---------------------------------------------------------------------------
# Step 07: optional<T&> tests using beman::optional (vendored).
# FVO_HAS_OPTIONAL_REF=1 is injected by fvo_add_test, so the feature-gated
# sections are active.  std::optional<T&> (P2988) is NOT available on this
# toolchain (libstdc++ 15.2, __cpp_lib_optional = 202110); beman::optional
# provides the reference-optional support instead.
# ---------------------------------------------------------------------------
fvo_add_test(beman.free_value_or.tests.optional_ref optional_ref.test.cpp)
