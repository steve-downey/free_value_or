# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(GTest REQUIRED)

add_executable(beman.expected.tests.identity)
target_sources(beman.expected.tests.identity PRIVATE identity.test.cpp)
target_link_libraries(
    beman.expected.tests.identity
    PRIVATE beman::expected GTest::gtest GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(beman.expected.tests.identity)
