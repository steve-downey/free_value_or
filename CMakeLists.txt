# tests/beman/expected/CMakeLists.txt                               -*-cmake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

add_executable(beman.expected.tests.expected)
target_sources(
    beman.expected.tests.expected
    PRIVATE
        bad_expected_access.test.cpp
        unexpected.test.cpp
        expected.test.cpp
        expected_void.test.cpp
        expected_monadic.test.cpp
        expected_void_monadic.test.cpp
        expected_constraints.test.cpp
        expected_trivial.test.cpp
        expected_monadic_constraints.test.cpp
        todo.test.cpp
)
target_link_libraries(
    beman.expected.tests.expected
    PRIVATE beman::expected Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(beman.expected.tests.expected)

# =============================================================================
# Negative compile tests (WILL_FAIL = compile failure is the expected outcome)
# =============================================================================

# Helper macro to reduce repetition
macro(add_fail_test name source)
    add_library(beman.expected.tests.${name} OBJECT)
    target_sources(beman.expected.tests.${name} PRIVATE ${source})
    target_link_libraries(beman.expected.tests.${name} PRIVATE beman::expected)
    set_target_properties(
        beman.expected.tests.${name}
        PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true
    )
    add_test(
        NAME ${name}
        COMMAND
            ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target
            beman.expected.tests.${name} --config $<CONFIGURATION>
    )
    set_tests_properties(${name} PROPERTIES WILL_FAIL TRUE)
endmacro()

# Step 1 — unexpected<E> ill-formed instantiations [expected.un.general]
add_fail_test(unexpected_array_fail unexpected_array_fail.cpp)
add_fail_test(unexpected_cvref_fail unexpected_cvref_fail.cpp)
add_fail_test(unexpected_self_fail  unexpected_self_fail.cpp)

# Step 3 — expected<T,E> ill-formed T/E [expected.object.general]
add_fail_test(expected_t_ref_fail   expected_t_ref_fail.cpp)
add_fail_test(expected_e_ref_fail   expected_e_ref_fail.cpp)
add_fail_test(expected_t_array_fail expected_t_array_fail.cpp)

# Step 3 — expected<T,E> emplace Mandates: nothrow_constructible_v<T,Args...>
add_fail_test(expected_emplace_throwing_fail expected_emplace_throwing_fail.cpp)

# Step 4 — expected<void,E> ill-formed E [expected.void.general]
add_fail_test(expected_void_ref_fail   expected_void_ref_fail.cpp)
add_fail_test(expected_void_array_fail expected_void_array_fail.cpp)

# Step 5 — monadic Mandates [expected.object.monadic]
add_fail_test(and_then_wrong_error_type_fail    and_then_wrong_error_type_fail.cpp)
add_fail_test(and_then_not_expected_fail        and_then_not_expected_fail.cpp)
add_fail_test(or_else_wrong_value_type_fail     or_else_wrong_value_type_fail.cpp)
add_fail_test(transform_error_ref_result_fail   transform_error_ref_result_fail.cpp)

# Step 6 — void monadic Mandates [expected.void.monadic]
add_fail_test(void_and_then_wrong_error_type_fail void_and_then_wrong_error_type_fail.cpp)
add_fail_test(void_or_else_wrong_value_type_fail  void_or_else_wrong_value_type_fail.cpp)

# Fix 1 — constraint 23.6: value ctor blocked for T=bool, U=expected specialization
add_fail_test(expected_bool_value_ctor_from_expected_fail expected_bool_value_ctor_from_expected_fail.cpp)
