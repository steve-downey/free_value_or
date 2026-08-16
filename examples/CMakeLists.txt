# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

set(ALL_EXAMPLES free_value_or)
message("Examples to be built: ${ALL_EXAMPLES}")

foreach(example ${ALL_EXAMPLES})
    add_executable(beman.free_value_or.examples.${example})
    target_sources(
        beman.free_value_or.examples.${example}
        PRIVATE ${example}.cpp
    )
    target_link_libraries(
        beman.free_value_or.examples.${example}
        PRIVATE beman::free_value_or
    )
    if(BEMAN_FREE_VALUE_OR_USE_MODULES)
        set_target_properties(
            beman.free_value_or.examples.${example}
            PROPERTIES CXX_MODULE_STD ON
        )
    endif()
endforeach()
