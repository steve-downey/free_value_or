# tests/beman/expected/CMakeLists.txt                               -*-cmake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

add_executable(beman.expected.tests.expected)
target_sources(
    beman.expected.tests.expected
    PRIVATE bad_expected_access.test.cpp unexpected.test.cpp expected.test.cpp
)
target_link_libraries(
    beman.expected.tests.expected
    PRIVATE beman::expected GTest::gtest GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(beman.expected.tests.expected)
