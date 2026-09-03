# tests/beman/optional/CMakeLists.txt                               -*-cmake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

target_sources(
    beman.optional.test
    PRIVATE
        optional.test.cpp
        optional_constexpr.test.cpp
        optional_monadic.test.cpp
        optional_range_support.test.cpp
        optional_ref.test.cpp
        optional_ref_monadic.test.cpp
        value_or_construct.test.cpp
)

target_sources(
    beman.optional.test
    PRIVATE
        FILE_SET beman_optional_test_headers
            TYPE HEADERS
            FILES test_types.hpp test_utilities.hpp
)

target_link_libraries(
    beman.optional.test
    PRIVATE beman.optional GTest::gtest GTest::gtest_main
)

function(add_optional_fail_test name source reason)
    add_library(${name} OBJECT)
    target_sources(${name} PRIVATE ${source})
    target_link_libraries(${name} PRIVATE beman.optional)
    set_target_properties(
        ${name}
        PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true
    )
    add_test(
        NAME ${name}
        COMMAND
            ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target ${name}
            --config $<CONFIGURATION>
    )
    set_tests_properties(${name} PROPERTIES PASS_REGULAR_EXPRESSION "${reason}")
endfunction()

add_library(constructor_fails test_constructor_fail.test.cpp)

target_link_libraries(constructor_fails PRIVATE beman.optional)

set_target_properties(
    constructor_fails
    PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true
)

add_test(
    NAME constructor_fails
    COMMAND
        ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target
        constructor_fails --config $<CONFIGURATION>
)

set_tests_properties(constructor_fails PROPERTIES WILL_FAIL true)
# Alternatively -- check for a particular regex to pass
# set_tests_properties(constructor_fails PROPERTIES PASS_REGULAR_EXPRESSION
# "Assert true")

add_optional_fail_test(
    optional_value_or_construct_bad_args_fail
    value_or_construct_bad_args_fail.test.cpp
    "Must be able to construct remove_cv_t<T> from args"
)

add_optional_fail_test(
    optional_value_or_construct_bad_init_list_fail
    value_or_construct_bad_init_list_fail.test.cpp
    "Must be able to construct remove_cv_t<T> from the initializer_list and args"
)

add_optional_fail_test(
    optional_value_or_else_bad_conversion_fail
    value_or_else_bad_conversion_fail.test.cpp
    "Must be able to convert the result of f\\(\\) to remove_cv_t<T>"
)

add_optional_fail_test(
    optional_value_or_else_non_invocable_fail
    value_or_else_non_invocable_fail.test.cpp
    "invoke_result|no type named.*type|not invocable"
)
