# include/beman/expected/CMakeLists.txt                             -*-cmake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

target_sources(
    beman.expected
    PUBLIC FILE_SET HEADERS FILES expected.hpp unexpected.hpp
)
