# Thin helpers for AlloLib module conventions.
# Module CMakeLists keep add_library, target_sources, and deps visible;
# these functions only apply shared policy (standard, export name, install).
#
# Vendored third-party deps are temporarily bundled into AlloLib's install for
# usability. Longer term they may move to vcpkg/Conan + find_dependency(); the
# al:: module export surface is designed to stay the same either way.

include(GNUInstallDirs)

# Apply shared properties to an existing library target.
# Usage: al_configure_module(<target> EXPORT_NAME <name>)
function(al_configure_module target)
  cmake_parse_arguments(ARG "" "EXPORT_NAME" "" ${ARGN})

  if(NOT TARGET ${target})
    message(FATAL_ERROR "al_configure_module: target '${target}' does not exist")
  endif()
  if(NOT ARG_EXPORT_NAME)
    message(FATAL_ERROR "al_configure_module(${target}): EXPORT_NAME is required")
  endif()

  set_target_properties(${target} PROPERTIES
    EXPORT_NAME ${ARG_EXPORT_NAME}
    DEBUG_POSTFIX d
    VERIFY_INTERFACE_HEADER_SETS ON
  )

  target_compile_features(${target} PUBLIC cxx_std_20)
endfunction()

# Install a module library (and FILE_SET HEADERS when present) into AlloLibTargets.
# No-op unless ALLOLIB_INSTALL=ON (package install is opt-in future work).
# Usage: al_install_module(<target>)
function(al_install_module target)
  if(NOT TARGET ${target})
    message(FATAL_ERROR "al_install_module: target '${target}' does not exist")
  endif()

  if(NOT ALLOLIB_INSTALL)
    return()
  endif()

  get_target_property(_header_sets ${target} HEADER_SETS)

  if(_header_sets)
    install(
      TARGETS ${target}
      EXPORT AlloLibTargets
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
  else()
    install(
      TARGETS ${target}
      EXPORT AlloLibTargets
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
  endif()
endfunction()
