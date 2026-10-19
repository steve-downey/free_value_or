# tests/beman/expected/CMakeLists.txt                               -*-cmake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(Catch2 3 REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/extras")

# Detect libc++ (explicit -stdlib=libc++, or AppleClang's implicit default) so
# the std::expected parity target below can be skipped on it. See the
# "Known CI failures" section of docs/std-parity.md: several libc++ versions
# have their own std::expected/std::unexpected bugs (a cross-specialization
# friend-access error, and a Catch2-decomposition-triggered self-referential
# constraint check) that are toolchain defects, not beman::expected behavioral
# differences, and are not worth chasing here.
include(CheckCXXSymbolExists)
check_cxx_symbol_exists(_LIBCPP_VERSION "version" BEMAN_EXPECTED_USING_LIBCXX)

# Portable behavioral tests: run against every implementation under test.
# Parameterized over the implementation via test_expected.hpp (see that header).
set(beman_expected_behavioral_tests
    bad_expected_access.test.cpp
    unexpected.test.cpp
    expected.test.cpp
    expected_void.test.cpp
    expected_monadic.test.cpp
    expected_void_monadic.test.cpp
)

# Beman-only tests: encode behavior not guaranteed by other implementations
# (SFINAE-friendly constraints, triviality of special members, header
# idempotence, beman utilities, reference specializations not yet
# standardized). These are not parameterized.
set(beman_expected_beman_only_tests
    expected_constraints.test.cpp
    expected_trivial.test.cpp
    expected_monadic_constraints.test.cpp
    expected_ref.test.cpp
    expected_ref_constraints.test.cpp
    expected_ref_e.test.cpp
    expected_ref_both.test.cpp
    expected_void_ref_e.test.cpp
    expected_review_corrections.test.cpp
    expected_smf_regressions.test.cpp
    header_idempotence.test.cpp
    type_name.test.cpp
)

include(Catch)

# --- beman::expected (default) ---------------------------------------------
add_executable(beman.expected.tests.expected)
target_sources(
    beman.expected.tests.expected
    PRIVATE
        ${beman_expected_behavioral_tests}
        ${beman_expected_beman_only_tests}
)
target_include_directories(
    beman.expected.tests.expected
    PRIVATE ${PROJECT_SOURCE_DIR}/tests
)
target_link_libraries(
    beman.expected.tests.expected
    PRIVATE beman::expected Catch2::Catch2WithMain
)
catch_discover_tests(beman.expected.tests.expected DISCOVERY_MODE PRE_TEST)

if(BEMAN_EXPECTED_USE_MODULES)
    add_library(beman.expected.tests.module_parity OBJECT)
    target_sources(
        beman.expected.tests.module_parity
        PRIVATE header_idempotence.test.cpp
    )
    target_compile_definitions(
        beman.expected.tests.module_parity
        PRIVATE BEMAN_EXPECTED_TEST_MODULE
    )
    target_link_libraries(
        beman.expected.tests.module_parity
        PRIVATE beman::expected
    )
endif()

# --- std::expected parity gate ---------------------------------------------
# Compile the behavioral tests against std::expected (requires C++23). No
# behavioral difference from beman::expected is expected except those that
# involve reference specializations, which are not yet parameterized. Any
# beman-specific assertion is guarded with #ifndef BEMAN_EXPECTED_TEST_STD.
# Skipped on libc++ — see docs/std-parity.md.
if(NOT BEMAN_EXPECTED_USING_LIBCXX)
    add_executable(beman.expected.tests.expected.std)
    target_sources(
        beman.expected.tests.expected.std
        PRIVATE ${beman_expected_behavioral_tests}
    )
    target_compile_definitions(
        beman.expected.tests.expected.std
        PRIVATE BEMAN_EXPECTED_TEST_STD
    )
    target_compile_features(
        beman.expected.tests.expected.std
        PRIVATE cxx_std_23
    )
    target_include_directories(
        beman.expected.tests.expected.std
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/tests
    )
    target_link_libraries(
        beman.expected.tests.expected.std
        PRIVATE Catch2::Catch2WithMain
    )
    catch_discover_tests(
        beman.expected.tests.expected.std
        TEST_PREFIX "std."
        PROPERTIES LABELS std
        DISCOVERY_MODE PRE_TEST
    )
endif()

# --- beman::expected vs std::expected trait-equivalence gate ----------------
# The exposition-only held member is unexpected<E>, but every observable
# special-member property is keyed on E. This target asserts (both headers in
# one TU) that the resulting traits match std::expected across a battery of
# adversarial error types -- i.e. the unexpected<E> member is behaviourally
# inert against a T/E rendering. Requires C++23; skipped on libc++ for the same
# reason as the std parity gate above (see docs/std-parity.md).
if(NOT BEMAN_EXPECTED_USING_LIBCXX)
    add_executable(beman.expected.tests.equivalence)
    target_sources(
        beman.expected.tests.equivalence
        PRIVATE expected_std_equivalence.test.cpp
    )
    target_compile_features(beman.expected.tests.equivalence PRIVATE cxx_std_23)
    target_include_directories(
        beman.expected.tests.equivalence
        PRIVATE ${PROJECT_SOURCE_DIR}/tests
    )
    target_link_libraries(
        beman.expected.tests.equivalence
        PRIVATE beman::expected Catch2::Catch2WithMain
    )
    catch_discover_tests(
        beman.expected.tests.equivalence
        DISCOVERY_MODE PRE_TEST
    )
endif()

# =============================================================================
# Negative compile tests: compile failure with a specific diagnostic is the
# expected outcome. See cmake/beman-xfail-compile.cmake.
# =============================================================================

include(${PROJECT_SOURCE_DIR}/cmake/beman-xfail-compile.cmake)

# Step 1 — unexpected<E> ill-formed instantiations [expected.un.general]
beman_add_xfail_compile_test(
    unexpected_array_fail
    SOURCE unexpected_array_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "E must not be an array type"
)
beman_add_xfail_compile_test(
    unexpected_cvref_fail
    SOURCE unexpected_cvref_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "E must not be cv-qualified"
)
beman_add_xfail_compile_test(
    unexpected_self_fail
    SOURCE unexpected_self_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "E must not be a specialization of unexpected"
)

# Step 3 — expected<T,E> ill-formed T/E [expected.object.general]
beman_add_xfail_compile_test(
    expected_t_ref_fail
    SOURCE expected_t_ref_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "no default constructor|use of deleted function|call to deleted|attempting to reference"
)
# Note: expected_e_ref_fail removed — expected<int, int&> is now valid via expected<T,E&>
beman_add_xfail_compile_test(
    expected_t_array_fail
    SOURCE expected_t_array_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "T must not be an array type"
)

# Step 3 — expected<T,E> emplace Mandates: nothrow_constructible_v<T,Args...>
beman_add_xfail_compile_test(
    expected_emplace_throwing_fail
    SOURCE expected_emplace_throwing_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "is_nothrow_constructible|no matching overloaded function found"
)

# Step 4 — expected<void,E> ill-formed E [expected.void.general]
# Note: expected_void_ref_fail.cpp is removed — expected<void, E&> is now valid via
# the expected<void, E&> specialization added in Step 10.
beman_add_xfail_compile_test(
    expected_void_array_fail
    SOURCE expected_void_array_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "E must not be an array type"
)

# Step 5 — monadic Mandates [expected.object.monadic]
beman_add_xfail_compile_test(
    and_then_wrong_error_type_fail
    SOURCE and_then_wrong_error_type_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "F must return expected with the same error_type"
)
beman_add_xfail_compile_test(
    and_then_not_expected_fail
    SOURCE and_then_not_expected_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "F must return a specialization of expected"
)
beman_add_xfail_compile_test(
    or_else_wrong_value_type_fail
    SOURCE or_else_wrong_value_type_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "F must return expected with the same value_type"
)
beman_add_xfail_compile_test(
    transform_error_ref_result_fail
    SOURCE transform_error_ref_result_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "G must be an object type"
)

# Step 6 — void monadic Mandates [expected.void.monadic]
beman_add_xfail_compile_test(
    void_and_then_wrong_error_type_fail
    SOURCE void_and_then_wrong_error_type_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "F must return expected with the same error_type"
)
beman_add_xfail_compile_test(
    void_or_else_wrong_value_type_fail
    SOURCE void_or_else_wrong_value_type_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "F must return expected with the same value_type"
)

# Fix 1 — constraint 23.6: value ctor blocked for T=bool, U=expected specialization
beman_add_xfail_compile_test(
    expected_bool_value_ctor_from_expected_fail
    SOURCE expected_bool_value_ctor_from_expected_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "no matching function|no matching constructor|no overloaded function could convert"
)

# Fix 4 — Mandates: T must not be a specialization of unexpected
beman_add_xfail_compile_test(
    expected_unexpected_value_type_fail
    SOURCE expected_unexpected_value_type_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "T must not be a specialization of unexpected"
)

# Step 7 — expected<T&, E> reference specialization: constructor constraints
beman_add_xfail_compile_test(
    expected_ref_temporary_fail
    SOURCE expected_ref_temporary_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "binding a temporary to T& creates a dangling reference|use of deleted function|call to deleted|no matching function|no matching constructor|no overloaded function could convert|attempting to reference"
)
beman_add_xfail_compile_test(
    expected_ref_no_default_fail
    SOURCE expected_ref_no_default_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "no default constructor|use of deleted function|call to deleted|attempting to reference"
)
beman_add_xfail_compile_test(
    expected_ref_inplace_value_fail
    SOURCE expected_ref_inplace_value_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "no in_place value constructor|use of deleted function|call to deleted|attempting to reference"
)

# Step 8 — expected<T, E&> reference error: dangling prevention
beman_add_xfail_compile_test(
    expected_ref_e_temporary_error_fail
    SOURCE expected_ref_e_temporary_error_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "cannot bind to non-const E&|use of deleted function|call to deleted|attempting to reference"
)
beman_add_xfail_compile_test(
    expected_ref_e_const_lvalue_fail
    SOURCE expected_ref_e_const_lvalue_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "cannot bind to non-const E&|use of deleted function|call to deleted|attempting to reference"
)

# Step 8 — expected<T, E&>: no constructor or assignment from unexpected<G>
# (would bind E& to storage inside the temporary unexpected — dangling)
beman_add_xfail_compile_test(
    expected_ref_e_construct_from_unexpected_fail
    SOURCE expected_ref_e_construct_from_unexpected_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "no constructor from unexpected|use of deleted function|call to deleted|invokes a deleted function|attempting to reference"
)
beman_add_xfail_compile_test(
    expected_ref_e_assign_unexpected_fail
    SOURCE expected_ref_e_assign_unexpected_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "no assignment from unexpected|use of deleted function|call to deleted|selected deleted operator|attempting to reference"
)

# Step 7/8 parity — expected<T&,E> or_else value_type mandate (added in Step 8 review)
beman_add_xfail_compile_test(
    expected_ref_or_else_wrong_value_type_fail
    SOURCE expected_ref_or_else_wrong_value_type_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "F must return expected with the same value_type"
)

# Step 9 — expected<T&, E&> dangling prevention and no-default constraint
beman_add_xfail_compile_test(
    expected_ref_both_temporary_val_fail
    SOURCE expected_ref_both_temporary_val_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "binding a temporary to T& creates a dangling reference|use of deleted function|call to deleted|no matching function|no matching constructor|no overloaded function could convert|attempting to reference"
)
beman_add_xfail_compile_test(
    expected_ref_both_temporary_err_fail
    SOURCE expected_ref_both_temporary_err_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "cannot bind to non-const E&|use of deleted function|call to deleted|attempting to reference"
)
beman_add_xfail_compile_test(
    expected_ref_both_no_default_fail
    SOURCE expected_ref_both_no_default_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "no default constructor|use of deleted function|call to deleted|attempting to reference"
)

# Step 9 — expected<T&, E&>: no constructor or assignment from unexpected<G>
# (would bind E& to storage inside the temporary unexpected — dangling)
beman_add_xfail_compile_test(
    expected_ref_both_construct_from_unexpected_fail
    SOURCE expected_ref_both_construct_from_unexpected_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "no constructor from unexpected|use of deleted function|call to deleted|invokes a deleted function|attempting to reference"
)
beman_add_xfail_compile_test(
    expected_ref_both_assign_unexpected_fail
    SOURCE expected_ref_both_assign_unexpected_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "no assignment from unexpected|use of deleted function|call to deleted|selected deleted operator|attempting to reference"
)

# Step 9 — expected<T&, E&>: or_else value_type mandate
beman_add_xfail_compile_test(
    expected_ref_both_or_else_wrong_value_type_fail
    SOURCE expected_ref_both_or_else_wrong_value_type_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "F must return expected with the same value_type"
)

# Step 7 — expected<T&, E> Mandates: E constraints
beman_add_xfail_compile_test(
    expected_ref_e_ref_fail
    SOURCE expected_ref_e_ref_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "E must not be an rvalue reference"
)
beman_add_xfail_compile_test(
    expected_ref_e_void_fail
    SOURCE expected_ref_e_void_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "E must not be void"
)
beman_add_xfail_compile_test(
    expected_ref_e_array_fail
    SOURCE expected_ref_e_array_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "E must not be an array type"
)
beman_add_xfail_compile_test(
    expected_ref_e_cv_fail
    SOURCE expected_ref_e_cv_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "E must not be cv-qualified"
)

# Step 7 — expected<T&, E> monadic mandates
beman_add_xfail_compile_test(
    expected_ref_and_then_wrong_error_type_fail
    SOURCE expected_ref_and_then_wrong_error_type_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "F must return expected with the same error_type"
)
beman_add_xfail_compile_test(
    expected_ref_transform_error_ref_result_fail
    SOURCE expected_ref_transform_error_ref_result_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "G must be an object type"
)

# Step 8 — expected<T, E&> Mandates: T and E type constraints
beman_add_xfail_compile_test(
    expected_ref_e_t_array_fail
    SOURCE expected_ref_e_t_array_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "T must not be an array type"
)
beman_add_xfail_compile_test(
    expected_ref_e_e_array_fail
    SOURCE expected_ref_e_e_array_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "E must not be an array type"
)
beman_add_xfail_compile_test(
    expected_ref_e_t_inplace_fail
    SOURCE expected_ref_e_t_inplace_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "T must not be in_place_t"
)
beman_add_xfail_compile_test(
    expected_ref_e_t_unexpect_fail
    SOURCE expected_ref_e_t_unexpect_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "T must not be unexpect_t"
)
beman_add_xfail_compile_test(
    expected_ref_e_t_unexpected_fail
    SOURCE expected_ref_e_t_unexpected_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "T must not be a specialization of unexpected"
)
beman_add_xfail_compile_test(
    expected_ref_e_initializer_list_fail
    SOURCE expected_ref_e_initializer_list_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "initializer-list error construction cannot bind a reference|use of deleted function|call to deleted"
)

# Step 8 — expected<T, E&> monadic mandates
beman_add_xfail_compile_test(
    expected_ref_e_and_then_wrong_error_type_fail
    SOURCE expected_ref_e_and_then_wrong_error_type_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "F must return expected with the same error_type"
)
beman_add_xfail_compile_test(
    expected_ref_e_or_else_wrong_value_type_fail
    SOURCE expected_ref_e_or_else_wrong_value_type_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "F must return expected with the same value_type"
)
beman_add_xfail_compile_test(
    expected_ref_e_transform_error_ref_result_fail
    SOURCE expected_ref_e_transform_error_ref_result_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "G must be an object type"
)

# Step 9 — expected<T&, E&> Mandates: T and E type constraints
beman_add_xfail_compile_test(
    expected_ref_both_t_array_fail
    SOURCE expected_ref_both_t_array_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "T must not be an array type"
)
beman_add_xfail_compile_test(
    expected_ref_both_e_array_fail
    SOURCE expected_ref_both_e_array_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "E must not be an array type"
)
beman_add_xfail_compile_test(
    expected_ref_both_t_inplace_fail
    SOURCE expected_ref_both_t_inplace_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "T must not be in_place_t"
)
beman_add_xfail_compile_test(
    expected_ref_both_t_unexpect_fail
    SOURCE expected_ref_both_t_unexpect_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "T must not be unexpect_t"
)
beman_add_xfail_compile_test(
    expected_ref_both_t_unexpected_fail
    SOURCE expected_ref_both_t_unexpected_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "T must not be a specialization of unexpected"
)
beman_add_xfail_compile_test(
    expected_ref_both_inplace_value_fail
    SOURCE expected_ref_both_inplace_value_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "no in_place value constructor|use of deleted function|call to deleted|attempting to reference"
)
beman_add_xfail_compile_test(
    expected_ref_both_initializer_list_fail
    SOURCE expected_ref_both_initializer_list_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "initializer-list error construction cannot bind a reference|use of deleted function|call to deleted"
)

# Step 9 — expected<T&, E&> monadic mandates
beman_add_xfail_compile_test(
    expected_ref_both_and_then_wrong_error_type_fail
    SOURCE expected_ref_both_and_then_wrong_error_type_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "F must return expected with the same error_type"
)
beman_add_xfail_compile_test(
    expected_ref_both_transform_error_ref_result_fail
    SOURCE expected_ref_both_transform_error_ref_result_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT "G must be an object type"
)

# Step 10 — expected<void, E&> dangling prevention and no value_or
beman_add_xfail_compile_test(
    expected_void_ref_e_temporary_fail
    SOURCE expected_void_ref_e_temporary_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "cannot bind to non-const E&|use of deleted function|call to deleted|attempting to reference"
)
beman_add_xfail_compile_test(
    expected_void_ref_e_const_lvalue_fail
    SOURCE expected_void_ref_e_const_lvalue_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "cannot bind to non-const E&|use of deleted function|call to deleted|attempting to reference"
)
beman_add_xfail_compile_test(
    expected_void_ref_e_no_value_or_fail
    SOURCE expected_void_ref_e_no_value_or_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "no value_or for void|use of deleted function|call to deleted|attempting to reference"
)
beman_add_xfail_compile_test(
    expected_void_ref_e_construct_from_unexpected_fail
    SOURCE expected_void_ref_e_construct_from_unexpected_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "no constructor from unexpected|use of deleted function|call to deleted|invokes a deleted function|attempting to reference"
)
beman_add_xfail_compile_test(
    expected_void_ref_e_assign_unexpected_fail
    SOURCE expected_void_ref_e_assign_unexpected_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "no assignment from unexpected|use of deleted function|call to deleted|invokes a deleted function|selected deleted operator|attempting to reference"
)
beman_add_xfail_compile_test(
    expected_void_ref_e_initializer_list_fail
    SOURCE expected_void_ref_e_initializer_list_fail.test.cpp
    LINK_LIBRARIES beman::expected
    EXPECT
        "initializer-list error construction cannot bind a reference|use of deleted function|call to deleted"
)

# =============================================================================
# Hardened precondition tests (compiled with -DBEMAN_EXPECTED_HARDENED)
# =============================================================================

add_executable(beman.expected.tests.hardened)
target_sources(beman.expected.tests.hardened PRIVATE expected_hardened.test.cpp)
target_include_directories(
    beman.expected.tests.hardened
    PRIVATE ${PROJECT_SOURCE_DIR}/tests
)
target_link_libraries(
    beman.expected.tests.hardened
    PRIVATE beman::expected Catch2::Catch2WithMain
)
target_compile_definitions(
    beman.expected.tests.hardened
    PRIVATE BEMAN_EXPECTED_HARDENED
)
catch_discover_tests(beman.expected.tests.hardened DISCOVERY_MODE PRE_TEST)
