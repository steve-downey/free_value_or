# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

if(BEMAN_FREE_VALUE_OR_USE_MODULES)
    target_sources(
        beman.free_value_or
        PUBLIC
            FILE_SET CXX_MODULES FILES free_value_or.cppm
            FILE_SET HEADERS
                FILES
                    config.hpp
                    free_value_or.hpp
                    value_or.hpp
                    "${PROJECT_BINARY_DIR}/include/beman/free_value_or/config_generated.hpp"
    )
else()
    target_sources(
        beman.free_value_or
        PUBLIC
            FILE_SET HEADERS
                FILES
                    config.hpp
                    free_value_or.hpp
                    value_or.hpp
                    "${PROJECT_BINARY_DIR}/include/beman/free_value_or/config_generated.hpp"
    )
endif()
