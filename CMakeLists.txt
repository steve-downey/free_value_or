# include/beman/optional/CMakeLists.txt -*-cmake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

target_sources(
    beman.optional
    PUBLIC
        FILE_SET HEADERS
            TYPE HEADERS
            FILES
                optional.hpp
                detail/iterator.hpp
                detail/stl_interfaces/config.hpp
                detail/stl_interfaces/fwd.hpp
                detail/stl_interfaces/iterator_interface.hpp
)

# install(
#     TARGETS beman.optional
#     FILE_SET beman.optional_headers
#         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#         COMPONENT beman.optional_development
# )

# install(
#     TARGETS beman.optional
#     EXPORT beman.optional_export
#     DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     FILE_SET beman.optional_headers
#         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#         COMPONENT beman.optional_development
# )

# install(
#     EXPORT beman.optional_export
#     DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/beman/optional/
#     NAMESPACE Beman::Optional::
#     FILE beman.optional.cmake
#     EXPORT_LINK_INTERFACE_LIBRARIES
#     COMPONENT beman.optional_development
# )
