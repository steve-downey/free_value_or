# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(Catch2 3 REQUIRED)

add_executable(beman.free_value_or.tests.value_or)
target_sources(beman.free_value_or.tests.value_or PRIVATE value_or.test.cpp)
target_link_libraries(
    beman.free_value_or.tests.value_or
    PRIVATE beman::free_value_or Catch2::Catch2WithMain
)
if(BEMAN_EXEMPLAR_USE_MODULES)
    set_target_properties(
        beman.free_value_or.tests.value_or
        PROPERTIES CXX_MODULE_STD ON
    )
endif()

include(Catch)
catch_discover_tests(beman.free_value_or.tests.value_or)
