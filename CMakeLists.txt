# include/beman/optional/CMakeLists.txt -*-cmake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

target_sources(
    beman_optional
    PUBLIC
        FILE_SET beman_optional_headers
        TYPE HEADERS
        FILES
            optional.hpp
            detail/iterator.hpp
            detail/stl_interfaces/config.hpp
            detail/stl_interfaces/fwd.hpp
            detail/stl_interfaces/iterator_interface.hpp
)

install(
    TARGETS beman_optional
    FILE_SET beman_optional_headers
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT beman_optional_development
)

install(
    TARGETS beman_optional
    EXPORT beman_optional_export
    DESTINATION
    ${CMAKE_INSTALL_LIBDIR}
    FILE_SET beman_optional_headers
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT beman_optional_development
)

install(
    EXPORT beman_optional_export
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/beman/optional/
    NAMESPACE Beman::Optional::
    FILE beman_optional.cmake
    EXPORT_LINK_INTERFACE_LIBRARIES
    COMPONENT beman_optional_development
)
