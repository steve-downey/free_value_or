# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(GTest REQUIRED)

add_executable(beman.expected.tests.todo)
target_sources(beman.expected.tests.todo PRIVATE todo.test.cpp)
target_link_libraries(
    beman.expected.tests.todo
    PRIVATE beman::expected GTest::gtest_main
)
if(BEMAN_EXEMPLAR_USE_MODULES)
    set_target_properties(
        beman.expected.tests.todo
        PROPERTIES CXX_MODULE_STD ON
    )
endif()

include(GoogleTest)
gtest_discover_tests(beman.expected.tests.todo DISCOVERY_TIMEOUT 60)
